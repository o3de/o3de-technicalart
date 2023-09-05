#include "StdAfx.h"

#include <Game/HoudiniMeshData.h>

#include <AzCore/Component/TickBus.h>
#include <IIndexedMesh.h>

namespace HoudiniEngine
{
    AZStd::mutex HoudiniMeshData::m_dataLock;

    HoudiniMeshData::~HoudiniMeshData()
    {
    }

    void HoudiniMeshData::UpdateStatObject()
    {
        // ATOMCONVERT
        /*
        AZ_PROFILE_FUNCTION(Editor);

        IStatObj* staticObject = GetStatObject();
        int vertexCount = Indices.size();
        int pointCount = Positions.size();

        //Create a geometry object in engine and set any counts to things we know.
        IIndexedMesh *idxMesh = staticObject->GetIndexedMesh();
        idxMesh->SetVertexCount(pointCount);
        idxMesh->SetIndexCount(vertexCount);
        idxMesh->SetColorCount(Colors.size() > 0 ? pointCount : 0);
        idxMesh->SetTexCoordCount(UVs.size() > 0 ? pointCount : 0);
        idxMesh->SetTangentCount(pointCount);

        //Update the mesh
        CMesh* mesh = idxMesh->GetMesh();

        vtx_idx* const indices = mesh->GetStreamPtr<vtx_idx>(CMesh::INDICES);
        //Streams:
        Vec3* positions = mesh->GetStreamPtr<Vec3>(CMesh::POSITIONS);
        Vec3* normals = mesh->GetStreamPtr<Vec3>(CMesh::NORMALS);
        SMeshColor* colors = mesh->GetStreamPtr<SMeshColor>(CMesh::COLORS);
        SMeshTexCoord* texcoords = mesh->GetStreamPtr<SMeshTexCoord>(CMesh::TEXCOORDS);
        SMeshTangents* tangents = mesh->GetStreamPtr<SMeshTangents>(CMesh::TANGENTS);

        for (int i = 0; i < Indices.size(); i++)
        {
            const auto& val = Indices[i];
            indices[i] = val;
        }

        for (int i = 0; i < Positions.size(); i++)
        {
            const auto& val = Positions[i];
            positions[i].Set(val.GetX(), val.GetY(), val.GetZ());
        }

        for (int i = 0; i < Normals.size(); i++)
        {
            const auto& val = Normals[i];
            normals[i].Set(val.GetX(), val.GetY(), val.GetZ());
        }

        for(int i =0; i < Colors.size(); i++)
        {
            AZ::Vector4 color = Colors[i];

            uint8 r = 0, g = 0, b = 0, a = 255;
            r = (uint8)((float)color.GetX() * 255);
            g = (uint8)((float)color.GetY() * 255);
            b = (uint8)((float)color.GetZ() * 255);
            a = (uint8)((float)color.GetW() * 255);            

            colors[i] = SMeshColor(r, g, b, a);
        }

        for (int i = 0; i < UVs.size(); i++)
        {
            const auto& val = UVs[i];
            texcoords[i] = SMeshTexCoord(val.GetX(), val.GetY());
        }

        bool warned = false;

        if (Tangents.empty() == false && Tangents.size() == Bitangents.size() && Tangents.size() == Normals.size())
        {            
            for (int i = 0; i < Tangents.size(); i++)
            {
                auto& azTangent = Tangents[i];
                auto& azBitangent = Bitangents[i];
                auto& azNormal = Normals[i];

                if (NumberValid(azTangent.GetX()) == false || NumberValid(azTangent.GetY()) == false || NumberValid(azTangent.GetZ()) == false ||
                    NumberValid(azBitangent.GetX()) == false || NumberValid(azBitangent.GetY()) == false || NumberValid(azBitangent.GetZ()) == false ||
                    NumberValid(azNormal.GetX()) == false || NumberValid(azNormal.GetY()) == false || NumberValid(azNormal.GetZ()) == false)
                {
                    if (!warned)
                    {
#ifdef HOUDINI_ENGINE_EDITOR
                        AZ_Warning("[HOUDINI]", false, "Invalid normal data on mesh '%s', Normal: (%f, %f, %f) Tangent: (%f, %f, %f) Bitangent:  (%f, %f, %f) \nUsing identity normals instead, lighting may look incorrect on this mesh.",
                            m_meshName.c_str(), azNormal.GetX(), azNormal.GetY(), azNormal.GetZ(), azTangent.GetX(), azTangent.GetY(), azTangent.GetZ(), azBitangent.GetX(), azBitangent.GetY(), azBitangent.GetZ());
#endif
                        warned = true;
                    }

                    SMeshTangents finalTangents = SMeshTangents(Vec3(1, 0, 0), Vec3(0, 1, 0), Vec3(0, 0, 1));
                    tangents[i] = finalTangents;
                    continue;
                }

                Vec3 tangent = AZVec3ToLYVec3(Tangents[i]);
                Vec3 bitangent = AZVec3ToLYVec3(Bitangents[i]);
                Vec3 sNorm = AZVec3ToLYVec3(Normals[i]);
                
                tangent.Normalize();
                bitangent.Normalize();
                sNorm.Normalize();

                SMeshTangents finalTangents = SMeshTangents(tangent, bitangent, sNorm);
                tangents[i] = finalTangents;
            }
        }
        else
        {
            //Compute some tangents:  There will be seams between multiple objects this way.
            for (int i = 0; i < vertexCount / 3; i++)
            {
                // compute tangent from the 3 positions
                Vec3 tang = (positions[indices[i * 3 + 0]] - positions[indices[i * 3 + 1]]).GetNormalized();
                Vec3 bino = (positions[indices[i * 3 + 0]] - positions[indices[i * 3 + 2]]).GetNormalized();
                                
                Vec3 n0 = normals[indices[i * 3 + 0]];
                Vec3 n1 = normals[indices[i * 3 + 1]];
                Vec3 n2 = normals[indices[i * 3 + 2]];

                Vec3 sNorm = (n0 + n1 + n2) * 0.3333333333f;

                Vec3& v0 = positions[indices[i * 3 + 0]];
                Vec3& v1 = positions[indices[i * 3 + 1]];
                Vec3& v2 = positions[indices[i * 3 + 2]];

                Vec2 uv0((float)indices[i * 3 + 0], (float)indices[i * 3 + 0]);
                Vec2 uv1((float)indices[i * 3 + 1], (float)indices[i * 3 + 1]);
                Vec2 uv2((float)indices[i * 3 + 2], (float)indices[i * 3 + 2]);

                //Verify we have tex coords.
                if ( texcoords != nullptr)
                { 
                    uv0 = texcoords[indices[i * 3 + 0]].GetUV();
                    uv1 = texcoords[indices[i * 3 + 1]].GetUV();
                    uv2 = texcoords[indices[i * 3 + 2]].GetUV();
                }
                
                // Edges of the triangle : position delta
                Vec3 deltaPos1 = v1 - v0;
                Vec3 deltaPos2 = v2 - v0;

                // UV delta
                Vec2 deltaUV1 = uv1 - uv0;
                Vec2 deltaUV2 = uv2 - uv0;
                
                float denom = (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
                if (denom == 0) 
                {
                    denom = 0.001f;
                }

                float r = 1.0f / denom;
                Vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
                Vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r;

                if (tangent.IsValid() == false || bitangent.IsValid() == false || sNorm.IsValid() == false)
                {
                    if (!warned)
                    {
#ifdef HOUDINI_ENGINE_EDITOR
                        AZ_Warning("[HOUDINI]", false, "Invalid normal data on mesh '%s', Normal: (%f, %f, %f) Tangent: (%f, %f, %f) Bitangent:  (%f, %f, %f) \nUsing identity normals instead, lighting may look incorrect on this mesh.",
                            m_meshName.c_str(), sNorm.x, sNorm.y, sNorm.z, tangent.x, tangent.y, tangent.z, bitangent.x, bitangent.y, bitangent.z);
#endif
                        warned = true;
                    }

                    SMeshTangents finalTangents = SMeshTangents(Vec3(1, 0, 0), Vec3(0, 1, 0), Vec3(0, 0, 1));
                    tangents[i] = finalTangents;
                    continue;
                }

                tangent.Normalize();
                bitangent.Normalize();
                sNorm.Normalize();

                SMeshTangents finalTangents = SMeshTangents(tangent, bitangent, sNorm);
                for (int t = 0; t < 3; t++)
                {
                    tangents[indices[i * 3 + t]] = finalTangents;
                }
            }
        }

        idxMesh->CalcBBox();
        idxMesh->SetSubSetCount(1);
        idxMesh->SetSubsetMaterialId(0, 0);
        //idxMesh->SetSubsetMaterialProperties(0, m_matFlags, PHYS_GEOM_TYPE_DEFAULT, eVF_P3F_C4B_T2F);
        idxMesh->SetSubsetIndexVertexRanges(0, 0, mesh->GetIndexCount(), 0, mesh->GetVertexCount());
        idxMesh->SetSubsetBounds(0, Vec3(0, 0, 0), 1);
        idxMesh->CalcBBox();
        
        if (m_renderNode)
        {
            m_renderNode->SetMaxViewDistance(m_maxViewDistance);

            if (m_highPriorityTextureStreaming)
            {
                //m_renderNode->m_nInternalFlags |= IRenderNode::HIGH_PRIORITY_TEXTURE_STREAMING;
            }
        }

        UpdateWorldTransform(m_entityTransform);

        //Performance improvements?
        //staticObject->SetFlags(STATIC_OBJECT_GENERATED | STATIC_OBJECT_DYNAMIC);
        
        if (m_physics && gEnv && gEnv->pPhysicalWorld)
        {
            IPhysicalWorld* pWorld = gEnv->pPhysicalWorld;
            IGeomManager* pGeomManager = pWorld->GetGeomManager();

            auto* physics = m_physicalEntity.get();
            if (physics == nullptr)
            {
                EnablePhysics();
                physics = m_physicalEntity.get();
            }

            pe_geomparams geometryParameters;
            geometryParameters.flags = geom_collides | geom_floats | geom_colltype_ray;

            Matrix34 geometryTransform;
            geometryTransform.SetIdentity();
            geometryParameters.pMtx3x4 = &geometryTransform;
            
            if (IsSafeForCryPhysics(positions, indices, Indices.size() / 3))
            {
                //Order matters:
                staticObject->Invalidate(true);
                staticObject->Physicalize(physics, &geometryParameters);
                
                if (m_physicalEntity)
                {
                    pe_params_pos positionParameters;
                    Matrix34 physicsEntityTransform = AZTransformToLYTransform(m_entityTransform);
                    positionParameters.pMtx3x4 = &physicsEntityTransform;
                    m_physicalEntity->SetParams(&positionParameters);
                }
            }
            else
            {
                DisablePhysics();

                staticObject->SetPhysGeom(nullptr, 0);
                staticObject->Invalidate(false);

                AZ_Warning("HOUDINI", false, "Could not initialize cry physics on object.");
            }
        }
        else
        {
            DisablePhysics();

            staticObject->SetPhysGeom(nullptr, 0);
            staticObject->Invalidate(false);
        }
        */
    }

