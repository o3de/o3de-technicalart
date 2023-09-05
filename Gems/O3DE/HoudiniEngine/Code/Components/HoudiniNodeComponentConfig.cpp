#include "StdAfx.h"

//Removed for now, we might use something like this later:
#include "SideFX/HE_Viewer.h" 

#include <HoudiniEngine/HoudiniApi.h>
#include <HoudiniEngine/HoudiniEngineBus.h>
#include <Components/HoudiniNodeComponentConfig.h>
#include <HoudiniCommon.h>
#include "ExtractPoints.h"
#include <HoudiniSplineTranslator.h>

#include "OperatorSelection.h"

#include <ISystem.h>
#include <Windows.h>

#include <AzToolsFramework/Entity/EditorEntityContextBus.h>
#include <AzToolsFramework/Entity/EditorEntityInfoBus.h>

#include <QInputDialog>
#include <QLineEdit>


namespace HoudiniEngine
{
    /*static*/ void HoudiniNodeComponentConfig::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<HoudiniNodeComponentConfig>()
                ->Version(2, HoudiniNodeComponentConfig::VersionConverter)
                ->Field("Locked", &HoudiniNodeComponentConfig::m_locked)
                ->Field("OperatorType", &HoudiniNodeComponentConfig::m_operatorName)
                ->Field("NodeName", &HoudiniNodeComponentConfig::m_nodeName)
                //->Field("HelpText", &HoudiniNodeComponentConfig::m_helpTextField)
                ->Field("Props", &HoudiniNodeComponentConfig::m_properties)
                ->Field("LastError", &HoudiniNodeComponentConfig::m_lastError)
                ->Field("ViewButton", &HoudiniNodeComponentConfig::m_viewButton)
                ->Field("ViewReloadButton", &HoudiniNodeComponentConfig::m_viewReloadButton)
                ->Field("ViewReloadPropertiesButton", &HoudiniNodeComponentConfig::m_viewReloadPropertiesButton) // FL[FD-15480] Update parm values from Houdini side
                ->Field("ExtractGroupSpline", &HoudiniNodeComponentConfig::m_viewExtractGroup)
                //->Field("ViewDebugButton", &HoudiniNodeComponentConfig::m_viewDebugButton)
                ->Field("HasSpline", &HoudiniNodeComponentConfig::m_hasSpline)
                ;

            if (auto ec = serialize->GetEditContext())
            {
                ec->Class<HoudiniNodeComponentConfig>("HoudiniNodeComponentConfig", "Base container for a Houdini Digital Asset")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)

                    /*->DataElement(AZ::Edit::UIHandlers::MultiLineEdit, &HoudiniNodeComponentConfig::m_helpTextField, "Help Text", "Useful information")
                    ->Attribute(AZ_CRC("PlaceholderText", 0xa23ec278), &HoudiniNodeComponentConfig::GetHelpText)
                    ->Attribute(AZ::Edit::Attributes::ReadOnly, true)*/

