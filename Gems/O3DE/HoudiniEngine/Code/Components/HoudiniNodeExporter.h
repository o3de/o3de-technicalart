#pragma once

#include <HAPI/HAPI.h>
#include <HAPI/HAPI_Common.h>

//Cry Includes:
#include <IIndexedMesh.h>
#include <ISystem.h>
#include <ILevelSystem.h>
//#include <IGame.h> //O3DECONVERT
//#include <IGameFramework.h>

//Houdini Includes:
#include <HoudiniEngine/HoudiniApi.h>
#include <Game/HoudiniMeshComponent.h>
#include <Game/HoudiniRenderMesh.h>
#include <HoudiniEngine/HoudiniCommonForwards.h>

#include <AzFramework/Asset/AssetCatalogBus.h>
#include <AzFramework/Visibility/BoundsBus.h>
#include <AzToolsFramework/API/ComponentEntitySelectionBus.h>
//#include <IEntityRenderState.h>

namespace HoudiniEngine
{
    
	class HoudiniNodeExporter
		: public AzFramework::BoundsRequestBus::Handler
		, public AzToolsFramework::EditorComponentSelectionRequestsBus::Handler
        , private AzFramework::AssetCatalogEventBus::Handler
    {
        public:
            AZ::EntityId m_entityId;
            
            IHoudini* m_hou;
            HAPI_Session* m_session;

            IHoudiniNode * m_node = nullptr;
            AZStd::string m_operatorName;
            
            bool m_dirty = true;
            AZStd::vector<AZStd::string> m_materialHintNames;
            AZStd::vector<AZStd::string> m_attributeHintNames;

            AZStd::unordered_map<AZStd::string, AZStd::string> m_materialDefaults;
            AZStd::vector<HoudiniMaterialSettings> m_materialSettings;
            //AZStd::vector<IMaterialRef> m_materials;
            //AZStd::vector<HoudiniMeshData> m_meshData;
            
            HoudiniModelData m_modelData;
            AZStd::unique_ptr<HoudiniRenderMesh> m_renderMesh;

            AZStd::vector<HAPI_NodeId> m_matIndexLookup;

            AZStd::map<int, AZ::EntityId> m_meshEntities;

            AZStd::mutex m_lock;
            AZStd::vector<AZStd::function<void()>> m_tickFunctions;

			//! Current world transform of the object.
			AZ::Transform m_world = AZ::Transform::CreateIdentity();
			//! Cached world aabb (used for selection/view determination).
			AZStd::optional<AZ::Aabb> m_worldAabb;
			//! Cached local aabb (used for center pivot calculation).
			AZStd::optional<AZ::Aabb> m_localAabb;

			//! List for materials building in AP. Having an empty list all materials are built and ready for loading.
			AZStd::vector<AZStd::string> m_materialWaitList;
        public:

            AZ_CLASS_ALLOCATOR(HoudiniNodeExporter, AZ::SystemAllocator);
            AZ_RTTI(HoudiniNodeExporter, HOUDINI_NODE_EXPORT_GUID);
            static void Reflect(AZ::ReflectContext* context)
            {
                if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
                {
                    serialize->Class<HoudiniNodeExporter>()
                        ->Version(1)
                        ->Field("MaterialSettings", &HoudiniNodeExporter::m_materialSettings)
                        //->Field("MeshData", &HoudiniNodeExporter::m_meshData)
                        ;

                    if (AZ::EditContext* ec = serialize->GetEditContext())
                    {
                        ec->Class<HoudiniNodeExporter>("HoudiniNodeExporter", "HoudiniNodeExporter")
                            ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                            ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                            ->Attribute(AZ::Edit::Attributes::AutoExpand, true)

                            ->DataElement(AZ::Edit::UIHandlers::Default, &HoudiniNodeExporter::m_materialSettings, "Material Settings", "")
                            ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniNodeExporter::OnChanged)
                            ;
                    }
                }
            }

            HoudiniNodeExporter() = default;
            virtual ~HoudiniNodeExporter();                   

