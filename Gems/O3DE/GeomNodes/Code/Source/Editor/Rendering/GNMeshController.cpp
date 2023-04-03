#include <Editor/Rendering/GNMeshController.h>
#include <Editor/Rendering/GNRenderMesh.h>
#include <Editor/UI/Utils.h>
#include <Editor/Common/GNConstants.h>
#include <Editor/EBus/EditorGeomNodesComponentBus.h>

#include <AzCore/Debug/Profiler.h>
#include <AzCore/Component/NonUniformScaleBus.h>
#include <AzCore/std/string/regex.h>
#include <AzCore/Utils/Utils.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzToolsFramework/API/EntityCompositionRequestBus.h>
#include <AzToolsFramework/ToolsComponents/TransformComponent.h>
#include <AzToolsFramework/Entity/EditorEntityHelpers.h>
#include <AtomLyIntegration/CommonFeatures/Mesh/MeshComponentConstants.h>
#include <AtomLyIntegration/CommonFeatures/Material/MaterialComponentConstants.h>
#include <AtomLyIntegration/CommonFeatures/Mesh/MeshComponentBus.h>

namespace GeomNodes
{
    GNMeshController::GNMeshController(AZ::EntityId entityId)
        : m_entityId(entityId)
    {
        if (!m_renderMesh)
        {
            m_renderMesh = AZStd::make_unique<GNRenderMesh>(m_entityId);
        }

		AZ::TransformNotificationBus::Handler::BusConnect(m_entityId);
		AzFramework::AssetCatalogEventBus::Handler::BusConnect();

		m_worldFromLocal = AZ::Transform::CreateIdentity();
		AZ::TransformBus::EventResult(m_worldFromLocal, m_entityId, &AZ::TransformBus::Events::GetWorldTM);
    }

    GNMeshController::~GNMeshController()
    {
		AzFramework::AssetCatalogEventBus::Handler::BusDisconnect();
		AzToolsFramework::EditorComponentSelectionRequestsBus::Handler::BusDisconnect();
		AzFramework::BoundsRequestBus::Handler::BusDisconnect();
		AZ::TransformNotificationBus::Handler::BusDisconnect();
		m_renderMesh.reset();
    }

    AZ::Aabb GNMeshController::GetEditorSelectionBoundsViewport(const AzFramework::ViewportInfo& /*viewportInfo*/)
    {
        return GetWorldBounds();
    }
    bool GNMeshController::EditorSelectionIntersectRayViewport(const AzFramework::ViewportInfo& /*viewportInfo*/, const AZ::Vector3& src, const AZ::Vector3& dir, float& distance)
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
    bool GNMeshController::SupportsEditorRayIntersect()
    {
        return true;
    }
    AZ::Aabb GNMeshController::GetWorldBounds()
    {
		if (!m_worldAabb.has_value())
		{
			m_worldAabb = GetLocalBounds();
			m_worldAabb->ApplyTransform(m_worldFromLocal);
		}

		return m_worldAabb.value();
    }
    AZ::Aabb GNMeshController::GetLocalBounds()
    {
		if (!m_localAabb.has_value() && m_modelData.MeshCount() > 0)
		{
			m_localAabb = m_modelData.GetAabb();
		}

		return m_localAabb.value();
    }
	void GNMeshController::RebuildRenderMesh()
	{
		if (!m_materialWaitList.empty())
			return;

		m_worldAabb.reset();
		m_localAabb.reset();

		if (m_modelData.MeshCount() > 0)
		{
			AZ::SystemTickBus::QueueFunction(
				[=]() {
				AzToolsFramework::EditorComponentSelectionRequestsBus::Handler::BusDisconnect();
				AzFramework::BoundsRequestBus::Handler::BusDisconnect();
				m_renderMesh->BuildMesh(m_modelData, m_worldFromLocal);
				m_renderMesh->UpdateTransform(m_worldFromLocal);
				AzFramework::BoundsRequestBus::Handler::BusConnect(m_entityId);
				AzToolsFramework::EditorComponentSelectionRequestsBus::Handler::BusConnect(m_entityId);
			});
		}

		m_requestingMeshRebuild = false;
	}

	void GNMeshController::ReadData(AZ::u64 mapId)
	{
		m_modelData.ReadData(mapId);

		// set the render mesh's material list here so it only get the materials for the current object and not the whole scene.
		MaterialList materialList;
		AZStd::string materialFilePath = AZStd::string(AssetsFolderPath) + m_blenderFilename + "/" + MaterialsFolder.data() + "/";
		for (auto materialName : m_modelData.GetMaterials())
		{
			AZStd::string azMaterialPath = materialFilePath + materialName + AzMaterialExtension.data();
			materialList.push_back(azMaterialPath);
		}

		m_renderMesh->SetMaterialList(materialList);
	}

