#include "StdAfx.h"

#ifdef AZ_PLATFORM_WINDOWS
#include <Shlobj.h>

//AZ_Warning("HOUDINI-COOK", false, (AZStd::string(__FILE__)+ AZStd::string(": ") + QString::number(__LINE__)).c_str());\

#define ENSURE_SUCCESS( result ) \
if ( (result) != HAPI_RESULT_SUCCESS ) \
{ \
    CheckForErrors(true);\
}
#define ENSURE_COOK_SUCCESS( result ) \
if ( (result) != HAPI_RESULT_SUCCESS ) \
{ \
    CheckForErrors(true);\
}


#endif

#define HOUDINI_MODE_EMBEDDED 0 
#define HOUDINI_MODE_ASYNC 1 

#define HOUDINI_NAMED_PIPE "HOUDINI_LUMBERYARD"
// #pragma optimize("", off) // do we really need this?

#include <HoudiniCommon.h>
#include "Util/EditorUtils.h"

#include <AzFramework/IO/LocalFileIO.h>

namespace HoudiniEngine
{
    Houdini::Houdini() 
        : m_isActive(false)
        , m_initialized(false)
    {
        GetIEditor()->RegisterNotifyListener(this);
        // FL[FD-8467] [Warning] Unknown command: hou_otl_path
        cVar_THRIFT = gEnv->pConsole->GetCVar("hou_multi_threaded");
        m_globalStateVar = gEnv->pConsole->GetCVar("hou_state");
        ICVar * cVar_OTL = gEnv->pConsole->GetCVar("hou_otl_path");
        ICVar * cVar_PIPE = gEnv->pConsole->GetCVar("hou_named_pipe");
        // FL[FD-10789] Support Mesh as Input to Houdini Digital Asset
        //ICVar * cVar_UpdatePeriod = gEnv->pConsole->GetCVar("hou_update_period");
        
        m_inputNodeManager = InputNodeManagerPtr(new InputNodeManager(this));
        m_namedPipe = cVar_PIPE->GetString();
        
        std::string otl = std::string(cVar_OTL->GetString());
        auto start = 0U;
        auto end = otl.find(';');
        while (end != std::string::npos)
        {
            m_searchPaths.push_back(AZStd::string(otl.substr(start, end - start).c_str()));
            start = (unsigned int)(end + 1);
            end = otl.find(';', start);
        }

        m_searchPaths.push_back(AZStd::string(otl.substr(start, end).c_str()));
        
        if (cVar_THRIFT->GetIVal() != 0)
        {
            m_workerThreadDesc.m_name = "Houdini Processor";
            //TODO:
            /*m_workerThread.push_back(
                new AZStd::thread([this]() { this->ThreadProcessor(); }, &m_workerThreadDesc)
            );*/
            m_workerThread[0]->detach();
        }

        CreateNewSession();
    }

    void Houdini::CreateNewSession()
    {
        AZ_PROFILE_FUNCTION(Editor);

        HAPI_Session newSession;
        newSession.type = HAPI_SESSION_MAX;
        newSession.id = 0;
        
        if (cVar_THRIFT->GetIVal() != 0)
        {
            m_thriftServerOptions = HAPI_ThriftServerOptions();
            m_thriftServerOptions.autoClose = true;
            m_thriftServerOptions.timeoutMs = 10.0f * 1000.0f;
            m_thriftServerProcId = 0;

            HAPI_StartThriftNamedPipeServer(&m_thriftServerOptions, HOUDINI_NAMED_PIPE, &m_thriftServerProcId, "");
            HAPI_CreateThriftNamedPipeSession(&newSession, HOUDINI_NAMED_PIPE);
        }
        else if(m_namedPipe.size() != 0)
        {
            HAPI_CreateThriftNamedPipeSession(&newSession, m_namedPipe.c_str());
        }
        
        //No custom sessions, create in session:
        if ( newSession.type == HAPI_SESSION_MAX)
        {
            HAPI_CreateInProcessSession(&newSession);
        }

        m_session = newSession;

        m_cookOptions = HAPI_CookOptions_Create();
        m_cookOptions.curveRefineLOD = 8.0f;
        m_cookOptions.clearErrorsAndWarnings = false;
        m_cookOptions.maxVerticesPerPrimitive = 3;
        m_cookOptions.splitGeosByGroup = false;
        m_cookOptions.refineCurveToLinear = true;
        m_cookOptions.handleBoxPartTypes = false;
        m_cookOptions.handleSpherePartTypes = false;
        m_cookOptions.splitPointsByVertexAttributes = false;
        m_cookOptions.packedPrimInstancingMode = HAPI_PACKEDPRIM_INSTANCING_MODE_DISABLED;
        m_cookOptions.clearErrorsAndWarnings = true;

        if (m_initialized == false)
        {
            HAPI_Initialize(
                &m_session, // session
                &m_cookOptions,
                true, // use_cooking_thread
                -1, // cooking_thread_stack_size
                NULL, // environment files
                NULL, // otl_search_path
                NULL, // dso_search_path
                NULL, // image_dso_search_path
                NULL); // audio_dso_search_path
            m_initialized = true;
        }
        m_rootNode = HoudiniNodePtr(new HoudiniNode(this, nullptr, -1, "null", "root"));
        
        HAPI_NodeId initNode;
        HAPI_Result createResult = HAPI_CreateNode(&m_session, -1, "sop/null", "INITIALIZE_NODE", true, &initNode);
        HAPI_Result cookResult = HAPI_CookNode(&m_session, initNode, &m_cookOptions);
        
        AZ_TracePrintf("HOUDINI", "Initializing Houdini");
        if (CheckForErrors())
        {
            // FL[FD-10714] Houdini integration to 1.21
            AZStd::string codeString;
            EBUS_EVENT_RESULT(codeString, HoudiniEngineRequestBus, GetHoudiniResultByCode, createResult);
            AZ_Warning("HOUDINI", false, (AZStd::string("Failed to initialize Houdini Engine with error: ") + codeString).c_str());
            return;
        }

        if (createResult == HAPI_RESULT_NO_LICENSE_FOUND
            || createResult == HAPI_RESULT_DISALLOWED_NC_LICENSE_FOUND
            || createResult == HAPI_RESULT_DISALLOWED_NC_ASSET_WITH_C_LICENSE
            || createResult == HAPI_RESULT_DISALLOWED_NC_ASSET_WITH_LC_LICENSE
            || createResult == HAPI_RESULT_DISALLOWED_LC_ASSET_WITH_C_LICENSE
            || createResult == HAPI_RESULT_DISALLOWED_HENGINEINDIE_W_3PARTY_PLUGIN
            || cookResult == HAPI_RESULT_NO_LICENSE_FOUND
            || cookResult == HAPI_RESULT_DISALLOWED_NC_LICENSE_FOUND
            || cookResult == HAPI_RESULT_DISALLOWED_NC_ASSET_WITH_C_LICENSE
            || cookResult == HAPI_RESULT_DISALLOWED_NC_ASSET_WITH_LC_LICENSE
            || cookResult == HAPI_RESULT_DISALLOWED_LC_ASSET_WITH_C_LICENSE
            || cookResult == HAPI_RESULT_DISALLOWED_HENGINEINDIE_W_3PARTY_PLUGIN)
        {
            return;
        }

        m_isActive = true;
        LoadAllAssets();
    }

