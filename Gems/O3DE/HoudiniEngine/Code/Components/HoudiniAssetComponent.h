#pragma once

#include <HoudiniEngine/HoudiniEngineBus.h>
#include <HoudiniEngine/HoudiniCommonForwards.h>
#include <Components/HoudiniNodeComponentConfig.h>
#include <Components/HoudiniFbxConfig.h>
#include <Components/HoudiniNodeExporter.h>

#include <AzCore/Component/EntityBus.h>
#include <AzFramework/Entity/EntityDebugDisplayBus.h>
#include <AzToolsFramework/API/ComponentEntitySelectionBus.h>
#include <AzToolsFramework/ToolsComponents/EditorVisibilityBus.h>
#include <LmbrCentral/Shape/SplineComponentBus.h>

namespace HoudiniEngine
{
    class HoudiniNode;
    
    class HoudiniAssetComponent
        : public AzToolsFramework::Components::EditorComponentBase
        , public AZ::EntityBus::Handler
        , public HoudiniAssetRequestBus::Handler
        //, public HoudiniMeshRequestBus::Handler
        , public HoudiniMaterialRequestBus::Handler
        , public AZ::TickBus::Handler
        , public AZ::TransformNotificationBus::Handler
        , public AzFramework::EntityDebugDisplayEventBus::Handler
        //, public AzToolsFramework::EditorComponentSelectionRequestsBus::Handler
        , public AzToolsFramework::EditorComponentSelectionNotificationsBus::Handler
        , public AzToolsFramework::EditorVisibilityNotificationBus::Handler
        , public IEditorNotifyListener
        , public LmbrCentral::SplineComponentNotificationBus::Handler
    {
    public:
        AZ_EDITOR_COMPONENT(HoudiniAssetComponent, HOUDINI_ASSET_COMPONENT_GUID, AzToolsFramework::Components::EditorComponentBase)
        
        static void Reflect(AZ::ReflectContext* context);
        static bool VersionConverter(AZ::SerializeContext& context, AZ::SerializeContext::DataElementNode& classElement);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
        AZ::ExportedComponent ExportMeshComponent(AZ::Component* thisComponent, const AZ::PlatformTagSet& /*platformTags*/);


        HoudiniAssetComponent() = default;
        ~HoudiniAssetComponent() override;
        
        // -- AZ::TickBus Interface -------------------------------------------------------------------------
        virtual void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        // -- AZ::TickBus Interface -------------------------------------------------------------------------
        
        // IEditorNotifyListener
        void OnEditorNotifyEvent(EEditorNotifyEvent event) override;

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        
        // AZ::EntityEvents interface implementation
        void OnEntityNameChanged(const AZStd::string& name) override;
        void OnEntityDestruction(const AZ::EntityId& id) override;
        void OnEntityVisibilityChanged(bool visibility) override;

        // AzFramework::EntityDebugDisplayEventBus interface implementation
        void DisplayEntityViewport(const AzFramework::ViewportInfo& viewportInfo, AzFramework::DebugDisplayRequests& debugDisplay) override;
        
        // AZ:: TransformNotificationBus interface implementation
        void OnTransformChanged(const AZ::Transform& /*local*/, const AZ::Transform& /*world*/) override;
        
        //// EditorComponentSelectionRequestsBus
        //AZ::Aabb GetEditorSelectionBoundsViewport(const AzFramework::ViewportInfo& viewportInfo) override;
        //bool EditorSelectionIntersectRayViewport(const AzFramework::ViewportInfo& viewportInfo, const AZ::Vector3& src, const AZ::Vector3& dir, float& distance) override;
        //bool SupportsEditorRayIntersect() override { return true; };

        // EditorComponentSelectionNotificationsBus
        void OnAccentTypeChanged(AzToolsFramework::EntityAccentType accent) override;

		// SplineAttributeNotificationBus
		void OnSplineChanged() override;
        void OnVerticesSet(const AZStd::vector<AZ::Vector3>& vertices) override;
        void OnVerticesCleared() override;

        //Houdini Mesh
        //AZStd::vector<HoudiniMeshStatObject> GetStatObjects() override;
        
        //Houdini Materials
        void SetMaterialPath(const AZStd::string& materialName, const AZStd::string& materialPath);


        HoudiniNodeComponentConfig m_config;
        HoudiniNodeExporter m_nodeExporter;
        HoudiniFbxConfig m_fbxConfig;

        IHoudiniNode* LoadHda(const AZStd::string& operatorName, AZStd::function<void(IHoudiniNode*)> onLoad = {})
        {
            if (m_config.m_nodeName.empty() )
                m_config.m_nodeName = m_entity->GetName();

            return m_config.LoadHda(operatorName, m_config.m_nodeName, onLoad);
        }

        void FixEntityPointers() override
        {
            m_config.FixEntityPointers();
        }

        AZ::EntityId GetEntityIdFromNodeName(const AZStd::string& nodeName) override
        {
            if (m_config.m_nodeName == nodeName)
            {
                return GetEntityId();
            }

            return AZ::EntityId();
        }

        const AZStd::string& GetNodeName() override
        {
            return m_config.GetNodeName(); 
        }

        IHoudiniNode* GetNode() override { return m_config.GetNode(); }
        IHoudiniNodeComponentConfig * GetConfig() { return &m_config; }
        void SaveToFbx() override;

        bool m_updating = false;
        bool m_loaded = false;
        float m_updateTime = 0;
        AzToolsFramework::EntityAccentType m_accentType = AzToolsFramework::EntityAccentType::None; ///< State of the entity selection in the viewport.
    };
}
