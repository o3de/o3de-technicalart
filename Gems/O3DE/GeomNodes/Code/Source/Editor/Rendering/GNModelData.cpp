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
				int materialIdx = 0;
				for (auto& materialName : mesh.GetMaterials())
				{
					auto indices = mesh.GetIndicesByMaterialIndex(materialIdx);
					if (indices.size() > 0) {
						auto& meshInstance = meshMap[materialName].emplace_back(mesh);
						meshInstance.SetIndices(indices);
						meshInstance.SetMaterial(materialName);
						meshInstance.CalculateTangents();
						meshInstance.ClearMaterialList();
					}
					materialIdx++;
				}
			}

			for ([[maybe_unused]] AZ::s32 instanceIdx = 0; instanceIdx < instanceCount; instanceIdx++)
			{
				AZ::s64 hash = Read<AZ::s64>(mapId);

				[[maybe_unused]] AZ::Matrix4x4 LocalMatrix = Read<AZ::Matrix4x4>(mapId);
				[[maybe_unused]] AZ::Matrix4x4 WorldMatrix = Read<AZ::Matrix4x4>(mapId);
				AZ::Matrix4x4 instanceMatrix = LocalMatrix;

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
			for (auto& meshGroup : meshMap)
			{
				GNMeshData meshData;
				for (const auto& mesh : meshGroup.second)
				{
					meshData += mesh;
				}

				if (meshData.VertexCount() > 0)
				{
					meshData.CalculateAABB();
					m_meshes.push_back(meshData);
				}
			}

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
