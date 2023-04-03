#pragma once

#include <Editor/Rendering/GNMeshData.h>
#include <AzCore/std/containers/map.h>

namespace GeomNodes
{
    class GNModelData
    {
    public:
        using MeshDataList = AZStd::vector<GNMeshData>;
        
        GNModelData();
        GNModelData(AZ::u64 mapId);
        ~GNModelData() = default;

        void ReadData(AZ::u64 mapId);

        const AZ::u32 MeshCount() const;
        const MeshDataList GetMeshes() const;
        MaterialList GetMaterials();

        // These functions are the combination of all mesh buffers in order based on their position in MeshDataList
		const U32Vector& GetIndices() const;
		const Vert3Vector& GetPositions() const;
		const Vert3Vector& GetNormals() const;
		const Vert4Vector& GetTangents() const;
		const Vert3Vector& GetBitangents() const;
		const Vert2Vector& GetUVs() const;
		const Vert4Vector& GetColors() const;
		
        AZ::Aabb GetAabb() const;

    private:
        void MergeMeshBuffers();

        template<typename T>
        AZStd::vector<T> ReadArray(AZ::u64 mapId);

        template<typename T>
        T Read(AZ::u64 mapId);

        MeshDataList m_meshes;
        MaterialList m_materials;

        // These are all data combined from the meshes in single arrays
		U32Vector m_indices;

		Vert3Vector m_positions;
		Vert3Vector m_normals;
		Vert4Vector m_tangents;
		Vert3Vector m_bitangents;
		Vert2Vector m_uvs;
		Vert4Vector m_colors;

        AZ::Aabb m_aabb = AZ::Aabb::CreateNull();
    };
}