	void GNMeshController::LoadMaterials(const rapidjson::Value& materialArray)
	{
		m_materialWaitList.clear();
		// iterate through the material arrays and write them into files.
		AZStd::string projectRootPath = GetProjectRoot() + "/";
		AZStd::string materialFilePath = AZStd::string(AssetsFolderPath) + m_blenderFilename + "/" + MaterialsFolder.data() + "/";
		for (rapidjson::Value::ConstValueIterator itr = materialArray.Begin(); itr != materialArray.End(); ++itr)
		{
			const auto matItr = itr->MemberBegin();
			AZStd::string materialName = matItr->name.GetString();
			AZStd::string materialContent = matItr->value.GetString();

			AZStd::string fullFilePath = projectRootPath + materialFilePath + materialName + MaterialExtension.data();

			AZ::Utils::WriteFile(materialContent, fullFilePath.c_str());

			AZStd::string azMaterialPath = materialFilePath + materialName + AzMaterialExtension.data();
			if (AZ::IO::FileIOBase::GetInstance()->Exists(azMaterialPath.c_str()))
			{
				AZ::Data::AssetId materialAssetId;
				EBUS_EVENT_RESULT(materialAssetId, AZ::Data::AssetCatalogRequestBus, GetAssetIdByPath, azMaterialPath.c_str(), AZ::Data::s_invalidAssetType, false);

				// If found, notify mesh that the mesh data is assigned and material is ready.
				if (!materialAssetId.IsValid())
				{
					m_materialWaitList.push_back(azMaterialPath);
				}
			}
		}
	}

	void GNMeshController::SetFileName(const AZStd::string& path)
	{
		AzFramework::StringFunc::Path::GetFileName(path.c_str(), m_blenderFilename);

		AZStd::regex reg("[^\\w\\s]+");
		m_blenderFilename = AZStd::regex_replace(m_blenderFilename, reg, "_");
	}

	AZStd::string GNMeshController::GenerateFBXPath()
	{
		AZStd::string fullFilePath = GetProjectRoot() + "/";
		AZStd::string filePath = AZStd::string(AssetsFolderPath) + m_blenderFilename + "/";
		fullFilePath += filePath + GenerateModelAssetName() + FbxExtension.data();
		return fullFilePath;
	}

	AZStd::string GNMeshController::GenerateModelAssetName()
	{
		return m_blenderFilename + "_" + AZStd::string::format("%llu", (AZ::u64)m_entityId);
	}

	AZStd::string GNMeshController::GenerateAZModelFilename()
	{
		return GenerateModelAssetName() + AzModelExtension.data();
	}

    void GNMeshController::OnTransformChanged(const AZ::Transform& /*local*/, const AZ::Transform& world)
    {
		m_worldAabb.reset();
		m_localAabb.reset();

		m_worldFromLocal = world;

		if (m_renderMesh)
		{
			m_renderMesh->UpdateTransform(world);
		}
    }
	void GNMeshController::OnCatalogAssetAdded(const AZ::Data::AssetId& assetId)
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

			bool workInProgress = false;
			EditorGeomNodesComponentRequestBus::EventResult(workInProgress, m_entityId, &EditorGeomNodesComponentRequests::GetWorkInProgress);
			if (workInProgress && (assetName == GenerateModelAssetName()))
			{
				auto entity = AzToolsFramework::GetEntity(m_entityId);
				auto transformComponent = entity->FindComponent<AzToolsFramework::Components::TransformComponent>();
				AZ::EntityId parentId = transformComponent->GetParentId();
				auto worldTransform = transformComponent->GetWorldTM();

				AZ::EntityId entityId;
				EBUS_EVENT_RESULT(entityId, AzToolsFramework::EditorRequests::Bus, CreateNewEntity, parentId);

				AzToolsFramework::EntityIdList entityIdList = { entityId };

				AzToolsFramework::EntityCompositionRequests::AddComponentsOutcome addedComponentsResult = AZ::Failure(AZStd::string("Failed to call AddComponentsToEntities on EntityCompositionRequestBus"));
				AzToolsFramework::EntityCompositionRequestBus::BroadcastResult(addedComponentsResult, &AzToolsFramework::EntityCompositionRequests::AddComponentsToEntities, entityIdList, AZ::ComponentTypeList{ AZ::Render::EditorMeshComponentTypeId });

				if (addedComponentsResult.IsSuccess())
				{
					AZ::TransformBus::Event(
						entityId, &AZ::TransformBus::Events::SetWorldTM, worldTransform);

					AZ::Render::MeshComponentRequestBus::Event(
						entityId, &AZ::Render::MeshComponentRequestBus::Events::SetModelAssetPath, assetInfo.m_relativePath);

					EBUS_EVENT(AzToolsFramework::ToolsApplicationRequests::Bus, DeleteEntitiesAndAllDescendants, AzToolsFramework::EntityIdList{ m_entityId });
				}

				EditorGeomNodesComponentRequestBus::Event(m_entityId, &EditorGeomNodesComponentRequests::SetWorkInProgress, false);
			}
			else
			{
				if (!m_materialWaitList.empty())
				{
					auto iter = AZStd::find(m_materialWaitList.begin(), m_materialWaitList.end(), assetInfo.m_relativePath);
					if (iter != m_materialWaitList.end())
					{
						m_materialWaitList.erase(iter);
						if (m_requestingMeshRebuild && m_materialWaitList.empty()) {
							RebuildRenderMesh();
						}
					}
				}
			}
		}
	}
	void GNMeshController::OnCatalogAssetChanged(const AZ::Data::AssetId& assetId)
	{
		OnCatalogAssetAdded(assetId);
	}
} // namespace GeomNodes