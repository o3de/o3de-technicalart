#include "StdAfx.h"

#include <HoudiniCommon.h>
#include "InputNodeManager.h"
#include "Components/HoudiniCurveAttributeComponent.h"
#include <LmbrCentral/Shape/SplineComponentBus.h>
#include <LmbrCentral/Shape/SplineComponentBus.h>
#include <../Source/Shape/SplineComponent.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/ViewportContext.h>
#include <Atom/RPI.Public/ViewportContextBus.h>
#include <Atom/RPI.Public/Model/ModelLodUtils.h>
// FL[FD-10789] Support Mesh as Input to Houdini Digital Asset
#include <AzToolsFramework/Entity/EditorEntityHelpers.h>
//#include <LmbrCentral/Rendering/MeshComponentBus.h> //ATOMCONVERT
#include <IIndexedMesh.h>

namespace HoudiniEngine
{
    #define EDITOR_SPLINE_COMPONENT_GUID "{5B29D788-4885-4D56-BD9B-C0C45BE08EC1}"

	// Gets the view for the specified scene
	static const AZ::RPI::ViewPtr GetViewFromScene(const AZ::RPI::Scene* scene)
	{
		const auto viewportContextRequests = AZ::RPI::ViewportContextRequests::Get();
		const auto viewportContext = viewportContextRequests->GetViewportContextByScene(scene);
		const AZ::RPI::ViewPtr viewPtr = viewportContext->GetDefaultView();
		return viewPtr;
	}

	// utility class based on DrawableMetaData
	class ModelMetaData
	{
	public:
		ModelMetaData(const AZ::EntityId& entityId);
        AZ::RPI::Scene* GetScene() const;
		const AZ::RPI::ViewPtr GetView() const;
		const AZ::Render::MeshFeatureProcessorInterface* GetFeatureProcessor() const;

	private:
        AZ::RPI::Scene* m_scene = nullptr;
        AZ::RPI::ViewPtr m_view = nullptr;
        AZ::Render::MeshFeatureProcessorInterface* m_featureProcessor = nullptr;
	};

	ModelMetaData::ModelMetaData(const AZ::EntityId& entityId)
	{
		m_scene = AZ::RPI::Scene::GetSceneForEntityId(entityId);
		m_view = GetViewFromScene(m_scene);
		m_featureProcessor = m_scene->GetFeatureProcessor<AZ::Render::MeshFeatureProcessorInterface>();
	}

    AZ::RPI::Scene* ModelMetaData::GetScene() const
	{
		return m_scene;
	}

	const AZ::RPI::ViewPtr ModelMetaData::GetView() const
	{
		return m_view;
	}

	const AZ::Render::MeshFeatureProcessorInterface* ModelMetaData::GetFeatureProcessor() const
	{
		return m_featureProcessor;
	}

    void InputNodeManager::Reset()
    {
        m_terrainCache = HOUDINI_INVALID_ID;
        m_inputCache.clear();
    }
   
    void InputNodeManager::OnSplineChanged()
    {
        //Update any splines that have changed.  This has to be done on the next tick since 
        //Custom attributes need to also react to changes from OnSplineChanged.

        const AZ::EntityId* changedSplineId = LmbrCentral::SplineComponentNotificationBus::GetCurrentBusId();
        
        auto previousInputNodeItr = m_inputCache.find(*changedSplineId);

        if (previousInputNodeItr != m_inputCache.end())
        {
            auto& nodeContext = previousInputNodeItr->second;
            nodeContext.m_dirty = true;
        }
    }

    void InputNodeManager::OnTick(float /*deltaTime*/, AZ::ScriptTimePoint /*time*/)
    {
        if (m_hou != nullptr && m_hou->IsActive())
        {
            for (auto& pair : m_inputCache)
            {
                if (pair.second.m_dirty && pair.second.m_cooking == false)
                {                    
                    pair.second.m_cooking = true;
                    m_hou->ExecuteCommand(pair.first, [this, &pair]()
                    {
                        CreateInputNodeFromSpline(pair.first);
                        pair.second.m_cooking = false;
                        return true;
                    });

                    m_hou->RaiseCommandPriority(pair.first);
                }
            }
        }
    }