                    ->DataElement(AZ::Edit::UIHandlers::Button, &HoudiniNodeComponentConfig::m_nodeName, "Node Name", "This must be unique per file.  The cache files will be generated with this name.")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniNodeComponentConfig::OnNodeNameChanged)
                    ->Attribute(AZ::Edit::Attributes::ButtonText, &HoudiniNodeComponentConfig::m_nodeName)

                    ->DataElement(AZ::Edit::UIHandlers::Button, &HoudiniNodeComponentConfig::m_operatorName, "OperatorType", "List of Houdini Digital Assets")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniNodeComponentConfig::OnSelectOperator)
                    ->Attribute(AZ::Edit::Attributes::ButtonText, &HoudiniNodeComponentConfig::m_operatorName)

                    ->DataElement(AZ::Edit::UIHandlers::Button, &HoudiniNodeComponentConfig::m_viewButton, "", "Creates an instance of this node.")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniNodeComponentConfig::OnLoadHoudiniInstance)
                    ->Attribute(AZ::Edit::Attributes::ButtonText, "Load")

                    ->DataElement(AZ::Edit::UIHandlers::Button, &HoudiniNodeComponentConfig::m_viewReloadButton, "", "Reloads the instance")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniNodeComponentConfig::OnReloadHoudiniInstance)
                    ->Attribute(AZ::Edit::Attributes::ButtonText, "Reload Asset")

                    // FL[FD-15480] Update parm values from Houdini side
                    ->DataElement(AZ::Edit::UIHandlers::Button, &HoudiniNodeComponentConfig::m_viewReloadPropertiesButton, "", "Reloads properties in LY without affecting Houdini engine")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniNodeComponentConfig::OnReloadProperties)
                    ->Attribute(AZ::Edit::Attributes::ButtonText, "Reload Properties")

                    ->DataElement(AZ::Edit::UIHandlers::Button, &HoudiniNodeComponentConfig::m_viewExtractGroup, "", "Extracts a given group as a spline.")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniNodeComponentConfig::OnExtractGroupPoints)
                    ->Attribute(AZ::Edit::Attributes::ButtonText, "Extract Group as Spline")

                    /*->DataElement(AZ::Edit::UIHandlers::Button, &HoudiniNodeComponentConfig::m_viewDebugButton, "", "Saves a file to your desktop - DebugAssetp.hip with the current houdini session.")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniNodeComponentConfig::OnSaveDebugAsset)
                    ->Attribute(AZ::Edit::Attributes::ButtonText, "Save Debug Asset")*/

                    ->DataElement(0, &HoudiniNodeComponentConfig::m_properties, "Properties", "Lua script properties")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    
                    ->DataElement(AZ::Edit::UIHandlers::LineEdit, &HoudiniNodeComponentConfig::m_lastError, "Errors:", "The last error reported")
                    
                    ->DataElement(AZ::Edit::UIHandlers::CheckBox, &HoudiniNodeComponentConfig::m_locked, "Lock Geometry", "When checked, geometry will not re-generate.")

                    ;
            }
        }
    }

    bool HoudiniNodeComponentConfig::VersionConverter(AZ::SerializeContext& context, AZ::SerializeContext::DataElementNode& classElement)
    {
        if (classElement.GetVersion() == 1)
        {           
            classElement.AddElementWithData<bool>(context, "Locked", false);
        }
        return true;
    }

    AZStd::vector<AZStd::string> HoudiniNodeComponentConfig::getOperatorNames()
    {
        AZ_PROFILE_FUNCTION(Editor);

        HoudiniPtr hou;
        HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);
        AZStd::vector<AZStd::string> output;
        output.push_back("");

        if (hou != nullptr && hou->IsActive())
        {
            auto assets = hou->GetAvailableAssets();

            for (auto asset : assets)
            {
                auto result = asset->getAssets();
                if (result.size() == 0)
                {
                    m_operatorName = "";
                }

                if (m_selectionMode == OperatorMode::Scatter)
                {
                    QString file = asset->GetHdaFile().c_str();
                    file = file.toLower();
                    if (file.indexOf("scatter") >= 0)
                    {
                        for (auto op : result)
                        {
                            output.push_back(op);
                        }
                    }
                }
                else if (m_selectionMode == OperatorMode::Terrain)
                {
                    QString file = asset->GetHdaFile().c_str();
                    file = file.toLower();
                    if (file.indexOf("terrain") >= 0)
                    {
                        for (auto op : result)
                        {
                            output.push_back(op);
                        }
                    }
                }
                else 
                {
                    for (auto op : result)
                    {
                        output.push_back(op);
                    }
                }
            }
        }

        return output;
    }

    AZ::Crc32 HoudiniNodeComponentConfig::OnSelectOperator()
    {        
        if (m_creating == false)
        {
            OperatorSelection window(m_selectionMode, m_operatorName);
            window.exec();

            if (window.result() == QDialog::Accepted)
            {
                if (m_operatorName != window.GetSelectedOperator())
                {
                    m_operatorName = window.GetSelectedOperator();
                    OnLoadHoudiniInstance();
                }
                return AZ::Edit::PropertyRefreshLevels::AttributesAndValues;
            }
        }

        return AZ::Edit::PropertyRefreshLevels::None;
    }

    AZ::Crc32 HoudiniNodeComponentConfig::OnNodeNameChanged()
    {
        AZ_PROFILE_FUNCTION(Editor);

        bool ok;
        auto str = QInputDialog::getText(nullptr, "Rename Node", "What would you like to name this node?  This will also be the name of the fbx files generated.", QLineEdit::EchoMode::Normal, m_nodeName.c_str(), &ok);
        
        if (ok)
        {
            AZStd::string newNodeName = str.toUtf8().data();
            RenameNode(newNodeName);
        }

        return AZ::Edit::PropertyRefreshLevels::AttributesAndValues;
    }

    AZ::Crc32 HoudiniNodeComponentConfig::OnSaveDebugAsset()
    {
        AZ_PROFILE_FUNCTION(Editor);

        HoudiniPtr hou;
        HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);

        if (hou != nullptr && hou->IsActive())
        {
            hou->SaveDebugFile();
        }

        return AZ::Edit::PropertyRefreshLevels::AttributesAndValues;
    }

    const AZStd::string& HoudiniNodeComponentConfig::GetLastError()
    {
        m_lastError = "";
        if (m_node != nullptr) {
            m_lastError = m_node->GetLastError();
        }

        return m_lastError;
    }

    AZ::ScriptProperty* HoudiniNodeComponentConfig::GetProperty(const AZStd::string& name)
    {
        return m_properties.GetProperty(name.c_str());
    }

    AZ::Crc32 HoudiniNodeComponentConfig::OnLoadHoudiniInstance()
    {
        AZ_PROFILE_FUNCTION(Editor);

        if (m_nodeName.empty()) 
        {
            AZ::ComponentApplicationBus::BroadcastResult(m_nodeName, &AZ::ComponentApplicationRequests::GetEntityName, m_entityId);            
        }

        auto* result = LoadHda(m_operatorName, m_nodeName);

        if ( result != nullptr)
        {
            return AZ::Edit::PropertyRefreshLevels::AttributesAndValues;
        }
        return AZ::Edit::PropertyRefreshLevels::None;
    }

    AZ::Crc32 HoudiniNodeComponentConfig::OnReloadHoudiniInstance()
    {
        BackupInputTypeProperties(); // FL[FD-15480] Update parm values from Houdini side

        m_properties.Clear();
        
        /*auto* result =*/ LoadHda(m_operatorName, m_nodeName);
        return AZ::Edit::PropertyRefreshLevels::EntireTree;
    }


    // Creates a new entity with an Editor spline component.
    AZ::EntityId HoudiniNodeComponentConfig::CreateSplineEntity(const AZStd::string& name, AZ::EntityId parent)
    {
        AZ::EntityId newEntityId;
        AZ::Entity* newSplineEntity;

        AzToolsFramework::EditorEntityContextRequestBus::BroadcastResult(newEntityId, &AzToolsFramework::EditorEntityContextRequests::CreateNewEditorEntity, name.c_str());

        AZ::ComponentApplicationBus::BroadcastResult(newSplineEntity, &AZ::ComponentApplicationRequests::FindEntity, newEntityId);

        AZ::Transform transform = AZ::Transform::CreateIdentity();

        if (parent.IsValid())
        {
            AZ::TransformBus::Event(newEntityId, &AZ::TransformInterface::SetParent, parent);
        }

        AZ::TransformBus::Event(newEntityId, &AZ::TransformInterface::SetLocalTM, transform);

        newSplineEntity->Deactivate();
        newSplineEntity->CreateComponent(AZ::Uuid("{5B29D788-4885-4D56-BD9B-C0C45BE08EC1}") /*Editor Spline component*/);
        newSplineEntity->Activate();

        return newEntityId;
    }


    AZ::Crc32 HoudiniNodeComponentConfig::OnExtractGroupPoints()
    {
        HoudiniPtr hou;
        HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);

        if (hou == nullptr || hou->IsActive() == false)
        {
            return AZ::Edit::PropertyRefreshLevels::None;
        }
        
        HE_Viewer * viewer = new HE_Viewer();
        viewer->SetNode(this->m_node->GetId());
        viewer->SetSession(&hou->GetSession());
        viewer->Refresh();
        viewer->show();
        
        ExtractPoints window(m_node.get());
        window.resize(260, 600);

        window.exec();

        if (window.result() == QDialog::Accepted)
        {
            auto groupPoints = m_node->GetGeometryPointGroup(window.GetSelectedGroups());
            if (groupPoints.size() > 0)
            {
                auto& groups = window.GetSelectedGroups();
                AZStd::string newName = "";

                for (auto& groupName : groups)
                {
                    newName += groupName + "_";
                }

                AZ::Transform transform = AZ::Transform::CreateIdentity();
                AZ::TransformBus::EventResult(transform, m_entityId, &AZ::TransformBus::Events::GetWorldTM);

                //Move to world space:
                for (auto& point : groupPoints)
                {
                    //O3DECONVERT double check transform point
                    point = transform.TransformPoint(point);
                }

                //Compute the center point
                AZ::Aabb box = AZ::Aabb::CreatePoints(groupPoints.data(), groupPoints.size());

                //Move to the center of the aabb
                AZ::Transform centerTransform = AZ::Transform::CreateTranslation(box.GetCenter());
                AZ::Transform centerTransformInv = AZ::Transform::CreateTranslation(-box.GetCenter());

                for (auto& point : groupPoints)
                {
                    // O3DECONVERT double check transform point
                    point = centerTransformInv.TransformPoint(point);
                }

                AZ::EntityId newId;

                if (window.IsReuseChildren())
                {
                    AzToolsFramework::EntityIdList childEntityIds;
                    AzToolsFramework::EditorEntityInfoRequestBus::EventResult(childEntityIds, m_entityId, &AzToolsFramework::EditorEntityInfoRequests::GetChildren);

                    for (auto& childId : childEntityIds)
                    {
                        AZStd::string childName;
                        AzToolsFramework::EditorEntityInfoRequestBus::EventResult(childName, childId, &AzToolsFramework::EditorEntityInfoRequests::GetName);

                        if (childName == newName)
                        {
                            newId = childId;
                        }
                    }
                }

                if (newId.IsValid() == false)
                {

					AZ::EntityId newEntityId;
					//AZ::Entity* newSplineEntity;

					AzToolsFramework::EditorEntityContextRequestBus::BroadcastResult(newEntityId, &AzToolsFramework::EditorEntityContextRequests::CreateNewEditorEntity, newName.c_str());

					//AZ::ComponentApplicationBus::BroadcastResult(newSplineEntity, &AZ::ComponentApplicationRequests::FindEntity, newEntityId);

					if (m_entityId.IsValid())
					{
						AZ::TransformBus::Event(newEntityId, &AZ::TransformInterface::SetParent, m_entityId);
					}

					AZ::TransformBus::Event(newEntityId, &AZ::TransformInterface::SetLocalTM, AZ::Transform::CreateIdentity());

					AzToolsFramework::EntityIdList entityIdList = { newEntityId };

					AzToolsFramework::EntityCompositionRequests::AddComponentsOutcome addedComponentsResult =
						AZ::Failure(AZStd::string("Failed to call AddComponentsToEntities on EntityCompositionRequestBus"));
					AzToolsFramework::EntityCompositionRequestBus::BroadcastResult(
						addedComponentsResult,
						&AzToolsFramework::EntityCompositionRequests::AddComponentsToEntities,
						entityIdList,
						AZ::ComponentTypeList{ AZ::Uuid("{5B29D788-4885-4D56-BD9B-C0C45BE08EC1}") });

					//newSplineEntity->Deactivate();
					//newSplineEntity->CreateComponent(AZ::Uuid("{5B29D788-4885-4D56-BD9B-C0C45BE08EC1}") /*Editor Spline component*/);
					//newSplineEntity->Activate();

                    newId = newEntityId;
                    //newId = CreateSplineEntity(newName, m_entityId);
                }
                
                AZ::TransformBus::Event(newId, &AZ::TransformBus::Events::SetWorldTM, centerTransform);
                LmbrCentral::SplineComponentRequestBus::Event(newId, &LmbrCentral::SplineComponentRequests::SetVertices, groupPoints);
            }
        }

        return AZ::Edit::PropertyRefreshLevels::None;
    }

    bool HoudiniNodeComponentConfig::UpdateNode()
    {
        AZ_PROFILE_FUNCTION(Editor);
        
        if (m_node != nullptr && m_initialized )
        {
            //Update any properties that tried to connect to a node that didn't exist yet.
            for (auto param : m_properties.m_properties)
            {
                HoudiniScriptProperty* propOld = azrtti_cast<HoudiniScriptProperty*>(param);
                if (propOld != nullptr)
                {
                    propOld->Update();
                }
            }

            bool updated = false;
            if (!m_locked)
            {
                updated = m_node->UpdateData();
                m_lastError = m_node->GetLastError();
            }

            return updated;
        }

        return false;
    }

    void HoudiniNodeComponentConfig::RenameNode(const AZStd::string& newName)
    {
        auto oldName = m_nodeName;
        m_nodeName = newName;

        if (oldName == newName)
        { 
            return;
        }  

        HoudiniPtr hou;
        HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);

        if (hou == nullptr || hou->IsActive() == false)
        {
            return;
        }

        if (GetNode() != nullptr) 
        {            
            HAPI_NodeId parentId = GetNode()->GetNodeInfo().parentId;

            if (parentId != HOUDINI_ROOT_NODE_ID)
            {
                HAPI_NodeInfo parentInfo;
                HAPI_GetNodeInfo(&hou->GetSession(), parentId, &parentInfo);
                auto parentName = hou->GetString(parentInfo.nameSH);

                if (parentName == oldName)
                {
                    HAPI_RenameNode(&hou->GetSession(), parentId, newName.c_str());
                }
            }

            HAPI_RenameNode(&hou->GetSession(), GetNode()->GetId(), newName.c_str());
            
            GetNode()->Cook();
            GetNode()->SetDirty();

            //Names must be unique in Houdini:
            m_nodeName = hou->GetString(GetNode()->GetNodeInfo().nameSH);
            
            hou->RenameNode(oldName, m_node.get());
        }
        
        HoudiniAssetRequestBus::Broadcast(&HoudiniAssetRequests::FixEntityPointers);
    }

    void HoudiniNodeComponentConfig::FixEntityPointers()
    {
        //Properties do not auto-update in Houdini since they send strings =(
        for (auto param : m_properties.m_properties)
        {
            HoudiniScriptPropertyEntity* propOld = azrtti_cast<HoudiniScriptPropertyEntity*>(param);
            if (propOld != nullptr)
            {
                propOld->ScriptHasChanged();
            }
        }
    }

    AZStd::string HoudiniNodeComponentConfig::GetHelpText()
    {
        AZ_PROFILE_FUNCTION(Editor);

        if ( m_node != nullptr)
        {
            return m_node->GetHelpText();
        }

        return "";
    }    

    void populateGroup(HoudiniNodePtr node, const AZStd::vector<HoudiniParameterPtr>& params, HoudiniPropertyGroup& group, bool isReload = false, HAPI_ParmId parentId = -1)
    {
        for (int i = 0; i < params.size(); i++)
        {
            auto& param = params[i];
            if (param->IsProcessed() || param->GetParentId() != parentId)
            {
                continue;
            }

			if (param->GetInfo().invisible || param->GetInfo().disabled)
			{
                param->SetProcessed(true);
                continue;
			}

			if (param->GetType() == HAPI_PARMTYPE_FOLDERLIST)
			{
				//TODO: We don't need to react here unless we want to support multi-level nesting which atm we do not.
				//TODO: add a folder list property and make that the active folder list
				//      the property will show the tabs or maybe buttons
				param->SetProcessed(true);
				continue;
			}

			if (param->GetType() == HAPI_PARMTYPE_FOLDER)
			{
				auto subGroup = group.GetGroup(param->GetLabel().c_str());
				if (subGroup == nullptr)
				{
					group.m_groups.emplace_back();
					subGroup = &group.m_groups.back();
					subGroup->m_name = param->GetLabel();
				}
				param->SetProcessed(true);
                
				populateGroup(node, params, *subGroup, isReload, param->GetId());

                continue;
			}

			HoudiniScriptProperty* prop = nullptr;
			HoudiniScriptProperty* propOld = isReload ? nullptr : azrtti_cast<HoudiniScriptProperty*>(group.GetProperty(param->GetName().c_str()));

			// FL[FD-11761] Add support for multiparm blocks (lists) to lumberyard houdini engine
			if (param->GetInfo().isChildOfMultiParm)
			{
				param->SetProcessed(true);
				continue;
			}

			if (propOld == nullptr)
			{
				prop = HoudiniScriptPropertyFactory::CreateNew(param);
				if (prop != nullptr)
				{
					prop->SetNode(node);
					//prop->SetGroupName(group);
					group.m_properties.push_back(prop);
				}
			}
			else
			{
				propOld->SetNode(node);
				propOld->SetParameter(param);
				//propOld->SetGroupName(group);
				propOld->ScriptHasChanged();
			}

			param->SetProcessed(true);
        }
    }

    IHoudiniNode* HoudiniNodeComponentConfig::LoadHda(const AZStd::string& operatorName, const AZStd::string& nodeName, AZStd::function<void(IHoudiniNode*)> onLoad)
    {
        AZ_PROFILE_FUNCTION(Editor);

        if (m_creating || m_locked)
        {
            //Already creating:
            return nullptr;
        }

        AZStd::string newName = nodeName;        
        bool nameConflict = false;
        do
        {
            AZ::EBusAggregateResults<AZ::EntityId> idsWithSameName;
            HoudiniAssetRequestBus::BroadcastResult(idsWithSameName, &HoudiniAssetRequests::GetEntityIdFromNodeName, newName);
            nameConflict = false;

            for (auto idWithSameName : idsWithSameName.values)
            {
                //If its valid and its not ours then its a conflict.
                if (idWithSameName.IsValid() && idWithSameName != m_entityId)
                {
                    nameConflict = true;
                }
            }

            if (nameConflict)
            {
                newName += "_";
            }
        } while (nameConflict);

        m_operatorName = operatorName;
        m_nodeName = newName;
        m_initialized = false;
        
        if (m_operatorName.length() > 0)
        {
            HoudiniPtr hou;
            HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);
            if (hou == nullptr || hou->IsActive() == false)
            {        
                return nullptr;
            }

            m_creating = true;
            //TODO: with support for groups this doesn't work anymore as we have properties from other groups.
            auto propertiesCopy = m_properties.m_properties;
            
            //Update right now!
            hou->LookupId(m_entityId);
                        
            AZStd::function<bool()> createFunction = [this, hou, onLoad, propertiesCopy] () mutable
            {
                AZStd::vector<AZ::ScriptProperty*> newProps;

                if (m_node != nullptr)
                {
                    if (m_node->GetOperatorName() != m_operatorName)
                    {
                        //The type changed, let it all go.
                        propertiesCopy.clear();
                        m_properties.Clear();
                        m_node->DeleteNode();
                        hou->RemoveNode(m_nodeName, m_node.get());
                        //try to remove spline component if any and reset the flag
                        HoudiniSplineTranslator::RemoveSplineComponent(m_entityId);
                        m_hasSpline = false; //reset to false since we switched HDA
                    }                    
                }
                
                AZ::Transform transform = hou->LookupTransform(m_entityId);
                AZStd::string entityName = hou->LookupEntityName(m_entityId);

                m_node = hou->CreateNode(m_operatorName, m_nodeName);

                if (m_node == nullptr)
                {
                    AZ_Warning("[HOUDINI]", false, "Unable to load node %s as entity: %s ", m_operatorName.c_str(), m_nodeName.c_str());
                    return false;
                }

                m_node->SetEntityId(m_entityId);
                m_node->SetObjectTransform(transform);

                m_node->UpdateParamInfoFromEngine(); // FL[FD-15480] Update parm values from Houdini side
                m_node->UpdateEditableNodeFromEngine();
                if (m_hasSpline) //we have a spline component added
                {
                    m_node->SetEditableGeometryBuilt(true);
                    HAPI_GeoInfo geometryInfo = m_node->GetEditableGeometryInfo();
                    HoudiniSplineTranslator::HapiUpdateNodeForHoudiniSplineComponent(m_entityId, geometryInfo.nodeId, false);
                }
                m_properties.m_name = "Houdini Properties";

                // TODO: check how this works
                for (int i = 0; i < m_node->GetNodeInfo().inputCount; i++)
                {
                    AZ_PROFILE_SCOPE(Editor, "HoudiniNodeComponentConfig::LoadHda::BuildInputs");
                    //HoudiniScriptProperty * prop = nullptr; // FL[FD-15480] Update parm values from Houdini side

                    HAPI_StringHandle nameHandle;
                    HAPI_GetNodeInputName(&m_node->GetHou()->GetSession(), m_node->GetId(), i, &nameHandle);
                    auto nameGen = m_node->GetHou()->GetString(nameHandle);

                    HoudiniScriptPropertyInput* propOld = azrtti_cast<HoudiniScriptPropertyInput*>(m_properties.GetProperty(nameGen.c_str()));

                    if (propOld == nullptr)
                    {
                        // FL[FD-15480] Update parm values from Houdini side
                        HoudiniScriptPropertyInput* prop = aznew HoudiniScriptPropertyInput(m_node, i);
                        if (m_inputTypePropertiesBackup.find(nameGen) != m_inputTypePropertiesBackup.end())
                        {
                            prop->m_value = m_inputTypePropertiesBackup[nameGen];
                        }
                        prop->ScriptHasChanged();

                        propertiesCopy.push_back(prop);
                    }
                    else
                    {
                        propOld->SetNode(m_node);
                        propOld->SetInputIndex(i);
                        propOld->ScriptHasChanged();
                    }
                }

                AZStd::vector<AZ::ScriptProperty*> unUsedProps;

                //Find missing properties and remove them:
                for (AZ::ScriptProperty* oldProp : propertiesCopy)
                {
                    bool found = false;
                    for (auto param : m_node->GetParameters())
                    {
                        if (oldProp->m_name == param->GetName() && (!param->GetInfo().invisible && !param->GetInfo().disabled))
                        {
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                    {
                        //Check inputs too!
                        for (int i = 0; i < m_node->GetNodeInfo().inputCount; i++)
                        {
                            HAPI_StringHandle nameHandle;
                            HAPI_GetNodeInputName(&m_node->GetHou()->GetSession(), m_node->GetId(), i, &nameHandle);
                            auto nameGen = m_node->GetHou()->GetString(nameHandle);

                            if (oldProp->m_name == nameGen)
                            {
                                found = true;
                                break;
                            }
                        }
                    }

                    if (!found)
                    {
                        unUsedProps.push_back(oldProp);
                    }
                }

                for (AZ::ScriptProperty* oldProp : unUsedProps)
                {
                    auto result = AZStd::find(propertiesCopy.begin(), propertiesCopy.end(), oldProp);
                    if (result != propertiesCopy.end())
                    {
                        propertiesCopy.erase(result);

                        AZ_Warning("HOUDINI", false, "[Entity: %s][Node: %s] - Removing missing property %s"
                            , (entityName + " " + m_entityId.ToString()).c_str()
                            , m_node->GetNodeName().c_str()
                            , oldProp->m_name.c_str()
                        );
                    }
                }

                //Add new properties or link up existing ones
                auto& params = m_node->GetParameters();
                for (auto param : params)
                {
                    param->SetProcessed(false);
                }
                populateGroup(m_node, params, m_properties);

                //You cannot set this while creating or you will break m_isAlreadyQueuedRefresh and you will not be able to select this object properly:
                
                if (hou->LookupIsSelected(m_entityId))
                {
                    AZ::TickBus::QueueFunction([this]()
                    {
                        AzToolsFramework::ToolsApplicationEvents::Bus::Broadcast(
                            &AzToolsFramework::ToolsApplicationEvents::Bus::Events::InvalidatePropertyDisplay, AzToolsFramework::PropertyModificationRefreshLevel::Refresh_EntireTree);
                    });
                }
                
                for (auto it = m_defaultEntities.begin(); it != m_defaultEntities.end(); it++)
                {
                    auto propNameToSet = it->first;
                    auto entityIdToSet = it->second;
                    this->SetPropertyValueEntityId(propNameToSet, entityIdToSet);
                }

                m_defaultEntities.clear();
                m_initialized = true;
                m_creating = false;
                

                if (m_node != nullptr && onLoad)
                {                    
                    onLoad(m_node.get());
                }

                return m_node.get() != nullptr;
            };
            
            hou->ExecuteCommand(m_entityId, createFunction);

            //Selected nodes get priority over other stuff.
            bool isSelected = false;
            AzToolsFramework::ToolsApplicationRequests::Bus::BroadcastResult(isSelected, &AzToolsFramework::ToolsApplicationRequests::IsSelected, m_entityId);

            if (isSelected)
            {
                hou->RaiseCommandPriority(m_entityId);
            }
        }        

        return nullptr;
    }


    bool HoudiniNodeComponentConfig::SetPropertyValueBool(const AZStd::string& name, bool value)
    {
        auto* property = reinterpret_cast<HoudiniScriptPropertyBoolean*>(this->GetProperty(name));
        if (property != nullptr) 
        {
            property->m_value = value;
            property->ScriptHasChanged();
            return true;
        }
        return false;
    }

    bool HoudiniNodeComponentConfig::SetPropertyValueInt(const AZStd::string& name, int value)
    {
        auto* property = reinterpret_cast<HoudiniScriptPropertyInt*>(this->GetProperty(name));
        if (property != nullptr)
        {
            property->m_value = value;
            property->ScriptHasChanged();
            return true;
        }
        return false;
    }

    bool HoudiniNodeComponentConfig::SetPropertyValueFloat(const AZStd::string& name, float value)
    {
        auto* property = reinterpret_cast<HoudiniScriptPropertyFloat*>(this->GetProperty(name));
        if (property != nullptr)
        {
            property->m_value = value;
            property->ScriptHasChanged();
            return true;
        }
        return false;
    }
   
    bool HoudiniNodeComponentConfig::SetPropertyValueVec2(const AZStd::string& name, const AZ::Vector2& value)
    {
        auto* property = reinterpret_cast<HoudiniScriptPropertyVector2*>(this->GetProperty(name));
        if (property != nullptr)
        {
            property->m_value = value;
            property->ScriptHasChanged();
            return true;
        }
        return false;
    }
   
    bool HoudiniNodeComponentConfig::SetPropertyValueVec3(const AZStd::string& name, const AZ::Vector3& value)
    {
        auto* property = reinterpret_cast<HoudiniScriptPropertyVector3*>(this->GetProperty(name));
        if (property != nullptr)
        {
            property->m_value = value;
            property->ScriptHasChanged();
            return true;
        }
        return false;
    }
    
    bool HoudiniNodeComponentConfig::SetPropertyValueVec4(const AZStd::string& name, const AZ::Vector4& value)
    {
        auto* property = reinterpret_cast<HoudiniScriptPropertyVector4*>(this->GetProperty(name));
        if (property != nullptr)
        {
            property->m_value = value;
            property->ScriptHasChanged();
            return true;
        }
        return false;
    }
   
    bool HoudiniNodeComponentConfig::SetPropertyValueEntityId(const AZStd::string& name, const AZ::EntityId & value)
    {
        auto* property = reinterpret_cast<HoudiniScriptPropertyEntity*>(this->GetProperty(name));
        if (property != nullptr)
        {
            property->m_value = value;
            property->ScriptHasChanged();
            return true;
        }
        else
        {
            m_defaultEntities[name] = value;
        }

        return false;
    }

    bool HoudiniNodeComponentConfig::SetInputEntityIdByName(const AZStd::string& name, const AZ::EntityId & value)
    {        
        if (m_node != nullptr)
        {            
            HoudiniScriptPropertyInput* propOld = azrtti_cast<HoudiniScriptPropertyInput*>(m_properties.GetProperty(name.c_str()));
            if (propOld != nullptr)
            {
                propOld->m_value = value;
                propOld->ScriptHasChanged();
            }
        }
        return false;
    }
    
    bool HoudiniNodeComponentConfig::SetInputEntityId(int index, const AZ::EntityId & value)
    {
        if (m_node != nullptr)
        {
            AZStd::string inputName;

            auto inputs = m_node->GetInputs();
            if ( index < inputs.size() )
            {
                inputName = inputs[index];
            }

            HoudiniScriptPropertyInput* propOld = azrtti_cast<HoudiniScriptPropertyInput*>(m_properties.GetProperty(inputName.c_str()));
            if (propOld != nullptr)
            {             
                propOld->m_value = value;
                propOld->ScriptHasChanged();
                return true;
            }
        }
        return false;
    }
   
    bool HoudiniNodeComponentConfig::SetPropertyValueString(const AZStd::string& name, const AZStd::string & value)
    {
        auto* property = reinterpret_cast<HoudiniScriptPropertyString*>(this->GetProperty(name));
        if (property != nullptr)
        {
            property->m_value = value;
        }
        return false;
    }

    int HoudiniNodeComponentConfig::GetPropertyValueInt(const AZStd::string& name)
    {
        auto* property = reinterpret_cast<HoudiniScriptPropertyInt*>(this->GetProperty(name));
        if (property != nullptr)
        {
            return property->m_value;
        }

        return 0;
    }

    float HoudiniNodeComponentConfig::GetPropertyValueFloat(const AZStd::string& name)
    {
        auto* property = reinterpret_cast<HoudiniScriptPropertyFloat*>(this->GetProperty(name));
        if (property != nullptr)
        {
            return property->m_value;
        }

        return 0.0f;
    }

    AZ::Vector2 HoudiniNodeComponentConfig::GetPropertyValueVec2(const AZStd::string& name)
    {
        auto* property = reinterpret_cast<HoudiniScriptPropertyVector2*>(this->GetProperty(name));
        if (property != nullptr)
        {
            return property->m_value;
        }

        return AZ::Vector2(0,0);
    }
    
    AZ::Vector3 HoudiniNodeComponentConfig::GetPropertyValueVec3(const AZStd::string& name)
    {
        auto* property = reinterpret_cast<HoudiniScriptPropertyVector3*>(this->GetProperty(name));
        if (property != nullptr)
        {
            return property->m_value;
        }

        return AZ::Vector3(0, 0, 0);
    }
    
    AZ::Vector4 HoudiniNodeComponentConfig::GetPropertyValueVec4(const AZStd::string& name)
    {
        auto* property = reinterpret_cast<HoudiniScriptPropertyVector4*>(this->GetProperty(name));
        if (property != nullptr)
        {
            return property->m_value;
        }

        return AZ::Vector4(0, 0, 0, 0);
    }

    AZStd::string HoudiniNodeComponentConfig::GetPropertyValueString(const AZStd::string& name)
    {
        auto* property = reinterpret_cast<HoudiniScriptPropertyString*>(this->GetProperty(name));
        if (property != nullptr)
        {
            return property->m_value;
        }        

        return "";
    }
    
    AZ::EntityId HoudiniNodeComponentConfig::GetPropertyValueEntityId(const AZStd::string& name)
    {
        auto* property = reinterpret_cast<HoudiniScriptPropertyEntity*>(this->GetProperty(name));
        if (property != nullptr)
        {
            return property->m_value;
        }

        return AZ::EntityId();
    }

    AZ::EntityId HoudiniNodeComponentConfig::GetInputEntityIdByName(const AZStd::string& /*name*/)
    {
        //Not supported by Houdini engine yet
        return AZ::EntityId();
    }

    AZ::EntityId HoudiniNodeComponentConfig::GetInputEntityId(int /*index*/)
    {
        //Not supported by Houdini engine yet
        return AZ::EntityId();
    }

    void HoudiniNodeComponentConfig::UpdateWorldTransformData(const AZ::Transform& transform)
    {
        if (m_node)
        {
            m_node->SetObjectTransform(transform);
        }
    }

    // FL[FD-15480] Update parm values from Houdini side
    void HoudiniNodeComponentConfig::BackupInputTypeProperties()
    {
        m_inputTypePropertiesBackup.clear();

        for (const auto& i : m_properties.m_properties)
        {
            if (HoudiniScriptPropertyInput* prop = azrtti_cast<HoudiniScriptPropertyInput*>(i))
            {
                if (prop->m_value.IsValid())
                {
                    m_inputTypePropertiesBackup.insert({ prop->m_name, prop->m_value });
                }
            }
        }
    }

    void HoudiniNodeComponentConfig::ReloadProperties()
    {
        if (!m_node)
        {
            return;
        }

        BackupInputTypeProperties();

        m_properties.Clear();

        m_node->UpdateParamInfoFromEngine();

        m_properties.m_name = "Houdini Properties";

        for (int i = 0; i < m_node->GetNodeInfo().inputCount; i++)
        {
            HAPI_StringHandle nameHandle;
            HAPI_GetNodeInputName(&m_node->GetHou()->GetSession(), m_node->GetId(), i, &nameHandle);
            auto nameGen = m_node->GetHou()->GetString(nameHandle);

            HoudiniScriptPropertyInput* prop = aznew HoudiniScriptPropertyInput(m_node, i);

            if (m_inputTypePropertiesBackup.find(nameGen) != m_inputTypePropertiesBackup.end())
            {
                prop->m_value = m_inputTypePropertiesBackup[nameGen];
            }
            prop->ScriptHasChanged();

            m_properties.m_properties.push_back(prop);
        }

		auto& params = m_node->GetParameters();
		for (auto param : params)
		{
			param->SetProcessed(false);
		}
		populateGroup(m_node, params, m_properties, true);

        AZ::TickBus::QueueFunction([]()
            {
                AzToolsFramework::ToolsApplicationEvents::Bus::Broadcast(
                    &AzToolsFramework::ToolsApplicationEvents::Bus::Events::InvalidatePropertyDisplay, AzToolsFramework::PropertyModificationRefreshLevel::Refresh_EntireTree);
            });
    }

    AZ::Crc32 HoudiniNodeComponentConfig::OnReloadProperties()
    {
        ReloadProperties();

        return AZ::Edit::PropertyRefreshLevels::EntireTree;
    }

}