    void Houdini::DeleteAllObjects()
    {
        AZ_PROFILE_FUNCTION(Editor);

        int childCount = 0;
        HAPI_NodeId rootId;
        
        *this << "Delete ALL NODES: " << HAPI_NODETYPE_OBJ << "";

        HAPI_GetManagerNodeId(&m_session, HAPI_NODETYPE_OBJ, &rootId);
        HAPI_ComposeChildNodeList(&m_session, rootId, HAPI_NODETYPE_OBJ, HAPI_NODEFLAGS_ANY, false, &childCount);

        AZStd::vector<HAPI_NodeId> finalChildList(childCount);
        if (childCount > 0)
        {
            *this << "HAPI_GetComposedChildNodeList: " << rootId << " Reading: " << childCount << " into buffer sized: " << finalChildList.size() << "";
            HAPI_GetComposedChildNodeList(&m_session, rootId, &finalChildList.front(), childCount);

            for (auto child : finalChildList)
            {
                HAPI_NodeInfo childInfo;
                HAPI_GetNodeInfo(&m_session, child, &childInfo);                
                
                *this << "Delete ALL NODES: " << HAPI_NODETYPE_OBJ << "";

                if (childInfo.isValid)
                {
                    HAPI_DeleteNode(&m_session, child);
                }
            }
        }
    }

    Houdini::~Houdini()
    {
        //Unload all assets and cleanup, destroy session.
        Shutdown();
        m_rootNode.reset();
    }

    void Houdini::ExecuteCommand(AZ::EntityId newId, AZStd::function<bool()> functionToCall)
    {
        auto thisThreadId = AZStd::this_thread::get_id();

        if (m_workerThread.size() > 0 && m_workerThreadIds.size() > 0 && thisThreadId != m_workerThreadIds[0])
        {
            AZStd::unique_lock<AZStd::mutex> theLock(m_lock);
            m_work.push(AZStd::make_pair(newId, functionToCall));
        }
        else
        {
            //Inline call since we are not in multi-threaded mode, or we are already on the processor thread!
            functionToCall();
        }
    }

    void Houdini::RaiseCommandPriority(AZ::EntityId /*newId*/)
    {
        AZStd::unique_lock<AZStd::mutex> theLock(m_lock);

        /*auto& buffer = m_work.get_container();

        AZStd::vector<AZStd::pair<AZ::EntityId, AZStd::function<bool()>>> items;
        for( auto it = buffer.begin(); it < buffer.end(); it++)
        {
            if (it->first == newId)
            {
                items.push_back(*it);
                buffer.erase(it);
            }
        }

        if (items.size() > 0)
        {
            for (auto& item : items)
            {
                buffer.push_front(item);
            }
        }*/
    }

