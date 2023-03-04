#include "GNRenderMesh.h"

//#include <Rendering/Atom/WhiteBoxMeshAtomData.h>
//#include <Rendering/WhiteBoxRenderData.h>
//#include <Util/WhiteBoxMathUtil.h>
//#include <Viewport/WhiteBoxViewportConstants.h>

#include <Atom/RPI.Public/Model/Model.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Reflect/Asset/AssetUtils.h>
#include <Atom/RPI.Reflect/Model/ModelAssetCreator.h>
#include <Atom/RPI.Reflect/Model/ModelLodAssetCreator.h>
#include <Atom/RPI.Reflect/ResourcePoolAssetCreator.h>
#include <AtomLyIntegration/CommonFeatures/Material/MaterialComponentBus.h>
#include <AzCore/Math/PackedVector3.h>

namespace GeomNodes
{
    GNRenderMesh::GNRenderMesh(AZ::EntityId entityId)
        : m_entityId(entityId)
    {
        AZ::Render::MeshHandleStateRequestBus::Handler::BusConnect(m_entityId);
    }

    GNRenderMesh::~GNRenderMesh()
    {
        if (m_meshHandle.IsValid() && m_meshFeatureProcessor)
        {
            m_meshFeatureProcessor->ReleaseMesh(m_meshHandle);
            AZ::Render::MeshHandleStateNotificationBus::Event(
                m_entityId, &AZ::Render::MeshHandleStateNotificationBus::Events::OnMeshHandleSet, &m_meshHandle);
        }

        AZ::Render::MeshHandleStateRequestBus::Handler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
    }

    bool GNRenderMesh::AreAttributesValid() const
    {
        bool attributesAreValid = true;

        for (const auto& attribute : m_attributes)
        {
            AZStd::visit(
                [&attributesAreValid](const auto& att)
                {
                    if (!att->IsValid())
                    {
                        attributesAreValid = false;
                    }
                },
                attribute);
        }

        return attributesAreValid;
    }

    bool GNRenderMesh::CreateMeshBuffers(const GNMeshData& meshData)
    {
        m_indexBuffer = AZStd::make_unique<IndexBuffer>(meshData.GetIndices());

        CreateAttributeBuffer<AttributeType::Position>(meshData.GetPositions());
        CreateAttributeBuffer<AttributeType::Normal>(meshData.GetNormals());
        CreateAttributeBuffer<AttributeType::Tangent>(meshData.GetTangents());
        CreateAttributeBuffer<AttributeType::Bitangent>(meshData.GetBitangents());
        CreateAttributeBuffer<AttributeType::UV>(meshData.GetUVs());
        CreateAttributeBuffer<AttributeType::Color>(meshData.GetColors());

        return AreAttributesValid();
    }

    bool GNRenderMesh::UpdateMeshBuffers(const GNMeshData& meshData)
    {
        UpdateAttributeBuffer<AttributeType::Position>(meshData.GetPositions());
        UpdateAttributeBuffer<AttributeType::Normal>(meshData.GetNormals());
        UpdateAttributeBuffer<AttributeType::Tangent>(meshData.GetTangents());
        UpdateAttributeBuffer<AttributeType::Bitangent>(meshData.GetBitangents());
        UpdateAttributeBuffer<AttributeType::UV>(meshData.GetUVs());
        UpdateAttributeBuffer<AttributeType::Color>(meshData.GetColors());

        return AreAttributesValid();
    }

    void GNRenderMesh::AddLodBuffers(AZ::RPI::ModelLodAssetCreator& modelLodCreator)
    {
        modelLodCreator.SetLodIndexBuffer(m_indexBuffer->GetBuffer());

        for (auto& attribute : m_attributes)
        {
            AZStd::visit(
                [&modelLodCreator](auto& att)
                {
                    att->AddLodStreamBuffer(modelLodCreator);
                },
                attribute);
        }
    }

