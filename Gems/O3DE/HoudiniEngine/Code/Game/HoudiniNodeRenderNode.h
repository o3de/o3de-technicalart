#pragma once

//Cry Includes:
#include <IIndexedMesh.h>
#include <ISystem.h>
#include <ILevelSystem.h>

#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
#include <AtomLyIntegration/CommonFeatures/Material/MaterialComponentBus.h>
#include <AtomLyIntegration/CommonFeatures/Mesh/MeshHandleStateBus.h>
#include <AzCore/Component/TransformBus.h>
#include <AzFramework/Visibility/BoundsBus.h>
#include <AzToolsFramework/API/ComponentEntitySelectionBus.h>

namespace AZ::RPI
{
	class ModelLodAsset;
	class ModelAsset;
	class Model;
} // namespace AZ::RPI

namespace HoudiniEngine
{
    class HoudiniNodeRenderNode
		: private AZ::Render::MeshHandleStateRequestBus::Handler
        , private AZ::TransformNotificationBus::Handler
		, public AZ::Render::MaterialConsumerRequestBus::Handler
		, public AZ::Render::MaterialComponentNotificationBus::Handler
		, public AzFramework::BoundsRequestBus::Handler
		, public AzToolsFramework::EditorComponentSelectionRequestsBus::Handler
    {
        public:
            bool m_visible = true;
            bool m_visibleInEditor = true;
            //IMaterialRef m_material = nullptr;
            //IStatObj* m_statObject;            
            Matrix34 m_renderTransform;

            bool m_dirtyEntity = true;
            float m_maxViewDistance = 8000;
            AABB m_localBoundingBox;
            AABB m_worldBoundingBox;

            HoudiniNodeRenderNode(AZ::EntityId entityId);
            ~HoudiniNodeRenderNode();

            //void Render(const SRendParams & EntDrawParams, const SRenderingPassInfo & passInfo) override;
            //IStatObj* GetEntityStatObj(unsigned int nPartId = 0, unsigned int nSubPartId = 0, Matrix34A* pMatrix = nullptr, bool bReturnOnlyVisible = false) override;
            //void SetEntityStatObj(unsigned int nSlot, IStatObj* pStatObj, const Matrix34A* pMatrix = NULL) override;
            
            //! Updates the render node's world transform based on the entity's.
            void UpdateWorldTransform(const AZ::Transform& entityTransform);

            void ApplyRenderOptions();

            void SetVisibleInEditor(bool state)
            {
                m_visibleInEditor = state;
            }

            void SetVisible(bool state)
            {
                m_visible = state;
                m_meshFeatureProcessor->SetVisible(m_meshHandle, m_visible);
            }

            void SetMaxViewDistance(float maxViewDistance)
            {
                m_maxViewDistance = maxViewDistance;
            }

			// MaterialConsumerRequestBus::Handler overrides...
			AZ::Render::MaterialAssignmentId FindMaterialAssignmentId(
				const AZ::Render::MaterialAssignmentLodIndex lod, const AZStd::string& label) const override;
			AZ::Render::MaterialAssignmentLabelMap GetMaterialLabels() const override;
			AZ::Render::MaterialAssignmentMap GetDefautMaterialMap() const override;
			AZStd::unordered_set<AZ::Name> GetModelUvNames() const override;

			// MaterialComponentNotificationBus::Handler overrides...
			void OnMaterialsUpdated(const AZ::Render::MaterialAssignmentMap& materials) override;

			// EditorComponentSelectionRequestsBus overrides ...
			AZ::Aabb GetEditorSelectionBoundsViewport(const AzFramework::ViewportInfo& viewportInfo) override;
			bool EditorSelectionIntersectRayViewport(
				const AzFramework::ViewportInfo& viewportInfo, const AZ::Vector3& src, const AZ::Vector3& dir, float& distance) override;
			bool SupportsEditorRayIntersect() override;

			// BoundsRequestBus overrides ...
			AZ::Aabb GetWorldBounds() override;
			AZ::Aabb GetLocalBounds() override;

			//! Returns the Model's data instance
			AZ::Data::Instance<AZ::RPI::Model> GetModel() const;

            // Inherited via IRenderNode O3DECONVERT
            /*const char * GetName() const override;
            const char * GetEntityClassName() const override;
            Vec3 GetPos(bool bWorldOnly = true) const override;
            void UpdateWorldBoundingBox();
            const AABB GetBBox() const override;
            void SetBBox(const AABB & WSBBox) override;
            void OffsetPosition(const Vec3 & delta) override;        
            IPhysicalEntity * GetPhysics() const override;
            void SetPhysics(IPhysicalEntity * pPhys) override;
            void SetMaterial(_smart_ptr<IMaterial> pMat) override;
            _smart_ptr<IMaterial> GetMaterial(Vec3 * pHitPos = NULL) override;
            _smart_ptr<IMaterial> GetMaterialOverride() override;
            float GetMaxViewDist() override;
            EERType GetRenderNodeType() override;
            void GetMemoryUsage(ICrySizer * pSizer) const override;            */


        private:

			// MeshHandleStateRequestBus overrides ...
			const AZ::Render::MeshFeatureProcessorInterface::MeshHandle* GetMeshHandle() const override;
			// TransformNotificationBus overrides ...
			void OnTransformChanged(const AZ::Transform& local, const AZ::Transform& world) override;

            //AZStd::unordered_map<IMaterial*, bool> m_materialReady;
            AZ::EntityId m_entityId;
			AZ::Data::Asset<AZ::RPI::ModelLodAsset> m_lodAsset;
			AZ::Data::Asset<AZ::RPI::ModelAsset> m_modelAsset;
            AZ::Data::Instance<AZ::RPI::Model> m_model;
            AZ::Render::MeshFeatureProcessorInterface* m_meshFeatureProcessor = nullptr;
            AZ::Render::MeshFeatureProcessorInterface::MeshHandle m_meshHandle;

			AZ::Transform m_world = AZ::Transform::CreateIdentity();
			AZStd::optional<AZ::Aabb> m_worldAabb;
			AZStd::optional<AZ::Aabb> m_localAabb;
    };

    
}
