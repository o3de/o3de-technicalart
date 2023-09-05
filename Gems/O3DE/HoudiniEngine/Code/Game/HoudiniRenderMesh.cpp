/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "HoudiniRenderMesh.h"

#include <Atom/RPI.Public/Model/Model.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Reflect/Asset/AssetUtils.h>
#include <Atom/RPI.Reflect/Model/ModelAssetCreator.h>
#include <Atom/RPI.Reflect/Model/ModelLodAssetCreator.h>
#include <Atom/RPI.Reflect/ResourcePoolAssetCreator.h>
#include <AtomLyIntegration/CommonFeatures/Material/MaterialComponentBus.h>
#include <AzCore/Math/PackedVector3.h>

namespace HoudiniEngine
{
    HoudiniRenderMesh::HoudiniRenderMesh(AZ::EntityId entityId)
        : m_entityId(entityId)
    {
        AZ::Render::MeshHandleStateRequestBus::Handler::BusConnect(m_entityId);
        AZ::Render::MaterialConsumerRequestBus::Handler::BusConnect(m_entityId);
        AZ::Render::MaterialComponentNotificationBus::Handler::BusConnect(m_entityId);
    }

    HoudiniRenderMesh::~HoudiniRenderMesh()
    {
        if (m_meshHandle.IsValid() && m_meshFeatureProcessor)
        {
            m_meshFeatureProcessor->ReleaseMesh(m_meshHandle);
            AZ::Render::MeshHandleStateNotificationBus::Event(
                m_entityId, &AZ::Render::MeshHandleStateNotificationBus::Events::OnMeshHandleSet, &m_meshHandle);
        }

        AZ::Render::MaterialConsumerRequestBus::Handler::BusDisconnect();
        AZ::Render::MaterialComponentNotificationBus::Handler::BusDisconnect();
        AZ::Render::MeshHandleStateRequestBus::Handler::BusDisconnect();
    }

    bool HoudiniRenderMesh::AreAttributesValid() const
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

    bool HoudiniRenderMesh::CreateMeshBuffers(const HoudiniModelData& modelData)
    {
        m_indexBuffer = AZStd::make_unique<IndexBuffer>(modelData.GetIndices());

        CreateAttributeBuffer<AttributeType::Position>(modelData.GetPositions());
        CreateAttributeBuffer<AttributeType::Normal>(modelData.GetNormals());
        CreateAttributeBuffer<AttributeType::Tangent>(modelData.GetTangents());
        CreateAttributeBuffer<AttributeType::Bitangent>(modelData.GetBitangents());
        CreateAttributeBuffer<AttributeType::UV>(modelData.GetUVs());
        CreateAttributeBuffer<AttributeType::Color>(modelData.GetColors());

        return AreAttributesValid();
    }

    void HoudiniRenderMesh::AddLodBuffers(AZ::RPI::ModelLodAssetCreator& modelLodCreator)
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

    void HoudiniRenderMesh::AddMeshBuffers(AZ::RPI::ModelLodAssetCreator& modelLodCreator, const HoudiniMeshData& meshData)
    {
        modelLodCreator.SetMeshIndexBuffer(
            { m_indexBuffer->GetBuffer(),
              AZ::RHI::BufferViewDescriptor::CreateTyped(
                  meshData.GetIndexOffset(), meshData.GetIndexCount(), GetFormatForVertexStreamDataType<uint32_t>()) });

        for (auto& attribute : m_attributes)
        {
            AZStd::visit(
                [&modelLodCreator, meshData](auto&& att)
                {
                    att->AddMeshStreamBuffer(modelLodCreator, meshData);
                },
                attribute);
        }
    }