    void HoudiniMeshData::EnablePhysics()
    {
        // O3DECONVERT
        /*
        DisablePhysics();

        if (m_physicalEntity == nullptr)
        {
            Matrix34 cryTransform = AZTransformToLYTransform(m_entityTransform);
            
            pe_params_pos positionParameters;
            positionParameters.pMtx3x4 = &cryTransform;
            IPhysicalEntity* rawPhysicalEntityPtr = gEnv->pPhysicalWorld->CreatePhysicalEntity(pe_type::PE_STATIC, &positionParameters, 0, PHYS_FOREIGN_ID_COMPONENT_ENTITY, /id/ -1, nullptr);

            if (!rawPhysicalEntityPtr) // probably can't happen, but handle it just in case
            {
                AZ_Assert(false, "Failed to create physical entity.");
                return;
            }

            // IPhysicalEntity is owned by IPhysicalWorld and will not be destroyed until both:
            // - IPhysicalEntity's internal refcount has dropped to zero.
            // - IPhysicalWorld::DestroyPhysicalEntity() has been called on it.
            // We store IPhysicalEntity in a ptr whose custom destructor
            // ensures that these steps are all followed.
            rawPhysicalEntityPtr->AddRef();
            m_physicalEntity.reset(rawPhysicalEntityPtr, [](IPhysicalEntity* physicalEntity)
            {
                physicalEntity->Release();
                gEnv->pPhysicalWorld->DestroyPhysicalEntity(physicalEntity);
            });
        }*/
    }

