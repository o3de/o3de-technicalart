/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Editor/Rendering/GNMeshData.h>
#include <AzCore/std/containers/map.h>

namespace GeomNodes
{
    //! Handle parsing, handling and storing model data.
    class GNModelData
    {
    public:
        using MeshDataList = AZStd::vector<GNMeshData>;
        
        GNModelData();
        GNModelData(AZ::u64 mapId);
        ~GNModelData() = default;

        //! Read all model data in the shared memory given the map id.
        void ReadData(AZ::u64 mapId);

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

    private:
        //! Merges all indices and buffers into one buffer. Keeping track of the offsets and sizes.
        void MergeMeshBuffers();
        //! Given the map id as the SHM name read data array from the shared memory based on typename T.
        template<typename T>
        AZStd::vector<T> ReadArray(AZ::u64 mapId);
		//! Given the map id as the SHM name read data from the shared memory based on typename T.
		template<typename T>
        T Read(AZ::u64 mapId);

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
}