    HAPI_NodeId InputNodeManager::GetNodeIdFromEntity(const AZ::EntityId& value)
    {        
        if (value.IsValid() == false)
        {            
            return HOUDINI_INVALID_ID;
        }

        //Check to see if it is a houdini node too!
        auto* otherEntity = m_hou->LookupFindEntity(value);

        if (otherEntity != nullptr)
        {
            AZ::SplinePtr spline = m_hou->LookupSpline(value);

            auto* houdiniAssetComponent = otherEntity->FindComponent<HoudiniEngine::HoudiniAssetComponent>();
            // FL[FD-10789] Support Mesh as Input to Houdini Digital Asset
            auto* houdiniTerrainComponent = otherEntity->FindComponent<HoudiniEngine::HoudiniTerrainComponent>();
            const AZ::Uuid meshComponentUuid{ "{DCE68F6E-2E16-4CB4-A834-B6C2F900A7E9}" };
            AZ::Component* meshComponent = otherEntity->FindComponent(meshComponentUuid);

            if (houdiniTerrainComponent != nullptr)
            {
                //Must be first, otherwise this can and will recreate a recursive input
                HAPI_NodeId valueId = m_hou->GetInputNodeManager()->CreateInputNodeFromTerrain(value);
                return valueId;
            }
            else if (houdiniAssetComponent != nullptr)
            {
                IHoudiniNode* otherNode = houdiniAssetComponent->GetNode();
                if (otherNode != nullptr)
                {
                    HAPI_NodeId valueId = otherNode->GetNodeInfo().id;
                    return valueId;
                }
                else
                {
                    return HOUDINI_NOT_READY_ID;
                }
            }            
            else if (spline != nullptr) 
            {
                HAPI_NodeId valueId = CreateInputNodeFromSpline(value);
                return valueId;
            }
            // FL[FD-10789] Support Mesh as Input to Houdini Digital Asset
            else if (meshComponent != nullptr)
            {
                HAPI_NodeId valueId = m_hou->GetInputNodeManager()->CreateInputNodeFromMesh(value);
                return valueId;
            }

        }
        
        return HOUDINI_INVALID_ID;
    }