    void Houdini::ThreadProcessor()
    {
        m_workerThreadIds.push_back(AZStd::this_thread::get_id());
        while (true)
        {
            //Lock the work thread, this lock signals that we are processing an item:
            {
                AZStd::unique_lock<AZStd::mutex> theWorkLock(m_threadWorkingLock);

                bool hasFunction = false;
                AZStd::function<bool()> function;
                {
                    //Lock the fetch for new work, so no one adds something while we're getting work.
                    AZStd::unique_lock<AZStd::mutex> theFetchNewWorkLock(m_lock);
                    if (m_work.empty() == false)
                    {
                        hasFunction = true;
                        function = m_work.front().second;
                        m_work.pop();
                    }
                }

                if (hasFunction)
                {
                    function();
                }
            }

            //Work finished, should we keep going? Or Sleep for a bit?
            bool shouldSleep = false;
            {
                AZStd::unique_lock<AZStd::mutex> theFetchNewWorkLock(m_lock);
                if (m_work.empty())
                {
                    shouldSleep = true;
                }
            }

            if (shouldSleep) 
            {
                AZStd::this_thread::sleep_for(AZStd::chrono::milliseconds(32));
            }
            else
            {
                AZStd::this_thread::yield();
            }
        }
    }

    void Houdini::CancelProcessorThread()
    {
        //Make sure we have the queue lock and remove all future work:
        {
            AZStd::unique_lock<AZStd::mutex> theFetchNewWorkLock(m_lock);
            while (m_work.empty() == false)
            {
                //Clear out the future work.
                m_work.pop();
            }
        }

        //Wait until the thread finishes whatever its working on now:
        AZStd::unique_lock<AZStd::mutex> theWorkLock(m_threadWorkingLock);
        
        //Thread is done at this point.
    }

    void Houdini::CancelProcessorJob(AZ::EntityId /*entityIdToRemove*/)
    {
        //Grab the work list:
        AZStd::unique_lock<AZStd::mutex> theFetchNewWorkLock(m_lock);

        //Wait for whatever is currently processing to finish:
        AZStd::unique_lock<AZStd::mutex> theWorkLock(m_threadWorkingLock);

        //We now have full control of the list:
        /*auto& buffer = m_work.get_container();

        AZStd::remove_if(buffer.begin(), buffer.end(), [this, entityIdToRemove](AZStd::pair<AZ::EntityId, AZStd::function<bool()>> pair)
        {
            if (pair.first == entityIdToRemove)
            {
                return true;
            }
            
            return false;
        });*/

    }

    void Houdini::JoinProcessorThread()
    {
        //Check if the work to do is empty:
        while (true)
        {
            AZStd::unique_lock<AZStd::mutex> theFetchNewWorkLock(m_lock);
            if (m_work.empty() == false)
            {
                AZStd::this_thread::yield();
            }
            else 
            {
                //ALL work is done now
                break;
            }
        }                
        
        //Wait until the thread finishes whatever its working on now:
        AZStd::unique_lock<AZStd::mutex> theWorkLock(m_threadWorkingLock);
    }

    int Houdini::GetHoudiniMode()
    {
        if (m_workerThread.size() > 0 )
        {
            return HOUDINI_MODE_ASYNC;
        }

        return HOUDINI_MODE_EMBEDDED;
    }

    void Houdini::GetProgress(AZStd::string& statusText, int& statusPercent, int& assetsInQueue)
    {
        AZStd::unique_lock<AZStd::mutex> theWorkLock(m_statusLock);
        statusText = m_currentStatus;
        statusPercent = m_currentPercent;

        AZStd::unique_lock<AZStd::mutex> theFetchNewWorkLock(m_lock);        
        assetsInQueue = (int)m_work.size();
    }

    void Houdini::SetProgress(const AZStd::string& statusText, int statusPercent)
    {
        AZStd::unique_lock<AZStd::mutex> theWorkLock(m_statusLock);
        m_currentStatus = statusText;
        m_currentPercent = statusPercent;
    }

    void Houdini::OnTick(float /*deltaTime*/, AZ::ScriptTimePoint /*time*/)
    {

        //Apply any requested refreshes
        //Uses a time slice to keep framerate high in entity heavy levels.
        {
            AZStd::unique_lock<AZStd::mutex> lookupLock(m_lookupLock);

            AZ::EntityId start = m_timeSlice;
            auto firstEntity = m_lookups.lower_bound(start);

            if (firstEntity == m_lookups.end())
            {
                firstEntity = m_lookups.begin();
                m_timeSlice = AZ::EntityId();
            }

            int count = 0;
            for (auto& entityPair = firstEntity; entityPair != m_lookups.end(); )
            {
                if (entityPair->first.IsValid() == false)
                {
                    entityPair++;
                    continue;
                }

                LookupId(entityPair->first);

                count++;
                if (count == m_maxSliceCount)
                {
                    m_timeSlice = entityPair->first;
                    break;
                }                

                entityPair++;
                if (entityPair == m_lookups.end())
                {
                    //Clear the last scan.
                    m_timeSlice = AZ::EntityId();
                }
            }
        }
    }

    //Collects data that might be needed by update systems or creation systems. This is required to ensure thread safety.
    void Houdini::RemoveLookupId(const AZ::EntityId& id)
    {
        AZStd::unique_lock<AZStd::mutex> lookupLock(m_lookupLock);
        auto existing = m_lookups.find(id);
        if (existing != m_lookups.end())
        {
            m_lookups.erase(existing);
        }
    }


