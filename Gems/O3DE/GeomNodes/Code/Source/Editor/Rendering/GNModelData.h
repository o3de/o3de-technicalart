#pragma once

#include <Editor/Rendering/GNMeshData.h>

namespace GeomNodes
{
    class GNModelData
    {
    public:
        using MeshList = AZStd::vector<GNMeshData>;

        GNModelData();
        GNModelData(AZ::u64 mapId);
        ~GNModelData() = default;

        const AZ::u32 MeshCount() const;
        const MeshList GetMeshes() const;

    private:
        template<typename T>
        AZStd::vector<T> ReadArray(AZ::u64 mapId);

        template<typename T>
        T Read(AZ::u64 mapId);

        MeshList m_meshes;
    };
}