    HAPI_NodeId InputNodeManager::CreateInputNodeFromSpline(const AZ::EntityId& id)
    {
        AZ_PROFILE_FUNCTION(Editor);

        HAPI_NodeId previousInputNode = -1;
        auto previousInputNodeItr = m_inputCache.find(id);

        if (previousInputNodeItr != m_inputCache.end())
        {
            auto& nodeContext = previousInputNodeItr->second;
            previousInputNode = nodeContext.m_node;

            if (nodeContext.m_dirty == false)
            {
                return previousInputNode;
            }
        }

        const HAPI_Session& session = m_hou->GetSession();

        AZ::Entity* entity = m_hou->LookupFindEntity(id);
        AZ::SplinePtr spline = m_hou->LookupSpline(id);

        if (entity != nullptr && spline != nullptr)
        {
            AZ_PROFILE_SCOPE(Editor, "InputNodeManager::CreateInputNodeFromSpline::ProcessingLySpline");

            //TODO: we need to keep a list of already generated ids!
            int numVerts = aznumeric_cast<int>(spline->GetVertexCount());

            HAPI_NodeId newInput = previousInputNode;
            HAPI_PartInfo newInfo = HAPI_PartInfo_Create();
            HAPI_CurveInfo curveInfo = HAPI_CurveInfo_Create();

            curveInfo.curveCount = 1;
            curveInfo.curveType = HAPI_CURVETYPE_LINEAR;
            curveInfo.hasKnots = false;
            curveInfo.knotCount = 0;
            curveInfo.isPeriodic = false;
            curveInfo.vertexCount = numVerts;

            newInfo.pointCount = numVerts;
            newInfo.vertexCount = numVerts;
            newInfo.faceCount = 1;
            newInfo.type = HAPI_PARTTYPE_CURVE;
            
            bool hasSplineChanged = false;
            AZStd::string name = id.ToString();
            if (entity != nullptr)
            {
                name = entity->GetName();
            }

            if (newInput == HOUDINI_INVALID_ID)
            {
                AZ_PROFILE_SCOPE(Editor, "InputNodeManager::CreateInputNodeFromSpline::HAPI_CreateInputNodeForSpline");

                hasSplineChanged = true;
                HAPI_CreateInputNode(&session, &newInput, name.c_str());
                AddSplineChangeHandler(id);
                *m_hou << "Create Input Node: Spline: " << newInput << " verts: " << numVerts << " " << name;
            }
            else
            {
                //So far this spline already exists, we will do more diffs below to see if things really changed.
                hasSplineChanged = false;
            }

            *m_hou << "---Updating Spline: " << name << "---" << "";
            m_hou->SetProgress("Updating Input Spline: " + name, 0);

            //Set Spline Info:
            {
                HAPI_PartInfo previousPartInfo = HAPI_PartInfo_Create();
                HAPI_GetPartInfo(&session, newInput, 0, &previousPartInfo);

                if (previousPartInfo.pointCount != numVerts)
                {
                    hasSplineChanged = true;
                    AZ_PROFILE_SCOPE(Editor, "InputNodeManager::CreateInputNodeFromSpline::SetSplineInfo");
                    HAPI_SetPartInfo(&session, newInput, 0, &newInfo);
                    HAPI_SetCurveInfo(&session, newInput, 0, &curveInfo);
                    HAPI_SetCurveCounts(&session, newInput, 0, &numVerts, 0, 1);
                }
            }
            
            *m_hou << "HAPI_SetCurveCounts: " << newInput <<  " verts: " << numVerts << "";

            HAPI_AttributeInfo pos_attr_info = HAPI_AttributeInfo_Create();
            pos_attr_info.exists = true;
            pos_attr_info.owner = HAPI_ATTROWNER_POINT;
            pos_attr_info.originalOwner = HAPI_ATTROWNER_POINT;
            pos_attr_info.storage = HAPI_STORAGETYPE_FLOAT;
            pos_attr_info.count = numVerts;
            pos_attr_info.typeInfo = HAPI_ATTRIBUTE_TYPE_VECTOR;
            pos_attr_info.tupleSize = 3;

            AZ::Transform transform = m_hou->LookupTransform(id);

            AZStd::vector<float> points(numVerts * 3);

            //Gather Point Data From Spline:
            {
                AZ_PROFILE_SCOPE(Editor, "InputNodeManager::CreateInputNodeFromSpline::GatherPointDataFromSpline");
                for (int i = 0; i < numVerts; i++)
                {
                    AZ::Vector3 point = spline->GetPosition(AZ::SplineAddress(i));
                    point = transform.TransformPoint(point); //O3DECONVERT check

                    points[i * 3 + 0] = point.GetX();
                    points[i * 3 + 1] = point.GetY();
                    points[i * 3 + 2] = point.GetZ();
                }
            }

            HAPI_Result result;
            
            *m_hou << "HAPI_SetAttributeFloatData: " << newInput << " writing " << numVerts << " from buffer of size: " << points.size() << "";

            //Push Spline PointData to Houdini
            {
                AZ_PROFILE_SCOPE(Editor, "InputNodeManager::CreateInputNodeFromSpline::PushSplinePointData2Houdini");
                                
                //Check to see if anything has changed:
                HAPI_AttributeInfo previousAttribInfo = HAPI_AttributeInfo_Create();
                previousAttribInfo.exists = true;
                previousAttribInfo.owner = HAPI_ATTROWNER_POINT;
                previousAttribInfo.originalOwner = HAPI_ATTROWNER_POINT;
                previousAttribInfo.storage = HAPI_STORAGETYPE_FLOAT;
                previousAttribInfo.count = numVerts;
                previousAttribInfo.typeInfo = HAPI_ATTRIBUTE_TYPE_VECTOR;
                previousAttribInfo.tupleSize = 3;

                HAPI_GetAttributeInfo(&session, newInput, 0, "P", HAPI_ATTROWNER_POINT, &previousAttribInfo);

                //Check each point to see if it changed:
                int previousCount = previousAttribInfo.count * previousAttribInfo.tupleSize;
                if (previousAttribInfo.exists && previousCount == points.size())
                {
                    AZStd::vector<float> previousPointData(previousAttribInfo.count * previousAttribInfo.tupleSize);
                    HAPI_GetAttributeFloatData(&session, newInput, 0, "P", &previousAttribInfo, previousAttribInfo.tupleSize, &previousPointData.front(), 0, previousAttribInfo.count);

                    for (int i = 0; i < previousCount; i++)
                    {
                        if (fabs(previousPointData[i] - points[i]) > 0.00001)
                        {
                            hasSplineChanged = true;
                            break;
                        }
                    }
                }
                else 
                {
                    //The previous value doesn't exist, or has changed size, re-create it:
                    result = HAPI_AddAttribute(&session, newInput, 0, "P", &pos_attr_info);
                    hasSplineChanged = true;
                }
                
                if (hasSplineChanged == true && numVerts > 0)
                {
                    //Only send changes if something changed!
                    result = HAPI_SetAttributeFloatData(&session, newInput, 0, "P", &pos_attr_info, &points.front(), 0, numVerts);
                }

                m_hou->CheckForErrors();
            }

            auto attribList = m_hou->LookupAttributeNames(id);
            
            //Push arbitrary attributes over to Houdini!
            for (auto attrib : attribList)
            {
                auto* attribData = m_hou->LookupAttributeData(id, attrib);
                if (attribData != nullptr)
                {
                    //The spline attribute points might not be updated yet!  If thats the case, we need to push 0 data until its updated.
                    AZStd::vector<float> data;
                    {
                        AZ_PROFILE_SCOPE(Editor, "InputNodeManager::CreateInputNodeFromSpline::GatherSplineAttributeData");
                        for (int i = 0; i < numVerts; i++)
                        {
                            if (i < attribData->Size())
                            {
                                data.push_back(attribData->GetElement(i));
                            }
                            else
                            {
                                data.push_back(0.0f);
                            }
                        }
                    }

                    HAPI_AttributeInfo attribInfo;
                    attribInfo.exists = true;                    
                    attribInfo.owner = HAPI_ATTROWNER_POINT;
                    attribInfo.originalOwner = HAPI_ATTROWNER_POINT;
                    attribInfo.storage = HAPI_STORAGETYPE_FLOAT;                    
                    attribInfo.typeInfo = HAPI_ATTRIBUTE_TYPE_VECTOR;
                    attribInfo.count = numVerts;
                    attribInfo.tupleSize = 1;
                    
                    *m_hou << "HAPI_SetAttributeFloatData: " << attrib << " id:" << newInput << " writing " << numVerts << " from buffer of size: " << data.size() << "";

                    //Push Spline Attribute Data To Houdini
                    {
                        AZ_PROFILE_SCOPE(Editor, "InputNodeManager::CreateInputNodeFromSpline::PushSplineAttributeData2Houdini");
                        
                        //Check to see if anything has changed:
                        HAPI_AttributeInfo previousAttribDataInfo = HAPI_AttributeInfo_Create();
                        HAPI_GetAttributeInfo(&session, newInput, 0, attrib.c_str(), HAPI_ATTROWNER_POINT, &previousAttribDataInfo);
                        if (previousAttribDataInfo.exists == false || previousAttribDataInfo.count != data.size())
                        {   
                            //Something has changed for sure: update the attribute:
                            HAPI_AddAttribute(&session, newInput, 0, attrib.c_str(), &attribInfo);
                            hasSplineChanged = true;
                        }                        
                        else
                        {
                            //Check to see if the attributes that are there are the same still:
                            AZStd::vector<float> previousPointData(attribInfo.count * attribInfo.tupleSize);
                            HAPI_GetAttributeFloatData(&session, newInput, 0, "P", &attribInfo, attribInfo.tupleSize, &previousPointData.front(), 0, attribInfo.count);

                            //Check each point:
                            for (int i = 0; i < attribInfo.count; i ++)
                            {
                                if (fabs(previousPointData[i] - points[i]) > 0.00001f)
                                {
                                    hasSplineChanged = true;
                                    break;
                                }
                            }
                        }

                        //Resize to new size:
                        if (hasSplineChanged || attribInfo.count != numVerts)
                        {
                            attribInfo.count = numVerts;
                            HAPI_SetAttributeFloatData(&session, newInput, 0, attrib.c_str(), &attribInfo, &data.front(), 0, attribInfo.count);
                        }
                    }
                }
            }

            //Commit Geo:
            {
                if (hasSplineChanged)
                {
                    AZ_PROFILE_SCOPE(Editor, "InputNodeManager::CreateInputNodeFromSpline::CommitSplineGeo");
                    *m_hou << "HAPI_CommitGeo: " << " id:" << newInput << "";
                    HAPI_CommitGeo(&session, newInput);
                }
            }

            HoudiniCurveContext nodeContext;
            nodeContext.m_dirty = false;
            nodeContext.m_node = newInput;

            m_inputCache[id] = nodeContext;
            return newInput;
        }

        return HOUDINI_INVALID_ID;
    }

