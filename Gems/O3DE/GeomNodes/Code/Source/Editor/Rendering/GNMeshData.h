#pragma once

#include <Editor/Common/GNConstants.h>
#include <Editor/Math/MathHelper.h>
#include <AzCore/Math/Aabb.h>
#include <AZCore/std/string/string.h>

namespace GeomNodes
{
    using MaterialList = AZStd::vector<AZStd::string>;

    class GNMeshData
    {
    public:
        struct DataRange
        {
            AZ::u32 offset = 0; 
            AZ::u32 count = 0;
        };

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

        AZ::u32 GetIndexCount() const;
        AZ::u32 GetIndexOffset() const;
        void SetIndexOffset(AZ::u32 offset);
        void SetIndexCount(AZ::u32 count);

		template<AttributeType AttributeTypeT>
		AZ::u32 GetCount() const;
		template<AttributeType AttributeTypeT>
		AZ::u32 GetOffset() const;
		template<AttributeType AttributeTypeT>
		void SetCount(AZ::u32 count);
		template<AttributeType AttributeTypeT>
		void SetOffset(AZ::u32 offset);

        const U32Vector& GetIndices() const;
        void SetIndices(const U32Vector& indices);

        template<AttributeType AttributeTypeT>
        const auto& GetDataBuffer() const;
        
        const Mat4Vector& GetInstances() const;
        const MaterialList& GetMaterials() const;
        void ClearMaterialList();
        void SetMaterialIndex(AZ::u32 materialIndex);
        AZ::u32 GetMaterialIndex() const;
        void AddInstance(const AZ::Matrix4x4& mat4);
		
        U32Vector GetIndicesByMaterialIndex(int materialIndex);

        AZ::Aabb GetAabb() const;
        AZ::s64 GetHash() const;
        void CalculateTangents();
        void CalculateAABB();

        GNMeshData& operator+=(const GNMeshData& rhs);

        void ClearBuffers();

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
        
        DataRange m_indicesRange;
        DataRange m_positionsRange;
        DataRange m_normalsRange;
        DataRange m_tangentsRange;
        DataRange m_bitangentsRange;
        DataRange m_uvsRange;
        DataRange m_colorsRange;

        MaterialList m_materialNames;

        AZ::u32 m_materialIndex = 0;

        AZ::Aabb m_aabb = AZ::Aabb::CreateNull();
        AZ::s64 m_hash = 0;

        Mat4Vector m_instances;
    };
}
