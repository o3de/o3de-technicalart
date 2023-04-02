#include <Editor/Rendering/GNModelData.h>
#include <Editor/Systems/GNParamContext.h>
#include <AzCore/std/containers/set.h>
#include <AzCore/std/containers/map.h>
#include "Bridge.h"

namespace GeomNodes
{
    GNModelData::GNModelData()
    {
    }

    GNModelData::GNModelData(AZ::u64 mapId)
    {
		ReadData(mapId);
    }

    void GNModelData::ReadData(AZ::u64 mapId)
    {
		m_meshes.clear();
		m_assignedMeshmap.clear();

		if (OpenSHM(mapId))
		{
			AZ::s32 meshCount = Read<AZ::s32>(mapId);
			AZ::s32 instanceCount = Read<AZ::s32>(mapId);

			AZ_Printf("GNInstance", "meshCount : %i, instanceCount : %i", meshCount, instanceCount);

			AZStd::map<AZStd::string, AZStd::vector<GNMeshData>> meshMap;
			for ([[maybe_unused]] AZ::s32 meshIdx = 0; meshIdx < meshCount; meshIdx++)
			{
				auto positions = ReadArray<Vector3f>(mapId);
				auto normals = ReadArray<Vector3f>(mapId);
				auto indices = ReadArray<AZ::s32>(mapId);
				auto triangleLoops = ReadArray<AZ::s32>(mapId);
				auto loops = ReadArray<AZ::s32>(mapId);
				auto hash = Read<AZ::s64>(mapId);

				auto colors = ReadArray<Vector4f>(mapId);
				bool bIndexedColors = Read<bool>(mapId);
				auto uvs = ReadArray<Vector2f>(mapId);
				bool bIndexedUVs = Read<bool>(mapId);
				auto materialIndices = ReadArray<AZ::s32>(mapId);
				auto materials = ReadArray<char>(mapId);

				if (positions.empty())
				{
					continue;
				}

				MaterialList materialList;
				rapidjson::Document jsonDocument;
				jsonDocument.Parse((const char*)materials.data(), materials.size());
				if (!jsonDocument.HasParseError())
				{
					if (jsonDocument.HasMember(Field::Materials))
					{
						for (auto& entry : jsonDocument[Field::Materials].GetArray())
						{
							const auto materialName = entry.GetString();
							materialList.push_back(materialName);
							if (meshMap.find(materialName) == meshMap.end()) // check if we have the key in the map
							{
								meshMap.emplace(AZStd::make_pair(materialName, AZStd::vector<GNMeshData>({})));
							}
						}
					}
				}

				GNMeshData meshData(positions, normals, indices, triangleLoops, loops, uvs, colors, materialIndices, materialList, hash, bIndexedUVs, bIndexedColors);
				m_meshes.push_back(meshData);
			}

			// at this stage meshes can have multiple materials and we need to create their own mesh copy 
			// but with a different indices based on the material indices.
			for (auto& mesh : m_meshes)
			{
				int materialMeshIndex = 0;
				for (auto& materialName : mesh.GetMaterials())
				{
					auto indices = mesh.GetIndicesByMaterialIndex(materialMeshIndex);
					if (indices.size() > 0) {
						auto& meshInstance = meshMap[materialName].emplace_back(mesh);
						meshInstance.SetIndices(indices);
						meshInstance.CalculateTangents();
						meshInstance.ClearMaterialList();
					}
					materialMeshIndex++;
				}
			}

			for ([[maybe_unused]] AZ::s32 instanceIdx = 0; instanceIdx < instanceCount; instanceIdx++)
			{
				AZ::s64 hash = Read<AZ::s64>(mapId);

				[[maybe_unused]] AZ::Matrix4x4 localMatrix = Read<AZ::Matrix4x4>(mapId);
				[[maybe_unused]] AZ::Matrix4x4 worldMatrix = Read<AZ::Matrix4x4>(mapId);
				
				AZ::Matrix3x3 rotateZ = AZ::Matrix3x3::CreateRotationZ(AZ::DegToRad(180.0f));
				
				auto rotate4x4 = AZ::Matrix4x4::CreateFromMatrix3x4(AZ::Matrix3x4::CreateFromMatrix3x3(rotateZ));
				AZ::Matrix4x4 instanceMatrix = rotate4x4 * localMatrix;

				for (auto& meshGroup : meshMap)
				{
					for (auto& mesh : meshGroup.second)
					{
						if (mesh.GetHash() == hash)
						{
							mesh.AddInstance(instanceMatrix);
						}
					}
				}
			}

			m_meshes.clear();

			// merge all meshes in each mesh group along with their instances
			AZ::u32 materialIndex = 0;
			for (auto& meshGroup : meshMap)
			{
				GNMeshData meshData;
				for (const auto& mesh : meshGroup.second)
				{
					meshData += mesh;
				}

				if (meshData.GetCount<AttributeType::Position>() > 0)
				{
					meshData.SetMaterialIndex(materialIndex);
					meshData.CalculateAABB();
					m_aabb.AddAabb(meshData.GetAabb()); // add mesh data's aabb to get the aabb for the whole model
					m_meshes.push_back(meshData);
				}

				materialIndex++;
			}

			// Convert to only one buffers/arrays and keep track of the offsets and element counts
			MergeMeshBuffers();

			ClearSHM(mapId);
		}
    }

