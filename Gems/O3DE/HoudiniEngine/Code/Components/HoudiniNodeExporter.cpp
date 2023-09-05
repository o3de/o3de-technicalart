#include "StdAfx.h"

#include <HoudiniCommon.h>
#include <Components/HoudiniNodeExporter.h>
#include <Game/HoudiniMeshComponent.h>
#include <HoudiniSplineTranslator.h>
#include <HoudiniMaterialTranslator.h>

#include <IIndexedMesh.h>

#include <AzQtComponents/Utilities/DesktopUtilities.h>
#include <AtomLyIntegration/CommonFeatures/Mesh/MeshComponentBus.h>
#include <AtomLyIntegration/CommonFeatures/Mesh/MeshComponentConstants.h>

namespace HoudiniEngine
{
    HoudiniNodeExporter::~HoudiniNodeExporter()
    {
        RemoveMeshData();
		AzFramework::AssetCatalogEventBus::Handler::BusDisconnect();
		AzToolsFramework::EditorComponentSelectionRequestsBus::Handler::BusDisconnect();
        AzFramework::BoundsRequestBus::Handler::BusDisconnect();
		m_renderMesh.reset();
    }

    AZ::Aabb HoudiniNodeExporter::GetEditorSelectionBoundsViewport([[maybe_unused]] const AzFramework::ViewportInfo& viewportInfo)
    {
        return GetWorldBounds();
    }

    bool HoudiniNodeExporter::EditorSelectionIntersectRayViewport([[maybe_unused]] const AzFramework::ViewportInfo& viewportInfo, const AZ::Vector3& src, const AZ::Vector3& dir, float& distance)
    {
		AZ_PROFILE_FUNCTION(AzToolsFramework);

		if (!m_renderMesh->GetModel())
		{
			return false;
		}

		AZ::Transform transform = AZ::Transform::CreateIdentity();
		AZ::TransformBus::EventResult(transform, m_entityId, &AZ::TransformBus::Events::GetWorldTM);

		AZ::Vector3 nonUniformScale = AZ::Vector3::CreateOne();
		AZ::NonUniformScaleRequestBus::EventResult(nonUniformScale, m_entityId, &AZ::NonUniformScaleRequests::GetScale);

		float t;
		AZ::Vector3 ignoreNormal;
		constexpr float rayLength = 1000.0f;
		if (m_renderMesh->GetModel()->RayIntersection(transform, nonUniformScale, src, dir * rayLength, t, ignoreNormal))
		{
			distance = rayLength * t;
			return true;
		}

		return false;
    }

    bool HoudiniNodeExporter::SupportsEditorRayIntersect()
    {
        return true;
    }

    AZ::Aabb HoudiniNodeExporter::GetWorldBounds()
    {
		if (!m_worldAabb.has_value())
		{
			m_worldAabb = GetLocalBounds();
			m_worldAabb->ApplyTransform(m_world);
		}

		return m_worldAabb.value();
    }

    AZ::Aabb HoudiniNodeExporter::GetLocalBounds()
    {
		if (!m_localAabb.has_value() && m_modelData.MeshCount() > 0)
		{
			m_localAabb = m_modelData.GetAabb();
		}

		return m_localAabb.value();
    }

    void HoudiniNodeExporter::OnSplineChanged()
    {
		if (!m_node->HasEditableGeometryInfo())
			return;

        HAPI_GeoInfo geometryInfo = m_node->GetEditableGeometryInfo();
        HoudiniSplineTranslator::HapiUpdateNodeForHoudiniSplineComponent(m_entityId, geometryInfo.nodeId, false);
    }

    void HoudiniNodeExporter::OnCatalogAssetAdded(const AZ::Data::AssetId& assetId)
    {
		AZ::Data::AssetInfo assetInfo;
		EBUS_EVENT_RESULT(assetInfo, AZ::Data::AssetCatalogRequestBus, GetAssetInfoById, assetId);

		// note that this will get called twice, once with the real assetId and once with legacy assetId.
		// we only want to add the real asset to the list, in which the assetId passed in is equal to the final assetId returned
		// otherwise, you look up assetId (and its a legacy assetId) and the actual asset will be different.
        if ((assetInfo.m_assetId.IsValid()) && (assetInfo.m_assetId == assetId))
        {
            AZStd::string assetName;
            AzFramework::StringFunc::Path::GetFileName(assetInfo.m_relativePath.c_str(), assetName);

            if (assetName == m_fbxFilename)
            {
				auto entity = AzToolsFramework::GetEntity(m_entityId);
				auto transformComponent = entity->FindComponent<AzToolsFramework::Components::TransformComponent>();
				AZ::EntityId parentId = transformComponent->GetParentId();
				auto worldTransform = transformComponent->GetWorldTM();

				AZ::EntityId entityId;
				EBUS_EVENT_RESULT(entityId, AzToolsFramework::EditorRequests::Bus, CreateNewEntity, parentId);

				AzToolsFramework::EntityIdList entityIdList = { entityId };

				AzToolsFramework::EntityCompositionRequests::AddComponentsOutcome addedComponentsResult =
					AZ::Failure(AZStd::string("Failed to call AddComponentsToEntities on EntityCompositionRequestBus"));
				AzToolsFramework::EntityCompositionRequestBus::BroadcastResult(
					addedComponentsResult,
					&AzToolsFramework::EntityCompositionRequests::AddComponentsToEntities,
					entityIdList,
					AZ::ComponentTypeList{ AZ::Render::EditorMeshComponentTypeId });

				if (addedComponentsResult.IsSuccess())
				{
					AZ::TransformBus::Event(entityId, &AZ::TransformBus::Events::SetWorldTM, worldTransform);

					AZ::Render::MeshComponentRequestBus::Event(
						entityId, &AZ::Render::MeshComponentRequestBus::Events::SetModelAssetPath, assetInfo.m_relativePath);

                    if (m_fbxConfig->m_removeHdaAfterBake)
                    {
						EBUS_EVENT(
							AzToolsFramework::ToolsApplicationRequests::Bus,
							DeleteEntitiesAndAllDescendants,
							AzToolsFramework::EntityIdList{ m_entityId });
                    }
					
				}
            }
            else
			{
				if (!m_materialWaitList.empty())
				{
					auto iter = AZStd::find(m_materialWaitList.begin(), m_materialWaitList.end(), assetInfo.m_relativePath);
					if (iter != m_materialWaitList.end())
					{
						m_materialWaitList.erase(iter);
						if (m_materialWaitList.empty())
						{
							RebuildRenderMesh();
						}
					}
				}
			}
        }
    }

    void HoudiniNodeExporter::OnCatalogAssetChanged(const AZ::Data::AssetId& assetId)
    {
        OnCatalogAssetAdded(assetId);
    }

    void HoudiniNodeExporter::UpdateWorldTransformData(const AZ::Transform& transform)
    {
		m_worldAabb.reset();
		m_localAabb.reset();

        m_world = transform;

		if (m_renderMesh)
		{
			m_renderMesh->UpdateTransform(transform);
		}
    }

    void HoudiniNodeExporter::OnTick()
    {
        AZStd::unique_lock<AZStd::mutex> theLock(m_lock);

        for (auto functionCall : m_tickFunctions)
        {
            functionCall();
        }

        m_tickFunctions.clear();

    }

    void HoudiniNodeExporter::CheckForErrors()
    {
        if (m_node != nullptr)
        {
            m_node->GetHou()->CheckForErrors();
        }
    }

    void HoudiniNodeExporter::OnChanged()
    {
        //Sync material assets to materials:
        /*m_materials.clear();
        for (auto& materialSetting : m_materialSettings)
        {
            m_materials.push_back(materialSetting.LoadMaterial());
        }*/

        bool applied = false;
        //Apply the materials to the renderers
        /*for (int i = 0; i < m_meshData.size(); i++)
        {
            auto& meshData = m_meshData[i];
            
            ApplyMaterialsToMesh(meshData);

            applied = true;
        }*/

        if (applied)
        {
            for (auto& materialSetting : m_materialSettings)
            {
                materialSetting.m_dirty = false;
                
                if (materialSetting.m_dirtyPhysics)
                {
                    materialSetting.m_dirtyPhysics = false;
                    m_dirty = true;
                }
            }
        }
    }


    void HoudiniNodeExporter::SetMaterialPath(const AZStd::string& materialName, const AZStd::string& materialPath)
    {
        QString qMaterialName = materialName.c_str();
        bool found = false;

        for (auto& materialSetting : m_materialSettings)
        {
            if (qMaterialName.toLower() == QString(materialSetting.m_materialName.c_str()).toLower())
            {
                //TODO
                //materialSetting.m_materialAsset.SetAssetPath(materialPath.c_str());
                materialSetting.m_dirty = true;
                found = true;
            }
        }

        //Might not be loaded yet, configure a default for later:
        if (!found)
        {
            m_materialDefaults[materialName] = materialPath;
        }
    }

    bool HoudiniNodeExporter::CheckHoudiniAccess()
    {
        //Safety first
        if (m_node == nullptr || m_node->HasCookingError())
            return false;

        m_hou = m_node->GetHou();
        if (m_hou == nullptr || m_hou->IsActive() == false)
            return false;

        m_session = (HAPI_Session*)&m_hou->GetSession();

        return true;
    }

    void HoudiniNodeExporter::RemoveMeshData()
    {
        /*for (auto& mesh : m_meshData)
        {
            mesh.DestroyRenderNode();
        }*/

        for (auto& setting : m_materialSettings)
        {
            //A new node will require new materials applied.
            setting.m_dirty = true;
        }
    }

