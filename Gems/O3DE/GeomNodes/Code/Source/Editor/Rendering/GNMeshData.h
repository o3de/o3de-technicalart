#pragma once

#include <Editor/Math/MathHelper.h>
#include <AzCore/Math/Aabb.h>
#include <AZCore/std/string/string.h>

namespace GeomNodes
{
    using MaterialList = AZStd::vector<AZStd::string>;

    class GNMeshData
    {
    public:
        GNMeshData();
        explicit GNMeshData(
            const Vert3Vector& positions
            , const Vert3Vector& normals
            , const S32Vector& indices
            , const S32Vector& triangleLoops
            , const S32Vector& loops
            , const Vert2Vector& uvs
            , const Vert4Vector& colors
            , const S32Vector& materialIndices
            , const MaterialList& materials
            , AZ::u64 hash
            , bool isIndexedUVs
            , bool isIndexedColors);

        ~GNMeshData() = default;

        const AZ::u32 VertexCount() const;
        const U32Vector& GetIndices() const;
        void SetIndices(const U32Vector& indices);
        const Vert3Vector& GetPositions() const;
        const Vert3Vector& GetNormals() const;
        const Vert4Vector& GetTangents() const;
        const Vert3Vector& GetBitangents() const;
        const Vert2Vector& GetUVs() const;
        const Vert4Vector& GetColors() const;
        const Mat4Vector& GetInstances() const;
        const MaterialList& GetMaterials() const;
        void ClearMaterialList();
        void SetMaterial(AZStd::string materialName);
        AZStd::string GetMaterial();
        void AddInstance(const AZ::Matrix4x4& mat4);
		
        U32Vector GetIndicesByMaterialIndex(int materialIndex);

        AZ::Aabb GetAabb() const;
        AZ::s64 GetHash() const;
        void CalculateTangents();
        void CalculateAABB();

        GNMeshData& operator+=(const GNMeshData& rhs);

    private:
        U32Vector m_indices;
        S32Vector m_loops;
        S32Vector m_materialIndices;

        Vert3Vector m_positions;
        Vert3Vector m_normals;
        Vert4Vector m_tangents;
        Vert3Vector m_bitangents;
        Vert2Vector m_uvs;
        Vert4Vector m_colors;
        
        MaterialList m_materialNames;

        AZStd::string m_materialName;
        
        AZ::Aabb m_aabb = AZ::Aabb::CreateNull();
        AZ::s64 m_hash = 0;

        Mat4Vector m_instances;
    };
}