    //Collects data that might be needed by update systems or creation systems. This is required to ensure thread safety.
    void Houdini::LookupId(const AZ::EntityId& id)
    {
        AZStd::unique_lock<AZStd::mutex> lookupLock(m_lookupLock);
        auto existing = m_lookups.find(id);
        if (existing == m_lookups.end())
        {
            m_lookups[id] = HoudiniEntityContext();
        }

        HoudiniEntityContext& data = m_lookups[id];
        AZ::ComponentApplicationBus::BroadcastResult(data.m_entity, &AZ::ComponentApplicationRequests::FindEntity, id);
        AZ::ComponentApplicationBus::BroadcastResult(data.m_entityName, &AZ::ComponentApplicationRequests::GetEntityName, id);
        LmbrCentral::SplineComponentRequestBus::EventResult(data.m_spline, id, &LmbrCentral::SplineComponentRequests::GetSpline);
        AzToolsFramework::ToolsApplicationRequests::Bus::BroadcastResult(data.m_isSelected, &AzToolsFramework::ToolsApplicationRequests::IsSelected, id);
        AZ::TransformBus::EventResult(data.m_transform, id, &AZ::TransformBus::Events::GetWorldTM);

        AZ::EBusAggregateResults<AZStd::string> attribList;
        HoudiniCurveAttributeRequestBus::EventResult(attribList, id, &HoudiniCurveAttributeRequests::GetName);        

        data.m_attributeNames.clear();
        for (auto attrib : attribList.values)
        {
            LmbrCentral::SplineAttribute<float>* attribData;
            HoudiniCurveAttributeRequestBus::Event(id, &HoudiniCurveAttributeRequests::GetFloatAttribute, attrib, attribData);

            if (attribData)
            {
                data.m_attributeNames.push_back(attrib);
                data.m_attributes[attrib] = attribData;
            }
        }
    }

    bool Houdini::LookupIsSelected(const AZ::EntityId& id)
    {
        AZStd::unique_lock<AZStd::mutex> lookupLock(m_lookupLock);
        auto existing = m_lookups.find(id);
        if (existing == m_lookups.end())
        {
            return false;
        }

        return m_lookups[id].m_isSelected;
    }

    AZ::Entity* Houdini::LookupFindEntity(const AZ::EntityId& id)
    {
        AZStd::unique_lock<AZStd::mutex> lookupLock(m_lookupLock);
        auto existing = m_lookups.find(id);
        if (existing == m_lookups.end())
        {
            return nullptr;
        }

        return m_lookups[id].m_entity;
    }

    AZStd::string Houdini::LookupEntityName(const AZ::EntityId& id)
    {
        AZStd::unique_lock<AZStd::mutex> lookupLock(m_lookupLock);
        auto existing = m_lookups.find(id);
        if (existing == m_lookups.end())
        {
            return "";
        }

        return m_lookups[id].m_entityName;
    }

    AZ::SplinePtr Houdini::LookupSpline(const AZ::EntityId& id)
    {
        AZStd::unique_lock<AZStd::mutex> lookupLock(m_lookupLock);
        auto existing = m_lookups.find(id);
        if (existing == m_lookups.end())
        {
            return nullptr;
        }
        return m_lookups[id].m_spline;
    }

    AZStd::vector<AZStd::string> Houdini::LookupAttributeNames(const AZ::EntityId& id)
    {
        AZStd::unique_lock<AZStd::mutex> lookupLock(m_lookupLock);
        auto existing = m_lookups.find(id);
        if (existing == m_lookups.end())
        {
            return AZStd::vector<AZStd::string>();
        }
        return m_lookups[id].m_attributeNames;
    }

    LmbrCentral::SplineAttribute<float>* Houdini::LookupAttributeData(const AZ::EntityId& id, const AZStd::string& attribName)
    {
        AZStd::unique_lock<AZStd::mutex> lookupLock(m_lookupLock);
        auto existing = m_lookups.find(id);
        if (existing == m_lookups.end())
        {
            return nullptr;
        }

        auto existingData = m_lookups[id].m_attributes.find(attribName);
        if (existingData == m_lookups[id].m_attributes.end())
        {
            return nullptr;
        }

        return m_lookups[id].m_attributes[attribName];
    }

    AZ::Transform Houdini::LookupTransform(const AZ::EntityId& id)
    {
        AZStd::unique_lock<AZStd::mutex> lookupLock(m_lookupLock);
        auto existing = m_lookups.find(id);
        if (existing == m_lookups.end())
        {
            return AZ::Transform::CreateIdentity();
        }

        return m_lookups[id].m_transform;
    }

    HoudiniEngine::HoudiniEntityContext Houdini::LookupEntityData(const AZ::EntityId& id)
    {
        AZStd::unique_lock<AZStd::mutex> lookupLock(m_lookupLock);
        auto existing = m_lookups.find(id);
        if (existing == m_lookups.end())
        {
            return HoudiniEntityContext();
        }
        
        return m_lookups[id];
    }

