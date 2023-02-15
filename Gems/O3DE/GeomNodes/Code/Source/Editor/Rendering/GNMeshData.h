#pragma once

#include <Editor/Math/MathHelper.h>
#include <AzCore/Math/Aabb.h>
namespace GeomNodes
{
    class GNMeshData
    {
    public:
        explicit GNMeshData(
            const Vert3Vector& positions
            , const Vert3Vector& normals
            , const S32Vector& indices
            , const S32Vector& triangleLoops
            , const S32Vector& loops
            , const Vert2Vector& uvs
            , const Vert4Vector& colors
            , const S32Vector& materialIndices
            , AZ::u64 hash
            , bool isIndexedUVs
            , bool isIndexedColors);

        ~GNMeshData() = default;

        const AZ::u32 VertexCount() const;
        const U32Vector& GetIndices() const;
        const Vert3Vector& GetPositions() const;
        const Vert3Vector& GetNormals() const;
        const Vert4Vector& GetTangents() const;
        const Vert3Vector& GetBitangents() const;
        const Vert2Vector& GetUVs() const;
        const Vert4Vector& GetColors() const;
        const Mat4Vector& GetInstances() const;
        const AZ::Matrix4x4 GetTransform() const;
        const AZ::Transform GetO3DETransform() const;
        const AZ::Vector3 GetO3DEScale() const;
        static void GetO3DETransformAndScale(const AZ::Matrix4x4& mat4Transform, AZ::Transform& transform, AZ::Vector3& scale);

        void AddInstance(const AZ::Matrix4x4& mat4);
        void SetTransform(const AZ::Matrix4x4& transform);
        
        AZ::Aabb GetAabb() const;
        AZ::s64 GetHash() const;

    private:
        void CalculateTangents();

        U32Vector m_indices;
        S32Vector m_loops;

        Vert3Vector m_positions;
        Vert3Vector m_normals;
        Vert4Vector m_tangents;
        Vert3Vector m_bitangents;
        Vert2Vector m_uvs;
        Vert4Vector m_colors;
        
        AZ::Aabb m_aabb = AZ::Aabb::CreateNull();
        AZ::s64 m_hash = 0;

        Mat4Vector m_instances;
        AZ::Matrix4x4 m_transform;
        AZ::Transform m_o3deTransform = AZ::Transform::CreateIdentity();
        AZ::Vector3 m_o3deScale = AZ::Vector3::CreateOne();
        bool bO3DETransformCalculated = false;
    };
}