    void HoudiniMeshData::DisablePhysics()
    {
        //O3DECONVERT
        /*
        if (m_physicalEntity)
        {
            m_physicalEntity.reset();
        }*/
    }

    AZ::u32 HoudiniMeshData::GetIndexCount() const
    {
        return m_indicesRange.count;
    }

    AZ::u32 HoudiniMeshData::GetIndexOffset() const
    {
        return m_indicesRange.offset;
    }

    void HoudiniMeshData::SetIndexOffset(AZ::u32 offset)
    {
        m_indicesRange.offset = offset;
    }

    void HoudiniMeshData::SetIndexOffset(AZ::u64 offset)
    {
        SetIndexOffset(aznumeric_cast<AZ::u32>(offset));
    }

    void HoudiniMeshData::SetIndexCount(AZ::u32 count)
    {
        m_indicesRange.count = count;
    }

    void HoudiniMeshData::SetIndexCount(AZ::u64 count)
    {
        SetIndexCount(aznumeric_cast<AZ::u32>(count));
    }

    const U32Vector& HoudiniMeshData::GetIndices() const
    {
        return m_indices;
    }

    void HoudiniMeshData::SetIndices(const U32Vector& indices)
    {
        m_indices = indices;
    }

	template<AttributeType AttributeTypeT>
	AZ::u32 HoudiniMeshData::GetCount() const
	{
		if constexpr (AttributeTypeT == AttributeType::Position)
		{
			return aznumeric_cast<AZ::u32>(m_positionsRange.count);
		}
		else if constexpr (AttributeTypeT == AttributeType::Normal)
		{
			return aznumeric_cast<AZ::u32>(m_normalsRange.count);
		}
		else if constexpr (AttributeTypeT == AttributeType::Tangent)
		{
			return aznumeric_cast<AZ::u32>(m_tangentsRange.count);
		}
		else if constexpr (AttributeTypeT == AttributeType::Bitangent)
		{
			return aznumeric_cast<AZ::u32>(m_bitangentsRange.count);
		}
		else if constexpr (AttributeTypeT == AttributeType::UV)
		{
			return aznumeric_cast<AZ::u32>(m_uvsRange.count);
		}
		else if constexpr (AttributeTypeT == AttributeType::Color)
		{
			return aznumeric_cast<AZ::u32>(m_colorsRange.count);
		}
	}
	template<AttributeType AttributeTypeT>
	AZ::u32 HoudiniMeshData::GetOffset() const
	{
		if constexpr (AttributeTypeT == AttributeType::Position)
		{
			return m_positionsRange.offset;
		}
		else if constexpr (AttributeTypeT == AttributeType::Normal)
		{
			return m_normalsRange.offset;
		}
		else if constexpr (AttributeTypeT == AttributeType::Tangent)
		{
			return m_tangentsRange.offset;
		}
		else if constexpr (AttributeTypeT == AttributeType::Bitangent)
		{
			return m_bitangentsRange.offset;
		}
		else if constexpr (AttributeTypeT == AttributeType::UV)
		{
			return m_uvsRange.offset;
		}
		else if constexpr (AttributeTypeT == AttributeType::Color)
		{
			return m_colorsRange.offset;
		}
	}
	template<AttributeType AttributeTypeT>
	void HoudiniMeshData::SetCount(AZ::u32 count)
	{
		if constexpr (AttributeTypeT == AttributeType::Position)
		{
			m_positionsRange.count = count;
		}
		else if constexpr (AttributeTypeT == AttributeType::Normal)
		{
			m_normalsRange.count = count;
		}
		else if constexpr (AttributeTypeT == AttributeType::Tangent)
		{
			m_tangentsRange.count = count;
		}
		else if constexpr (AttributeTypeT == AttributeType::Bitangent)
		{
			m_bitangentsRange.count = count;
		}
		else if constexpr (AttributeTypeT == AttributeType::UV)
		{
			m_uvsRange.count = count;
		}
		else if constexpr (AttributeTypeT == AttributeType::Color)
		{
			m_colorsRange.count = count;
		}
	}
	template<AttributeType AttributeTypeT>
	void HoudiniMeshData::SetCount(AZ::u64 count)
	{
        SetCount<AttributeTypeT>(aznumeric_cast<AZ::u32>(count));
	}
	template<AttributeType AttributeTypeT>
	void HoudiniMeshData::SetOffset(AZ::u32 offset)
	{
		if constexpr (AttributeTypeT == AttributeType::Position)
		{
			m_positionsRange.offset = offset;
		}
		else if constexpr (AttributeTypeT == AttributeType::Normal)
		{
			m_normalsRange.offset = offset;
		}
		else if constexpr (AttributeTypeT == AttributeType::Tangent)
		{
			m_tangentsRange.offset = offset;
		}
		else if constexpr (AttributeTypeT == AttributeType::Bitangent)
		{
			m_bitangentsRange.offset = offset;
		}
		else if constexpr (AttributeTypeT == AttributeType::UV)
		{
			m_uvsRange.offset = offset;
		}
		else if constexpr (AttributeTypeT == AttributeType::Color)
		{
			m_colorsRange.offset = offset;
		}
	}
	template<AttributeType AttributeTypeT>
	void HoudiniMeshData::SetOffset(AZ::u64 offset)
	{
        SetOffset<AttributeTypeT>(aznumeric_cast<AZ::u32>(offset));
	}
	template<AttributeType AttributeTypeT>
	const auto& HoudiniMeshData::GetDataBuffer() const
	{
		if constexpr (AttributeTypeT == AttributeType::Position)
		{
			return m_positions;
		}
		else if constexpr (AttributeTypeT == AttributeType::Normal)
		{
			return m_normals;
		}
		else if constexpr (AttributeTypeT == AttributeType::Tangent)
		{
			return m_tangents;
		}
		else if constexpr (AttributeTypeT == AttributeType::Bitangent)
		{
			return m_bitangents;
		}
		else if constexpr (AttributeTypeT == AttributeType::UV)
		{
			return m_uvs;
		}
		else if constexpr (AttributeTypeT == AttributeType::Color)
		{
			return m_colors;
		}
	}