    bool Houdini::CheckForErrors(bool printErrors, bool includeCookingErrors)
    {
        AZ_PROFILE_FUNCTION(Editor);

        bool hasError = false;

        auto err = GetLastHoudiniError();

        if (err.length() > 0)
        {
            AZ_Warning("HOUDINI", printErrors == false, err.c_str());
            hasError = true;
        }

        err = GetLastHoudiniCookError();
        if (err.length() > 0)
        {
            //This error is spammy and more of a warning really, so we will not print it:
            QString error = err.c_str();
            if (error.contains("No geometry generated!", Qt::CaseInsensitive) == false)
            {
                AZ_Warning("HOUDINI-COOKING", printErrors == false, err.c_str());                
            }

            if (includeCookingErrors)
            {
                hasError = true;
            }
        }

        return hasError;
    }

    AZStd::string Houdini::FindHda(const AZStd::string& hdaFile)
    {
        AZ_PROFILE_FUNCTION(Editor);

        if (IsActive())
        {
            for (auto searchPath : m_searchPaths)
            {
                AZStd::string file = searchPath + "/" + hdaFile;

                char filePath[AZ_MAX_PATH_LEN] = { 0 };
                AZ::IO::FileIOBase::GetInstance()->ResolvePath(file.c_str(), filePath, AZ_MAX_PATH_LEN);
                file = AZStd::string(filePath);

                if (AZ::IO::FileIOBase::GetInstance()->Exists(file.c_str()))
                {
                    return file;
                }
            }
        }
        return hdaFile;
    }

    void Houdini::LoadAllAssets()
    {
        AZ_PROFILE_FUNCTION(Editor);

        if (IsActive() == false)
        {
            return;
        }

        for (auto searchPath : m_searchPaths)
        {
            //AZStd::string folder = searchPath + "/";
            AZStd::string folder = searchPath;

            char filePath[AZ_MAX_PATH_LEN] = { 0 };
            AZ::IO::FileIOBase::GetInstance()->ResolvePath(folder.c_str(), filePath, AZ_MAX_PATH_LEN);
            folder = AZStd::string(filePath);

            if (AZ::IO::FileIOBase::GetInstance()->Exists(folder.c_str()))
            {
                AZ::IO::LocalFileIO fileIo;
                AZStd::string errorString;

                // Handles each file and directory found
                // Safe to capture all by reference because the find will run sync
                AZ::IO::LocalFileIO::FindFilesCallbackType fileFinderCb;
                fileFinderCb = [&](const char* fullPath) -> bool
                {
                    if (fileIo.IsDirectory(fullPath) == false)
                    {
                        LoadHoudiniDigitalAsset(fullPath);
                    }

                    return true; // keep searching
                };

                // Scans subdirectories
                fileIo.FindFiles(folder.c_str(), "*.hda", fileFinderCb);
            }
        }
    }

    // FL[FD-10790] Houdini Digital Asset List Hot Reload
    void Houdini::ReloadAllAssets()
    {
        m_assetCache.clear();

        LoadAllAssets();
    }

    HAPI_NodeId Houdini::FindNode(HAPI_NodeType networkType, const AZStd::string& path)
    {
        AZ_PROFILE_FUNCTION(Editor);

        int childCount = 0;
        HAPI_NodeId rootId;

        *this << "FindNode: " << networkType << " " << path << "";

        HAPI_GetManagerNodeId(&m_session, networkType, &rootId);
        HAPI_ComposeChildNodeList(&m_session, rootId, networkType, HAPI_NODEFLAGS_ANY, true, &childCount);

        AZStd::vector<HAPI_NodeId> finalChildList(childCount);
        if (childCount > 0) 
        {
            *this << "HAPI_GetComposedChildNodeList: " << rootId << " Reading: "<< childCount << " into buffer sized: " << finalChildList.size() << "";
            HAPI_GetComposedChildNodeList(&m_session, rootId, &finalChildList.front(), childCount);

            //TODO: Need Houdini 17 for string batch:
            /*
            AZStd::vector<HAPI_StringHandle> nameHandles;
            for (auto child : finalChildList)
            {
                HAPI_NodeInfo childInfo;
                HAPI_GetNodeInfo(&m_session, child, &childInfo);
                nameHandles.push_back(childInfo.internalNodePathSH);
            }

            int size = 0;            
            HAPI_GetStringBatchSize(&m_session, nameHandles.data(), nameHandles.size(), &size);

            if ( size > 0 )
            {
                char* allNames = new char[size];
                HAPI_GetStringBatch(&m_session, allNames, size);

                delete[] allNames;
            }
            */

            for (auto child : finalChildList)
            {
                HAPI_NodeInfo childInfo;
                HAPI_GetNodeInfo(&m_session, child, &childInfo);

                AZStd::string childPath = GetString(childInfo.internalNodePathSH);
                if (childPath == path)
                {
                    return childInfo.id;
                }
            }
        }

        return HOUDINI_INVALID_ID;
    }

    void Houdini::RemoveNode(const AZStd::string& oldNodeName, IHoudiniNode* node)
    {
        //Check the cache to see if we already created this node.
        auto cacheOutput = m_nodeNameCache.find(oldNodeName);
        auto cacheOutputPtr = m_nodeCache.find(node);

        if (cacheOutput != m_nodeNameCache.end())
        {
            m_nodeNameCache.erase(cacheOutput);            
        }
        
        if (cacheOutputPtr != m_nodeCache.end())
        {
            m_nodeCache.erase(cacheOutputPtr);
        }
    }


