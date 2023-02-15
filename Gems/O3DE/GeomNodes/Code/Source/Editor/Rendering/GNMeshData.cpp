#include <Editor/Rendering/GNMeshData.h>
#include <AzCore/std/containers/unordered_map.h>
#include "Bridge.h"

namespace GeomNodes
{
    GNMeshData::GNMeshData(
        const Vert3Vector& positions,
        const Vert3Vector& normals,
        const S32Vector& indices,
        const S32Vector& triangleLoops,
        const S32Vector& loops,
        const Vert2Vector& uvs,
        const Vert4Vector& colors,
        const S32Vector& /*materialIndices*/,
        AZ::u64 hash,
        bool isIndexedUVs,
        bool isIndexedColors)
    {
        m_positions = positions;
        m_normals = normals;
        m_loops = loops;
        m_hash = hash;
        m_colors = colors;
        m_uvs = uvs;
		m_indices.resize(indices.size());
		AZStd::transform(indices.begin(), indices.end(), m_indices.begin(),
			[](int x)
		{
			return aznumeric_cast<AZ::u32>(x);
		});


        // we still need to build all the vertices, uvs, normals and colors as what we have is incomplete. 
        // the initial data are shared and is done this way so the data coming from blender will be small.
        Vert2Vector finalUVs;
        Vert3Vector finalPositions, finalNormals;
		Vert4Vector finalColors;

        finalPositions.reserve(m_positions.size());
        finalNormals.reserve(m_normals.size());
        finalUVs.reserve(m_positions.size());
        finalColors.reserve(m_positions.size());

        AZ_Assert(m_normals.size() == m_indices.size(), "normals and indices count should match!");

        // iterate through the indices
        AZStd::unordered_map<UniqueKey, AZ::s32> uniqueKeys;
        for (int i = 0; i < m_indices.size(); i++)
        {
            // build unique keys based on the current index, uv, normal and color.
            auto* indexPtr = &m_indices[i];
            const auto& normal = m_normals[i];
            UniqueKey key(
                *indexPtr,
                normal,
                m_uvs[isIndexedUVs ? *indexPtr : triangleLoops[i]],
                m_colors[isIndexedColors ? *indexPtr : triangleLoops[i]]);
            
            auto iter = uniqueKeys.find(key);
            if (iter == uniqueKeys.end())
            {
				finalPositions.emplace_back(m_positions[*indexPtr]);

				if (isIndexedUVs)
				{
					finalUVs.emplace_back(m_uvs[*indexPtr]);
				}

				if (isIndexedColors)
				{
					finalColors.emplace_back(m_colors[*indexPtr]);
				}

                // update the indexes using the new one
                *indexPtr = m_loops[triangleLoops[i]] = aznumeric_cast<AZ::s32>(finalPositions.size() - 1);
				finalNormals.emplace_back(normal);
				uniqueKeys.emplace(AZStd::make_pair<UniqueKey, AZ::s32>(key, i));
            }
            else
            {
				auto splitVertexIdx = iter->second;
                *indexPtr = m_indices[splitVertexIdx];
				m_loops[triangleLoops[i]] = m_loops[triangleLoops[splitVertexIdx]];
            }
        }

        m_positions = finalPositions;
        m_normals = finalNormals;

		AZStd::vector<AZ::s32> cornerIdxList;
		cornerIdxList.resize(m_loops.size());
		for (AZ::s32 i = 0; i < m_loops.size(); i++)
		{
			cornerIdxList[m_loops[i]] = i;
		}

        if (isIndexedUVs)
        {
            m_uvs = finalUVs;

			std::transform(m_uvs.begin(), m_uvs.end(), m_uvs.begin(),
				[](Vector2f& uv) {
				uv[1] = 1.f - uv[1];
				return uv;
			});
        }
        else {
			AZStd::vector<Vector2f> indexedUVs;
			indexedUVs.reserve(m_positions.size());
			for (AZ::s32 i = 0; i < m_positions.size(); i++)
			{
				const auto cornerIdx = cornerIdxList[i];
				indexedUVs.emplace_back(Vector2f{ m_uvs[cornerIdx][0], 1.f - m_uvs[cornerIdx][1] });
			}

			m_uvs = indexedUVs;
        }
        
        if (isIndexedColors)
        {
            m_colors = finalColors;
        }
        
        // calculate the aabb
        for (const auto& vert : m_positions)
        {
            m_aabb.AddPoint(AZ::Vector3(vert[0], vert[1], vert[2]));
        }

        CalculateTangents();
    }

