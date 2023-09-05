/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/std/containers/map.h>
#include <AzCore/std/containers/set.h>
#include <Game/HoudiniModelData.h>


namespace HoudiniEngine
{
    const AZ::u32 HoudiniModelData::MeshCount() const
    {
        return aznumeric_cast<AZ::u32>(m_meshes.size());
    }

    const HoudiniModelData::MeshDataList HoudiniModelData::GetMeshes() const
    {
        return m_meshes;
    }

    MaterialList HoudiniModelData::GetMaterials()
    {
        return m_materials;
    }

    const U32Vector& HoudiniModelData::GetIndices() const
    {
        return m_indices;
    }

    const Vert3Vector& HoudiniModelData::GetPositions() const
    {
        return m_positions;
    }

    const Vert3Vector& HoudiniModelData::GetNormals() const
    {
        return m_normals;
    }

    const Vert4Vector& HoudiniModelData::GetTangents() const
    {
        return m_tangents;
    }

    const Vert3Vector& HoudiniModelData::GetBitangents() const
    {
        return m_bitangents;
    }

    const Vert2Vector& HoudiniModelData::GetUVs() const
    {
        return m_uvs;
    }

    const Vert4Vector& HoudiniModelData::GetColors() const
    {
        return m_colors;
    }

    AZ::Aabb HoudiniModelData::GetAabb() const
    {
        return m_aabb;
    }

    void HoudiniModelData::MergeMeshBuffers()
    {
        m_indices.clear();
        m_positions.clear();
        m_normals.clear();
        m_tangents.clear();
        m_bitangents.clear();
        m_uvs.clear();
        m_colors.clear();

        //NOTE: set the count here as well.
        for (auto& mesh : m_meshes)
        {
            mesh.SetIndexOffset(m_indices.size());
            m_indices.insert(m_indices.end(), mesh.GetIndices().begin(), mesh.GetIndices().end());
            mesh.SetIndexCount(mesh.GetIndices().size());

            mesh.SetOffset<AttributeType::Position>(m_positions.size());
            m_positions.insert(
                m_positions.end(),
                mesh.GetDataBuffer<AttributeType::Position>().begin(),
                mesh.GetDataBuffer<AttributeType::Position>().end());
            
            mesh.SetOffset<AttributeType::Normal>(m_normals.size());
            m_normals.insert(m_normals.end(), mesh.GetDataBuffer<AttributeType::Normal>().begin(), mesh.GetDataBuffer<AttributeType::Normal>().end());
            
            mesh.SetOffset<AttributeType::Tangent>(m_tangents.size());
            m_tangents.insert(m_tangents.end(), mesh.GetDataBuffer<AttributeType::Tangent>().begin(), mesh.GetDataBuffer<AttributeType::Tangent>().end());
            
            mesh.SetOffset<AttributeType::Bitangent>(m_bitangents.size());
            m_bitangents.insert(
                m_bitangents.end(),
                mesh.GetDataBuffer<AttributeType::Bitangent>().begin(),
                mesh.GetDataBuffer<AttributeType::Bitangent>().end());
            
            mesh.SetOffset<AttributeType::UV>(m_uvs.size());
            m_uvs.insert(m_uvs.end(), mesh.GetDataBuffer<AttributeType::UV>().begin(), mesh.GetDataBuffer<AttributeType::UV>().end());
            
            mesh.SetOffset<AttributeType::Color>(m_colors.size());
            m_colors.insert(m_colors.end(), mesh.GetDataBuffer<AttributeType::Color>().begin(), mesh.GetDataBuffer<AttributeType::Color>().end());
            
            // clear the buffers since we already copied them.
            mesh.ClearBuffers();
        }

        // need to convert positions to O3DE
		AZ::Matrix3x3 rotateX = AZ::Matrix3x3::CreateRotationX(AZ::DegToRad(90.0f));

		auto rotate4x4 = AZ::Matrix4x4::CreateFromMatrix3x4(AZ::Matrix3x4::CreateFromMatrix3x3(rotateX));

		for (auto& position : m_positions)
		{
			auto vec3 = rotate4x4 * MathHelper::Vec3fToVec3(position);
			position = MathHelper::Vec3ToVec3f(vec3);
		}

		auto normalMatrix = rotate4x4.GetInverseFull();
		normalMatrix.Transpose();
		for (auto& normal : m_normals)
		{
			auto vec3 = normalMatrix * MathHelper::Vec3fToVec3(normal);
            vec3.Normalize();
			normal = MathHelper::Vec3ToVec3f(vec3);
		}
    }
} // namespace HoudiniEngine