    void Houdini::RenameNode(const AZStd::string& oldNodeName, IHoudiniNode* node)
    {
        //Check the cache to see if we already created this node.
        auto cacheOutput = m_nodeNameCache.find(oldNodeName);
        auto cacheOutputPtr = m_nodeCache.find(node);

        if (cacheOutput != m_nodeNameCache.end() && cacheOutputPtr != m_nodeCache.end())
        {
            m_nodeNameCache.erase(cacheOutput);
            m_nodeNameCache[node->GetNodeName()] = cacheOutputPtr->second;
        }
        else
        {
            AZ_Warning("HOUDINI", false, "Could not rename node %s to %s", oldNodeName.c_str(), node->GetNodeName().c_str());
        }
    }


    HoudiniNodePtr Houdini::CreateNode(const AZStd::string& operatorName, const AZStd::string& nodeName, IHoudiniNode* parent)
    {
        AZ_PROFILE_FUNCTION(Editor);

        if (IsActive() == false)
        {
            return nullptr;
        }

        //Check the cache to see if we already created this node.
        auto cacheOutput = m_nodeNameCache.find(nodeName);
        if (cacheOutput != m_nodeNameCache.end())
        {
            return cacheOutput->second;
        }

        //Find the best casing incase someone wrote it differently:
        AZStd::string safeOperatorName = operatorName;

        for (auto& asset : m_assetCache)
        {
            for (auto& opName : asset.second->getAssets())
            {
                if (AzFramework::StringFunc::Equal(opName.c_str(), operatorName.c_str(), false))
                {
                    safeOperatorName = opName;
                }
            }
        }

        HAPI_NodeId nodeId = -1;
        HAPI_NodeId parentId = -1;

        if (parent != nullptr || parent == m_rootNode.get())
        {
            parentId = parent->GetId();
        }
            
        //Lock the status and update it:
        {
            AZStd::unique_lock<AZStd::mutex> theLock(m_statusLock); 
            m_currentStatus = "Creating HDA: " + nodeName + " as type: " + safeOperatorName;
            m_currentPercent = 0;
        }

        HAPI_Result result = HAPI_CreateNode(&m_session, parentId, safeOperatorName.c_str(), nullptr, false/*nodeName.c_str(), true*/, &nodeId);

        //If the system was activated late, we'll need to reload assets:
        QString error = GetLastHoudiniError().c_str();
        if (result == HAPI_RESULT_INVALID_ARGUMENT && error.contains("HAPI_LoadAssetLibraryFromFile"))
        {
            LoadAllAssets();
            result = HAPI_CreateNode(&m_session, parentId, safeOperatorName.c_str(), nodeName.c_str(), true, &nodeId);
            error = GetLastHoudiniError().c_str();
            if (result == HAPI_RESULT_INVALID_ARGUMENT && error.contains("HAPI_LoadAssetLibraryFromFile"))
            {
                return nullptr;
            }
        }

        //Check if there was an error creating the node:
        //An error about geometry missing is normal. This happens when a node is created but not configured yet
        if (result != HAPI_RESULT_SUCCESS && result != HAPI_RESULT_CANT_LOAD_GEO)
        {
            *this << " HAPI_CreateNode: FAILED TO CREATE NODE! parent:" << parentId << " operator: " << safeOperatorName << " name:" << nodeName << " nodeId: " << nodeId << " RESULT:" << result;
            return nullptr;
        }

        *this << " HAPI_CreateNode: parent:" << parentId << " operator: " << safeOperatorName << " name:" << nodeName << " nodeId: " << nodeId << "";
        CheckForErrors(result != HAPI_RESULT_CANT_LOAD_GEO);

        HoudiniNodePtr node = HoudiniNodePtr(new HoudiniNode(this, nullptr, nodeId, safeOperatorName, nodeName));
        if (node->GetNodeInfo().isValid == false)
        {
            *this << " HAPI_CreateNode: FAILED TO CREATE NODE! parent:" << parentId << " operator: " << safeOperatorName << " name:" << nodeName << " nodeId: " << nodeId << "Operator not valid";
            return nullptr;
        }

        m_nodeCache[node.get()] = node;
        m_nodeNameCache[nodeName] = node;

        if (parent != nullptr && m_nodeCache[parent] != nullptr)
        {
            node->SetParent(m_nodeCache[parent]);
        }

        //Lock the status and update it:
        {
            AZStd::unique_lock<AZStd::mutex> theLock(m_statusLock);
            m_currentStatus = "Creating HDA: " + nodeName + " as type: " + safeOperatorName + " Finished";
            m_currentPercent = 100;
        }

        return node;
    }