    HAPI_NodeId InputNodeManager::CreateInputNodeFromTerrain(const AZ::EntityId& id)
    {
        AZ_PROFILE_FUNCTION(Editor);
        
        const HAPI_Session& session = m_hou->GetSession();

        HAPI_NodeId previousInputNode = m_terrainCache;

        AZ::Entity* entity = m_hou->LookupFindEntity(id);
        
        if (entity != nullptr)
        {
            AZ_PROFILE_SCOPE(Editor, "InputNodeManager::CreateInputNodeFromTerrain::CheckForTerrain");

            *m_hou << "---Update terrain--- " << "";
            
            m_hou->SetProgress("Updating Input terrain: " + entity->GetName(), 0);

            int width = 0, height = 0;
            AZStd::vector<AZ::Vector3> data;

            auto terrainComp = entity->FindComponent<HoudiniTerrainComponent>();
            if (previousInputNode == HOUDINI_INVALID_ID || terrainComp->IsDirty())
            {
                AZ_PROFILE_SCOPE(Editor, "InputNodeManager::CreateInputNodeFromTerrain::GetTerrainData");
                terrainComp->GetTerrainData(width, height, data);
            }            
            else 
            {
                return previousInputNode;
            }

            HAPI_NodeId newInput = previousInputNode;
            HAPI_PartInfo newInfo = HAPI_PartInfo_Create();

            HAPI_GeoInfo geoInfo;

            int numVerts = width * height;

            newInfo.pointCount = numVerts;
            newInfo.faceCount = (width - 1) * (height - 1);
            newInfo.vertexCount = newInfo.faceCount * 4;

            newInfo.type = HAPI_PARTTYPE_MESH;

            if (newInput == HOUDINI_INVALID_ID)
            {
                AZ_PROFILE_SCOPE(Editor, "InputNodeManager::CreateInputNodeFromTerrain::CreateNewTerrainInputNode");
                HAPI_CreateInputNode(&session, &newInput, "TERRAIN");
                *m_hou << "Create Input Node: TERRAIN: " << newInput << " verts: " << numVerts << "";
            }

            HAPI_GetDisplayGeoInfo(&session, newInput, &geoInfo);
            HAPI_SetPartInfo(&session, newInput, 0, &newInfo);

            HAPI_AttributeInfo pos_attr_info;
            pos_attr_info.exists = true;
            pos_attr_info.owner = HAPI_ATTROWNER_POINT;
            pos_attr_info.storage = HAPI_STORAGETYPE_FLOAT;
            pos_attr_info.count = numVerts;
            pos_attr_info.tupleSize = 3;
            
            //Save buffers for performance:
            {
                AZ_PROFILE_SCOPE(Editor, "InputNodeManager::CreateInputNodeFromTerrain::AllocateTempFaceBufferx4");
                m_faces.resize(newInfo.faceCount * 4);
                m_faceCounts.resize(newInfo.faceCount, 4);
                m_points.resize(numVerts * 3);
            }

            //Generate Index Data For Input Terrain:
            {
                AZ_PROFILE_SCOPE(Editor, "InputNodeManager::CreateInputNodeFromTerrain::GenerateIndexDataForInputTerrain");
                for (int y = 0; y < height - 1; y++)
                {
                    for (int x = 0; x < width - 1; x++)
                    {
                        int i = y * (width - 1) + x;

                        int indexBL = y * width + x;
                        int indexBR = indexBL + 1;
                        int indexTL = (y + 1) * width + x;
                        int indexTR = indexTL + 1;

                        m_faces[i * 4 + 0] = indexBL;
                        m_faces[i * 4 + 1] = indexBR;
                        m_faces[i * 4 + 2] = indexTR;
                        m_faces[i * 4 + 3] = indexTL;
                    }
                }
            }            

            //Copy points from LY Data
            {
                AZ_PROFILE_SCOPE(Editor, "InputNodeManager::CreateInputNodeFromTerrain::CopyPointsFromLYData");
                for (int y = 0; y < height; y++)
                {
                    for (int x = 0; x < width; x++)
                    {
                        int i = y * width + x;
                        const AZ::Vector3& pos = data[i];

                        m_points[i * 3 + 0] = pos.GetX();
                        m_points[i * 3 + 1] = pos.GetY();
                        m_points[i * 3 + 2] = pos.GetZ();
                    }
                }
            }

            *m_hou << "HAPI_SetFaceCounts: " << geoInfo.nodeId << " writing " << m_faceCounts.size() << " from buffer of size: " << m_points.size() << "";

            //Push vertex data and index data to Houdini
            {
                AZ_PROFILE_SCOPE(Editor, "InputNodeManager::CreateInputNodeFromTerrain::PushVertexDataAndIndexData2Houdini");
                HAPI_SetVertexList(&session, geoInfo.nodeId, 0, &m_faces[0], 0, m_faces.size());
                HAPI_SetFaceCounts(&session, geoInfo.nodeId, 0, &m_faceCounts[0], 0, m_faceCounts.size());
            }

            *m_hou << "HAPI_SetAttributeFloatData: " << geoInfo.nodeId << " writing " << numVerts << " from buffer of size: " << m_points.size() << "";

            //PushAttributeData
            {
                AZ_PROFILE_SCOPE(Editor, "InputNodeManager::CreateInputNodeFromTerrain::PushAttributePointData");
                HAPI_AddAttribute(&session, geoInfo.nodeId, 0, "P", &pos_attr_info);
                HAPI_SetAttributeFloatData(&session, geoInfo.nodeId, 0, "P", &pos_attr_info, &m_points.front(), 0, numVerts);
            }

            //Commit Geometry to Houdini
            {
                AZ_PROFILE_SCOPE(Editor, "InputNodeManager::CreateInputNodeFromTerrain::CommitGeo");
                HAPI_CommitGeo(&session, geoInfo.nodeId);
            }


            m_terrainCache = newInput;
            terrainComp->SetDirty(false);
            return newInput;
        }

        return HOUDINI_INVALID_ID;
    }

