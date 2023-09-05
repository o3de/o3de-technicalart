#pragma once

#include <Components/HoudiniNodeComponentConfig.h>
#include <HoudiniEngine/HoudiniCommonForwards.h>

#include <AzToolsFramework/Entity/EditorEntityContextBus.h>
#include <AzToolsFramework/API/ComponentEntitySelectionBus.h>

namespace HoudiniEngine
{
    #define DEFAULT_HOUDINI_TERRAIN_RESOLUTION 2

    class HoudiniScatterPoint
    {
        public:
            int m_distributionIndex;
            AZ::Vector3 m_pos;
            AZ::Transform m_transform;            
    };

    class HoudiniScatterElement
    {
    public:
        AZ_CLASS_ALLOCATOR(HoudiniScatterElement, AZ::SystemAllocator, 0);
        AZ_RTTI(HoudiniScatterElement, HOUDINI_SCATTER_ELEMENT_GUID);
        static void Reflect(AZ::ReflectContext* context);

        virtual ~HoudiniScatterElement() = default;
        AZ::Data::Asset<AZ::SliceAsset> m_asset;
        float m_weight;
        float m_radius;
    };


    class HoudiniScatterComponent
        : public AzToolsFramework::Components::EditorComponentBase
        , public HoudiniAssetRequestBus::Handler
        , public AZ::TickBus::Handler
        , public AZ::TransformNotificationBus::Handler
        , public AzFramework::EntityDebugDisplayEventBus::Handler
        , public AzToolsFramework::EditorEntityContextNotificationBus::Handler
        , public AzToolsFramework::EditorComponentSelectionRequestsBus::Handler
    {
    public:
        AZ_EDITOR_COMPONENT(HoudiniScatterComponent, HOUDINI_SCATTER_COMPONENT_GUID, AzToolsFramework::Components::EditorComponentBase);

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
        
        HoudiniScatterComponent() = default;
        ~HoudiniScatterComponent() override = default;
     
        // -- AZ::TickBus Interface -------------------------------------------------------------------------
        virtual void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        void OnTransformChanged(const AZ::Transform& localTM, const AZ::Transform& worldTM);
        // -- AZ::TickBus Interface -------------------------------------------------------------------------        

        IHoudiniNode* GetNode() override
        {
            return m_config.GetNode();
        }

        IHoudiniNodeComponentConfig * GetConfig()
        {
            return &m_config;
        }

        IHoudiniNode* LoadHda(const AZStd::string& operatorName, AZStd::function<void(IHoudiniNode*)> onLoad = {})
        {
            if (m_config.m_nodeName.empty())
                m_config.m_nodeName = m_entity->GetName();

            return m_config.LoadHda(operatorName, m_config.m_nodeName);
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

        void SaveToFbx() override;

        AZ::Crc32 OnLiveUpdateChanged();
        AZ::Crc32 OnScatterElements();
    protected:
        ////////////////////////////////////////////////////////////////////////
        // LyFactorRequestBus interface implementation
        ////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        void LoadHoudiniInstance();
        ////////////////////////////////////////////////////////////////////////

        
        // AzFramework::EntityDebugDisplayEventBus interface implementation
        void DisplayEntityViewport(const AzFramework::ViewportInfo& viewportInfo, AzFramework::DebugDisplayRequests& debugDisplay) override;

        void GatherAllEntities(const AZ::EntityId& rootId, AZStd::vector<AZ::EntityId>& listOfEntities);

        // AzToolsFramework::EditorEntityContextNotificationBus
        // O3DECONVERT
        //void OnSliceInstantiated(const AZ::Data::AssetId& sliceAssetId, AZ::SliceComponent::SliceInstanceAddress& sliceAddress, const AzFramework::SliceInstantiationTicket& ticket) override;
        
        // EditorComponentSelectionRequestsBus::Handler
        AZ::Aabb GetEditorSelectionBoundsViewport(const AzFramework::ViewportInfo& viewportInfo) override;
        bool EditorSelectionIntersectRayViewport(const AzFramework::ViewportInfo& viewportInfo, const AZ::Vector3& src, const AZ::Vector3& dir, float& distance) override;
        bool SupportsEditorRayIntersect() override { return true; };

        //Utils
        AZStd::vector<AZStd::queue<AZ::EntityId>> FindSlices(AZ::EntityId id);

        // Houdini Scatter Components
        HoudiniNodeComponentConfig m_config;
        
        bool m_loaded = false;
        bool m_liveUpdate = false;
        bool m_cmdScatter = false;

        AZStd::vector<HoudiniScatterElement> m_distribution;
        AZStd::unordered_map<AzFramework::SliceInstantiationTicket, HoudiniScatterPoint> m_tickets;


        bool IsLiveUpdate() override { return m_liveUpdate; }
        void SetLiveUpdate(bool value) override { m_liveUpdate = value; }        

    };
}