	void HoudiniMeshData::SetMaterialIndex(AZ::u32 materialIndex)
	{
		m_materialIndex = materialIndex;
	}

    AZ::u32 HoudiniMeshData::GetMaterialIndex() const
    {
        return m_materialIndex;
    }

	U32Vector HoudiniMeshData::GetIndicesByMaterialIndex(int materialIndex)
	{
		U32Vector indices;
		indices.reserve(m_indices.size());

		for (AZ::s32 i = 0; i < m_indices.size(); i += 3)
		{
			const AZ::s32 faceIndex = i / 3;
			if (m_materialIndices[faceIndex] == materialIndex)
			{
				indices.push_back(m_indices[i]);
				indices.push_back(m_indices[i + 1]);
				indices.push_back(m_indices[i + 2]);
			}
		}

		return indices;
	}

	void HoudiniMeshData::ClearMaterialList()
	{
		m_materialNames.clear();
	}

    AZ::Aabb HoudiniMeshData::GetAabb() const
	{
		return m_aabb;
	}

	void HoudiniMeshData::CalculateTangents()
	{
		m_tangents.resize(m_positions.size());
		m_bitangents.resize(m_positions.size());

		for (AZ::s32 i = 0; i < m_indices.size() / 3; ++i)
		{
			const AZ::Vector3 V0 = MathHelper::Vec3fToVec3(m_positions[m_indices[i + 0]]);
			const AZ::Vector3 V1 = MathHelper::Vec3fToVec3(m_positions[m_indices[i + 1]]);
			const AZ::Vector3 V2 = MathHelper::Vec3fToVec3(m_positions[m_indices[i + 2]]);

			const AZ::Vector2 UV0 = MathHelper::Vec2fToVec2(m_uvs[m_indices[i + 0]]);
			const AZ::Vector2 UV1 = MathHelper::Vec2fToVec2(m_uvs[m_indices[i + 1]]);
			const AZ::Vector2 UV2 = MathHelper::Vec2fToVec2(m_uvs[m_indices[i + 2]]);

			// Calculate edge vectors of the triangle
			AZ::Vector3 edge1 = V1 - V0;
			AZ::Vector3 edge2 = V2 - V0;

			// Calculate delta m_uvs
			AZ::Vector2 deltaUV1 = UV1 - UV0;
			AZ::Vector2 deltaUV2 = UV2 - UV0;

			// Calculate the determinant to calculate the direction of the tangent and bitangent
			float det = 1.0f / (deltaUV1.GetX() * deltaUV2.GetY() - deltaUV2.GetX() * deltaUV1.GetY());

			// Calculate the tangent vector
			AZ::Vector3 tangent = (edge1 * deltaUV2.GetX() - edge2 * deltaUV1.GetY()) * det;
			tangent.Normalize();

			// Calculate the bitangent vector
			AZ::Vector3 bitangent = (edge2 * deltaUV1.GetY() - edge1 * deltaUV2.GetX()) * det;
			bitangent.Normalize();

			// Calculate the handedness of the tangent space
			float tangentW = (edge1.Cross(edge2).Dot(tangent) < 0.0f) ? -1.0f : 1.0f;

			AZ::Vector4 tangent4 = AZ::Vector4(tangent, tangentW);
			m_tangents[m_indices[i]] = MathHelper::Vec4ToVec4f(tangent4);
			m_bitangents[m_indices[i]] = MathHelper::Vec3ToVec3f(bitangent);
			m_tangents[m_indices[i + 1]] = MathHelper::Vec4ToVec4f(tangent4);
			m_bitangents[m_indices[i + 1]] = MathHelper::Vec3ToVec3f(bitangent);
			m_tangents[m_indices[i + 2]] = MathHelper::Vec4ToVec4f(tangent4);
			m_bitangents[m_indices[i + 2]] = MathHelper::Vec3ToVec3f(bitangent);
		}
	}