    void Houdini::CookNode(HAPI_NodeId node, const AZStd::string& entityName)
    {
        AZ_PROFILE_FUNCTION(Editor);

        if (IsActive() == false)
        {
            return;
        }
        
        HAPI_NodeInfo info = HAPI_NodeInfo_Create();
        HAPI_GetNodeInfo(&m_session, node, &info);
        
        if (info.id == HOUDINI_INVALID_ID)
        {
            AZ_Error("HOUDINI", false, (entityName + " has invalid node id! Cannot Cook this node!").c_str());
            return;
        }

        //Save the name:
        {
            auto nodeName = GetString(info.nameSH);
            AZ_PROFILE_SCOPE(Editor, nodeName.c_str());

            *this << " HAPI_CookNode: node:" << node << " name:" << nodeName << " cooked " << info.totalCookCount << " times" << "";
            HAPI_CookNode(&m_session, node, &m_cookOptions);
            int cookStatus;
            HAPI_Result cookResult;

            bool done = false;
            do
            {
                //Embedded mode doesn't benefit from this since it cannot be displayed!  No point in slowing it down by asking:
                if (GetHoudiniMode() == HOUDINI_MODE_ASYNC)
                {
                    int statusBufSize = 0;
                    HAPI_GetStatusStringBufLength(&m_session, HAPI_STATUS_COOK_STATE, HAPI_STATUSVERBOSITY_ERRORS, &statusBufSize);

                    if (statusBufSize > 0)
                    {
                        AZStd::unique_lock<AZStd::mutex> theLock(m_statusLock);
                        m_currentStatus.resize(statusBufSize);
                        HAPI_GetStatusString(&m_session, HAPI_STATUS_COOK_STATE, m_currentStatus.data(), statusBufSize);

                        m_currentStatus = entityName + " [" + nodeName + "]=>" + m_currentStatus;

                        int cur = 0;
                        int tot = 1;
                        HAPI_GetCookingCurrentCount(&m_session, &cur);
                        HAPI_GetCookingTotalCount(&m_session, &tot);
                        m_currentPercent = (int)(((double)cur / (double)(tot == 0 ? 1 : tot)) * 100);
                    }
                }
                
                cookResult = HAPI_GetStatus(&m_session, HAPI_STATUS_COOK_STATE, &cookStatus);                
                done = !(cookStatus > HAPI_STATE_MAX_READY_STATE && cookResult == HAPI_RESULT_SUCCESS);

                if (cookResult == HAPI_RESULT_USER_INTERRUPTED)
                {
                    done = true;
                }

                AZStd::this_thread::yield();

            } while (!done);

            bool isReady = false;
            {
                AZStd::unique_lock<AZStd::mutex> theWorkLock(m_threadWorkingLock);
                isReady = m_work.size() == 0;
            }

            if (isReady)
            {
                SetProgress("Ready", 100);
            }

            *this << " HAPI_CookNode Done: node:" << node << " name:" << nodeName << " cooked " << info.totalCookCount << " times" << "";            
        }
    }

    void Houdini::SaveDebugFile()
    {
        AZ_PROFILE_FUNCTION(Editor);

        char desktopPath[1024];

#ifdef AZ_PLATFORM_WINDOWS
        SHGetFolderPathA(NULL, CSIDL_DESKTOPDIRECTORY | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, desktopPath);
#endif

        *this << " HAPI_SaveHIPFile to " << desktopPath << "\\DebugAsset.hip" << "";
        HAPI_SaveHIPFile(&m_session, (AZStd::string(desktopPath) + "\\DebugAsset.hip").c_str(), false);
    }

    void Houdini::Test()
    {
        HoudiniAssetPtr asset = LoadHoudiniDigitalAsset("MetalRail.hda");

        auto err = GetLastHoudiniError();

        HoudiniNodePtr paintCanNode = CreateNode("Object/MetalRail", "MyMetalRail");
        HoudiniNodePtr paintCanNode2 = CreateNode("Object/MetalRail", "MyMetalRail_clone");
        HoudiniNodePtr curve = CreateNode("sop/curve", "NURBS");

        HAPI_NodeId curveNode = curve->GetId();

        int cookStatus;
        HAPI_Result cookResult;
        do
        {
            cookResult = HAPI_GetStatus(&m_session, HAPI_STATUS_COOK_STATE, &cookStatus);
        } while (cookStatus > HAPI_STATE_MAX_READY_STATE && cookResult == HAPI_RESULT_SUCCESS);


        HAPI_NodeInfo curveNodeInfo;
        (HAPI_GetNodeInfo(&m_session, curveNode, &curveNodeInfo));

        AZStd::vector<HAPI_ParmInfo> parmInfos(curveNodeInfo.parmCount);
        (HAPI_GetParameters(&m_session, curveNode, &parmInfos[0], 0, curveNodeInfo.parmCount));

        int coordsParmIndex = -1;
        int typeParmIndex = -1;

        for (int i = 0; i < curveNodeInfo.parmCount; i++)
        {
            AZStd::string parmName = GetString(parmInfos[i].nameSH);
            if (parmName == "coords")
            {
                coordsParmIndex = i;
            }
            if (parmName == "type")
            {
                typeParmIndex = i;
            }
        }
        if (coordsParmIndex == -1 || typeParmIndex == -1)
        {
            //std::cout << "Failure at " << __FILE__ << ": " << __LINE__ << std::endl;
            //std::cout << "Could not find coords/type parameter on curve node" << std::endl;
        }
        HAPI_ParmInfo parm;
        HAPI_GetParameters(&m_session, curveNode, &parm, typeParmIndex, 1);
        int typeValue = 1;
        HAPI_SetParmIntValues(&m_session, curveNode, &typeValue, parm.intValuesIndex, 1);
        HAPI_GetParameters(&m_session, curveNode, &parm, coordsParmIndex, 1);
        HAPI_SetParmStringValue(&m_session, curveNode, "-4,0,4 -4,0,-4 4,0,-4 4,0,4", parm.id, 0);

        CheckForErrors();
    }

