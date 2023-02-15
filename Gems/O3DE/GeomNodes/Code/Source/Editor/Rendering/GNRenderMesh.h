#pragma once
#include <AzCore/Component/TickBus.h>

#include <Editor/Rendering/Atom/GNAttributeBuffer.h>
#include <Editor/Rendering/Atom/GNBuffer.h>
#include <Editor/Rendering/GNMeshData.h>
//#include <Editor/Rendering/GNRenderMeshInterface.h>

#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
#include <AtomLyIntegration/CommonFeatures/Mesh/MeshHandleStateBus.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Name/Name.h>

namespace AZ::RPI
{
    class ModelLodAsset;
    class ModelAsset;
    class Model;
} // namespace AZ::RPI

namespace GeomNodes
{
    //! A concrete implementation of GNRenderMeshInterface to support Atom rendering for the GeomNodes gem.
    //! This is very similar tho how WhiteBox's AtomRenderMesh implementation. Would be nice if some classes can be extensible or reusable.
    class GNRenderMesh
        :
        // public GNRenderMeshInterface
        //, private AZ::Render::MeshHandleStateRequestBus::Handler
        //, private AZ::TickBus::Handler
        private AZ::TickBus::Handler
    {
    public:
        // AZ_RTTI(GNRenderMesh, "{4E293CD2-F9E6-417C-92B7-DDAF312F46CF}", GNRenderMeshInterface);

        explicit GNRenderMesh(AZ::EntityId entityId);
        ~GNRenderMesh();

        // RenderMeshInterface ...
        void BuildMesh(const GNMeshData& renderData, const AZ::Transform& worldFromLocal);
        void UpdateTransform(const AZ::Transform& worldFromLocal, const AZ::Vector3& scale = AZ::Vector3::CreateOne());
        //void UpdateMaterial(const GNMaterial& material);
        bool IsVisible() const;
        void SetVisiblity(bool visibility);

        // AZ::TickBus overrides ...
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

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
        //const AZ::Render::MeshFeatureProcessorInterface::MeshHandle* GetMeshHandle() const override;

        bool CreateMeshBuffers(const GNMeshData& meshData);
        bool UpdateMeshBuffers(const GNMeshData& meshData);
        bool MeshRequiresFullRebuild(const GNMeshData& meshData) const;
        bool CreateMesh(const GNMeshData& meshData);
        bool CreateLodAsset(const GNMeshData& meshData);
        void CreateModelAsset();
        bool CreateModel();
        void AddLodBuffers(AZ::RPI::ModelLodAssetCreator& modelLodCreator);
        void AddMeshBuffers(AZ::RPI::ModelLodAssetCreator& modelLodCreator);
        bool AreAttributesValid() const;
        bool DoesMeshRequireFullRebuild(const GNMeshData& meshData) const;

        AZ::EntityId m_entityId;
        AZ::Data::Asset<AZ::RPI::ModelLodAsset> m_lodAsset;
        AZ::Data::Asset<AZ::RPI::ModelAsset> m_modelAsset;
        AZ::Data::Instance<AZ::RPI::Model> m_model;
        AZ::Render::MeshFeatureProcessorInterface* m_meshFeatureProcessor = nullptr;
        AZ::Render::MeshFeatureProcessorInterface::MeshHandle m_meshHandle;
        AZ::Render::MaterialAssignmentMap m_materialMap;
        uint32_t m_vertexCount = 0;
        AZStd::unique_ptr<IndexBuffer> m_indexBuffer;
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
        bool m_visible = true;

        //! Default mesh material.
        static constexpr AZStd::string_view TexturedMaterialPath = "materials/basic_grey.azmaterial";
        static constexpr AZStd::string_view SolidMaterialPath = "materials/basic_grey.azmaterial";
        static constexpr AZ::RPI::ModelMaterialSlot::StableId OneMaterialSlotId = 0;

        //! model name.
        static constexpr AZStd::string_view ModelName = "GeomNodesMesh";

        AZ::Transform m_transform;
        AZ::Vector3 m_scale;
    };
}
