#include "StdAfx.h"

#include <Game/HoudiniNodeRenderNode.h>
#include <AzCore/Component/NonUniformScaleBus.h>
#include <IIndexedMesh.h>
// #include <LmbrCentral/Rendering/MaterialOwnerBus.h>

namespace HoudiniEngine
{
    HoudiniNodeRenderNode::HoudiniNodeRenderNode(AZ::EntityId entityId)
        : m_entityId(entityId)
    {
        m_renderTransform.SetIdentity();
        m_localBoundingBox.Reset();
        m_worldBoundingBox.Reset();

		AZ::Render::MeshHandleStateRequestBus::Handler::BusConnect(m_entityId);
		AZ::Render::MaterialConsumerRequestBus::Handler::BusConnect(m_entityId);
		AZ::Render::MaterialComponentNotificationBus::Handler::BusConnect(m_entityId);

        //ATOMCONVERT
        /*m_material = gEnv->p3DEngine->GetMaterialManager()->GetDefaultMaterial();
        m_statObject = gEnv->p3DEngine->CreateStatObj();
        SetEntityStatObj(0, m_statObject);
        ApplyRenderOptions();*/
    }

    HoudiniNodeRenderNode::~HoudiniNodeRenderNode()
    {
		if (m_meshHandle.IsValid() && m_meshFeatureProcessor)
		{
			m_meshFeatureProcessor->ReleaseMesh(m_meshHandle);
			AZ::Render::MeshHandleStateNotificationBus::Event(
				m_entityId, &AZ::Render::MeshHandleStateNotificationBus::Events::OnMeshHandleSet, &m_meshHandle);
		}

		AZ::Render::MaterialConsumerRequestBus::Handler::BusDisconnect();
		AZ::Render::MaterialComponentNotificationBus::Handler::BusDisconnect();
		AZ::Render::MeshHandleStateRequestBus::Handler::BusDisconnect();
    }

    /*IStatObj* HoudiniNodeRenderNode::GetEntityStatObj(unsigned int nPartId, unsigned int nSubPartId, Matrix34A* pMatrix, bool bReturnOnlyVisible)
    {
        if (0 == nPartId)
        {
            if (pMatrix)
            {
                *pMatrix = m_renderTransform;
            }

            return m_statObject;
        }

        return nullptr;
    }

    void HoudiniNodeRenderNode::SetEntityStatObj(unsigned int nSlot, IStatObj* pStatObj, const Matrix34A* pMatrix)
    {
        if (m_statObject != pStatObj) 
        {
            delete m_statObject;
            m_statObject = pStatObj;
        }
    }*/

    void HoudiniNodeRenderNode::UpdateWorldTransform(const AZ::Transform& entityTransform)
    {
        m_renderTransform = AZTransformToLYTransform(entityTransform);
        //UpdateWorldBoundingBox(); //ATOMCONVERT
		if (m_meshFeatureProcessor && m_meshHandle.IsValid())
		{
			m_meshFeatureProcessor->SetTransform(m_meshHandle, entityTransform);
		}
    }

    void UpdateRenderFlag(bool enable, int mask, unsigned int& flags)
    {
        if (enable)
        {
            flags |= mask;
        }
        else
        {
            flags &= ~mask;
        }
    }

    void HoudiniNodeRenderNode::ApplyRenderOptions()
    {
        //ATOMCONVERT
        /*unsigned int flags = GetRndFlags();
        flags |= ERF_COMPONENT_ENTITY;

        // Update flags according to current render settings
        UpdateRenderFlag(true, ERF_CASTSHADOWMAPS | ERF_HAS_CASTSHADOWMAPS, flags);
        UpdateRenderFlag(m_visible == false, ERF_HIDDEN, flags);
        //UpdateRenderFlag(true, ERF_GOOD_OCCLUDER, flags);
        UpdateRenderFlag(false, ERF_NO_DECALNODE_DECALS, flags);
        UpdateRenderFlag(true, ERF_EXCLUDE_FROM_TRIANGULATION, flags);

        SetRndFlags(flags);*/
    }

