#include <Editor/Rendering/GNMeshData.h>
#include <AzCore/std/containers/unordered_map.h>
#include "Bridge.h"

namespace GeomNodes
{
    GNMeshData::GNMeshData()
    {
    }

    GNMeshData::GNMeshData(
        const Vert3Vector& positions,
        const Vert3Vector& normals,
        const S32Vector& indices,
        const S32Vector& triangleLoops,
        const S32Vector& loops,
        const Vert2Vector& uvs,
        const Vert4Vector& colors,
        const S32Vector& materialIndices,
        const MaterialList& materials,
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
        
        m_materialIndices = materialIndices;
        m_materialNames = materials;

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

            // Calculate delta m_uvs
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

    void GNMeshData::CalculateAABB()
    {
        // calculate the aabb
        for (const auto& vert : m_positions)
        {
            m_aabb.AddPoint(AZ::Vector3(vert[0], vert[1], vert[2]));
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

    void GNMeshData::SetIndices(const U32Vector& indices)
    {
        m_indices = indices;
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

    const MaterialList& GNMeshData::GetMaterials() const
    {
        return m_materialNames;
    }

    void GNMeshData::ClearMaterialList()
    {
        m_materialNames.clear();
    }

    void GNMeshData::SetMaterial(const AZStd::string& materialName, const AZStd::string& materialPath)
    {
        m_materialName = materialName;
        m_materialPath = materialPath;
    }

    AZStd::string GNMeshData::GetMaterial() const
    {
        return m_materialName;
    }

    AZStd::string GNMeshData::GetMaterialPath() const
    {
        return m_materialPath;
    }

    void GNMeshData::AddInstance(const AZ::Matrix4x4& mat4)
    {
        m_instances.emplace_back(mat4);
    }

    U32Vector GNMeshData::GetIndicesByMaterialIndex(int materialIndex)
    {
        U32Vector indices;
        indices.reserve(m_indices.size());

        for (AZ::s32 i = 0; i < m_indices.size(); i += 3)
        {
            const AZ::s32 faceIndex = i / 3;
            if (m_materialIndices[faceIndex] == materialIndex)
            {
                indices.push_back(m_indices[i]);
                indices.push_back(m_indices[i + 1]);
                indices.push_back(m_indices[i + 2]);
            }
        }

        return indices;
    }

    AZ::Aabb GNMeshData::GetAabb() const
    {
        return m_aabb;
    }

    AZ::s64 GNMeshData::GetHash() const
    {
        return m_hash;
    }

    GNMeshData& GNMeshData::operator+=(const GNMeshData& rhs)
    {
        m_materialName = rhs.m_materialName;
        m_materialPath = rhs.m_materialPath;

        AZ::u32 count = m_positions.max_size() + rhs.m_positions.size() * rhs.m_instances.size();
        m_indices.reserve(count * 3);
        m_positions.reserve(count);
        m_normals.reserve(count);
        m_tangents.reserve(count);
        m_bitangents.reserve(count);
        m_uvs.reserve(count);
        m_colors.reserve(count);

        for (const auto& instance : rhs.m_instances)
        {
            AZ::s32 indexOffsset = m_positions.size();
            U32Vector rhsIndices = rhs.m_indices;

            for (AZ::u32& index : rhsIndices)
            {
                index += indexOffsset;
            }

            m_indices.insert(m_indices.end(), rhsIndices.begin(), rhsIndices.end());

            Vert3Vector rhsPositions = rhs.m_positions;
            for (auto& position : rhsPositions)
            {
                auto vec3 = instance * MathHelper::Vec3fToVec3(position);
                position = MathHelper::Vec3ToVec3f(vec3);
            }

            m_positions.insert(m_positions.end(), rhsPositions.begin(), rhsPositions.end());

            Vert3Vector rhsNormals = rhs.m_normals;
            for (auto& normal : rhsNormals)
            {
                auto vec3 = instance * MathHelper::Vec3fToVec3(normal);
                vec3.Normalize();
                normal = MathHelper::Vec3ToVec3f(vec3);
            }

            m_normals.insert(m_normals.end(), rhsNormals.begin(), rhsNormals.end());

            m_colors.insert(m_colors.end(), rhs.m_colors.begin(), rhs.m_colors.end());
            m_uvs.insert(m_uvs.end(), rhs.m_uvs.begin(), rhs.m_uvs.end());
            m_tangents.insert(m_tangents.end(), rhs.m_tangents.begin(), rhs.m_tangents.end());
            m_bitangents.insert(m_bitangents.end(), rhs.m_bitangents.begin(), rhs.m_bitangents.end());
        }

        return *this;
    }
} // namespace GeomNodes