    void GNRenderMesh::AddMeshBuffers(AZ::RPI::ModelLodAssetCreator& modelLodCreator)
    {
        modelLodCreator.SetMeshIndexBuffer(m_indexBuffer->GetBufferAssetView());

        for (auto& attribute : m_attributes)
        {
            AZStd::visit(
                [&modelLodCreator](auto&& att)
                {
                    att->AddMeshStreamBuffer(modelLodCreator);
                },
                attribute);
        }
    }

    bool GNRenderMesh::CreateLodAsset(const GNMeshData& meshData)
    {
        if (!CreateMeshBuffers(meshData))
        {
            return false;
        }

        AZ::RPI::ModelLodAssetCreator modelLodCreator;
        modelLodCreator.Begin(AZ::Data::AssetId(AZ::Uuid::CreateRandom()));
        AddLodBuffers(modelLodCreator);
        modelLodCreator.BeginMesh();
        modelLodCreator.SetMeshAabb(meshData.GetAabb());

        modelLodCreator.SetMeshMaterialSlot(OneMaterialSlotId);

        AddMeshBuffers(modelLodCreator);
        modelLodCreator.EndMesh();

        if (!modelLodCreator.End(m_lodAsset))
        {
            AZ_Error("CreateLodAsset", false, "Couldn't create LoD asset.");
            return false;
        }

        if (!m_lodAsset.IsReady())
        {
            AZ_Error("CreateLodAsset", false, "LoD asset is not ready.");
            return false;
        }

        if (!m_lodAsset.Get())
        {
            AZ_Error("CreateLodAsset", false, "LoD asset is nullptr.");
            return false;
        }

        return true;
    }

    void GNRenderMesh::CreateModelAsset()
    {
        AZ::RPI::ModelAssetCreator modelCreator;
        modelCreator.Begin(AZ::Data::AssetId(AZ::Uuid::CreateRandom()));
        modelCreator.SetName(ModelName);
        modelCreator.AddLodAsset(AZStd::move(m_lodAsset));

        if (auto materialAsset = AZ::RPI::AssetUtils::LoadAssetByProductPath<AZ::RPI::MaterialAsset>(TexturedMaterialPath.data()))
        {
            auto materialOverrideInstance = AZ::RPI::Material::FindOrCreate(materialAsset);
            auto& materialAssignment = m_materialMap[AZ::Render::DefaultMaterialAssignmentId];
            materialAssignment.m_materialAsset = materialAsset;
            materialAssignment.m_materialInstance = materialOverrideInstance;

            AZ::RPI::ModelMaterialSlot materialSlot;
            materialSlot.m_stableId = OneMaterialSlotId;
            materialSlot.m_defaultMaterialAsset = materialAsset;
            modelCreator.AddMaterialSlot(materialSlot);
        }
        else
        {
            AZ_Error("CreateLodAsset", false, "Could not load material.");
            return;
        }

        modelCreator.End(m_modelAsset);
    }

    bool GNRenderMesh::CreateModel()
    {
        m_model = AZ::RPI::Model::FindOrCreate(m_modelAsset);
        m_meshFeatureProcessor = AZ::RPI::Scene::GetFeatureProcessorForEntity<AZ::Render::MeshFeatureProcessorInterface>(m_entityId);

        if (!m_meshFeatureProcessor)
        {
            AZ_Error("MeshComponentController", m_meshFeatureProcessor, "Unable to find a MeshFeatureProcessorInterface on the entityId.");
            return false;
        }

        m_meshFeatureProcessor->ReleaseMesh(m_meshHandle);
        m_meshHandle = m_meshFeatureProcessor->AcquireMesh(AZ::Render::MeshHandleDescriptor{ m_modelAsset });
        AZ::Render::MeshHandleStateNotificationBus::Event(
            m_entityId, &AZ::Render::MeshHandleStateNotificationBus::Events::OnMeshHandleSet, &m_meshHandle);

        return true;
    }