    void GNMeshData::CalculateTangents()
    {
        m_tangents.resize(m_positions.size());
        m_bitangents.resize(m_positions.size());

        for (AZ::s32 i = 0; i < m_indices.size() / 3; ++i)
        {
            const AZ::Vector3 V0 = MathHelper::Vec3fToVec3(m_positions[m_indices[i + 0]]);
            const AZ::Vector3 V1 = MathHelper::Vec3fToVec3(m_positions[m_indices[i + 1]]);
            const AZ::Vector3 V2 = MathHelper::Vec3fToVec3(m_positions[m_indices[i + 2]]);

            const AZ::Vector2 UV0 = MathHelper::Vec2fToVec2(m_uvs[m_indices[i + 0]]);
            const AZ::Vector2 UV1 = MathHelper::Vec2fToVec2(m_uvs[m_indices[i + 1]]);
            const AZ::Vector2 UV2 = MathHelper::Vec2fToVec2(m_uvs[m_indices[i + 2]]);

            // Calculate edge vectors of the triangle
            AZ::Vector3 edge1 = V1 - V0;
            AZ::Vector3 edge2 = V2 - V0;

            // Calculate delta UVs
            AZ::Vector2 deltaUV1 = UV1 - UV0;
            AZ::Vector2 deltaUV2 = UV2 - UV0;

            // Calculate the determinant to calculate the direction of the tangent and bitangent
            float det = 1.0f / (deltaUV1.GetX() * deltaUV2.GetY() - deltaUV2.GetX() * deltaUV1.GetY());

            // Calculate the tangent vector
            AZ::Vector3 tangent = (edge1 * deltaUV2.GetX() - edge2 * deltaUV1.GetY()) * det;
            tangent.Normalize();

            // Calculate the bitangent vector
            AZ::Vector3 bitangent = (edge2 * deltaUV1.GetY() - edge1 * deltaUV2.GetX()) * det;
            bitangent.Normalize();

            // Calculate the handedness of the tangent space
            float tangentW = (edge1.Cross(edge2).Dot(tangent) < 0.0f) ? -1.0f : 1.0f;

            AZ::Vector4 tangent4 = AZ::Vector4(tangent, tangentW);
            m_tangents[m_indices[i]] = MathHelper::Vec4ToVec4f(tangent4);
            m_bitangents[m_indices[i]] = MathHelper::Vec3ToVec3f(bitangent);
            m_tangents[m_indices[i + 1]] = MathHelper::Vec4ToVec4f(tangent4);
            m_bitangents[m_indices[i + 1]] = MathHelper::Vec3ToVec3f(bitangent);
            m_tangents[m_indices[i + 2]] = MathHelper::Vec4ToVec4f(tangent4);
            m_bitangents[m_indices[i + 2]] = MathHelper::Vec3ToVec3f(bitangent);
        }
    }

    const AZ::u32 GNMeshData::VertexCount() const
    {
        return aznumeric_cast<AZ::u32>(m_indices.size());
    }

    const U32Vector& GNMeshData::GetIndices() const
    {
        return m_indices;
    }

    const Vert3Vector& GNMeshData::GetPositions() const
    {
        return m_positions;
    }

    const Vert3Vector& GNMeshData::GetNormals() const
    {
        return m_normals;
    }

    const Vert4Vector& GNMeshData::GetTangents() const
    {
        return m_tangents;
    }

    const Vert3Vector& GNMeshData::GetBitangents() const
    {
        return m_bitangents;
    }

    const Vert2Vector& GNMeshData::GetUVs() const
    {
        return m_uvs;
    }

    const Vert4Vector& GNMeshData::GetColors() const
    {
        return m_colors;
    }

    const Mat4Vector& GNMeshData::GetInstances() const
    {
        return m_instances;
    }

    const AZ::Matrix4x4 GNMeshData::GetTransform() const
    {
        return m_transform;
    }

    const AZ::Transform GNMeshData::GetO3DETransform() const
    {
        return m_o3deTransform;
    }

    const AZ::Vector3 GNMeshData::GetO3DEScale() const
    {
        return m_o3deScale;
    }

    void GNMeshData::GetO3DETransformAndScale(const AZ::Matrix4x4& mat4Transform, AZ::Transform& transform, AZ::Vector3& scale)
    {
        AZ::Quaternion o3deQuarternion = AZ::Quaternion::CreateFromMatrix4x4(mat4Transform);
        AZ::Vector3 o3deTranslation = mat4Transform.GetTranslation();
        scale = mat4Transform.RetrieveScale();
        transform = AZ::Transform::CreateFromQuaternionAndTranslation(o3deQuarternion, o3deTranslation);
    }

    void GNMeshData::AddInstance(const AZ::Matrix4x4& mat4)
    {
        m_instances.emplace_back(mat4);
    }

    void GNMeshData::SetTransform(const AZ::Matrix4x4& transform)
    {
        m_transform = transform;

        auto transposedMatrix = m_transform.GetTranspose();

        // removed the scale from the 4x4 matrix first before creating the rotation. Note: Matrix4x4 has an ExtractScale method but it doesn't seem to worn well with non-uniform scale.
        m_o3deScale = MathHelper::ExtractScalingFromMatrix44(transposedMatrix);
        auto rotation = AZ::Quaternion::CreateFromMatrix4x4(transposedMatrix);
        rotation = AZ::Quaternion(rotation.GetX(), rotation.GetY(), -rotation.GetZ(), rotation.GetW()); // Flip the Z rotation.
        rotation.Normalize();

        auto translation = m_transform.GetTranslation();
        m_o3deTransform = AZ::Transform::CreateFromQuaternionAndTranslation(rotation, translation);
    }

    AZ::Aabb GNMeshData::GetAabb() const
    {
        return m_aabb;
    }

    AZ::s64 GNMeshData::GetHash() const
    {
        return m_hash;
    }
} // namespace GeomNodes