    HoudiniAssetPtr Houdini::LoadHoudiniDigitalAsset(const AZStd::string& hdaName)
    {
        AZ_PROFILE_FUNCTION(Editor);

        if (m_assetCache.find(hdaName) == m_assetCache.end())
        {
            HAPI_AssetLibraryId id;
            AZStd::string hdaFile = FindHda(hdaName);

            HAPI_LoadAssetLibraryFromFile(&m_session, hdaFile.c_str(), true, &id);
            auto err = GetLastHoudiniError();

            if (err.size() > 0)
            {
                AZ_Warning("HOUDINI", false, (AZStd::string("[HOUDINI] ") + err).c_str());
            }
            else
            {
                HoudiniAssetPtr asset = HoudiniAssetPtr(new HoudiniAsset(this, id, hdaFile.c_str(), hdaName.c_str()));
                m_assetCache[hdaName] = asset;
                return asset;
            }

            return nullptr;
        }
        else
        {
            return m_assetCache[hdaName];
        }
    }

    AZStd::vector<AZStd::string> Houdini::GetAvailableAssetNames()
    {
        AZStd::vector<AZStd::string> output;

        for (auto asset : m_assetCache)
        {
            output.push_back(asset.second->GetHdaName());
        }
        return output;
    }

    AZStd::vector<HoudiniAssetPtr> Houdini::GetAvailableAssets()
    {
        AZStd::vector<HoudiniAssetPtr> output;

        for (auto asset : m_assetCache)
        {
            output.push_back(asset.second);
        }
        return output;
    }

    HoudiniAssetPtr Houdini::GetAsset(const AZStd::string& assetName)
    {
        if (m_assetCache.find(assetName) != m_assetCache.end())
        {
            return m_assetCache[assetName];
        }

        return nullptr;
    }

    /* Houdini Util functions */
    AZStd::string Houdini::GetString(HAPI_StringHandle string_handle)
    {
        AZ_PROFILE_FUNCTION(Editor);

        // A string handle of 0 means an invalid string handle -- similar to
        // a null pointer.  Since we can't return NULL, though, return an empty
        // string.
        if (string_handle == 0)
        {
            return "";
        }

        int buffer_length = 0;
        HAPI_GetStringBufLength(&m_session, string_handle, &buffer_length);

        if (buffer_length <= 0)
        {
            return "";
        }

        char * buf = new char[buffer_length];
        HAPI_GetString(&m_session, string_handle, buf, buffer_length);
        AZStd::string result(buf);

        delete[] buf;
        return result;
    }

    AZStd::string Houdini::GetLastHoudiniError()
    {
        AZ_PROFILE_FUNCTION(Editor);

        int bufferLength = 0;
        HAPI_GetStatusStringBufLength(&m_session, HAPI_STATUS_CALL_RESULT, HAPI_STATUSVERBOSITY_ERRORS, &bufferLength);

        if (bufferLength <= 1)
        {
            return "";
        }

        char * buffer = new char[bufferLength];
        HAPI_GetStatusString(&m_session, HAPI_STATUS_CALL_RESULT, buffer, bufferLength);

        AZStd::string result(buffer);

        delete[] buffer;
        return result;
    }

    AZStd::string Houdini::GetLastHoudiniCookError()
    {
        AZ_PROFILE_FUNCTION(Editor);

        int bufferLength = 0;
        HAPI_GetStatusStringBufLength(&m_session, HAPI_STATUS_COOK_RESULT, HAPI_STATUSVERBOSITY_ERRORS, &bufferLength);

        if (bufferLength <= 1)
        {
            return "";
        }

        char * buffer = new char[bufferLength];
        HAPI_GetStatusString(&m_session, HAPI_STATUS_COOK_RESULT, buffer, bufferLength);
        AZStd::string result(buffer);

        delete[] buffer;
        
        if (result == "Cook succeeded.")
        {
            result = "";
        }
        
        return result;
    }

    //Specialization has to be in the CPP to prevent multi-instantiation
    template<>
    IHoudini& operator<< (IHoudini& os, const char* dt)
    {
        AZ_PROFILE_FUNCTION(Editor);
        os.Log(AZStd::string(dt));
        return os;
    }

    template<>
    IHoudini& operator<< (IHoudini& os, char* dt)
    {
        AZ_PROFILE_FUNCTION(Editor);
        os.Log(AZStd::string(dt));
        return os;
    }
    

    template<>
    IHoudini& operator<< (IHoudini& os, AZStd::string dt)
    {
        AZ_PROFILE_FUNCTION(Editor);
        os.Log(dt);
        return os;
    }

    template<>
    IHoudini& operator<< (IHoudini& os, const AZStd::string& dt)
    {
        AZ_PROFILE_FUNCTION(Editor);
        os.Log(dt);
        return os;
    }


}
