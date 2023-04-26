/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AZCore/std/string/string.h>
#include <AzCore/Math/Aabb.h>
#include <Editor/Common/GNConstants.h>
#include <Editor/Math/MathHelper.h>


namespace GeomNodes
{
    using MaterialList = AZStd::vector<AZStd::string>;

    //! Mesh specific data class
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
            bool isIndexedColors);

        ~GNMeshData() = default;

        AZ::u32 GetIndexCount() const;
        AZ::u32 GetIndexOffset() const;
        void SetIndexOffset(AZ::u32 offset);
        void SetIndexOffset(AZ::u64 offset);
        void SetIndexCount(AZ::u32 count);
        void SetIndexCount(AZ::u64 count);

        template<AttributeType AttributeTypeT>
        AZ::u32 GetCount() const;
        template<AttributeType AttributeTypeT>
        AZ::u32 GetOffset() const;
        template<AttributeType AttributeTypeT>
        void SetCount(AZ::u32 count);
        template<AttributeType AttributeTypeT>
        void SetCount(AZ::u64 count);
        template<AttributeType AttributeTypeT>
        void SetOffset(AZ::u32 offset);
        template<AttributeType AttributeTypeT>
        void SetOffset(AZ::u64 offset);

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

        //! hash are used to easily matched if you have the same mesh.
        AZ::s64 m_hash = 0;
        //! List of material names used by the mesh.
        MaterialList m_materialNames;
        //! List of instances. Instances are used so that you have the same mesh but on a different transform.
        Mat4Vector m_instances;

        AZ::Aabb m_aabb = AZ::Aabb::CreateNull();

        Vert3Vector m_positions;
        Vert3Vector m_normals;
        Vert4Vector m_tangents;
        Vert3Vector m_bitangents;
        Vert2Vector m_uvs;
        Vert4Vector m_colors;

        //! As meshes are eventually merged to common buffers we need to keep track of the offset and element count.
        //! When we create the Atom Buffers we will need these infos while the mesh buffers will be held in GNModelData.
        DataRange m_indicesRange;
        DataRange m_positionsRange;
        DataRange m_normalsRange;
        DataRange m_tangentsRange;
        DataRange m_bitangentsRange;
        DataRange m_uvsRange;
        DataRange m_colorsRange;

        //! Material index or slot that will be used by the mesh. Note that eventually a mesh will only be assigned to one material.
        //! This way they can be grouped together by material and merge the vertices.
        AZ::u32 m_materialIndex = 0;
    };
} // namespace GeomNodes
