#pragma once

#include <Editor/Rendering/GNMeshData.h>
#include <AzCore/std/containers/map.h>

namespace GeomNodes
{
    class GNModelData
    {
    public:
        using MeshDataList = AZStd::vector<GNMeshData>;
        using AssignedMeshMap = AZStd::map<AZ::u64, AZ::u32>;

        GNModelData();
        GNModelData(AZ::u64 mapId);
        ~GNModelData() = default;

        void ReadData(AZ::u64 mapId);

        const AZ::u32 MeshCount() const;
        const MeshDataList GetMeshes() const;

        GNMeshData GetMeshData(AZ::u64 entityId);
        void AssignMeshData(AZ::u64 entityId);

        void SetMaterialPathFormat(const AZStd::string& materialPathFormat);
    private:
        template<typename T>
        AZStd::vector<T> ReadArray(AZ::u64 mapId);

        template<typename T>
        T Read(AZ::u64 mapId);

        MeshDataList m_meshes;
        AssignedMeshMap m_assignedMeshmap;

        AZStd::string m_materialPathFormat;
    };
}