			// EditorComponentSelectionRequestsBus overrides ...
			AZ::Aabb GetEditorSelectionBoundsViewport(const AzFramework::ViewportInfo& viewportInfo) override;
			bool EditorSelectionIntersectRayViewport(
				const AzFramework::ViewportInfo& viewportInfo, const AZ::Vector3& src, const AZ::Vector3& dir, float& distance) override;
			bool SupportsEditorRayIntersect() override;

			// BoundsRequestBus overrides ...
			AZ::Aabb GetWorldBounds() override;
			AZ::Aabb GetLocalBounds() override;

			void OnSplineChanged();
            
			// AssetCatalogEventBus::Handler ...
		    //! we use these functions to know if our assets like materials are already loaded so we can proceed with rendering the model/meshes
			void OnCatalogAssetAdded(const AZ::Data::AssetId& assetId) override;
			void OnCatalogAssetChanged(const AZ::Data::AssetId& assetId) override;

            void UpdateWorldTransformData(const AZ::Transform& transform);

            void OnTick();
            bool CheckHoudiniAccess();
            void CheckForErrors();
            void OnChanged();      

            //Exporting:
            void SetMaterialPath(const AZStd::string& materialName, const AZStd::string& materialPath);
            void ReadAttributeHints(HAPI_NodeId nodeId, HAPI_PartInfo& partInfo);
            void ReadMaterialNameHints(HAPI_NodeId nodeId, HAPI_PartInfo& partInfo);
            void ReadMaterials(HAPI_NodeId, HAPI_PartInfo& partInfo, HoudiniMeshData& meshData);
            void ApplyMaterialsToMesh(HoudiniMeshData& meshData);

            void SaveToFbx(bool useClusters = false);
            void SetVisibleInEditor(bool visibility);
            int AddGeometry(int index, HAPI_NodeId nodeId, const AZStd::string& clusterId);
            bool Initialize(const AZ::EntityId& id, HoudiniFbxConfig* fbxConfig);
            bool GenerateMeshMaterials();
            bool GenerateMeshData();
            void RemoveMeshData();
            bool GenerateEditableMeshData();

            //Accessors:
            void SetDirty(bool state) { m_dirty = state; }
            bool IsDirty() { return m_dirty; }

            const AZ::EntityId & GetEntityId() const { return m_entityId; }
            
            void SetNode(IHoudiniNode* node) { m_node = node; }
            IHoudiniNode* GetNode() { return m_node; }

            //AZStd::vector<HoudiniMeshData>& GetMeshData() { return m_meshData; }
            //AZStd::vector<IStatObj*> GetStatObjects();
            AZStd::vector<AZStd::string> GetClusters();

            HAPI_NodeId CreateObjectMerge(const AZStd::string& rootName, const AZStd::string& nodeName);

            //Utils:
            AZStd::string GetCleanString(const AZStd::string & value);

            void RebuildRenderMesh();

            // NOTE: from HoudiniMeshTranslator
    	    // Material IDs per face
		    AZStd::vector<HAPI_NodeId> PartFaceMaterialIds;
			// Unique material IDs
            AZStd::vector<AZ::s32> PartUniqueMaterialIds;
			// Material infos for each unique Material
            AZStd::vector<HAPI_MaterialInfo> PartUniqueMaterialInfos;
			// Indicates we only have a single face material
			bool bOnlyOneFaceMaterial;

            AZStd::string m_fbxFilename;
            AZStd::string m_previousFbxFilename;
            HoudiniFbxConfig* m_fbxConfig;

			// Update th unique materials ids and infos needed for this part using the face materials and overrides
			bool UpdatePartNeededMaterials(HAPI_NodeId nodeId, HAPI_PartInfo& partInfo);

			// Update this part's face material IDs, unique material IDs and material Infos caches if we haven't already
			bool UpdatePartFaceMaterialIDsIfNeeded(HAPI_NodeId nodeId, HAPI_PartInfo& partInfo);

			// Update this part's material overrides cache if we haven't already
			bool UpdatePartFaceMaterialOverridesIfNeeded();

			// Updates and create the material that are needed for this part
			bool CreateNeededMaterials(const AZStd::string& objectName, HAPI_NodeId nodeId, HAPI_PartInfo& partInfo, HoudiniMeshData& meshData);
    };
}