	void HoudiniMeshData::CalculateAABB()
	{
		// calculate the aabb
		for (const auto& vert : m_positions)
		{
			m_aabb.AddPoint(MathHelper::Vec3fToVec3(vert));
		}
	}

    HoudiniMeshData& HoudiniMeshData::operator+=(const HoudiniMeshData& rhs)
	{
		AZ::u32 count = aznumeric_cast<AZ::u32>(m_positions.max_size() + rhs.m_positions.size());
		m_indices.reserve(count * 3);
		m_positions.reserve(count);
		m_normals.reserve(count);
		m_tangents.reserve(count);
		m_bitangents.reserve(count);
		m_uvs.reserve(count);
		m_colors.reserve(count);

		AZ::s32 indexOffsset = aznumeric_cast<AZ::u32>(m_positions.size());
		U32Vector rhsIndices = rhs.m_indices;

		for (AZ::u32& index : rhsIndices)
		{
			index += indexOffsset;
		}

		m_indices.insert(m_indices.end(), rhsIndices.begin(), rhsIndices.end());
		SetIndexCount(m_indices.size());

		Vert3Vector rhsPositions = rhs.m_positions;
		m_positions.insert(m_positions.end(), rhsPositions.begin(), rhsPositions.end());
		SetCount<AttributeType::Position>(m_positions.size());

		Vert3Vector rhsNormals = rhs.m_normals;
		m_normals.insert(m_normals.end(), rhsNormals.begin(), rhsNormals.end());
		SetCount<AttributeType::Normal>(m_normals.size());

		m_colors.insert(m_colors.end(), rhs.m_colors.begin(), rhs.m_colors.end());
		SetCount<AttributeType::Color>(m_colors.size());

		m_uvs.insert(m_uvs.end(), rhs.m_uvs.begin(), rhs.m_uvs.end());
		SetCount<AttributeType::UV>(m_uvs.size());
		m_tangents.insert(m_tangents.end(), rhs.m_tangents.begin(), rhs.m_tangents.end());
		SetCount<AttributeType::Tangent>(m_tangents.size());
		m_bitangents.insert(m_bitangents.end(), rhs.m_bitangents.begin(), rhs.m_bitangents.end());
		SetCount<AttributeType::Bitangent>(m_bitangents.size());
	
		return *this;
	}

	void HoudiniMeshData::ClearBuffers()
	{
		m_indices.clear();
		
		m_positions.clear();
		m_normals.clear();
		m_tangents.clear();
		m_bitangents.clear();
		m_uvs.clear();
		m_colors.clear();
	}

    void HoudiniMeshData::SetMaxViewDistance(float maxViewDistance)
    {
        m_maxViewDistance = maxViewDistance;
    }

    void HoudiniMeshData::SetHighPriorityTextureStreaming( bool value )
    {
        m_highPriorityTextureStreaming = value;
    }

    void HoudiniMeshData::SetVisibilityInEditor([[maybe_unused]] bool state)
    {
    }

    void HoudiniMeshData::SetVisible([[maybe_unused]] bool state)
    {

    }

    void HoudiniMeshData::SetPhysics(bool state)
    {
        m_physics = state;
    }

    void HoudiniMeshData::SetMaterial([[maybe_unused]] void*/*IMaterialRef*/ material)
    {
    }

    /*IStatObj* HoudiniMeshData::GetStatObject() const
    {
        if (m_renderNode != nullptr)
        {
            return m_renderNode->m_statObject;
        }
        return nullptr;
    }*/

    bool HoudiniMeshData::IsRenderNodeDirty() const
    {
        return false;
    }

    void HoudiniMeshData::UpdateWorldTransform(const AZ::Transform& entityTransform)
    {
        m_entityTransform = entityTransform;

        //TODO: there's another way to update the mesh or model's transform

        //O3DECONVERT
        /*if (m_physicalEntity)
        {
            Matrix34 cryTransform = AZTransformToLYTransform(entityTransform);
            pe_params_pos positionParameters;
            positionParameters.pMtx3x4 = &cryTransform;
            m_physicalEntity->SetParams(&positionParameters);
        }*/
    }

