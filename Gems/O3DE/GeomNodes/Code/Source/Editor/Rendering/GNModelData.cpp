#include <Editor/Rendering/GNModelData.h>
#include "Bridge.h"

namespace GeomNodes
{
    GNModelData::GNModelData()
    {
    }

    GNModelData::GNModelData(AZ::u64 mapId)
    {
        if (OpenSHM(mapId))
        {
            AZ::s32 meshCount = Read<AZ::s32>(mapId);
            AZ::s32 instanceCount = Read<AZ::s32>(mapId);

            AZ_Printf("GNInstance", "meshCount : %i, instanceCount : %i", meshCount, instanceCount);

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
                [[maybe_unused]] auto UsedMaterialsJson = ReadArray<char>(mapId);

                if (positions.empty())
                {
                    continue;
                }
                GNMeshData meshData(positions, normals, indices, triangleLoops, loops, uvs, colors, materialIndices, hash, bIndexedUVs, bIndexedColors);
                m_meshes.push_back(meshData);
            }

            for ([[maybe_unused]] AZ::s32 instanceIdx = 0; instanceIdx < instanceCount; instanceIdx++)
            {
                AZ::s64 hash = Read<AZ::s64>(mapId);

                [[maybe_unused]] AZ::Matrix4x4 LocalMatrix = Read<AZ::Matrix4x4>(mapId);
                [[maybe_unused]] AZ::Matrix4x4 WorldMatrix = Read<AZ::Matrix4x4>(mapId);
                AZ::Matrix4x4 instanceMatrix = LocalMatrix;
                
                for (auto& mesh : m_meshes)
                {
                    if (mesh.GetHash() == hash)
                    {
                        mesh.AddInstance(instanceMatrix);
                    }
                }
            }

            // process the mesh instances

            AZStd::vector<GNMeshData> meshInstances;
            for (auto& mesh : m_meshes)
            {
                int idx = 0;
                for (auto& instance : mesh.GetInstances())
                {
                    if (idx == 0)
                    {
                        mesh.SetTransform(instance);
                    }
                    else
                    {
                        auto& meshInstance = meshInstances.emplace_back(mesh);
                        meshInstance.SetTransform(instance);
                    }

                    idx++;
                }
            }

            m_meshes.insert(m_meshes.end(), meshInstances.begin(), meshInstances.end());

            ClearSHM(mapId);
        }
    }

    const AZ::u32 GNModelData::MeshCount() const
    {
        return aznumeric_cast<AZ::u32>(m_meshes.size());
    }

    const GNModelData::MeshList GNModelData::GetMeshes() const
    {
        return m_meshes;
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