    bool GNRenderMesh::MeshRequiresFullRebuild([[maybe_unused]] const GNMeshData& meshData) const
    {
        return meshData.VertexCount() != m_vertexCount;
    }

    bool GNRenderMesh::CreateMesh(const GNMeshData& meshData)
    {
        if (!CreateLodAsset(meshData))
        {
            return false;
        }

        CreateModelAsset();

        if (!CreateModel())
        {
            return false;
        }

        m_vertexCount = meshData.VertexCount();

        return true;
    }

    bool GNRenderMesh::DoesMeshRequireFullRebuild([[maybe_unused]] const GNMeshData& meshData) const
    {
        // this has been disabled due to a some recent updates with Atom that a) cause visual artefacts
        // when updating the buffers and b) have a big performance boost when rebuilding from scratch anyway.
        //
        // this method for building the mesh will probably be replace anyway when the Atom DynamicDraw support
        // comes online.
        return true; // meshData.VertexCount() != m_vertexCount;
    }

    void GNRenderMesh::BuildMesh(const GNMeshData& meshData, const AZ::Transform& /*worldFromLocal*/)
    {
        if (DoesMeshRequireFullRebuild(meshData))
        {
            if (!CreateMesh(meshData))
            {
                return;
            }
        }
        else
        {
            if (!UpdateMeshBuffers(meshData))
            {
                return;
            }
        }

        /*m_transform = meshData.GetO3DETransform();
        m_scale = meshData.GetO3DEScale();*/
    }

    void GNRenderMesh::UpdateTransform(const AZ::Transform& worldFromLocal, const AZ::Vector3& /*scale*/)
    {
        // TODO: multiply this to worldFromLocal transform
        m_meshFeatureProcessor->SetTransform(m_meshHandle, worldFromLocal);
    }

    //void GNRenderMesh::UpdateMaterial(const WhiteBoxMaterial& material)
    //{
    //    if (m_meshFeatureProcessor)
    //    {
    //        auto& materialAssignment = m_materialMap[AZ::Render::DefaultMaterialAssignmentId];
    //        materialAssignment.m_propertyOverrides[AZ::Name("baseColor.color")] = AZ::Color(material.m_tint);
    //        materialAssignment.m_propertyOverrides[AZ::Name("baseColor.useTexture")] = material.m_useTexture;
    //        // if ApplyProperties fails, defer updating the material assignment map
    //        // on the next tick, and try applying properties again
    //        if (materialAssignment.ApplyProperties())
    //        {
    //            if (AZ::TickBus::Handler::BusIsConnected())
    //            {
    //                AZ::TickBus::Handler::BusDisconnect();
    //            }
    //            m_meshFeatureProcessor->SetMaterialAssignmentMap(m_meshHandle, m_materialMap);
    //        }
    //        else if (!AZ::TickBus::Handler::BusIsConnected())
    //        {
    //            AZ::TickBus::Handler::BusConnect();
    //        }
    //    }
    //}

    void GNRenderMesh::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        auto& materialAssignment = m_materialMap[AZ::Render::DefaultMaterialAssignmentId];
        if (materialAssignment.ApplyProperties())
        {
            m_meshFeatureProcessor->SetMaterialAssignmentMap(m_meshHandle, m_materialMap);
            AZ::TickBus::Handler::BusDisconnect();
        }
    }

    AZ::Data::Instance<AZ::RPI::Model> GNRenderMesh::GetModel() const
    {
        return m_model;
    }

    void GNRenderMesh::SetVisiblity(bool visibility)
    {
        m_visible = visibility;
        m_meshFeatureProcessor->SetVisible(m_meshHandle, m_visible);
    }

    bool GNRenderMesh::IsVisible() const
    {
        return m_visible;
    }

    const AZ::Render::MeshFeatureProcessorInterface::MeshHandle* GNRenderMesh::GetMeshHandle() const
    {
        return &m_meshHandle;
    }
} // namespace WhiteBox