    size_t HoudiniMeshData::Save(const void* classPtr, AZ::IO::GenericStream& stream, bool isDataBigEndian /*= false*/)
    {
        AZ_PROFILE_FUNCTION(Editor);
        AZStd::unique_lock<AZStd::mutex> theLock(m_dataLock);

        (void)isDataBigEndian;

        size_t totalBytes = 0;
        size_t dataSize = 0;

        const HoudiniMeshData* data = static_cast<const HoudiniMeshData*>(classPtr);

        totalBytes += stream.Write(4, "HMD\0");

        auto* meshName = data->m_meshName.c_str();
        dataSize = data->m_meshName.size();
        totalBytes += stream.Write(sizeof(int), (void*)(&dataSize));
        if (dataSize != 0)
            totalBytes += stream.Write(dataSize, meshName);

        totalBytes += stream.Write(sizeof(int), (void*)&data->m_materialIndex);
        totalBytes += stream.Write(sizeof(bool), (void*)&data->m_physics);

        dataSize = data->m_indices.size() * sizeof(int);
        totalBytes += stream.Write(sizeof(int), (void*)&dataSize);
        if ( dataSize != 0 ) 
            totalBytes += stream.Write(dataSize, &data->m_indices.front());

        dataSize = data->m_positions.size() * sizeof(Vector3f);
        totalBytes += stream.Write(sizeof(int), (void*)(&dataSize));
        if (dataSize != 0)
            totalBytes += stream.Write(dataSize, &data->m_positions.front());

        dataSize = data->m_normals.size() * sizeof(Vector3f);
        totalBytes += stream.Write(sizeof(int), (void*)(&dataSize));
        if (dataSize != 0)
            totalBytes += stream.Write(dataSize, &data->m_normals.front());

        dataSize = data->m_colors.size() * sizeof(Vector4f);
        totalBytes += stream.Write(sizeof(int), (void*)(&dataSize));
        if (dataSize != 0)
            totalBytes += stream.Write(dataSize, &data->m_colors.front());

        dataSize = data->m_uvs.size() * sizeof(Vector2f);
        totalBytes += stream.Write(sizeof(int), (void*)(&dataSize));
        if (dataSize != 0)
            totalBytes += stream.Write(dataSize, &data->m_uvs.front());

        dataSize = data->m_tangents.size() * sizeof(Vector3f);
        totalBytes += stream.Write(sizeof(int), (void*)(&dataSize));
        if (dataSize != 0)
            totalBytes += stream.Write(dataSize, &data->m_tangents.front());

        dataSize = data->m_bitangents.size() * sizeof(Vector3f);
        totalBytes += stream.Write(sizeof(int), (void*)(&dataSize));
        if (dataSize != 0)
            totalBytes += stream.Write(dataSize, &data->m_bitangents.front());
        
        totalBytes += stream.Write(sizeof(float), (void*)&data->m_maxViewDistance);

        totalBytes += stream.Write(sizeof(bool), (void*)&data->m_highPriorityTextureStreaming);

        return totalBytes;
    }

    bool HoudiniMeshData::Load(void* classPtr, AZ::IO::GenericStream& stream, unsigned int version, bool isDataBigEndian /*= false*/)
    {
        AZ_PROFILE_FUNCTION(Editor);
        AZStd::unique_lock<AZStd::mutex> theLock(m_dataLock);

        (void)isDataBigEndian;
        
        HoudiniMeshData* data = static_cast<HoudiniMeshData*>(classPtr);

        if (stream.GetLength() < 4)
        {
            return false;
        }

        int dataSize = 0;
        char header[4];
        stream.Read(4, header);

        if (header != AZStd::string("HMD"))
        {
            return false;
        }

        if (version >= 5)
        {
            stream.Read(sizeof(int), &dataSize);
            data->m_meshName = "";
            if (dataSize != 0)
            {
                data->m_meshName.resize(dataSize);
                auto dest = data->m_meshName.data();
                stream.Read(dataSize, dest);
            }
        }

        if (version > 1)
        {
            stream.Read(sizeof(int), &data->m_materialIndex);
        }

        if (version >= 4)
        {
            stream.Read(sizeof(bool), &data->m_physics);
        }

        stream.Read(sizeof(int), &dataSize);
        data->m_indices.resize(dataSize / sizeof(int));
        AZ_Assert(dataSize / sizeof(int) * sizeof(int) == dataSize, "Invalid dataSize");
        if (dataSize != 0)
            stream.Read(dataSize, &data->m_indices.front());
        
        stream.Read(sizeof(int), &dataSize);
        data->m_positions.resize(dataSize / sizeof(Vector3f));
        AZ_Assert(dataSize / sizeof(Vector3f) * sizeof(Vector3f) == dataSize, "Invalid dataSize");
        if (dataSize != 0)
            stream.Read(dataSize, &data->m_positions.front());

        stream.Read(sizeof(int), &dataSize);
        data->m_normals.resize(dataSize / sizeof(Vector3f));
        AZ_Assert(dataSize / sizeof(Vector3f) * sizeof(Vector3f) == dataSize, "Invalid dataSize");
        if (dataSize != 0)
            stream.Read(dataSize, &data->m_normals.front());

        stream.Read(sizeof(int), &dataSize);
        data->m_colors.resize(dataSize / sizeof(Vector4f));
        AZ_Assert(dataSize / sizeof(Vector4f) * sizeof(Vector4f) == dataSize, "Invalid dataSize");
        if (dataSize != 0)
            stream.Read(dataSize, &data->m_colors.front());

        stream.Read(sizeof(int), &dataSize);
        data->m_uvs.resize(dataSize / sizeof(Vector3f));
        AZ_Assert(dataSize / sizeof(Vector3f) * sizeof(Vector3f) == dataSize, "Invalid dataSize");
        if (dataSize != 0)
            stream.Read(dataSize, &data->m_uvs.front());

        if (version >= 3)
        {
            stream.Read(sizeof(int), &dataSize);
            data->m_tangents.resize(dataSize / sizeof(Vector3f));
            AZ_Assert(dataSize / sizeof(Vector3f) * sizeof(Vector3f) == dataSize, "Invalid dataSize");
            if (dataSize != 0)
                stream.Read(dataSize, &data->m_tangents.front());

            stream.Read(sizeof(int), &dataSize);
            data->m_bitangents.resize(dataSize / sizeof(Vector3f));
            AZ_Assert(dataSize / sizeof(Vector3f) * sizeof(Vector3f) == dataSize, "Invalid dataSize");
            if (dataSize != 0)
                stream.Read(dataSize, &data->m_bitangents.front());
        }

        if (version == 6)
        {
            //Due to a mistake in version 6, we need to deserialize the max value instead.
            data->m_maxViewDistance = 8000;
        }
        else if (version > 6)
        {
            stream.Read(sizeof(float), &data->m_maxViewDistance);
        }
        else if (version > 7)
        {
            stream.Read(sizeof(bool), &data->m_highPriorityTextureStreaming);
        }

        return true;
    }

