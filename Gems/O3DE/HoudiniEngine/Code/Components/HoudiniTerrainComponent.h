#pragma once

#include <HoudiniEngine/HoudiniCommonForwards.h>
#include <Components/HoudiniNodeComponentConfig.h>

//O3DECONVERT
//#include <Terrain/Bus/LegacyTerrainBus.h>

namespace HoudiniEngine
{
    #define DEFAULT_HOUDINI_TERRAIN_RESOLUTION 2

    class HoudiniTerrainComponent
        : public AzToolsFramework::Components::EditorComponentBase
        , public HoudiniAssetRequestBus::Handler
        , public AzFramework::EntityDebugDisplayEventBus::Handler
        //, public LegacyTerrain::LegacyTerrainNotificationBus::Handler
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(HoudiniTerrainComponent, HOUDINI_TERRAIN_COMPONENT_GUID, AzToolsFramework::Components::EditorComponentBase);

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
        HoudiniTerrainComponent() = default;
        ~HoudiniTerrainComponent() override = default;
     
        // -- AZ::TickBus Interface -------------------------------------------------------------------------
        virtual void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        void LoadHoudiniInstance();

        //Terrain
        //void SetNumTiles(const AZ::u32 numTiles, const AZ::u32 tileSize) override {};
        //void UpdateTile(const AZ::u32 tileX, const AZ::u32 tileY, const AZ::u16* heightMap, const float heightMin, const float heightScale, const AZ::u32 tileSize, const AZ::u32 heightMapUnitSize) override;

        void GetTerrainData(int& width, int& height, AZStd::vector<AZ::Vector3>& terrain);

        IHoudiniNode* GetNode()
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

        void SaveToFbx() override;

        bool IsDirty() 
        {
            return m_dirty; 
        }

        void SetDirty(bool state)
        {
            m_dirty = state;
        }

    protected:
        ////////////////////////////////////////////////////////////////////////
        // LyFactorRequestBus interface implementation
        ////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////


        //////////////////////////////////////////////////////////////////////////
        // AzFramework::EntityDebugDisplayEventBus interface implementation
        void DisplayEntityViewport(const AzFramework::ViewportInfo& viewportInfo, AzFramework::DebugDisplayRequests& debugDisplay) override;
        //////////////////////////////////////////////////////////////////////////

        //Default is 1/4th resolution.
        int m_lod = DEFAULT_HOUDINI_TERRAIN_RESOLUTION;
        bool m_cmdApplyTerrainChanges = false;
        bool m_cmdUpdateSource = false;

        void OnUpdateSource();
        void OnApplyTerrainChanges();
        
        bool m_loaded = false;
        bool m_dirty = true;
        HoudiniNodeComponentConfig m_config;
    };
}
