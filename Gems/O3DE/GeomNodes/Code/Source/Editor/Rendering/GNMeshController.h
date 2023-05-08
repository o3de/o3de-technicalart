/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once
#include <AzCore/Component/TransformBus.h>
#include <AzFramework/Asset/AssetCatalogBus.h>
#include <AzFramework/Visibility/BoundsBus.h>
#include <AzToolsFramework/API/ComponentEntitySelectionBus.h>
#include <Editor/Rendering/GNModelData.h>


namespace GeomNodes
{
    class GNRenderMesh;
    //! A common class that handles everything from storing the model data, rendering the mesh via GNRenderMesh
    //! handling different mesh events like bound request, transform updates and others.
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
            const AzFramework::ViewportInfo& viewportInfo, const AZ::Vector3& src, const AZ::Vector3& dir, float& distance) override;
        bool SupportsEditorRayIntersect() override;

        // BoundsRequestBus overrides ...
        AZ::Aabb GetWorldBounds() override;
        AZ::Aabb GetLocalBounds() override;

        //! Builds the model for atom rendering.
        void RebuildRenderMesh();
        //! Read the data from the shared memory and setup everything before building the render mesh.
        void ReadData(AZ::u64 mapId);

        //! Load the materials giver the json array. If there materials that are still in the AP in will tag them for waiting
        //! so that the render mesh can properly load the materials.
        void LoadMaterials(const rapidjson::Value& materialArray);
        //! Sets the blender filename.
        void SetFileName(const AZStd::string& path);
        //! Generates the full FBX Path. This is usually used when exporting.
        AZStd::string GenerateFBXPath();
        //! Generates the Model asset name with the full path but without the file extension.
        AZStd::string GenerateModelAssetName();

    private:
        // TransformNotificationBus overrides ...
        void OnTransformChanged(const AZ::Transform& local, const AZ::Transform& world) override;

        // AssetCatalogEventBus::Handler ...
        //! we use these functions to know if our assets like materials are already loaded so we can proceed with rendering the model/meshes
        void OnCatalogAssetAdded(const AZ::Data::AssetId& assetId) override;
        void OnCatalogAssetChanged(const AZ::Data::AssetId& assetId) override;

        //! assigned EntityId to this Mesh Controller
        AZ::EntityId m_entityId;
        //! Stores the model's data.
        GNModelData m_modelData;
        //! Reference to our render mesh.
        AZStd::unique_ptr<GNRenderMesh> m_renderMesh;
        //! Current world transform of the object.
        AZ::Transform m_world = AZ::Transform::CreateIdentity();
        //! Cached world aabb (used for selection/view determination).
        AZStd::optional<AZ::Aabb> m_worldAabb;
        //! Cached local aabb (used for center pivot calculation).
        AZStd::optional<AZ::Aabb> m_localAabb;
        //! Current loaded blender filename
        AZStd::string m_blenderFilename;
        //! List for materials building in AP. Having an empty list all materials are built and ready for loading.
        AZStd::vector<AZStd::string> m_materialWaitList;
    };

} // namespace GeomNodes