    size_t HoudiniMeshData::DataToText(AZ::IO::GenericStream& in, AZ::IO::GenericStream& out, bool isDataBigEndian /*= false*/) 
    {
        AZ_PROFILE_FUNCTION(Editor);

        (void)isDataBigEndian;
        const size_t dataSize = static_cast<size_t>(in.GetLength()); // each byte is encoded in 2 chars (hex)

        AZStd::string outStr;
        if (dataSize)
        {
            AZ::u8* buffer = static_cast<AZ::u8*>(azmalloc(dataSize));
            
            if (in.GetLength() == dataSize)
            {
                in.Read(dataSize, buffer);
                outStr = AzFramework::StringFunc::Base64::Encode(buffer, dataSize);
            }

            azfree(buffer);
        }

        return static_cast<size_t>(out.Write(outStr.size(), outStr.data()));
    }

    size_t HoudiniMeshData::TextToData(const char* text, unsigned int textVersion, AZ::IO::GenericStream& stream, bool isDataBigEndian /*= false*/)
    {
        AZ_PROFILE_FUNCTION(Editor);

        (void)textVersion;
        (void)isDataBigEndian;
        size_t textLength = strlen(text);

        AZStd::vector<AZ::u8> buffer;
        AzFramework::StringFunc::Base64::Decode(buffer, text, textLength);
        stream.Write(buffer.size(), buffer.data());
        
        return buffer.size();
    }

    bool HoudiniMeshData::CompareValueData(const void* lhs, const void* rhs)
    {
        AZStd::vector<AZ::u8> lhsByteBuffer;
        AZStd::vector<AZ::u8> rhsByteBuffer;
        AZ::IO::ByteContainerStream<decltype(lhsByteBuffer)> lhsByteStream(&lhsByteBuffer);
        AZ::IO::ByteContainerStream<decltype(rhsByteBuffer)> rhsByteStream(&rhsByteBuffer);
        Save(lhs, lhsByteStream);
        Save(rhs, rhsByteStream);
        bool areSameLength = lhsByteBuffer.size() == rhsByteBuffer.size();
        return areSameLength && memcmp(lhsByteBuffer.data(), rhsByteBuffer.data(), lhsByteBuffer.size()) == 0;
    }

    void HoudiniMeshData::ClearSerializedMeshData()
    {
        m_indices.clear();
        m_positions.clear();
        m_normals.clear();
        m_colors.clear();
        m_uvs.clear();
        m_tangents.clear();
        m_bitangents.clear();

		m_indices.shrink_to_fit();
		m_positions.shrink_to_fit();
		m_normals.shrink_to_fit();
		m_colors.shrink_to_fit();
		m_uvs.shrink_to_fit();
		m_tangents.shrink_to_fit();
		m_bitangents.shrink_to_fit();
    }