    bool HoudiniRenderMesh::CreateLodAsset(const HoudiniModelData& modelData)
    {
        if (!CreateMeshBuffers(modelData))
        {
            return false;
        }

        AZ::RPI::ModelLodAssetCreator modelLodCreator;
        modelLodCreator.Begin(AZ::Data::AssetId(AZ::Uuid::CreateRandom()));
        AddLodBuffers(modelLodCreator);

        for (auto meshData : modelData.GetMeshes())
        {
            modelLodCreator.BeginMesh();
            modelLodCreator.SetMeshAabb(meshData.GetAabb());

            modelLodCreator.SetMeshMaterialSlot(meshData.GetMaterialIndex());

            AddMeshBuffers(modelLodCreator, meshData);
            modelLodCreator.EndMesh();
        }

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

    void HoudiniRenderMesh::CreateModelAsset()
    {
        AZ::RPI::ModelAssetCreator modelCreator;
        modelCreator.Begin(AZ::Data::AssetId(AZ::Uuid::CreateRandom()));
        modelCreator.SetName(ModelName);
        modelCreator.AddLodAsset(AZStd::move(m_lodAsset));

        m_materialMap.clear();
        AZ::RPI::ModelMaterialSlot::StableId slotId = 0;
        for (auto materialPath : m_materialPathList)
        {
            if (auto materialAsset = AZ::RPI::AssetUtils::LoadAssetByProductPath<AZ::RPI::MaterialAsset>(materialPath.c_str()))
            {
                auto materialOverrideInstance = AZ::RPI::Material::FindOrCreate(materialAsset);
                auto& materialAssignment = m_materialMap[AZ::Render::MaterialAssignmentId::CreateFromStableIdOnly(slotId)];
                materialAssignment.m_materialAsset = materialAsset;
                materialAssignment.m_materialInstance = materialOverrideInstance;

                AZ::RPI::ModelMaterialSlot materialSlot;
                materialSlot.m_stableId = slotId;
                materialSlot.m_defaultMaterialAsset = materialAsset;
                modelCreator.AddMaterialSlot(materialSlot);
            }
            else
            {
                AZ_Error("CreateLodAsset", false, "Could not load material.");
                return;
            }

            slotId++;
        }

        modelCreator.End(m_modelAsset);
    }

    bool HoudiniRenderMesh::CreateModel()
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

    bool HoudiniRenderMesh::CreateMesh(const HoudiniModelData& modelData)
    {
        if (!CreateLodAsset(modelData))
        {
            return false;
        }

        CreateModelAsset();

        if (!CreateModel())
        {
            return false;
        }

        SetMaterials();

        return true;
    }

    bool HoudiniRenderMesh::DoesMeshRequireFullRebuild([[maybe_unused]] const HoudiniMeshData& meshData) const
    {
        // this has been disabled due to a some recent updates with Atom that a) cause visual artefacts
        // when updating the buffers and b) have a big performance boost when rebuilding from scratch anyway.
        //
        // this method for building the mesh will probably be replace anyway when the Atom DynamicDraw support
        // comes online.
        return true; // meshData.VertexCount() != m_vertexCount;
    }

    void HoudiniRenderMesh::SetMaterials()
    {
        if (m_meshFeatureProcessor && m_meshHandle.IsValid())
        {
            m_meshFeatureProcessor->SetCustomMaterials(m_meshHandle, AZ::Render::ConvertToCustomMaterialMap(m_materialMap));
        }
    }

    void HoudiniRenderMesh::SetMaterialPathList(const AZStd::vector<AZStd::string>& materialPaths)
    {
        m_materialPathList = materialPaths;
    }

    void HoudiniRenderMesh::BuildMesh(const HoudiniModelData& modelData)
    {
        CreateMesh(modelData);
    }

    void HoudiniRenderMesh::UpdateTransform(const AZ::Transform& worldFromLocal)
    {
        if (m_meshFeatureProcessor && m_meshHandle.IsValid())
        {
            m_meshFeatureProcessor->SetTransform(m_meshHandle, worldFromLocal);
        }
    }

    void HoudiniRenderMesh::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
    }

    AZ::Render::MaterialAssignmentId HoudiniRenderMesh::FindMaterialAssignmentId(
        const AZ::Render::MaterialAssignmentLodIndex lod, const AZStd::string& label) const
    {
        return AZ::Render::GetMaterialSlotIdFromModelAsset(m_modelAsset, lod, label);
    }

    AZ::Render::MaterialAssignmentLabelMap HoudiniRenderMesh::GetMaterialLabels() const
    {
        return AZ::Render::GetMaterialSlotLabelsFromModelAsset(m_modelAsset);
    }

    AZ::Render::MaterialAssignmentMap HoudiniRenderMesh::GetDefautMaterialMap() const
    {
        return AZ::Render::GetDefautMaterialMapFromModelAsset(m_modelAsset);
    }

    AZStd::unordered_set<AZ::Name> HoudiniRenderMesh::GetModelUvNames() const
    {
        const AZ::Data::Instance<AZ::RPI::Model> model = GetModel();
        return model ? model->GetUvNames() : AZStd::unordered_set<AZ::Name>();
    }

    void HoudiniRenderMesh::OnMaterialsUpdated(const AZ::Render::MaterialAssignmentMap& materials)
    {
        if (m_meshFeatureProcessor)
        {
            m_meshFeatureProcessor->SetCustomMaterials(m_meshHandle, AZ::Render::ConvertToCustomMaterialMap(materials));
        }
    }

    AZ::Data::Instance<AZ::RPI::Model> HoudiniRenderMesh::GetModel() const
    {
        return m_model;
    }

    void HoudiniRenderMesh::SetVisiblity(bool visibility)
    {
        m_visible = visibility;
        m_meshFeatureProcessor->SetVisible(m_meshHandle, m_visible);
    }

    bool HoudiniRenderMesh::IsVisible() const
    {
        return m_visible;
    }

    const AZ::Render::MeshFeatureProcessorInterface::MeshHandle* HoudiniRenderMesh::GetMeshHandle() const
    {
        return &m_meshHandle;
    }
} // namespace HoudiniEngine