    const AZ::u32 GNModelData::MeshCount() const
    {
        return aznumeric_cast<AZ::u32>(m_meshes.size());
    }

    const GNModelData::MeshDataList GNModelData::GetMeshes() const
    {
        return m_meshes;
    }

	GNMeshData GNModelData::GetMeshData(AZ::u64 entityId)
	{
		auto iter = m_assignedMeshmap.find(entityId);
		AZ_Assert(iter != m_assignedMeshmap.end(), "Mesh data is not assigned yet. Check the entity id.");
		
		return m_meshes[iter->second];
	}

	void GNModelData::AssignMeshData(AZ::u64 entityId)
	{
		m_assignedMeshmap.emplace(AZStd::make_pair(entityId, aznumeric_cast<AZ::u32>(m_assignedMeshmap.size())));
	}

	const U32Vector& GNModelData::GetIndices() const
	{
		return m_indices;
	}

	const Vert3Vector& GNModelData::GetPositions() const
	{
		return m_positions;
	}

	const Vert3Vector& GNModelData::GetNormals() const
	{
		return m_normals;
	}

	const Vert4Vector& GNModelData::GetTangents() const
	{
		return m_tangents;
	}

	const Vert3Vector& GNModelData::GetBitangents() const
	{
		return m_bitangents;
	}

	const Vert2Vector& GNModelData::GetUVs() const
	{
		return m_uvs;
	}

	const Vert4Vector& GNModelData::GetColors() const
	{
		return m_colors;
	}

	AZ::Aabb GNModelData::GetAabb() const
	{
		return m_aabb;
	}

	void GNModelData::MergeMeshBuffers()
	{
		m_indices.clear();
		m_positions.clear();
		m_normals.clear();
		m_tangents.clear();
		m_bitangents.clear();
		m_uvs.clear();
		m_colors.clear();

		for (auto& mesh : m_meshes)
		{
			mesh.SetIndexOffset(m_indices.size());
			m_indices.insert(m_indices.end(), mesh.GetIndices().begin(), mesh.GetIndices().end());

			mesh.SetOffset<AttributeType::Position>(m_positions.size());
			m_positions.insert(m_positions.end(), mesh.GetDataBuffer<AttributeType::Position>().begin(), mesh.GetDataBuffer<AttributeType::Position>().end());

			mesh.SetOffset<AttributeType::Normal>(m_normals.size());
			m_normals.insert(m_normals.end(), mesh.GetDataBuffer<AttributeType::Normal>().begin(), mesh.GetDataBuffer<AttributeType::Normal>().end());

			mesh.SetOffset<AttributeType::Tangent>(m_tangents.size());
			m_tangents.insert(m_tangents.end(), mesh.GetDataBuffer<AttributeType::Tangent>().begin(), mesh.GetDataBuffer<AttributeType::Tangent>().end());

			mesh.SetOffset<AttributeType::Bitangent>(m_bitangents.size());
			m_bitangents.insert(m_bitangents.end(), mesh.GetDataBuffer<AttributeType::Bitangent>().begin(), mesh.GetDataBuffer<AttributeType::Bitangent>().end());

			mesh.SetOffset<AttributeType::UV>(m_uvs.size());
			m_uvs.insert(m_uvs.end(), mesh.GetDataBuffer<AttributeType::UV>().begin(), mesh.GetDataBuffer<AttributeType::UV>().end());

			mesh.SetOffset<AttributeType::Color>(m_colors.size());
			m_colors.insert(m_colors.end(), mesh.GetDataBuffer<AttributeType::Color>().begin(), mesh.GetDataBuffer<AttributeType::Color>().end());

			// clear the buffers since we already copied them.
			mesh.ClearBuffers();
		}
	}

    template<typename T>
    AZStd::vector<T> GNModelData::ReadArray(AZ::u64 mapId)
    {
        AZ::u64 length;
        void* address;
        ReadSHM(mapId, &address, &length);

        T* array = static_cast<T*>(address);
        return AZStd::vector<T>(array, array + (length / sizeof(T)));
    }

    template<typename T>
    T GNModelData::Read(AZ::u64 mapId)
    {
        AZ::u64 length;
        void* address;
        ReadSHM(mapId, &address, &length);
        T value{};
        memcpy(&value, address, length);
        return value;
    }
} // namespace GeomNodes