    //WARNING: CryPhysics code to detect a bug in CryPhysics before it happens:
    // O3DECONVERT
    /*template<class F>
    ILINE int intersect_lists(F* pSrc0, int nSrc0, F* pSrc1, int nSrc1, F* pDst)
    {
        int i0, i1, n;
        for (i0 = i1 = n = 0; ((i0 - nSrc0) & (i1 - nSrc1)) < 0; )
        {
            const F a = pSrc0[i0];
            const F b = pSrc1[i1];
            F equal = iszero(a - b);
            pDst[n] = a;
            n += equal;
            i0 += isneg(a - b) + equal;
            i1 += isneg(b - a) + equal;
        }
        return n;
    }

    inline void swap(int* v, int i1, int i2)
    {
        int ti = v[i1]; v[i1] = v[i2]; v[i2] = ti;
    }

    template<class ftype>
    void qsort(int* v, strided_pointer<ftype> pkey, int left, int right)
    {
        int i, j, mi;
        ftype m;

        i = left;
        j = right;
        mi = (left + right) / 2;
        m = pkey[v[mi]];
        while (i <= j)
        {
            while (pkey[v[i]] < m)
            {
                i++;
            }
            while (m < pkey[v[j]])
            {
                j--;
            }
            if (i <= j)
            {
                swap(v, i, j);
                i++;
                j--;
            }
        }
        if (left < j)
        {
            qsort(v, pkey, left, j);
        }
        if (i < right)
        {
            qsort(v, pkey, i, right);
        }
    }

    void qsort(int* v, int left, int right)
    {
        int i, j, mi;
        int m;

        i = left;
        j = right;
        mi = (left + right) / 2;
        m = v[mi];
        while (i <= j)
        {
            while (v[i] < m)
            {
                i++;
            }
            while (m < v[j])
            {
                j--;
            }
            if (i <= j)
            {
                swap(v, i, j);
                i++;
                j--;
            }
        }
        if (left < j)
        {
            qsort(v, left, j);
        }
        if (i < right)
        {
            qsort(v, i, right);
        }
    }
    */

    bool HoudiniMeshData::IsSafeForCryPhysics(/*strided_pointer<const Vec3> pVertices, strided_pointer<unsigned int> pIndices, int nTris*/)
    {
        // O3DECONVERT
        /*
        int i;
        int m_nTris = nTris;
        int m_nVertices = 0;
        for (i = 0; i < m_nTris; i++)
        {
            m_nVertices = max(m_nVertices, (int)max(max(pIndices[i * 3], pIndices[i * 3 + 1]), pIndices[i * 3 + 2]));
        }
        m_nVertices++;
        auto m_pVertices = strided_pointer<Vec3>((Vec3*)pVertices.data, pVertices.iStride); // removes const modifier
        auto m_pIndices = (index_t*)pIndices.data;

        int* vsort[3], j, k, *pList[4], nList[4];
        struct vtxinfo
        {
            int id;
            Vec3i isorted;
        };

        vtxinfo* pVtx;
        float coord;
        pVtx = new vtxinfo[m_nVertices];

        static float mergeTol = 1e-5f;
        for (i = 0; i < 3; i++)
        {
            vsort[i] = new int[max(m_nTris, m_nVertices)];
        }
        for (i = 0; i < 4; i++)
        {
            pList[i] = new int[m_nVertices];
        }
        for (i = 0; i < m_nVertices; i++)
        {
            vsort[0][i] = vsort[1][i] = vsort[2][i] = i;
        }
        for (j = 0; j < 3; j++)
        {
            qsort(vsort[j], strided_pointer<float>((float*)m_pVertices.data + j, m_pVertices.iStride), 0, m_nVertices - 1);
            for (i = 0; i < m_nVertices; i++)
            {
                pVtx[vsort[j][i]].isorted[j] = i;
            }
        }
        for (i = 0; i < m_nVertices; i++)
        {
            pVtx[i].id = -1;
        }
        for (i = 0; i < m_nVertices; i++)
        {
            if (pVtx[i].id < 0)
            {
                for (j = 0; j < 3; j++)
                {
                    for (k = pVtx[i].isorted[j], coord = m_pVertices[i][j]; k > 0 && fabs_tpl(m_pVertices[vsort[j][k - 1]][j] - coord) <= mergeTol; k--)
                    {
                        ;
                    }
                    for (nList[j] = 0; k < m_nVertices && fabs_tpl(m_pVertices[vsort[j][k]][j] - coord) <= mergeTol; k++)
                    {
                        pList[j][nList[j]++] = vsort[j][k];
                    }
                    qsort(pList[j], 0, nList[j] - 1);
                }
                nList[3] = intersect_lists(pList[0], nList[0], pList[1], nList[1], pList[3]);
                nList[0] = intersect_lists(pList[3], nList[3], pList[2], nList[2], pList[0]);
                for (k = 0; k < nList[0]; k++)
                {
                    pVtx[pList[0][k]].id = i;
                }
            }
        }
        for (i = 0; i < 4; i++)
        {
            delete[] pList[i];
        }
        for (i = 0; i < 3; i++)
        {
            delete[] vsort[i];
        }

        // move degenerate triangles to the end of the list
        for (i = 0, j = m_nTris; i < j; )
        {
            if (iszero(pVtx[m_pIndices[i * 3]].id ^ pVtx[m_pIndices[i * 3 + 1]].id) | iszero(pVtx[m_pIndices[i * 3 + 1]].id ^ pVtx[m_pIndices[i * 3 + 2]].id) |
                iszero(pVtx[m_pIndices[i * 3 + 2]].id ^ pVtx[m_pIndices[i * 3]].id))
            {
                j--;
                for (k = 0; k < 3; k++)
                {
                    int itmp = m_pIndices[j * 3 + k];
                    m_pIndices[j * 3 + k] = m_pIndices[i * 3 + k];
                    m_pIndices[i * 3 + k] = itmp;
                }
            }
            else
            {
                i++;
            }
        }
        m_nTris = i;

        delete[] pVtx;

        return m_nTris != 0;
        */

        return false;
    }


}