    void HoudiniNodeExporter::SaveToFbx(bool useClusters)
    {
        AZ_PROFILE_FUNCTION(Editor);
        if (!CheckHoudiniAccess())
            return;
        
        HAPI_GeoInfo geometryInfo = m_node->GetGeometryInfo();

        HAPI_NodeId newSceneRoot;
        const AZStd::string& objOutputName = "___FBX___" + m_node->GetNodeName();
        const AZStd::string& objOutputMergeName = "/obj/FBXs/" + objOutputName;

        //RemoveRootNode:
        HAPI_NodeId fbxRootId = m_hou->FindNode(HAPI_NODETYPE_OBJ, "/obj/FBXs");
        if (fbxRootId <= HOUDINI_ZERO_ID)
        {
            HAPI_CreateNode(m_session, -1, "Object/subnet", "FBXs", true, &fbxRootId);
        }
        
        //Remove any dups of this node:
        HAPI_NodeId sceneRootId = m_hou->FindNode(HAPI_NODETYPE_OBJ, objOutputMergeName);
        if (sceneRootId > HOUDINI_ZERO_ID)
        {
            HAPI_DeleteNode(m_session, sceneRootId);
        }

        /////////////////////////////
        // Transform the geo START //
        /////////////////////////////
        HAPI_GeoInfo geoInfo;
        HAPI_NodeInfo newSceneRootInfo;
        HAPI_CreateNode(m_session, fbxRootId, "geo", objOutputName.c_str(), true, &newSceneRoot);
        CheckForErrors();
            
        HAPI_GetDisplayGeoInfo(m_session, newSceneRoot, &geoInfo);
        CheckForErrors();

        HAPI_GetNodeInfo(m_session, newSceneRoot, &newSceneRootInfo);
        CheckForErrors();

        if (geoInfo.isDisplayGeo)
        {
            //Remove the "FILE" import object that is auto-added:
            HAPI_DeleteNode(m_session, geoInfo.nodeId);
        }

        HAPI_NodeId objTrans;
        HAPI_NodeInfo objTransInfo;
        HAPI_CreateNode(m_session, newSceneRoot, "xform", "transform", true, &objTrans);
        CheckForErrors();

        HAPI_GetNodeInfo(m_session, objTrans, &objTransInfo);

        HAPI_NodeId objMerge;
        HAPI_NodeInfo objMergeInfo;
        HAPI_CreateNode(m_session, newSceneRoot, "object_merge", "export_merged", true, &objMerge);
        CheckForErrors();

        HAPI_GetNodeInfo(m_session, objMerge, &objMergeInfo);

        const AZStd::string& objName = m_node->GetNodePath();

        HAPI_ParmId objPathParm;
        HAPI_GetParmIdFromName(m_session, objMerge, "objpath1", &objPathParm);
        CheckForErrors();

        HAPI_SetParmStringValue(m_session, objMerge, objName.c_str(), objPathParm, 0);
        CheckForErrors();

        /*HAPI_SetParmFloatValue(m_session, objTrans, "rx", 0, -90);
        CheckForErrors();

        HAPI_SetParmFloatValue(m_session, objTrans, "ry", 1, 180);
        CheckForErrors();
        */
		HAPI_SetParmFloatValue(m_session, objTrans, "rx", 0, -180);
		CheckForErrors();

		HAPI_SetParmFloatValue(m_session, objTrans, "rz", 2, -180);
		CheckForErrors();

        HAPI_SetParmFloatValue(m_session, objTrans, "sx", 0, 100);
        CheckForErrors();

        HAPI_SetParmFloatValue(m_session, objTrans, "sy", 1, 100);
        CheckForErrors();

        HAPI_SetParmFloatValue(m_session, objTrans, "sz", 2, 100);
        CheckForErrors();

        // Apply transform on base object
        HAPI_ConnectNodeInput(m_session, objTrans, 0, objMerge, 0);
        CheckForErrors();

        m_hou->CookNode(objTrans, objOutputName);
        CheckForErrors();
        ///////////////////////////
        // Transform the geo END //
        ///////////////////////////

        // Get cluster count
        int clusterCount = 1;   // default to 1 cluster
        HAPI_AttributeInfo clusters_Info = HAPI_AttributeInfo_Create();
        HAPI_GetAttributeInfo(m_session, geometryInfo.nodeId, 0, "TotalParts", HAPI_ATTROWNER_DETAIL, &clusters_Info);
        if (clusters_Info.exists)
        {
            HAPI_GetAttributeIntData(m_session, geometryInfo.nodeId, 0, "TotalParts", &clusters_Info, 1, &clusterCount, 0, 1);
        }

        // Get group reference
        HAPI_ParmId objGroupParm;
        HAPI_GetParmIdFromName(m_session, objMerge, "group1", &objGroupParm);
        CheckForErrors();                

        // Setup ExportToFBX
        HAPI_NodeId ropNode = m_hou->FindNode(HAPI_NODETYPE_ROP, "/out/ExportToFBX");
        if (ropNode <= HOUDINI_ZERO_ID)
        {
            HAPI_CreateNode(m_session, HOUDINI_ROOT_NODE_ID, "Driver/filmboxfbx", "ExportToFBX", true, &ropNode);
            CheckForErrors();
        }

        HAPI_ParmId startNodeParamId;
        HAPI_GetParmIdFromName(m_session, ropNode, "startnode", &startNodeParamId);
        HAPI_SetParmStringValue(m_session, ropNode, objOutputMergeName.c_str(), startNodeParamId, 0);
        CheckForErrors();

        HAPI_ParmId outputParamId;
        HAPI_GetParmIdFromName(m_session, ropNode, "sopoutput", &outputParamId);
        CheckForErrors();

        if (useClusters == false) 
        {
            clusterCount = 1;
            clusters_Info.exists = false;
        }

        // Export each cluster
        const AZStd::string& baseFileName = m_hou->GetString(m_node->GetNodeInfo().nameSH);
        for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++)
        {
            const auto& clusterIndexString = AZStd::to_string(clusterIndex);
            AZStd::string clusterName = "PART";

            // Set group reference if we have clusters
            if (clusters_Info.exists)
            {                
                const AZStd::string& groupName = "cluster" + clusterIndexString;
                HAPI_SetParmStringValue(m_session, objMerge, groupName.c_str(), objGroupParm, 0);
                CheckForErrors();

                HAPI_AttributeInfo clusters_Name = HAPI_AttributeInfo_Create();
                HAPI_GetAttributeInfo(m_session, objMerge, 0, "clusterName", HAPI_ATTROWNER_PRIM, &clusters_Name);
                if (clusters_Name.exists)
                {
                    HAPI_StringHandle handle;
                    HAPI_GetAttributeStringData(m_session, objMerge, 0, "clusterName", &clusters_Name, &handle, 0, 1);
                    clusterName = m_hou->GetString(handle);
                }
            }

            AZStd::string suffix = "_" + clusterName + "_" + clusterIndexString;

            m_fbxFilename = baseFileName + suffix;
            AZStd::to_lower(m_fbxFilename.begin(), m_fbxFilename.end());

            AZStd::string savePath;
            AZStd::string outPackageName;
            AZ::u32 bakeCounter = m_fbxConfig->m_bakeCounter;
            while (true)
            {
                outPackageName = m_fbxFilename;

                if (bakeCounter > 0)
                {
                    outPackageName += "_" + AZStd::string::format("%i", bakeCounter);
                }

                savePath = HoudiniEngineUtils::GetOutputExportFolder() + outPackageName + ".fbx";

                if (!m_fbxConfig->m_replacePreviousBake)
                {
                    if (AZ::IO::SystemFile::Exists(savePath.c_str()))
                    {
                        bakeCounter++;
                        continue;
                    }
                }

                break;
            }

            m_fbxFilename = outPackageName;
            m_fbxConfig->m_bakeCounter = bakeCounter;

            // Export cluster to FBX
            HAPI_SetParmStringValue(m_session, ropNode, savePath.c_str(), outputParamId, 0);
            CheckForErrors();
            HAPI_SetParmIntValue(m_session, ropNode, "execute", 0, 1);
            CheckForErrors();
            
            int cookStatus;
            HAPI_Result cookResult;
            do
            {
                cookResult = HAPI_GetStatus(m_session, HAPI_STATUS_COOK_STATE, &cookStatus);
            } while (cookStatus > HAPI_STATE_MAX_READY_STATE && cookResult == HAPI_RESULT_SUCCESS);
            
            CheckForErrors();

            if (clusterIndex == 0)
            {
                AzQtComponents::ShowFileOnDesktop(savePath.c_str());
            }
        }
    }

    void HoudiniNodeExporter::SetVisibleInEditor([[maybe_unused]] bool visibility)
    {
        /*for (auto& meshData : m_meshData)
        {
            meshData.SetVisibilityInEditor(visibility);
        }*/
    }

    AZStd::vector<AZStd::string> HoudiniNodeExporter::GetClusters()
    {
        AZ_PROFILE_FUNCTION(Editor);

        AZStd::vector<AZStd::string> output;
        if (!CheckHoudiniAccess())
            return output;

        int groupCount = m_node->GetGeometryInfo().primitiveGroupCount;

        if (groupCount == 0) 
        {
            return output;
        }

        AZStd::vector<HAPI_StringHandle> handles(groupCount);
        HAPI_GetGroupNames(m_session, m_node->GetGeometryInfo().nodeId, HAPI_GROUPTYPE_PRIM, &handles.front(), groupCount);
        *m_hou << "HAPI_GetGroupNames: " << m_node->GetGeometryInfo().nodeId << " reading " << groupCount << " into buffer sized: " << handles.size() << "";
                
        for (auto& groupNameHandle : handles)
        {
            auto groupName = m_hou->GetString(groupNameHandle);

            if (groupName.starts_with("cluster"))
            {
                output.push_back(groupName);
            }
        }

        return output;
    }

    //"/obj/FBXs"
    HAPI_NodeId HoudiniNodeExporter::CreateObjectMerge(const AZStd::string& rootName, const AZStd::string& clusterName)
    {
        AZ_PROFILE_FUNCTION(Editor);

        if (!CheckHoudiniAccess())
        {
            return HOUDINI_INVALID_ID;
        }

        HAPI_NodeId newSceneRoot;

        //RemoveRootNode:
        HAPI_NodeId fbxRootId = m_hou->FindNode(HAPI_NODETYPE_OBJ, ("/obj/" + rootName).c_str());
        if (fbxRootId <= HOUDINI_ZERO_ID)
        {
            HAPI_CreateNode(m_session, -1, "Object/subnet", rootName.c_str(), true, &fbxRootId);
        }

        //Remove any dups of this node:
        HAPI_NodeId sceneRootId = m_hou->FindNode(HAPI_NODETYPE_OBJ, clusterName);
        if (sceneRootId > HOUDINI_ZERO_ID)
        {
            HAPI_DeleteNode(m_session, sceneRootId);
            *m_hou << "HAPI_DeleteNode: " << sceneRootId << "";
        }

        /////////////////////////////
        // Transform the geo START //
        /////////////////////////////
        HAPI_GeoInfo geoInfo = HAPI_GeoInfo_Create();
        HAPI_NodeInfo newSceneRootInfo = HAPI_NodeInfo_Create();
        {
            AZ_PROFILE_SCOPE(Editor, ("Create Geo and remove File: " + clusterName).c_str());
            HAPI_CreateNode(m_session, fbxRootId, "geo", clusterName.c_str(), true, &newSceneRoot);
            HAPI_GetDisplayGeoInfo(m_session, newSceneRoot, &geoInfo);
            HAPI_GetNodeInfo(m_session, newSceneRoot, &newSceneRootInfo);                        
            if (geoInfo.isDisplayGeo)
            {
                //Remove the "FILE" import object that is auto-added:
                HAPI_DeleteNode(m_session, geoInfo.nodeId);
            }
        }

        *m_hou << "HAPI_CreateNode: " << fbxRootId << " " << clusterName << "";
        *m_hou << "HAPI_GetDisplayGeoInfo: " << newSceneRoot << " " << clusterName << "";
        
        if (geoInfo.isDisplayGeo)
        {
            *m_hou << "HAPI_DeleteNode (FILE/GEO): " << geoInfo.nodeId << " " << clusterName << "";
        }
        
        HAPI_NodeId objMerge;
        HAPI_NodeInfo objMergeInfo;
        {
            AZ_PROFILE_SCOPE(Editor, ("Create object_merge: " + clusterName).c_str());
            HAPI_CreateNode(m_session, newSceneRoot, "object_merge", (clusterName + "_export_merged").c_str(), true, &objMerge);
            *m_hou << "HAPI_CreateNode: " << objMerge << "object_merge : export_merged" << "";
        }

        HAPI_GetNodeInfo(m_session, objMerge, &objMergeInfo);
        HAPI_SetParmNodeValue(m_session, objMerge, "objpath1", m_node->GetNodeInfo().id);
        *m_hou << "HAPI_SetParmNodeValue: " << objMerge << " => objpath1 => " << m_node->GetNodeInfo().id << "";

        CheckForErrors();

        if (!clusterName.empty())
        {
            AZ_PROFILE_SCOPE(Editor, ("SetParam Cluster: " + clusterName).c_str());
            // Get group reference
            HAPI_ParmId objGroupParm;
            HAPI_GetParmIdFromName(m_session, objMerge, "group1", &objGroupParm);
            CheckForErrors();

            //Set the cluster group ID
            HAPI_SetParmStringValue(m_session, objMerge, clusterName.c_str(), objGroupParm, 0);
            *m_hou << "HAPI_SetParmStringValue: " << objMerge << " " << clusterName << " grp:" << objGroupParm << "";            

            CheckForErrors();
        }

        {
            AZ_PROFILE_SCOPE(Editor, ("Cooking Cluster: " + clusterName).c_str());
            m_hou->CookNode(objMerge, rootName + "_" + clusterName);
        }

        return objMerge;
    }
    

    void HoudiniNodeExporter::ReadAttributeHints(HAPI_NodeId nodeId, HAPI_PartInfo& partInfo)
    {
        AZ_PROFILE_FUNCTION(Editor);

        if (partInfo.attributeCounts[HAPI_ATTROWNER_DETAIL] > 0)
        {
            AZStd::vector<HAPI_StringHandle> attributeList(partInfo.attributeCounts[HAPI_ATTROWNER_DETAIL]);
            HAPI_GetAttributeNames(m_session, nodeId, partInfo.id, HAPI_ATTROWNER_DETAIL, &attributeList.front(), partInfo.attributeCounts[HAPI_ATTROWNER_DETAIL]);


            *m_hou << "---ReadAttributeHints--- " << "";
            *m_hou << "   HAPI_GetAttributeNames: " << nodeId << "";

            for (auto& attribHandle : attributeList)
            {
                AZStd::string newAttrib = m_hou->GetString(attribHandle);

                if (AZStd::find(m_attributeHintNames.begin(), m_attributeHintNames.end(), newAttrib) == m_attributeHintNames.end())
                {
                    m_attributeHintNames.push_back("");
                }
            }
        }
    }

    void HoudiniNodeExporter::ReadMaterialNameHints(HAPI_NodeId nodeId, HAPI_PartInfo& partInfo)
    {
        AZ_PROFILE_FUNCTION(Editor);
        *m_hou << "   ReadMaterialNameHints: " << nodeId << "";

        HAPI_AttributeInfo materialsCount = HAPI_AttributeInfo_Create();
        HAPI_GetAttributeInfo(m_session, nodeId, partInfo.id, "Materials", HAPI_ATTROWNER_DETAIL, &materialsCount);
        *m_hou << "   HAPI_GetAttributeInfo: " << " Materials " << nodeId << "";;

        if (materialsCount.exists)
        {
            HAPI_StringHandle materialListHandle;
            HAPI_GetAttributeStringData(m_session, nodeId, partInfo.id, "Materials", &materialsCount, &materialListHandle, 0, 1);
            AZStd::string materialList = m_hou->GetString(materialListHandle);
            QString qMaterialList = materialList.c_str();
            QStringList qMaterialNames = qMaterialList.split(",", Qt::SkipEmptyParts);

            AZStd::vector<AZStd::string> qMaterialsToCreate;
            
            for (QString& qMat : qMaterialNames)
            {
                qMat = qMat.trimmed();
                if (qMat.size() > 0)
                {
                    AZStd::string newMaterialName = qMat.toUtf8().data();
                    if (AZStd::find(m_materialHintNames.begin(), m_materialHintNames.end(), newMaterialName) == m_materialHintNames.end())
                    {
                        m_materialHintNames.push_back(newMaterialName);
                        qMaterialsToCreate.push_back(newMaterialName);
                    }
                }
            }
    
            //When creating a new object, there is a chance that by having just set the entity and that triggering a creation of new materials, this will cause
            //LY Entity Inspector to crash.  Therefore, we push the creation of the materials forward one frame when its safe to do so.
            {
                AZStd::unique_lock<AZStd::mutex> theLock(m_lock);
                m_tickFunctions.push_back([this, qMaterialsToCreate]() mutable
                {
                    bool changed = false;

                    for (int i = 0; i < qMaterialsToCreate.size(); i++)
                    {
                        if (i >= m_materialSettings.size())
                        {
                            m_materialSettings.push_back(HoudiniMaterialSettings());
                            changed = true;
                        }

                        if (m_materialSettings[i].m_materialName != qMaterialsToCreate[i])
                        {
                            m_materialSettings[i].m_materialName = qMaterialsToCreate[i];
                            changed = true;
                        }
                    }

                    //Check for any defaults there might be:
                    for (auto it = m_materialDefaults.begin(); it != m_materialDefaults.end(); it++)
                    {
                        auto materialName = it->first;
                        auto materialPath = it->second;
                        this->SetMaterialPath(materialName, materialPath);
                    }

                    for (int i = 0; i < m_materialSettings.size(); i++)
                    {
                        if (m_materialSettings[i].m_dirty)
                        {
                            changed = true;
                        }
                    }

                    m_materialDefaults.clear();

                    if (changed)
                    {
                        if (m_hou->LookupIsSelected(m_entityId))
                        {
                            AzToolsFramework::ToolsApplicationEvents::Bus::Broadcast(
                                &AzToolsFramework::ToolsApplicationEvents::Bus::Events::InvalidatePropertyDisplay, AzToolsFramework::PropertyModificationRefreshLevel::Refresh_EntireTree);
                        }
                    }
                });
            }
        }
    }

    void HoudiniNodeExporter::ReadMaterials(HAPI_NodeId nodeId, HAPI_PartInfo& partInfo, HoudiniMeshData& meshData)
    {
        AZ_PROFILE_FUNCTION(Editor);

        bool allSame = false;
        AZStd::vector<HAPI_NodeId> materialIds(partInfo.faceCount);
        HAPI_GetMaterialNodeIdsOnFaces(m_session, nodeId, partInfo.id, &allSame, &materialIds.front(), 0, partInfo.faceCount);
        *m_hou << "   HAPI_GetMaterialNodeIdsOnFaces: " << nodeId << " Face Count: " << partInfo.faceCount << " into buffer sized: " << materialIds.size() << "";

        //Get the material applied by name or ID
        bool foundMaterial = false;

        HAPI_AttributeInfo shopMaterialPath = HAPI_AttributeInfo_Create();
        HAPI_GetAttributeInfo(m_session, nodeId, partInfo.id, "shop_materialpath", HAPI_ATTROWNER_PRIM, &shopMaterialPath);
        if (shopMaterialPath.exists && shopMaterialPath.count > 0)
        {
            if (shopMaterialPath.storage == HAPI_STORAGETYPE_INT)
            {
                AZStd::vector<int> materialPathIdx(shopMaterialPath.count);
                HAPI_GetAttributeIntData(m_session, nodeId, partInfo.id, "shop_materialpath", &shopMaterialPath, 1, &materialPathIdx.front(), 0, shopMaterialPath.count);
                *m_hou << "    HAPI_GetAttributeIntData:" << nodeId << " " << partInfo.id << " shop_materialpath Reading: " << shopMaterialPath.count << " into buffer sized: " << materialPathIdx.size() << "";

                int index = materialPathIdx[0];
                if (index >= 0) 
                {
                    meshData.m_materialIndex = index;
                    foundMaterial = true;
                }
            }
            else if (shopMaterialPath.storage == HAPI_STORAGETYPE_STRING)
            {
                AZStd::vector<HAPI_StringHandle> materialPathHandles(shopMaterialPath.count);
                HAPI_GetAttributeStringData(m_session, nodeId, partInfo.id, "shop_materialpath", &shopMaterialPath, &materialPathHandles.front(), 0, shopMaterialPath.count);
                *m_hou << "    HAPI_GetAttributeStringData:" << nodeId << " " << partInfo.id << " shop_materialpath Reading: " << shopMaterialPath.count << " into buffer sized: " << materialPathHandles.size() << "";

                AZStd::string materialNameToSet = m_hou->GetString(materialPathHandles[0]);
                
                for (int i = 0; i < m_materialHintNames.size(); i++)
                {
                    const AZStd::string& matName = m_materialHintNames[i];
                    if (matName == materialNameToSet)
                    {
                        meshData.m_materialIndex = i;
                        foundMaterial = true;
                    }
                }
            }
        }

        if (materialIds[0] != HOUDINI_INVALID_ID)
        {
            HAPI_MaterialInfo material_info;
            HAPI_GetMaterialInfo(m_session, materialIds[0], &material_info);
            *m_hou << "   HAPI_GetMaterialInfo: " << materialIds[0] << " Mat: " << material_info.exists << " " << material_info.nodeId << "";

            //Auto Assign materials if not found:
            if (foundMaterial == false)
            {
                //See if this material is already referenced, use that ID.
                bool found = false;
                for (int i = 0; i < m_matIndexLookup.size(); i++)
                {
                    if (m_matIndexLookup[i] == material_info.nodeId)
                    {
                        meshData.m_materialIndex = i;
                        found = true;
                    }
                }

                if (!found)
                {
                    //Not found, lets make a new one and assign it to that.
                    m_matIndexLookup.push_back(material_info.nodeId);
                    meshData.m_materialIndex = m_matIndexLookup.size() - 1;
                }
            }
        }
    }

    void HoudiniNodeExporter::ApplyMaterialsToMesh([[maybe_unused]] HoudiniMeshData& meshData)
    {
        AZ_PROFILE_FUNCTION(Editor);

        //Update Material:
        /*if (meshData.m_materialIndex < m_materials.size() && meshData.m_materialIndex < m_materialSettings.size())
        {
            meshData.SetPhysics(m_materialSettings[meshData.m_materialIndex].m_physics);
            meshData.SetVisible(m_materialSettings[meshData.m_materialIndex].m_visible);
            meshData.SetMaterial(m_materials[meshData.m_materialIndex]);
            meshData.SetMaxViewDistance(m_materialSettings[meshData.m_materialIndex].GetMaxViewDistance());
            meshData.SetHighPriorityTextureStreaming(m_materialSettings[meshData.m_materialIndex].m_highPriorityTextureStreaming);
        }
        else
        {
            meshData.SetPhysics(false);
            meshData.SetVisible(false);
            meshData.SetMaterial(nullptr);
        }*/
    }

    int HoudiniNodeExporter::AddGeometry(int index, HAPI_NodeId nodeId, const AZStd::string& clusterId)
    {
        AZ_PROFILE_FUNCTION(Editor);

        int count = 0;

        if (!CheckHoudiniAccess())
        {
            return count;
        }

        HAPI_NodeInfo nodeInfo;
        HAPI_GeoInfo geomInfo = HAPI_GeoInfo_Create();
        
        HAPI_GetNodeInfo(m_session, nodeId, &nodeInfo);
        HAPI_GetGeoInfo(m_session, nodeId, &geomInfo);

        AZStd::vector<HAPI_PartInfo> parts;
        for (int i = 0; i < geomInfo.partCount; i++) 
        {
            HAPI_PartInfo partInfo;
            HAPI_GetPartInfo(m_session, nodeId, i, &partInfo);
            parts.push_back(partInfo);
        }

        for (auto partInfo : parts)
        {
            AZ_PROFILE_SCOPE(Editor, "GettingGeometryPart");
            //Setup attribute buffers
            HAPI_AttributeInfo p_info = HAPI_AttributeInfo_Create();
            HAPI_AttributeInfo n_info = HAPI_AttributeInfo_Create();
            HAPI_AttributeInfo cd_info = HAPI_AttributeInfo_Create();
            HAPI_AttributeInfo a_info = HAPI_AttributeInfo_Create();
            HAPI_AttributeInfo uv_info = HAPI_AttributeInfo_Create();
            HAPI_AttributeInfo tangent_info = HAPI_AttributeInfo_Create();
            HAPI_AttributeInfo bitangent_info = HAPI_AttributeInfo_Create();

            HAPI_AttributeInfo n_vinfo = HAPI_AttributeInfo_Create();
            HAPI_AttributeInfo cd_vinfo = HAPI_AttributeInfo_Create();
            HAPI_AttributeInfo a_vinfo = HAPI_AttributeInfo_Create();
            HAPI_AttributeInfo uv_vinfo = HAPI_AttributeInfo_Create();
            HAPI_AttributeInfo tangent_vinfo = HAPI_AttributeInfo_Create();
            HAPI_AttributeInfo bitangent_vinfo = HAPI_AttributeInfo_Create();

            AZStd::vector<HAPI_AttributeInfo> attrib_infos = { p_info, n_info, cd_info, uv_info, tangent_info, bitangent_info };

            HAPI_GetAttributeInfo(m_session, nodeId, partInfo.id, "P", HAPI_ATTROWNER_POINT, &p_info);
            HAPI_GetAttributeInfo(m_session, nodeId, partInfo.id, "N", HAPI_ATTROWNER_POINT, &n_info);
            HAPI_GetAttributeInfo(m_session, nodeId, partInfo.id, "Cd", HAPI_ATTROWNER_POINT, &cd_info);
            HAPI_GetAttributeInfo(m_session, nodeId, partInfo.id, "Alpha", HAPI_ATTROWNER_POINT, &a_info);
            HAPI_GetAttributeInfo(m_session, nodeId, partInfo.id, "uv", HAPI_ATTROWNER_POINT, &uv_info);
            HAPI_GetAttributeInfo(m_session, nodeId, partInfo.id, "tangent", HAPI_ATTROWNER_POINT, &tangent_info);
            HAPI_GetAttributeInfo(m_session, nodeId, partInfo.id, "bitangent", HAPI_ATTROWNER_POINT, &bitangent_info);

            HAPI_GetAttributeInfo(m_session, nodeId, partInfo.id, "N", HAPI_ATTROWNER_VERTEX, &n_vinfo);
            HAPI_GetAttributeInfo(m_session, nodeId, partInfo.id, "Cd", HAPI_ATTROWNER_VERTEX, &cd_vinfo);
            HAPI_GetAttributeInfo(m_session, nodeId, partInfo.id, "Alpha", HAPI_ATTROWNER_VERTEX, &a_vinfo);
            HAPI_GetAttributeInfo(m_session, nodeId, partInfo.id, "uv", HAPI_ATTROWNER_VERTEX, &uv_vinfo);                                    
            HAPI_GetAttributeInfo(m_session, nodeId, partInfo.id, "tangent", HAPI_ATTROWNER_VERTEX, &tangent_vinfo);
            HAPI_GetAttributeInfo(m_session, nodeId, partInfo.id, "bitangent", HAPI_ATTROWNER_VERTEX, &bitangent_vinfo);

            if (p_info.exists == false || partInfo.vertexCount == 0)
            {
                //No geometry! Skip
                continue;
            }

            //Face counting
            AZStd::vector<int> polyCount(partInfo.faceCount);
            if (partInfo.faceCount > 0)
            {
                HAPI_GetFaceCounts(m_session, nodeId, partInfo.id, &polyCount.front(), 0, partInfo.faceCount);
            }
            else
            {
                continue;
            }

            AZStd::unique_lock<AZStd::mutex> theLock(HoudiniMeshData::m_dataLock);

            while (index + count >= m_modelData.m_meshes.size())
            {
                //Put more until we have enough.
                m_modelData.m_meshes.push_back(HoudiniMeshData());
            }

            HoudiniMeshData& meshData = m_modelData.m_meshes[index + count];
            
            AZStd::string objectName = m_hou->GetString(m_node->GetNodeInfo().nameSH);
            AZStd::string geomName = objectName + "/" + m_hou->GetString(nodeInfo.nameSH) + "_" + clusterId;
            meshData.m_meshName = geomName;
            //staticObject->SetGeoName(geomName.c_str());
            AZ_PROFILE_SCOPE(Editor, geomName.c_str());

            ///Useful for debugging and verifying that the tangent generation is good.
            static bool forceComputeTangents = false;
            if (forceComputeTangents)
            {
                //Ignore tangents in the file.
                tangent_info.exists = false;
                tangent_info.tupleSize = 0;
                bitangent_info.exists = false;
                bitangent_info.tupleSize = 0;
                
                tangent_vinfo.exists = false;
                tangent_vinfo.tupleSize = 0;
                bitangent_vinfo.exists = false;
                bitangent_vinfo.tupleSize = 0;
            }

            int nSize = AZStd::max(n_info.tupleSize, n_vinfo.tupleSize);
            int cdSize = AZStd::max(cd_info.tupleSize, cd_vinfo.tupleSize);
            int alphaSize = AZStd::max(a_info.tupleSize, a_vinfo.tupleSize);
            int uvSize = AZStd::max(uv_info.tupleSize, uv_vinfo.tupleSize);
            int tanSize = AZStd::max(tangent_info.tupleSize, tangent_vinfo.tupleSize);
            int bitanSize = AZStd::max(bitangent_info.tupleSize, bitangent_vinfo.tupleSize);
            
            //Output Buffers:
            {
                AZ_PROFILE_SCOPE(Editor, "AllocateOutputBuffers");
                meshData.m_indices.resize(partInfo.vertexCount);
                meshData.m_positions.resize(partInfo.vertexCount);
                meshData.m_normals.resize(partInfo.vertexCount);
                /*data.m_colors.resize(cdSize == 0 ? 0 : partInfo.vertexCount);
                data.m_uvs.resize(uvSize == 0 ? 0 : partInfo.vertexCount);
                data.m_tangents.resize(tanSize == 0 ? 0 : partInfo.vertexCount);
                data.m_bitangents.resize(bitanSize == 0 ? 0 : partInfo.vertexCount);*/
				meshData.m_colors.resize(partInfo.vertexCount);
				meshData.m_uvs.resize(partInfo.vertexCount);
				meshData.m_tangents.resize(partInfo.vertexCount);
				meshData.m_bitangents.resize(partInfo.vertexCount);
            }

            //AZ_PROFILE_EVENT_BEGIN(EDITOR, "AllocateTempCopyBuffers");
            AZStd::vector<int> idx(partInfo.vertexCount);
            AZStd::vector<float> p(partInfo.vertexCount * p_info.tupleSize);
            AZStd::vector<float> n(partInfo.vertexCount *  nSize);
            AZStd::vector<float> cd(partInfo.vertexCount * cdSize);
            AZStd::vector<float> alpha(partInfo.vertexCount * alphaSize);
            AZStd::vector<float> uv(partInfo.vertexCount * uvSize);
            AZStd::vector<float> tangent(partInfo.vertexCount * tanSize);
            AZStd::vector<float> bitangent(partInfo.vertexCount * bitanSize);
            //AZ_PROFILE_EVENT_END(EDITOR);

            auto logger = [=](auto attribName, const HAPI_AttributeInfo& attribInfo, const AZStd::vector<float>& buffer)
            {
                *m_hou << "   HAPI_GetAttributeFloatData: " << clusterId << " " << nodeId << " " << partInfo.id << " " << attribName << " Reading:" << attribInfo.count * attribInfo.tupleSize << " into buffer size: " << buffer.size() << "";
            };

            bool needsUnwind = false;

            //Read all the data: N, CD, and UV can come in through the vertex direct stream..
            logger("P", p_info, p);
            {
                AZ_PROFILE_SCOPE(Editor, "ReadPointDataFromHOU");
                HAPI_GetAttributeFloatData(m_session, nodeId, partInfo.id, "P", &p_info, 3, &p.front(), 0, p_info.count);
            }

            if (n.empty() == false)
            {
                AZ_PROFILE_SCOPE(Editor, "ReadNormalDataFromHOU");
                if (n_info.exists)
                {
                    logger("N", n_info, n);
                    HAPI_GetAttributeFloatData(m_session, nodeId, partInfo.id, "N", &n_info, 3, &n.front(), 0, n_info.count);                    
                }
                if (n_vinfo.exists)
                {
                    logger("N (vertex)", n_vinfo, n);
                    HAPI_GetAttributeFloatData(m_session, nodeId, partInfo.id, "N", &n_vinfo, 3, &n.front(), 0, n_vinfo.count);
                    needsUnwind = true;
                }
            }

            if (cd.empty() == false)
            {
                AZ_PROFILE_SCOPE(Editor, "ReadColorDataFromHOU");
                if (cd_info.exists)
                {
                    logger("CD", cd_info, cd);
                    HAPI_GetAttributeFloatData(m_session, nodeId, partInfo.id, "Cd", &cd_info, cd_info.tupleSize, &cd.front(), 0, cd_info.count);
                }
                if (cd_vinfo.exists)
                {
                    logger("CD (vertex)", cd_vinfo, cd);
                    HAPI_GetAttributeFloatData(m_session, nodeId, partInfo.id, "Cd", &cd_vinfo, cd_vinfo.tupleSize, &cd.front(), 0, cd_vinfo.count);
                    needsUnwind = true;
                }
            }

            if (alpha.empty() == false)
            {
                AZ_PROFILE_SCOPE(Editor, "ReadColorAlphaDataFromHOU");
                if (a_info.exists)
                {
                    logger("Alpha", a_info, alpha);
                    HAPI_GetAttributeFloatData(m_session, nodeId, partInfo.id, "Alpha", &a_info, a_info.tupleSize, &alpha.front(), 0, a_info.count);
                }
                if (a_vinfo.exists)
                {
                    logger("Alpha (vertex)", cd_vinfo, cd);
                    HAPI_GetAttributeFloatData(m_session, nodeId, partInfo.id, "Alpha", &a_vinfo, a_vinfo.tupleSize, &alpha.front(), 0, a_vinfo.count);
                    needsUnwind = true;
                }
            }
            
            if (uv.empty() == false)
            {
                AZ_PROFILE_SCOPE(Editor, "ReadUVDataFromHOU");
                if (uv_info.exists)
                {
                    logger("UV", uv_info, uv);
                    HAPI_GetAttributeFloatData(m_session, nodeId, partInfo.id, "uv", &uv_info, uv_info.tupleSize, &uv.front(), 0, uv_info.count);
                }
                
                if (uv_vinfo.exists)
                {
                    logger("UV (vertex)", uv_vinfo, uv);
                    HAPI_GetAttributeFloatData(m_session, nodeId, partInfo.id, "uv", &uv_vinfo, uv_vinfo.tupleSize, &uv.front(), 0, uv_vinfo.count);
                    needsUnwind = true;
                }
            }
            
            if (tangent.empty() == false && bitangent.empty() == false)
            {
                AZ_PROFILE_SCOPE(Editor, "ReadTangentDataFromHOU");
                if (tangent_info.exists)
                {
                    logger("tangents", tangent_info, tangent);
                    logger("bitangent", bitangent_info, bitangent);
                    HAPI_GetAttributeFloatData(m_session, nodeId, partInfo.id, "tangent", &tangent_info, tangent_info.tupleSize, &tangent.front(), 0, tangent_info.count);
                    HAPI_GetAttributeFloatData(m_session, nodeId, partInfo.id, "bitangent", &bitangent_info, bitangent_info.tupleSize, &bitangent.front(), 0, bitangent_info.count);
                }

                if (tangent_vinfo.exists)
                {
                    logger("tangents (vertex)", tangent_vinfo, tangent);
                    logger("bitangent", bitangent_vinfo, bitangent);
                    HAPI_GetAttributeFloatData(m_session, nodeId, partInfo.id, "tangent", &tangent_vinfo, tangent_vinfo.tupleSize, &tangent.front(), 0, tangent_vinfo.count);
                    HAPI_GetAttributeFloatData(m_session, nodeId, partInfo.id, "bitangent", &bitangent_vinfo, bitangent_vinfo.tupleSize, &bitangent.front(), 0, bitangent_vinfo.count);
                    needsUnwind = true;
                }
            }

            //Faces
            if (idx.empty() == false)
            {
                AZ_PROFILE_SCOPE(Editor, "ReadFaceVertexDataFromHOU");
                *m_hou << "   HAPI_GetVertexList: " << nodeId << " " << partInfo.id << " idx Reading:" << partInfo.vertexCount << " into buffer size: " << idx.size() << "";
                HAPI_GetVertexList(m_session, nodeId, partInfo.id, &idx.front(), 0, partInfo.vertexCount);
            }

            //Apply material data:
            {
                AZ_PROFILE_SCOPE(Editor, "ApplyMaterialData");
                // NOTE: this is part of HoudiniMeshTranslator. For now we treat the node exporter as one since we do the geometries/mesh here.
                CreateNeededMaterials(objectName, nodeId, partInfo, meshData);

                //HACK:  
                //meshData.m_materialIndex = 1;
				/*ReadAttributeHints(nodeId, partInfo);
				ReadMaterialNameHints(nodeId, partInfo);
				ReadMaterials(nodeId, partInfo, data);
				ApplyMaterialsToMesh(data);*/
            }
            
            if (needsUnwind)
            {
                AZ_PROFILE_SCOPE(Editor, "UnwindVertexData");

                for (int i = 0; i < partInfo.vertexCount / 3; i++)
                {
                    meshData.m_indices[i * 3 + 0] = i * 3 + 0;
                    meshData.m_indices[i * 3 + 1] = i * 3 + 2;
                    meshData.m_indices[i * 3 + 2] = i * 3 + 1;
                }

                //Pull the data to be per vertex. Optimize will clean it up.
                for (int i = 0; i < partInfo.vertexCount; i++)
                {
                    int id = idx[i];
                    meshData.m_positions[i] = Vector3f{ p[id * 3 + 0], p[id * 3 + 1], p[id * 3 + 2] };

                    int lookup = id * 3 + 0;
                    if (lookup > p_info.count * p_info.tupleSize)
                    {
                        *m_hou << "ERROR index " << i << " out of bounds on read:  " << lookup << " into array of size: " << p_info.count * p_info.tupleSize << "";
                    }

                    if (n_info.exists || n_vinfo.exists)
                    {
                        int cid = n_vinfo.exists ? i : id;
                        meshData.m_normals[i] = Vector3f{ n[cid * 3 + 0], n[cid * 3 + 1], n[cid * 3 + 2] };
                    }

                    if (cd_info.exists || cd_vinfo.exists)
                    {
                        float r = 0, g = 0, b = 0, a = 1.0f;
                        int cid = cd_vinfo.exists ? i : id;
                        int size = AZStd::max(cd_info.tupleSize, cd_vinfo.tupleSize);

                        if (size >= 3)
                        {
                            r = cd[cid * size + 0];
                            g = cd[cid * size + 1];
                            b = cd[cid * size + 2];
                        }

                        if (size == 4)
                        {
                            a = cd[cid * size + 3];
                        }

                        if (a_info.exists || a_vinfo.exists)
                        {
                            a = alpha[cid * alphaSize];
                        }

                        meshData.m_colors[i] = Vector4f{ r, g, b, a };
                    }

                    if (uv_info.exists || uv_vinfo.exists)
                    {
                        int cid = uv_vinfo.exists ? i : id;
                        int size = AZStd::max(uv_info.tupleSize, uv_vinfo.tupleSize);
                        meshData.m_uvs[i] = Vector2f{ uv[cid * size + 0], -uv[cid * size + 1] };
                    }
                    else {
                        meshData.m_uvs[i] = Vector2f{ 0.f, 0.f };
                    }

                    if (tangent_info.exists && bitangent_info.exists ||
                        tangent_vinfo.exists && bitangent_vinfo.exists)
                    {
                        if (bitangent_info.count == tangent_info.count ||
                            bitangent_vinfo.count == tangent_vinfo.count)
                        {
                            int cid = tangent_vinfo.exists ? i : id;
                            int size = AZStd::max(tangent_info.tupleSize, tangent_vinfo.tupleSize);
                            if (size >= 3)
                            {
                                Vec3 tangentVal(tangent[cid * size + 0], tangent[cid * size + 1], tangent[cid * size + 2]);
                                Vec3 bitangentVal(bitangent[cid * size + 0], bitangent[cid * size + 1], bitangent[cid * size + 2]);

                                meshData.m_tangents[i] = Vector4f{ tangentVal.x, tangentVal.y, tangentVal.z, 0.f };
                                meshData.m_bitangents[i] = Vector3f{ bitangentVal.x, bitangentVal.y, bitangentVal.z };
                            }
                        }
                    }
                    
                }

                if (tangent_info.exists && bitangent_info.exists ||
                    tangent_vinfo.exists && bitangent_vinfo.exists)
                {
                    meshData.CalculateTangents();
                }
            }
            else
            {
                AZ_PROFILE_SCOPE(Editor, "CopyVertexData2HoudiniMeshData");

                //ResizeDownIndices
                {
                    AZ_PROFILE_SCOPE(Editor, "ResizeIndices");
                    meshData.m_indices.resize(idx.size());
                }

                //Copy Index Data:
                {
                    AZ_PROFILE_SCOPE(Editor, "CopyIndices");
                    for (int i = 0; i < idx.size() / 3; i++)
                    {
                        meshData.m_indices[i * 3 + 0] = idx[i * 3 + 0];
                        meshData.m_indices[i * 3 + 1] = idx[i * 3 + 2];
                        meshData.m_indices[i * 3 + 2] = idx[i * 3 + 1];
                    }
                }

                //Resize Down:
                {
                    AZ_PROFILE_SCOPE(Editor, "ResizeDown");
                    meshData.m_positions.resize(partInfo.pointCount);
                    meshData.m_normals.resize(partInfo.pointCount);
                    meshData.m_colors.resize(partInfo.pointCount);
                    meshData.m_uvs.resize(partInfo.pointCount);
                    meshData.m_tangents.resize(partInfo.pointCount);
                    meshData.m_bitangents.resize(partInfo.pointCount);
                }

                bool warnedNormal = false;
                bool warnedTangents = false;

                //Data is already optimized, pull the data directly (do not un-index)
                {
                    AZ_PROFILE_SCOPE(Editor, "CopyPointData");                
                    for (int i = 0; i < partInfo.pointCount; i++)
                    {
                        meshData.m_positions[i] = Vector3f{ p[i * 3 + 0], p[i * 3 + 1], p[i * 3 + 2] };
                        
                        if (n_info.exists)
                        {
                            bool zeroNormal = n[i * 3 + 0] == 0 && n[i * 3 + 1] == 0 && n[i * 3 + 2] == 0;
                            if (zeroNormal)
                            {
                                if (!warnedNormal)
                                {
                                    AZ_Warning("[HOUDINI]", false, "Invalid normal data on mesh from Houdini '%s' Normal: (%f, %f, %f) at index:%d \nUsing identity normals instead, lighting may look incorrect on this mesh.",
                                        geomName.c_str(), n[i * 3 + 0], n[i * 3 + 1], n[i * 3 + 2], i);
                                    warnedNormal = true;
                                }
                                
                                meshData.m_normals[i] = Vector3f{ 0, 0, 1 };
                            }
                            else 
                            {
                                meshData.m_normals[i] = Vector3f{ n[i * 3 + 0], n[i * 3 + 1], n[i * 3 + 2] };
                            }
                        }
                        else
                        {
                            if (!warnedNormal)
                            {
                                AZ_Warning("[HOUDINI]", false, "Normal Data missing from Houdini '%s', Using identity normals instead, lighting may look incorrect on this mesh.",
                                    geomName.c_str());
                                warnedNormal = true;
                            }

                            //TODO: Generate something better?
                            meshData.m_normals[i] = Vector3f{ 0, 0, 1 };
                        }

                        if (cd_info.exists)
                        {
                            float r = 0, g = 0, b = 0, a = 1.0f;
                            int size = cd_info.tupleSize;

                            if (cd_info.tupleSize >= 3)
                            {
                                r = cd[i * size + 0];
                                g = cd[i * size + 1];
                                b = cd[i * size + 2];
                            }

                            if (cd_info.tupleSize == 4)
                            {
                                a = cd[i * size + 3];
                            }
                            
                            if (a_info.exists && a_info.tupleSize == 1)
                            {
                                a = alpha[i * alphaSize];
                            }

                            meshData.m_colors[i] = Vector4f{ r, g, b, a };
                        }

                        if (uv_info.exists)
                        {
                            int size = uv_info.tupleSize;
                            meshData.m_uvs[i] = Vector2f{ uv[i * size + 0], -uv[i * size + 1] };
                        }
						else {
							meshData.m_uvs[i] = Vector2f{ 0.f, 0.f };
						}

                        if (tangent_info.exists && bitangent_info.exists && tangent_info.tupleSize == bitangent_info.tupleSize && tangent_info.tupleSize >= 3)
                        {
                            int cid = i;
                            int size = tangent_info.tupleSize;

                            const auto& azTangent = AZ::Vector3(tangent[cid * size + 0], tangent[cid * size + 1], tangent[cid * size + 2]);
                            const auto& azBitangent = AZ::Vector3(bitangent[cid * size + 0], bitangent[cid * size + 1], bitangent[cid * size + 2]);
                            
                            if (NumberValid(azTangent.GetX()) == false || NumberValid(azTangent.GetY()) == false || NumberValid(azTangent.GetZ()) == false ||
                                NumberValid(azBitangent.GetX()) == false || NumberValid(azBitangent.GetY()) == false || NumberValid(azBitangent.GetZ()) == false)                                
                            {
                                if (!warnedTangents)
                                {
                                    AZ_Warning("[HOUDINI]", false, "Invalid normal data on mesh from Houdini '%s' Tangent: (%f, %f, %f) Bitangent:  (%f, %f, %f) at index:%d \nUsing identity normals instead, lighting may look incorrect on this mesh.",
                                        geomName.c_str(), azTangent.GetX(), azTangent.GetY(), azTangent.GetZ(), azBitangent.GetX(), azBitangent.GetY(), azBitangent.GetZ(), cid);
                                    warnedTangents = true;
                                }

                                meshData.m_tangents[i] = Vector4f{ 1,0,0,0 };
                                meshData.m_bitangents[i] = Vector3f{ 0,1,0 };
                                continue;
                            }
                            
                            meshData.m_tangents[i] = Vector4f{ azTangent.GetX(), azTangent.GetY(), azTangent.GetZ(), 0.f };
                            meshData.m_bitangents[i] = Vector3f{ azBitangent.GetX(), azBitangent.GetY(), azBitangent.GetZ() };
                        }
                    }

					if (tangent_info.exists && bitangent_info.exists ||
						tangent_vinfo.exists && bitangent_vinfo.exists)
					{
						meshData.CalculateTangents();
					}
                }
            }

            count++;
            meshData.UpdateStatObject();

            AZ::Transform transform = m_hou->LookupTransform(m_entityId);
            meshData.UpdateWorldTransform(transform);

            meshData.CalculateAABB();
            m_modelData.m_aabb.AddAabb(meshData.GetAabb());
        }

        return count;
    }    

    bool HoudiniNodeExporter::Initialize(const AZ::EntityId& id, HoudiniFbxConfig* fbxConfig)
    {
        m_entityId = id;
        m_fbxConfig = fbxConfig;

        if (!m_renderMesh)
        {
            m_renderMesh = AZStd::make_unique<HoudiniRenderMesh>(m_entityId);
        }
        
		AzFramework::AssetCatalogEventBus::Handler::BusConnect();

		m_world = AZ::Transform::CreateIdentity();
		AZ::TransformBus::EventResult(m_world, m_entityId, &AZ::TransformBus::Events::GetWorldTM);
        return true;
    }

    bool HoudiniNodeExporter::GenerateMeshMaterials()
    {
        for (auto& materialSetting : m_materialSettings)
        {
            if (materialSetting.m_dirty)
            {
                OnChanged();
                break;
            }
        }

        //TODO: hardcoded to default for now
        //m_renderMesh->SetMaterialPathList({ "editor/materials/defaulthoudini.azmaterial" });
        m_renderMesh->SetMaterialPathList({ "materials/basic_grey.azmaterial" });
        return true;
    }

    bool HoudiniNodeExporter::GenerateMeshData()
    {
        AZ_PROFILE_FUNCTION(Editor);
        if (!CheckHoudiniAccess())
            return false;        

		if (!m_materialWaitList.empty())
			return false;

        HAPI_GeoInfo geometryInfo = m_node->GetGeometryInfo();
        //We only need to do this if things changed, or there is no cache.
        if (m_node->IsGeometryCached() == true && geometryInfo.hasGeoChanged == false && !m_dirty)
        {
            return false;
        }
        
        m_attributeHintNames.clear();
        m_materialHintNames.clear();
        m_matIndexLookup.clear();

        HAPI_NodeInfo nodeInfo = m_node->GetNodeInfo();

        AZ_PROFILE_SCOPE(Editor, m_node->GetNodeName().c_str());

        *m_hou << "--- GenerateMeshData --- " + m_hou->GetString(m_node->GetNodeInfo().nameSH) << "";

        auto clusters = GetClusters();
        if (clusters.size() == 0) 
        {
            //Just do all of it in one go:
            clusters.push_back("");
        }        

        AZStd::map<AZStd::string, AZStd::vector<HoudiniMeshData>> meshMap;
        int meshCount = 0;
        for (auto& cluster : clusters)
        {
            bool usingCluster = false;
            HAPI_NodeId geomNodeId = geometryInfo.nodeId;

            if (!cluster.empty())
            {
                usingCluster = true;
                geomNodeId = CreateObjectMerge("ExportNodes", cluster);                
                *m_hou << "  --- CreateObjectMerge --- " << geomNodeId << " " << cluster << "";
            }             

            //Reads the geometry data and loads it up into our mesh data.
            meshCount += AddGeometry(meshCount, geomNodeId, cluster);
            
            if (usingCluster)
            {
                //We no longer need this merge node.
                HAPI_NodeInfo mergeInfo = HAPI_NodeInfo_Create();
                HAPI_GetNodeInfo(m_session, geomNodeId, &mergeInfo);

                if (mergeInfo.parentId > HOUDINI_ZERO_ID)
                {
                    //Delete the parent node since we're in a scene context.
                    geomNodeId = mergeInfo.parentId;
                }

                HAPI_DeleteNode(m_session, geomNodeId);
                *m_hou << "HAPI_DeleteNode: " << geomNodeId << " " << cluster << "";
            }
        
            m_node->SetGeometryCached(true);
        }

        //Remove any extras:
        while (meshCount < m_modelData.m_meshes.size())
        {
            m_modelData.m_meshes.pop_back();
        }

        if (meshCount > 0)
        {
			// build the mesh map
            for (auto& mesh : m_modelData.m_meshes)
            {
                if (mesh.m_materialNames.size())
                {
					for (const auto& materialName : mesh.m_materialNames)
					{
						auto indices = mesh.GetIndicesByMaterialIndex(mesh.m_materialMap[materialName]);
						if (indices.size() > 0)
						{
							auto& meshInstance = meshMap[materialName].emplace_back(mesh);
							meshInstance.SetIndices(indices);
							meshInstance.CalculateTangents();
							meshInstance.ClearMaterialList();
						}
					}
                }
                else
                {
                    // set the default material here.
                    // defaulthoudini.azmaterial is an attempt to use vertex colors.
                    auto& meshInstance = meshMap["editor/materials/defaulthoudini.azmaterial"/*"materials/basic_grey.azmaterial"*/].emplace_back(mesh);
					meshInstance.CalculateTangents();
					meshInstance.ClearMaterialList();
                }
            }
            
            m_modelData.m_meshes.clear();
            m_modelData.m_materials.clear();

			// merge all meshes in each mesh group along with their instances
			AZ::u32 materialIndex = 0;
			for (auto& meshGroup : meshMap)
			{
				HoudiniMeshData meshData;
				for (const auto& mesh : meshGroup.second)
				{
					meshData += mesh;
				}

				if (meshData.GetCount<AttributeType::Position>() > 0)
				{
					meshData.SetMaterialIndex(materialIndex);
					meshData.CalculateAABB();
                    m_modelData.m_aabb.AddAabb(meshData.GetAabb()); // add mesh data's aabb to get the aabb for the whole model
                    m_modelData.m_meshes.push_back(meshData);
                    m_modelData.m_materials.push_back(meshGroup.first);
					materialIndex++;
				}
			}

            m_modelData.MergeMeshBuffers();

            m_renderMesh->SetMaterialPathList(m_modelData.GetMaterials());
			m_dirty = true;
        }
        
        return true;
    }

	bool HoudiniNodeExporter::GenerateEditableMeshData()
	{
		AZ_PROFILE_FUNCTION(Editor);
		if (!CheckHoudiniAccess())
			return false;

        if (!m_node->HasEditableGeometryInfo())
        {
            return false;
        }

		HAPI_GeoInfo geometryInfo = m_node->GetEditableGeometryInfo();
		//We only need to do this if things changed, or there is no cache.
		if (m_node->IsGeometryCached() == true && geometryInfo.hasGeoChanged == false)
		{
			return false;
		}

        if (!m_node->IsEditableGeometryBuilt())
        {
            //NOTE: originally called from an Output Translator(Unreal Plugin)
            HoudiniSplineTranslator::CreateHoudiniSplineComponentFromHoudiniEditableNode(m_entityId, geometryInfo.nodeId);
            m_node->SetEditableGeometryBuilt(true);
        }
		
        return true;
	}

    //AZStd::vector<IStatObj*> HoudiniNodeExporter::GetStatObjects()
    //{
    //    AZStd::vector<IStatObj*> output;

    //    //ATOMCONVERT
    //    /*for (auto& mesh : m_meshData)
    //    {
    //        auto* stat = mesh.GetStatObject();
    //        if (stat != nullptr)
    //        {
    //            output.push_back(stat);
    //        }
    //    }*/

    //    return output;
    //}

    AZStd::string HoudiniNodeExporter::GetCleanString(const AZStd::string & name)
    {
        QString qName = name.c_str();
        qName = qName.replace("R_", "");
        qName = qName.replace("ROAD_", "");
        qName = qName.replace("ROAD", "");
        qName = qName.replace("_SPLINE", "");
        qName = qName.replace("_HDA", "");
        int index = qName.indexOf("_PART_");
        if (index >= 0) 
        {
            qName = qName.mid(0, index);
        }

        return qName.toUtf8().data();
    }
    
    void HoudiniNodeExporter::RebuildRenderMesh()
    {
        if (!m_materialWaitList.empty())
            return;

		m_dirty = false;
		m_worldAabb.reset();
		m_localAabb.reset();

		if (m_modelData.MeshCount() > 0)
		{
			AZ::SystemTickBus::QueueFunction(
				[=]()
				{
					AzToolsFramework::EditorComponentSelectionRequestsBus::Handler::BusDisconnect();
					AzFramework::BoundsRequestBus::Handler::BusDisconnect();
                    m_renderMesh->BuildMesh(m_modelData);
					m_renderMesh->UpdateTransform(m_world);
					AzFramework::BoundsRequestBus::Handler::BusConnect(m_entityId);
					AzToolsFramework::EditorComponentSelectionRequestsBus::Handler::BusConnect(m_entityId);
				});
		}
    }

    bool HoudiniNodeExporter::UpdatePartNeededMaterials(HAPI_NodeId nodeId, HAPI_PartInfo& partInfo)
    {
		// Update the per face material IDs
		UpdatePartFaceMaterialIDsIfNeeded(nodeId, partInfo);

		// See if we have some material overrides
		UpdatePartFaceMaterialOverridesIfNeeded();

        // TODO: support material overrides
		//// If we have houdini materials AND overrides:
		//// We want to only create the Houdini materials that are not "covered" by overrides
		//// If we have material instance attributes, create all the houdini material anyway
		//// as their textures could be referenced by the material instance parameters
		//if (PartFaceMaterialOverrides.Num() > 0 && !bMaterialOverrideNeedsCreateInstance)
		//{
		//	// If the material override was set on the detail, no need to look for houdini material IDs, as only the override will be used
		//	if (AttribInfoFaceMaterialOverrides.exists && AttribInfoFaceMaterialOverrides.owner == HAPI_ATTROWNER_PRIM)
		//	{
		//		for (int32 MaterialIdx = 0; MaterialIdx < PartFaceMaterialIds.Num(); ++MaterialIdx)
		//		{
		//			// Add a material ID to the unique array only if that face is not using the override
		//			if (PartFaceMaterialOverrides[MaterialIdx].IsEmpty())
		//				PartUniqueMaterialIds.AddUnique(PartFaceMaterialIds[MaterialIdx]);
		//		}
		//	}
		//}
		//else
		{
			// No material overrides, simply update the unique material array
            AZStd::set<AZ::s32> uniqueMatIds;
            for (int32 MaterialIdx = 0; MaterialIdx < PartFaceMaterialIds.size(); ++MaterialIdx)
            {
                AZ::s32 faceMatId = PartFaceMaterialIds[MaterialIdx];
                if(uniqueMatIds.insert(faceMatId).second)
                    PartUniqueMaterialIds.push_back(faceMatId);
            }
				
		}

		//// Remove the invalid material ID from the unique array
		//PartUniqueMaterialIds.RemoveSingle(-1);

		{
			// Get the unique material infos
			PartUniqueMaterialInfos.resize(PartUniqueMaterialIds.size());
			for (int32 MaterialIdx = 0; MaterialIdx < PartUniqueMaterialIds.size(); MaterialIdx++)
			{

				HAPI_MaterialInfo_Init(&PartUniqueMaterialInfos[MaterialIdx]);
				if (HAPI_RESULT_SUCCESS != HAPI_GetMaterialInfo(m_session, PartUniqueMaterialIds[MaterialIdx], &PartUniqueMaterialInfos[MaterialIdx]))
				{
					// Error retrieving material face assignments.
					/*HOUDINI_LOG_MESSAGE(
						TEXT("Creating Static Meshes: Object [%d %s], Geo [%d], Part [%d %s] unable to retrieve material info for material %d"),
						HGPO.ObjectId, *HGPO.ObjectName, HGPO.GeoId, HGPO.PartId, *HGPO.PartName, PartUniqueMaterialIds[MaterialIdx]);*/
					continue;
				}
			}
		}
		return true;
    }

    bool HoudiniNodeExporter::UpdatePartFaceMaterialIDsIfNeeded(HAPI_NodeId nodeId, HAPI_PartInfo& partInfo)
    {
		// Only Retrieve the material IDs if necessary
		if (PartFaceMaterialIds.size() > 0)
			return true;

		AZ::s32 NumFaces = partInfo.faceCount;
		if (NumFaces <= 0)
			return true;

		PartFaceMaterialIds.resize(NumFaces);

		// Get the materials IDs per face
		bool bSingleFaceMaterial = false;
		if (HAPI_RESULT_SUCCESS != HAPI_GetMaterialNodeIdsOnFaces(m_session, nodeId, partInfo.id, &bSingleFaceMaterial, &PartFaceMaterialIds.front(), 0, NumFaces))
		{
			// Error retrieving material face assignments.
            //TODO:
            /*AZ_Warning("[HOUDINI]", false, "Creating Static Meshes: Object [%d %s], Geo [%d], Part [%d %s] unable to retrieve material face assignments",
				HGPO.ObjectId, *HGPO.ObjectName, HGPO.GeoId, HGPO.PartId, *HGPO.PartName);*/
			return false;
		}

		bOnlyOneFaceMaterial = bSingleFaceMaterial;

		return true;
    }

    bool HoudiniNodeExporter::UpdatePartFaceMaterialOverridesIfNeeded()
    {
        // TODO: port this for overrides
		//// Only Retrieve the material overrides if necessary
		//if (PartFaceMaterialOverrides.Num() > 0)
		//	return true;

		//bMaterialOverrideNeedsCreateInstance = false;

		//FHoudiniEngineUtils::HapiGetAttributeDataAsString(
		//	HGPO.GeoInfo.NodeId, HGPO.PartInfo.PartId,
		//	HAPI_UNREAL_ATTRIB_MATERIAL,
		//	AttribInfoFaceMaterialOverrides, PartFaceMaterialOverrides);

		//// If material attribute was not found, check fallback compatibility attribute.
		//if (!AttribInfoFaceMaterialOverrides.exists)
		//{
		//	PartFaceMaterialOverrides.Empty();
		//	FHoudiniEngineUtils::HapiGetAttributeDataAsString(
		//		HGPO.GeoInfo.NodeId, HGPO.PartInfo.PartId,
		//		HAPI_UNREAL_ATTRIB_MATERIAL_FALLBACK,
		//		AttribInfoFaceMaterialOverrides, PartFaceMaterialOverrides);
		//}

		//// If material attribute and fallbacks were not found, check the material instance attribute.
		//if (!AttribInfoFaceMaterialOverrides.exists)
		//{
		//	PartFaceMaterialOverrides.Empty();
		//	FHoudiniEngineUtils::HapiGetAttributeDataAsString(
		//		HGPO.GeoInfo.NodeId, HGPO.PartInfo.PartId,
		//		HAPI_UNREAL_ATTRIB_MATERIAL_INSTANCE,
		//		AttribInfoFaceMaterialOverrides, PartFaceMaterialOverrides);

		//	// We will we need to create material instances from the override attributes
		//	bMaterialOverrideNeedsCreateInstance = AttribInfoFaceMaterialOverrides.exists;
		//}

		//if (AttribInfoFaceMaterialOverrides.exists
		//	&& AttribInfoFaceMaterialOverrides.owner != HAPI_ATTROWNER_PRIM
		//	&& AttribInfoFaceMaterialOverrides.owner != HAPI_ATTROWNER_DETAIL)
		//{
		//	HOUDINI_LOG_WARNING(TEXT("Static Mesh [%d %s], Geo [%d], Part [%d %s]: unreal_material must be a primitive or detail attribute, ignoring attribute."),
		//		HGPO.ObjectId, *HGPO.ObjectName, HGPO.GeoId, HGPO.PartId, *HGPO.PartName);
		//	AttribInfoFaceMaterialOverrides.exists = false;
		//	bMaterialOverrideNeedsCreateInstance = false;
		//	PartFaceMaterialOverrides.Empty();
		//	return false;
		//}

		return true;
    }

    bool HoudiniNodeExporter::CreateNeededMaterials(const AZStd::string& objectName, HAPI_NodeId nodeId, HAPI_PartInfo& partInfo, HoudiniMeshData& meshData)
    {
		PartFaceMaterialIds.clear();
		PartUniqueMaterialIds.clear();
		PartUniqueMaterialInfos.clear();

        UpdatePartNeededMaterials(nodeId, partInfo);

        //// Update package params with resolved attributes
        //TMap<FString, FString> Attributes;
        //TMap<FString, FString> Tokens;
        //FHoudiniAttributeResolver Resolver;
        //FHoudiniPackageParams FinalPackageParams;

        //// Get the attributes from normal geo, or first LOD if there is no normal geo. Fallback to use index 0.
        //FString SplitToUse;
        //for (const FString& SplitGroupName : AllSplitGroups)
        //{
        //    const EHoudiniSplitType SplitType = GetSplitTypeFromSplitName(SplitGroupName);
        //    if (SplitType == EHoudiniSplitType::Normal)
        //    {
        //        SplitToUse = SplitGroupName;
        //        break;
        //    }
        //    else if (SplitType == EHoudiniSplitType::LOD && SplitToUse.IsEmpty())
        //    {
        //        SplitToUse = SplitGroupName;
        //        // don't break here since we might still find normal geo after the LOD splits
        //    }
        //}

        //if (!SplitToUse.IsEmpty())
        //{
        //    CopyAttributesFromHGPOForSplit(SplitToUse, Attributes, Tokens);
        //}
        //else
        //{
        //    CopyAttributesFromHGPOForSplit(0, 0, Attributes, Tokens);
        //}

        // TODO: need to fetch the asset file name.

        /*FHoudiniEngineUtils::UpdatePackageParamsForTempOutputWithResolver(
            PackageParams,
            IsValid(OuterComponent) ? OuterComponent->GetWorld() : nullptr,
            OuterComponent,
            Attributes,
            Tokens,
            FinalPackageParams,
            Resolver);*/

        
        auto assetInfo = m_node->GetAssetInfo();

        HoudiniMaterialTranslator::CreateHoudiniMaterials(
            assetInfo.nodeId,
            PartUniqueMaterialIds,
            PartUniqueMaterialInfos,
            objectName,
            meshData,
            m_materialWaitList,
            false,
            true/*bTreatExistingMaterialsAsUpToDate*/);

        // read the material indices
        bool allSame = false;
		AZStd::vector<HAPI_NodeId> materialIds(partInfo.faceCount);
		HAPI_GetMaterialNodeIdsOnFaces(m_session, nodeId, partInfo.id, &allSame, &materialIds.front(), 0, partInfo.faceCount);

        meshData.m_materialIndices = materialIds;

        //if (bMaterialOverrideNeedsCreateInstance && PartFaceMaterialOverrides.Num() > 0)
        //{
        //    // Map containing unique face materials override attribute
        //    // and their first valid prim index
        //    // We create only one material instance per attribute

        //    FHoudiniMaterialTranslator::SortUniqueFaceMaterialOverridesAndCreateMaterialInstances(PartFaceMaterialOverrides, HGPO, PackageParams, MaterialAndTexturePackages,
        //        InputAssignmentMaterials, OutputAssignmentMaterials,
        //        false);
        //}

        return true;
    }
}