    AZ::Render::MaterialAssignmentId HoudiniNodeRenderNode::FindMaterialAssignmentId(const AZ::Render::MaterialAssignmentLodIndex lod, const AZStd::string& label) const
    {
        return AZ::Render::GetMaterialSlotIdFromModelAsset(m_modelAsset, lod, label);
    }

    AZ::Render::MaterialAssignmentLabelMap HoudiniNodeRenderNode::GetMaterialLabels() const
    {
        return AZ::Render::GetMaterialSlotLabelsFromModelAsset(m_modelAsset);
    }

    AZ::Render::MaterialAssignmentMap HoudiniNodeRenderNode::GetDefautMaterialMap() const
    {
        return AZ::Render::GetDefautMaterialMapFromModelAsset(m_modelAsset);
    }

    AZStd::unordered_set<AZ::Name> HoudiniNodeRenderNode::GetModelUvNames() const
    {
		const AZ::Data::Instance<AZ::RPI::Model> model = GetModel();
		return model ? model->GetUvNames() : AZStd::unordered_set<AZ::Name>();
    }

    void HoudiniNodeRenderNode::OnMaterialsUpdated(const AZ::Render::MaterialAssignmentMap& materials)
    {
		if (m_meshFeatureProcessor)
		{
			m_meshFeatureProcessor->SetCustomMaterials(m_meshHandle, AZ::Render::ConvertToCustomMaterialMap(materials));
		}
    }

    AZ::Aabb HoudiniNodeRenderNode::GetEditorSelectionBoundsViewport([[maybe_unused]] const AzFramework::ViewportInfo& viewportInfo)
    {
        return GetWorldBounds();
    }

    bool HoudiniNodeRenderNode::EditorSelectionIntersectRayViewport([[maybe_unused]] const AzFramework::ViewportInfo& viewportInfo, const AZ::Vector3& src, const AZ::Vector3& dir, float& distance)
    {
		if (!GetModel())
		{
			return false;
		}

		AZ::Transform transform = AZ::Transform::CreateIdentity();
		AZ::TransformBus::EventResult(transform, m_entityId, &AZ::TransformBus::Events::GetWorldTM);

		AZ::Vector3 nonUniformScale = AZ::Vector3::CreateOne();
		AZ::NonUniformScaleRequestBus::EventResult(nonUniformScale, m_entityId, &AZ::NonUniformScaleRequests::GetScale);

		float t;
		AZ::Vector3 ignoreNormal;
		constexpr float rayLength = 1000.0f;
		if (GetModel()->RayIntersection(transform, nonUniformScale, src, dir * rayLength, t, ignoreNormal))
		{
			distance = rayLength * t;
			return true;
		}

		return false;
    }

    bool HoudiniNodeRenderNode::SupportsEditorRayIntersect()
    {
        return true;
    }

    AZ::Aabb HoudiniNodeRenderNode::GetWorldBounds()
    {
		if (!m_worldAabb.has_value())
		{
			m_worldAabb = GetLocalBounds();
			m_worldAabb->ApplyTransform(m_world);
		}

		return m_worldAabb.value();
    }

    AZ::Aabb HoudiniNodeRenderNode::GetLocalBounds()
    {
        // TODO: get aabb from MeshData
		/*if (!m_localAabb.has_value() && m_modelData.MeshCount() > 0)
		{
			m_localAabb = m_modelData.GetAabb();
		}*/

		return m_localAabb.value();
    }

    AZ::Data::Instance<AZ::RPI::Model> HoudiniNodeRenderNode::GetModel() const
    {
        return m_model;
    }

    const AZ::Render::MeshFeatureProcessorInterface::MeshHandle* HoudiniNodeRenderNode::GetMeshHandle() const
    {
        return &m_meshHandle;
    }

    void HoudiniNodeRenderNode::OnTransformChanged([[maybe_unused]] const AZ::Transform& local, const AZ::Transform& world)
    {
		m_worldAabb.reset();
		m_localAabb.reset();

		m_world = world;

        UpdateWorldTransform(world);
    }