    // FL[FD-10789] Support Mesh as Input to Houdini Digital Asset
    HAPI_NodeId InputNodeManager::CreateInputNodeFromMesh(const AZ::EntityId& id)
    {
        
        AZ_PROFILE_FUNCTION(Editor);

        const HAPI_Session& session = m_hou->GetSession();

        HAPI_NodeId previousInputNode = HOUDINI_INVALID_ID;
        if (m_meshNodesCache.find(id) != m_meshNodesCache.end())
        {
            previousInputNode = m_meshNodesCache[id];
        }

        AZ::Entity* entity = m_hou->LookupFindEntity(id);

        if (entity != nullptr)
        {
            // Subscribe on transform changes
            AZ::TransformNotificationBus::MultiHandler::BusConnect(id);

            *m_hou << ("-------- UPDATING MESH ------- " + entity->GetName());


			const AZ::Render::MeshFeatureProcessorInterface::MeshHandle* meshHandle = nullptr;
            AZ::Render::MeshHandleStateRequestBus::EventResult(meshHandle, id, &AZ::Render::MeshHandleStateRequestBus::Events::GetMeshHandle);

            if (meshHandle == nullptr || !meshHandle->IsValid()) {
				CryLog("(HOUDINI) Could not get a valid mesh handle");
				return HOUDINI_INVALID_ID;
            }
            
            HAPI_NodeId newInput = previousInputNode;

            HAPI_PartInfo newInfo = HAPI_PartInfo_Create();
            HAPI_GeoInfo geoInfo;

            // Mesh info
			const ModelMetaData modelMetaData(id);
			if (const auto model = modelMetaData.GetFeatureProcessor()->GetModel(*meshHandle))
			{
				const auto modelLodIndex = GetModelLodIndex(modelMetaData.GetView(), model, id);
                auto modelAsset = model->GetModelAsset();
				const AZ::Data::Asset<AZ::RPI::ModelLodAsset>& modelLodAsset = modelAsset->GetLodAssets()[modelLodIndex.m_index];
                AZ::Data::Instance<AZ::RPI::ModelLod> modelLod = AZ::RPI::ModelLod::FindOrCreate(modelLodAsset, modelAsset).get();

                // compute the counts
                AZ::u32 positionCount = 0;
                AZ::u32 indexCount = 0;
                
                for (auto mesh : modelLodAsset->GetMeshes())
                {
                    positionCount += mesh.GetVertexCount();
                    indexCount += mesh.GetIndexCount();
                }

				newInfo.pointCount = positionCount;      // Vertex Count
				newInfo.faceCount = indexCount / 3;  // Faces Count
				newInfo.vertexCount = indexCount;    // Index Count (for some reason in Houdini it is called vertex)

				newInfo.type = HAPI_PARTTYPE_MESH;

				if (newInput == HOUDINI_INVALID_ID)
				{
					AZ_PROFILE_SCOPE(Editor, "InputNodeManager::CreateNodeFromMesh::CreateNewMeshInputNode");
					AZStd::string nodeName = "MESH " + entity->GetName();
					HAPI_CreateInputNode(&session, &newInput, nodeName.c_str());
					*m_hou << "Create Input Node: " << nodeName << " verts: " << newInfo.pointCount << "";
					m_meshNodesCache[id] = newInput;
				}

				HAPI_GetDisplayGeoInfo(&session, newInput, &geoInfo);

				// Push Part Info
				HAPI_SetPartInfo(&session, newInput, 0, &newInfo);

				HAPI_AttributeInfo pos_attr_info;
				pos_attr_info.exists = true;
				pos_attr_info.owner = HAPI_ATTROWNER_POINT;
				pos_attr_info.storage = HAPI_STORAGETYPE_FLOAT;
				pos_attr_info.count = newInfo.pointCount;
				pos_attr_info.tupleSize = 3;

				// Normals attribute
				HAPI_AttributeInfo normal_attr_info;
				normal_attr_info.exists = true;
				normal_attr_info.owner = HAPI_ATTROWNER_VERTEX;
				normal_attr_info.storage = HAPI_STORAGETYPE_FLOAT;
				normal_attr_info.count = newInfo.vertexCount;
				normal_attr_info.tupleSize = HAPI_NORMAL_VECTOR_SIZE;

				// UV attribute
				HAPI_AttributeInfo uv1_attr_info;
				uv1_attr_info.exists = true;
				uv1_attr_info.owner = HAPI_ATTROWNER_VERTEX;
				uv1_attr_info.storage = HAPI_STORAGETYPE_FLOAT;
				uv1_attr_info.count = newInfo.vertexCount;
				uv1_attr_info.tupleSize = 3;
				
				// Color and alpha arrays
				HAPI_AttributeInfo color_attr_info;
				color_attr_info.exists = true;
				color_attr_info.owner = HAPI_ATTROWNER_VERTEX;
				color_attr_info.storage = HAPI_STORAGETYPE_FLOAT;
				color_attr_info.count = newInfo.vertexCount;
				color_attr_info.tupleSize = 3;

				HAPI_AttributeInfo alpha_attr_info;
				alpha_attr_info.exists = true;
				alpha_attr_info.owner = HAPI_ATTROWNER_VERTEX;
				alpha_attr_info.storage = HAPI_STORAGETYPE_FLOAT;
				alpha_attr_info.count = newInfo.vertexCount;
				alpha_attr_info.tupleSize = 1;

                // Get mesh data
				AZStd::vector<float> points;
				points.reserve(newInfo.pointCount * 3);

				AZStd::vector<int> faces;
				faces.reserve(newInfo.faceCount);

				AZStd::vector<int> vertices;
				vertices.reserve(newInfo.vertexCount);

				// Normals array
				AZStd::vector<float> normals;
				normals.reserve(newInfo.vertexCount * 3);

				// UV array
				AZStd::vector<float> uv1;
				uv1.reserve(newInfo.vertexCount * 3);

				AZStd::vector<float> colors;
				colors.reserve(newInfo.vertexCount * 3);

				AZStd::vector<float> alphas;
				alphas.reserve(newInfo.vertexCount);

				// Points array with transform data inside
				AZ::Transform transform = m_hou->LookupTransform(id);

				struct Vec2
				{
					float x, y;
				};
				struct Vec3
				{
					float x, y, z;
				};
				struct Vec4
				{
					float x, y, z, w;
				};

				for (int meshIdx = 0; meshIdx < modelLodAsset->GetMeshes().size(); meshIdx++)
				{
                    auto mesh = modelLodAsset->GetMeshes()[meshIdx];
					const auto sourceIndices = mesh.GetIndexBufferTyped<AZ::u32>();
                    //NOTE: don't use AZ::Vector3/2/4 when using GetSemanticBufferTyped those classes are not packed.
					const auto sourcePositions = mesh.GetSemanticBufferTyped<Vec3>(AZ::Name("POSITION"));
                    const auto sourceNormals = mesh.GetSemanticBufferTyped<Vec3>(AZ::Name("NORMAL"));
                    const auto sourceUVs = mesh.GetSemanticBufferTyped<Vec2>(AZ::Name("UV"));
                    const auto sourceColors = mesh.GetSemanticBufferTyped<Vec4>(AZ::Name("COLOR"));

					auto locPosCount = sourcePositions.size();      // Vertex Count
                    auto locVertexCount = sourceIndices.size(); 
                    auto locFaceCount = locVertexCount / 3;  // Faces Count
					
                    for (int i = 0; i < locPosCount; ++i)
					{
						AZ::Vector3 point = transform.TransformPoint(AZ::Vector3(sourcePositions[i].x, sourcePositions[i].y, sourcePositions[i].z)); //O3DECONVERT check

                        // switch the Y and Z and negate Z
						points.push_back(point.GetX());
						points.push_back(point.GetZ());
                        points.push_back(-point.GetY());
					}
					// Faces array
					for (int i = 0; i < locFaceCount; ++i)
					{
						faces.push_back(3);
					}
					// Indices array
					for (int i = 0; i < locFaceCount; ++i)
					{
                    	vertices.push_back(sourceIndices[i * 3 + 0]);
						vertices.push_back(sourceIndices[i * 3 + 2]);
						vertices.push_back(sourceIndices[i * 3 + 1]);
					}

                    int index = 0;
                    if (sourceNormals.size() > 0)
                    {
                        for (int i = 0; i < locFaceCount; ++i)
                        {
                            /*AZ::Vector3 lyNormal1 = pNorms.data ? pNorms.data[vertices[i * 3 + 0]] : Vec3(0.0f, 0.0f, 1.0f);
                            AZ::Vector3 lyNormal2 = pNorms.data ? pNorms.data[vertices[i * 3 + 1]] : Vec3(0.0f, 0.0f, 1.0f);
                            AZ::Vector3 lyNormal3 = pNorms.data ? pNorms.data[vertices[i * 3 + 2]] : Vec3(0.0f, 0.0f, 1.0f);*/
                            index = sourceIndices[i * 3 + 0];
                            normals.push_back(sourceNormals[index].x);
                            normals.push_back(sourceNormals[index].y);
                            normals.push_back(sourceNormals[index].z);

                            index = sourceIndices[i * 3 + 1];
                            normals.push_back(sourceNormals[index].x);
                            normals.push_back(sourceNormals[index].y);
                            normals.push_back(sourceNormals[index].z);

                            index = sourceIndices[i * 3 + 2];
                            normals.push_back(sourceNormals[index].x);
                            normals.push_back(sourceNormals[index].y);
                            normals.push_back(sourceNormals[index].z);
                        }
                    }

                    if (sourceUVs.size() > 0)
                    {
                        for (int i = 0; i < locFaceCount; ++i)
                        {
                            Vec2 lyUV1 = sourceUVs[sourceIndices[i * 3 + 0]];
                            Vec2 lyUV2 = sourceUVs[sourceIndices[i * 3 + 1]];
                            Vec2 lyUV3 = sourceUVs[sourceIndices[i * 3 + 2]];

                            uv1.push_back(lyUV1.x);
                            uv1.push_back(1.0f - lyUV1.y);
                            uv1.push_back(0.0f);

                            uv1.push_back(lyUV2.x);
                            uv1.push_back(1.0f - lyUV2.y);
                            uv1.push_back(0.0f);

                            uv1.push_back(lyUV3.x);
                            uv1.push_back(1.0f - lyUV3.y);
                            uv1.push_back(0.0f);
                        }
                    }

                    if (sourceColors.size() > 0)
                    {
						for (int i = 0; i < locFaceCount; ++i)
						{
							index = sourceIndices[i * 3 + 0];
							colors.push_back(sourceColors[index].x);
							colors.push_back(sourceColors[index].y);
							colors.push_back(sourceColors[index].z);

							alphas.push_back(sourceColors[index].w);

							index = sourceIndices[i * 3 + 1];
							colors.push_back(sourceColors[index].x);
							colors.push_back(sourceColors[index].y);
							colors.push_back(sourceColors[index].z);

							alphas.push_back(sourceColors[index].w);

							index = sourceIndices[i * 3 + 2];
							colors.push_back(sourceColors[index].x);
							colors.push_back(sourceColors[index].y);
							colors.push_back(sourceColors[index].z);

							alphas.push_back(sourceColors[index].w);
						}
                    }
                }
				
				// FL[FD-12389] Transfer entity name and id from lumberyard
				// Name attribute
				const char* kEntityNameAttrName{ "Ly_NameId" };
				// Create attribute info for <name>_<id> detail attribute
				HAPI_AttributeInfo entity_name_attr_info;
				entity_name_attr_info.exists = true;
				entity_name_attr_info.owner = HAPI_ATTROWNER_DETAIL;
				entity_name_attr_info.storage = HAPI_STORAGETYPE_STRING;
				entity_name_attr_info.count = 1;
				entity_name_attr_info.tupleSize = 1;

				HAPI_AddAttribute(&session, geoInfo.nodeId, 0, kEntityNameAttrName, &entity_name_attr_info);
				// Set Entity <name>_<id> data as detail attribute
				AZStd::string entityNameId = entity->GetName() + "_" + AZStd::to_string(static_cast<AZ::u64>(entity->GetId()));
				const char* entityNameIdAttribData = &entityNameId[0];

				HAPI_SetAttributeStringData(&session, geoInfo.nodeId, 0, kEntityNameAttrName, &entity_name_attr_info, &entityNameIdAttribData, 0, 1);

				// Push Vertex Data
				HAPI_SetVertexList(&session, geoInfo.nodeId, 0, &vertices[0], 0, newInfo.vertexCount);
				HAPI_SetFaceCounts(&session, geoInfo.nodeId, 0, &faces[0], 0, newInfo.faceCount);

				// Push Attribute Data
				// Point Position
				HAPI_AddAttribute(&session, geoInfo.nodeId, 0, HAPI_ATTRIB_POSITION, &pos_attr_info);
				HAPI_SetAttributeFloatData(&session, geoInfo.nodeId, 0, HAPI_ATTRIB_POSITION, &pos_attr_info, &points[0], 0, newInfo.pointCount);
				// Vertex Normal
				HAPI_AddAttribute(&session, geoInfo.nodeId, 0, HAPI_ATTRIB_NORMAL, &normal_attr_info);
				HAPI_SetAttributeFloatData(&session, geoInfo.nodeId, 0, HAPI_ATTRIB_NORMAL, &normal_attr_info, &normals[0], 0, newInfo.vertexCount);
				
                // Vertex UV1
                if (uv1.size() > 0)
                {
					HAPI_AddAttribute(&session, geoInfo.nodeId, 0, HAPI_ATTRIB_UV, &uv1_attr_info);
					HAPI_SetAttributeFloatData(&session, geoInfo.nodeId, 0, HAPI_ATTRIB_UV, &uv1_attr_info, &uv1[0], 0, newInfo.vertexCount);
                }
				
				
                // Vertex Colors
                if (colors.size() > 0)
                {
					HAPI_AddAttribute(&session, geoInfo.nodeId, 0, HAPI_ATTRIB_COLOR, &color_attr_info);
					HAPI_SetAttributeFloatData(&session, geoInfo.nodeId, 0, HAPI_ATTRIB_COLOR, &color_attr_info, &colors[0], 0, newInfo.vertexCount);
					// Vertex Alphas
					HAPI_AddAttribute(&session, geoInfo.nodeId, 0, "Alpha", &alpha_attr_info);
					HAPI_SetAttributeFloatData(&session, geoInfo.nodeId, 0, "Alpha", &alpha_attr_info, &alphas[0], 0, newInfo.vertexCount);
                }
				

				// Commit Geometry to Houdini
				auto result = HAPI_CommitGeo(&session, geoInfo.nodeId);

				if (result != HAPI_RESULT_SUCCESS)
				{
					CryLog("(HOUDINI) Commit GEOmetry Failure: %d", result);
				}
			}

            return newInput;
        }
        
        return HOUDINI_INVALID_ID;
    }

    void InputNodeManager::OnTransformChanged(const AZ::Transform& local, const AZ::Transform& world)
    {
        AZ_UNUSED(local);
        AZ_UNUSED(world);

        const AZ::EntityId* changedMeshId = AZ::TransformNotificationBus::GetCurrentBusId();
        CreateInputNodeFromMesh(*changedMeshId);
    }
    
    AZ::RPI::ModelLodIndex InputNodeManager::GetModelLodIndex(const AZ::RPI::ViewPtr view, AZ::Data::Instance<AZ::RPI::Model> model, AZ::EntityId id) const
    {
		const auto worldTM = AzToolsFramework::GetWorldTransform(id);
		return AZ::RPI::ModelLodUtils::SelectLod(view.get(), worldTM, *model);
    }
}
