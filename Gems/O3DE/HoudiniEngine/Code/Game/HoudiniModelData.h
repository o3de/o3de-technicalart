/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/std/containers/map.h>
#include <Game/HoudiniMeshData.h>


namespace HoudiniEngine
{
    //! Handle parsing, handling and storing model data.
    class HoudiniModelData
    {
    public:
        friend class HoudiniNodeExporter;

        using MeshDataList = AZStd::vector<HoudiniMeshData>;

        HoudiniModelData() = default;
        ~HoudiniModelData() = default;

        //! return number of meshes in the model.
        const AZ::u32 MeshCount() const;
        //! return list of mesh data.
        const MeshDataList GetMeshes() const;
        //! return the list of material names.
        MaterialList GetMaterials();

        //! These functions are the combination of all mesh buffers in order based on their position in MeshDataList
        const U32Vector& GetIndices() const;
        const Vert3Vector& GetPositions() const;
        const Vert3Vector& GetNormals() const;
        const Vert4Vector& GetTangents() const;
        const Vert3Vector& GetBitangents() const;
        const Vert2Vector& GetUVs() const;
        const Vert4Vector& GetColors() const;

        AZ::Aabb GetAabb() const;

    protected:
        //! Merges all indices and buffers into one buffer. Keeping track of the offsets and sizes.
        void MergeMeshBuffers();
        
        //! stores the mesh data.
        MeshDataList m_meshes;
        //! stores the material names.
        MaterialList m_materials;
        //! These are all data combined from the meshes in single arrays
        //! Combine them to one buffer to avoid the 256 mesh limit.
        U32Vector m_indices;
        Vert3Vector m_positions;
        Vert3Vector m_normals;
        Vert4Vector m_tangents;
        Vert3Vector m_bitangents;
        Vert2Vector m_uvs;
        Vert4Vector m_colors;

        AZ::Aabb m_aabb = AZ::Aabb::CreateNull();
    };
} // namespace HoudiniEngine