    /*void HoudiniNodeRenderNode::Render(const struct SRendParams& inRenderParams, const struct SRenderingPassInfo& passInfo)
    {
        AZ_PROFILE_FUNCTION(Editor);

        if (m_statObject == nullptr || m_visible == false || m_visibleInEditor == false)
        {
            return;
        }
        
        SRendParams rParams(inRenderParams);

        // Assign a unique pInstance pointer, otherwise effects involving SRenderObjData will not work for this object.  CEntityObject::Render does this for legacy entities.
        rParams.pInstance = this;
        rParams.fAlpha = 1.0f;        

        _smart_ptr<IMaterial> previousMaterial = rParams.pMaterial;
        const int previousObjectFlags = rParams.dwFObjFlags;

        if (m_material)
        {
            bool materialInitialized = m_materialReady.find(m_material) != m_materialReady.end();
            if (materialInitialized == false)
            {
                m_material->RequestTexturesLoading(0);
                m_materialReady[m_material] = true;
            }

            rParams.pMaterial = m_material;
        }

        //rParams.dwFObjFlags |= FOB_DYNAMIC_OBJECT;        
        rParams.pMatrix = &m_renderTransform;

        rParams.bForceDrawStatic = false;
        
        if (rParams.pMatrix->IsValid())
        {
            m_statObject->Render(rParams, passInfo);            
        }

        rParams.pMaterial = previousMaterial;
        rParams.dwFObjFlags = previousObjectFlags;
    }

    const char * HoudiniNodeRenderNode::GetName() const
    {
        return "HoudiniNodeRenderNode";
    }
    
    const char * HoudiniNodeRenderNode::GetEntityClassName() const
    {
        return "HoudiniNodeRenderNode";
    }

    Vec3 HoudiniNodeRenderNode::GetPos(bool bWorldOnly) const
    {
        return m_renderTransform.GetTranslation();
    }

    void HoudiniNodeRenderNode::UpdateWorldBoundingBox()
    {
        if (m_statObject)
        {
            m_localBoundingBox = m_statObject->GetAABB();
        }

        AABB previous = m_worldBoundingBox = m_worldBoundingBox;
        m_worldBoundingBox.SetTransformedAABB(m_renderTransform, m_localBoundingBox);
        
        if (gEnv->IsEditor() && !gEnv->IsEditorGameMode())
        {
            if (m_worldBoundingBox.min != previous.min || m_worldBoundingBox.max != previous.max)
            {
                // Re-register with the renderer to update culling info
                m_dirtyEntity = true;
            }
        }
        else
        {
            m_dirtyEntity = true;
        }
    }

    const AABB HoudiniNodeRenderNode::GetBBox() const
    {        
        return m_worldBoundingBox;
    }

    void HoudiniNodeRenderNode::SetBBox(const AABB & WSBBox)
    {
    }

    void HoudiniNodeRenderNode::OffsetPosition(const Vec3 & delta)
    {
    }

    IPhysicalEntity * HoudiniNodeRenderNode::GetPhysics() const
    {
        return nullptr;
    }

    void HoudiniNodeRenderNode::SetPhysics(IPhysicalEntity * pPhys)
    {
    }
    
    void HoudiniNodeRenderNode::SetMaterial(_smart_ptr<IMaterial> pMat)
    {
        if (pMat == nullptr)
        {
            m_material = gEnv->p3DEngine->GetMaterialManager()->GetDefaultMaterial();
        }
        else 
        {
            m_material = pMat;
        }
    }

    _smart_ptr<IMaterial> HoudiniNodeRenderNode::GetMaterial(Vec3 * pHitPos)
    {
        return m_material;
    }
    _smart_ptr<IMaterial> HoudiniNodeRenderNode::GetMaterialOverride()
    {
        return m_material;
    }
    float HoudiniNodeRenderNode::GetMaxViewDist()
    {
        float result = m_maxViewDistance;

        // Future addition:
        //Multiply by the view distance ratio.
        //static ICVar* pViewDistRatio = gEnv->pConsole->GetCVar("e_ViewDistRatio");
        //float viewDistRatio = 0.01f * (pViewDistRatio ? pViewDistRatio->GetFVal() : 100.0f);
        //result *= viewDistRatio;
        
        return result;
    }
    EERType HoudiniNodeRenderNode::GetRenderNodeType()
    {
        return eERType_Dummy_4; //It's not used anywhere!  Yet.
    }
    void HoudiniNodeRenderNode::GetMemoryUsage(ICrySizer * pSizer) const
    {
    }*/
}
