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
				uniqueKeys.emplace(AZStd::make_pair<UniqueKey, AZ::s32>(AZStd::move(key), AZStd::move(i)));
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

	template<AttributeType AttributeTypeT>
	AZ::u32 GNMeshData::GetCount() const
    {
		if constexpr (AttributeTypeT == AttributeType::Position) {
			return aznumeric_cast<AZ::u32>(m_positionsRange.count);
		}
		else if constexpr (AttributeTypeT == AttributeType::Normal) {
            return aznumeric_cast<AZ::u32>(m_normalsRange.count);
		}
		else if constexpr (AttributeTypeT == AttributeType::Tangent) {
            return aznumeric_cast<AZ::u32>(m_tangentsRange.count);
		}
		else if constexpr (AttributeTypeT == AttributeType::Bitangent) {
            return aznumeric_cast<AZ::u32>(m_bitangentsRange.count);
		}
		else if constexpr (AttributeTypeT == AttributeType::UV) {
            return aznumeric_cast<AZ::u32>(m_uvsRange.count);
		}
		else if constexpr (AttributeTypeT == AttributeType::Color) {
            return aznumeric_cast<AZ::u32>(m_colorsRange.count);
		}
    }

    template<AttributeType AttributeTypeT>
    AZ::u32 GNMeshData::GetOffset() const
    {
		if constexpr (AttributeTypeT == AttributeType::Position) {
			return m_positionsRange.offset;
		}
		else if constexpr (AttributeTypeT == AttributeType::Normal) {
			return m_normalsRange.offset;
		}
		else if constexpr (AttributeTypeT == AttributeType::Tangent) {
			return m_tangentsRange.offset;
		}
		else if constexpr (AttributeTypeT == AttributeType::Bitangent) {
			return m_bitangentsRange.offset;
		}
		else if constexpr (AttributeTypeT == AttributeType::UV) {
			return m_uvsRange.offset;
		}
		else if constexpr (AttributeTypeT == AttributeType::Color) {
			return m_colorsRange.offset;
		}
    }

    template<AttributeType AttributeTypeT>
    void GNMeshData::SetCount(AZ::u32 count)
    {
		if constexpr (AttributeTypeT == AttributeType::Position) {
			m_positionsRange.count = count;
		}
		else if constexpr (AttributeTypeT == AttributeType::Normal) {
			m_normalsRange.count = count;
		}
		else if constexpr (AttributeTypeT == AttributeType::Tangent) {
			m_tangentsRange.count = count;
		}
		else if constexpr (AttributeTypeT == AttributeType::Bitangent) {
			m_bitangentsRange.count = count;
		}
		else if constexpr (AttributeTypeT == AttributeType::UV) {
			m_uvsRange.count = count;
		}
		else if constexpr (AttributeTypeT == AttributeType::Color) {
			m_colorsRange.count = count;
		}
    }

    template<AttributeType AttributeTypeT>
    void GNMeshData::SetOffset(AZ::u32 offset)
    {
		if constexpr (AttributeTypeT == AttributeType::Position) {
			m_positionsRange.offset = offset;
		}
		else if constexpr (AttributeTypeT == AttributeType::Normal) {
			m_normalsRange.offset = offset;
		}
		else if constexpr (AttributeTypeT == AttributeType::Tangent) {
			m_tangentsRange.offset = offset;
		}
		else if constexpr (AttributeTypeT == AttributeType::Bitangent) {
			m_bitangentsRange.offset = offset;
		}
		else if constexpr (AttributeTypeT == AttributeType::UV) {
			m_uvsRange.offset = offset;
		}
		else if constexpr (AttributeTypeT == AttributeType::Color) {
			m_colorsRange.offset = offset;
		}
    }

    template<AttributeType AttributeTypeT>
    const auto& GNMeshData::GetDataBuffer() const
    {
		if constexpr (AttributeTypeT == AttributeType::Position) {
			return m_positions;
		}
		else if constexpr (AttributeTypeT == AttributeType::Normal) {
			return m_normals;
		}
		else if constexpr (AttributeTypeT == AttributeType::Tangent) {
			return m_tangents;
		}
		else if constexpr (AttributeTypeT == AttributeType::Bitangent) {
			return m_bitangents;
		}
		else if constexpr (AttributeTypeT == AttributeType::UV) {
			return m_uvs;
		}
		else if constexpr (AttributeTypeT == AttributeType::Color) {
			return m_colors;
		}
    }

    AZ::u32 GNMeshData::GetIndexCount() const
    {
        return m_indicesRange.count;
    }

    AZ::u32 GNMeshData::GetIndexOffset() const
    {
        return m_indicesRange.offset;
    }

    void GNMeshData::SetIndexOffset(AZ::u32 offset)
    {
        m_indicesRange.offset = offset;
    }

    void GNMeshData::SetIndexCount(AZ::u32 count)
    {
        m_indicesRange.count = count;
    }

    const U32Vector& GNMeshData::GetIndices() const
    {
        return m_indices;
    }

    void GNMeshData::SetIndices(const U32Vector& indices)
    {
        m_indices = indices;
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

    void GNMeshData::SetMaterialIndex(AZ::u32 materialIndex)
    {
        m_materialIndex = materialIndex;
    }

	AZ::u32 GNMeshData::GetMaterialIndex() const
	{
		return m_materialIndex;
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
        AZ::u32 count = aznumeric_cast<AZ::u32>(m_positions.max_size() + rhs.m_positions.size() * rhs.m_instances.size());
        m_indices.reserve(count * 3);
        m_positions.reserve(count);
        m_normals.reserve(count);
        m_tangents.reserve(count);
        m_bitangents.reserve(count);
        m_uvs.reserve(count);
        m_colors.reserve(count);

        for (const auto& instance : rhs.m_instances)
        {
            AZ::s32 indexOffsset = aznumeric_cast<AZ::u32>(m_positions.size());
            U32Vector rhsIndices = rhs.m_indices;

            for (AZ::u32& index : rhsIndices)
            {
                index += indexOffsset;
            }

            m_indices.insert(m_indices.end(), rhsIndices.begin(), rhsIndices.end());
            SetIndexCount(m_indices.size());

            Vert3Vector rhsPositions = rhs.m_positions;
            for (auto& position : rhsPositions)
            {
                auto vec3 = instance * MathHelper::Vec3fToVec3(position);
                position = MathHelper::Vec3ToVec3f(vec3);
            }

            m_positions.insert(m_positions.end(), rhsPositions.begin(), rhsPositions.end());
            SetCount<AttributeType::Position>(m_positions.size());

            Vert3Vector rhsNormals = rhs.m_normals;
            auto normalMatrix = instance.GetInverseFull();
            normalMatrix.Transpose();
            for (auto& normal : rhsNormals)
            {
                auto vec3 = normalMatrix * MathHelper::Vec3fToVec3(normal);
                vec3.Normalize();
                normal = MathHelper::Vec3ToVec3f(vec3);
            }

            m_normals.insert(m_normals.end(), rhsNormals.begin(), rhsNormals.end());
            SetCount<AttributeType::Normal>(m_normals.size());

            m_colors.insert(m_colors.end(), rhs.m_colors.begin(), rhs.m_colors.end());
            SetCount<AttributeType::Color>(m_colors.size());

            m_uvs.insert(m_uvs.end(), rhs.m_uvs.begin(), rhs.m_uvs.end());
            SetCount<AttributeType::UV>(m_uvs.size());
            m_tangents.insert(m_tangents.end(), rhs.m_tangents.begin(), rhs.m_tangents.end());
            SetCount<AttributeType::Tangent>(m_tangents.size());
            m_bitangents.insert(m_bitangents.end(), rhs.m_bitangents.begin(), rhs.m_bitangents.end());
            SetCount<AttributeType::Bitangent>(m_bitangents.size());
        }

        return *this;
    }

    void GNMeshData::ClearBuffers()
    {
		m_indices.clear();
		m_loops.clear();
		m_materialIndices.clear();

		m_positions.clear();
		m_normals.clear();
		m_tangents.clear();
		m_bitangents.clear();
		m_uvs.clear();
		m_colors.clear();

		m_instances.clear();
    }
} // namespace GeomNodes
