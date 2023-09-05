/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once
#include <AzCore/Component/TickBus.h>

#include <Game/HoudiniAttributeBuffer.h>
#include <Game/HoudiniBuffer.h>
#include <Game/HoudiniModelData.h>

#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
#include <AtomLyIntegration/CommonFeatures/Material/MaterialComponentBus.h>
#include <AtomLyIntegration/CommonFeatures/Mesh/MeshHandleStateBus.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Name/Name.h>

namespace AZ::RPI
{
    class ModelLodAsset;
    class ModelAsset;
    class Model;
} // namespace AZ::RPI

namespace HoudiniEngine
{
    //! An implementation to support Atom rendering for the HoudiniEngine gem.
    //! This is very similar tho how WhiteBox's AtomRenderMesh implementation. Would be nice if some classes can be extensible or reusable.
    class HoudiniRenderMesh
        : private AZ::Render::MeshHandleStateRequestBus::Handler
        , public AZ::Render::MaterialConsumerRequestBus::Handler
        , public AZ::Render::MaterialComponentNotificationBus::Handler
        , private AZ::TickBus::Handler
    {
    public:
        AZ_RTTI(HoudiniRenderMesh, "{71A3B7F0-33E9-477A-BC15-D6C73F321A92}");

        explicit HoudiniRenderMesh(AZ::EntityId entityId);
        ~HoudiniRenderMesh();

        //! For building the mesh.
        void BuildMesh(const HoudiniModelData& renderData);
        //! Updates the models transform
        void UpdateTransform(const AZ::Transform& worldFromLocal);
        //! Returns if the Model/Mesh is visible or not.
        bool IsVisible() const;
        //! Sets the visibility of the Model/Mesh
        void SetVisiblity(bool visibility);

        // AZ::TickBus overrides ...
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        // MaterialConsumerRequestBus::Handler overrides...
        AZ::Render::MaterialAssignmentId FindMaterialAssignmentId(
            const AZ::Render::MaterialAssignmentLodIndex lod, const AZStd::string& label) const override;
        AZ::Render::MaterialAssignmentLabelMap GetMaterialLabels() const override;
        AZ::Render::MaterialAssignmentMap GetDefautMaterialMap() const override;
        AZStd::unordered_set<AZ::Name> GetModelUvNames() const override;

        // MaterialComponentNotificationBus::Handler overrides...
        void OnMaterialsUpdated(const AZ::Render::MaterialAssignmentMap& materials) override;

        //! Returns the Model's data instance
        AZ::Data::Instance<AZ::RPI::Model> GetModel() const;

        //! Sets the material paths used by the model. These are full paths with the azmaterial extension.
        void SetMaterialPathList(const AZStd::vector<AZStd::string>& materialPaths);

    private:
        //! Creates an attribute buffer in the slot dictated by AttributeTypeT.
        template<AttributeType AttributeTypeT, typename VertexStreamDataType>
        void CreateAttributeBuffer(const AZStd::vector<VertexStreamDataType>& data)
        {
            const auto attribute_index = static_cast<size_t>(AttributeTypeT);
            m_attributes[attribute_index] = AZStd::make_unique<AttributeBuffer<AttributeTypeT>>(data);
        }

        //! Updates an attribute buffer in the slot dictated by AttributeTypeT.
        template<AttributeType AttributeTypeT, typename VertexStreamDataType>
        void UpdateAttributeBuffer(const AZStd::vector<VertexStreamDataType>& data)
        {
            const auto attribute_index = static_cast<size_t>(AttributeTypeT);
            auto& att = AZStd::get<attribute_index>(m_attributes[attribute_index]);
            att->UpdateData(data);
        }

        // MeshHandleStateRequestBus overrides ...
        const AZ::Render::MeshFeatureProcessorInterface::MeshHandle* GetMeshHandle() const override;

        //! Creates the attribute mesh buffers
        bool CreateMeshBuffers(const HoudiniModelData& modelData);
        //! Creates everything related to the model for Atom Rendering.
        bool CreateMesh(const HoudiniModelData& modelData);
        //! Using the ModelLodAssetCreator creates the Model LOD asset and the meshes.
        bool CreateLodAsset(const HoudiniModelData& modelData);
        //! Using the ModelAssetCreator creates the model asset and sets the material slots.
        void CreateModelAsset();
        //! Final step for create the model by acquiring the mesh handle from the mesh feature processor
        bool CreateModel();
        //! Add LOD buffers to ModelLodAsset
        void AddLodBuffers(AZ::RPI::ModelLodAssetCreator& modelLodCreator);
        //! Add Mesh Buffers to ModelLodAsset
        void AddMeshBuffers(AZ::RPI::ModelLodAssetCreator& modelLodCreator, const HoudiniMeshData& meshData);
        //! Checks if attributes are valid.
        bool AreAttributesValid() const;
        bool DoesMeshRequireFullRebuild(const HoudiniMeshData& meshData) const;
        //! Set the material map in the mesh feature processor.
        void SetMaterials();

        //! Stores the assigned EntityId.
        AZ::EntityId m_entityId;
        //! Reference to a ModelLodAsset
        AZ::Data::Asset<AZ::RPI::ModelLodAsset> m_lodAsset;
        //! Reference to a ModelAsset
        AZ::Data::Asset<AZ::RPI::ModelAsset> m_modelAsset;
        //! Reference to a Model instance
        AZ::Data::Instance<AZ::RPI::Model> m_model;
        //! Reference to MeshFeatureProcessor
        AZ::Render::MeshFeatureProcessorInterface* m_meshFeatureProcessor = nullptr;
        //! Reference to MeshHandle
        AZ::Render::MeshFeatureProcessorInterface::MeshHandle m_meshHandle;
        //! Material map
        AZ::Render::MaterialAssignmentMap m_materialMap;
        //! List of Material paths (i.e. azmodel)
        AZStd::vector<AZStd::string> m_materialPathList;
        //! LOD Index buffer
        AZStd::unique_ptr<IndexBuffer> m_indexBuffer;
        //! Buffers
        AZStd::array<
            AZStd::variant<
                AZStd::unique_ptr<PositionAttribute>,
                AZStd::unique_ptr<NormalAttribute>,
                AZStd::unique_ptr<TangentAttribute>,
                AZStd::unique_ptr<BitangentAttribute>,
                AZStd::unique_ptr<UVAttribute>,
                AZStd::unique_ptr<ColorAttribute>>,
            NumAttributes>
            m_attributes;
        //! For toggling visibility
        bool m_visible = true;

        //! model name.
        static constexpr AZStd::string_view ModelName = "HoudiniEngineMesh";

        //! current model transform
        AZ::Transform m_transform;
        //! current model scale
        AZ::Vector3 m_scale;
    };
} // namespace HoudiniEngine
