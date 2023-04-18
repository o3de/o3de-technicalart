/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once
#include <AzFramework/Visibility/BoundsBus.h>
#include <AzFramework/Asset/AssetCatalogBus.h>
#include <AzCore/Component/TransformBus.h>
#include <AzToolsFramework/API/ComponentEntitySelectionBus.h>
#include <Editor/Rendering/GNModelData.h>

namespace GeomNodes
{
	class GNRenderMesh;
    class GNMeshController
		: public AzFramework::BoundsRequestBus::Handler
		, public AzToolsFramework::EditorComponentSelectionRequestsBus::Handler
		, private AZ::TransformNotificationBus::Handler
		, private AzFramework::AssetCatalogEventBus::Handler
    {
    public:
        explicit GNMeshController(AZ::EntityId entityId);
        ~GNMeshController();

		// EditorComponentSelectionRequestsBus overrides ...
		AZ::Aabb GetEditorSelectionBoundsViewport(const AzFramework::ViewportInfo& viewportInfo) override;
		bool EditorSelectionIntersectRayViewport(
			const AzFramework::ViewportInfo& viewportInfo, const AZ::Vector3& src, const AZ::Vector3& dir,
			float& distance) override;
		bool SupportsEditorRayIntersect() override;

		// BoundsRequestBus overrides ...
		AZ::Aabb GetWorldBounds() override;
		AZ::Aabb GetLocalBounds() override;

		void RebuildRenderMesh();
		void ReadData(AZ::u64 mapId);

		void LoadMaterials(const rapidjson::Value& materialArray);
		void SetFileName(const AZStd::string& path);
		AZStd::string GenerateFBXPath();
		AZStd::string GenerateModelAssetName();
		AZStd::string GenerateAZModelFilename();


    private:
		// TransformNotificationBus overrides ...
		void OnTransformChanged(const AZ::Transform& local, const AZ::Transform& world) override;

		// AssetCatalogEventBus::Handler ...
		void OnCatalogAssetAdded(const AZ::Data::AssetId& assetId) override;
		void OnCatalogAssetChanged(const AZ::Data::AssetId& assetId) override;

        AZ::EntityId m_entityId;

		GNModelData m_modelData;

		AZStd::unique_ptr<GNRenderMesh> m_renderMesh;
		AZ::Transform m_worldFromLocal = AZ::Transform::CreateIdentity();

		AZStd::optional<AZ::Aabb> m_worldAabb; //!< Cached world aabb (used for selection/view determination).
		AZStd::optional<AZ::Aabb> m_localAabb; //!< Cached local aabb (used for center pivot calculation).

		AZStd::string m_blenderFilename;
		AZStd::vector<AZStd::string> m_materialWaitList;	//!< List for materials building in AP. Having an empty list all materials are built and ready for loading.
    };

} // namespace GeomNodes