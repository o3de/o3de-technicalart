#include "StdAfx.h"

#include <EditorDefs.h>

#include "Util/Image.h"

//O3DECONVERT
//#include "RGBLayer.h"
//#include "Heightmap.h"
//#include "LayerWeight.h"
//#include "TerrainManager.h"

#include <HoudiniEngine/HoudiniEngineBus.h>
#include "HoudiniTerrainComponent.h"

#include <HoudiniCommon.h>

#include <HAPI/HAPI.h>
#include <ISystem.h>
#include <Windows.h>

#include <AzCore/Component/TickBus.h>
#include <AzToolsFramework/Entity/EditorEntityInfoBus.h>


namespace HoudiniEngine
{
    /*static*/ void HoudiniTerrainComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<HoudiniTerrainComponent, AZ::Component>()
                ->Version(1)
                ->Field("LOD", &HoudiniTerrainComponent::m_lod)
                ->Field("Houdini", &HoudiniTerrainComponent::m_config)
                ->Field("UpdateSource", &HoudiniTerrainComponent::m_cmdUpdateSource)
                ->Field("ApplyTerrainChanges", &HoudiniTerrainComponent::m_cmdApplyTerrainChanges)
                
                ;

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<HoudiniTerrainComponent>("HoudiniTerrainComponent", "Houdini Terrain Asset")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Category, "Houdini")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                        ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Houdini.png")
                        ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Houdini.png")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    
                    ->DataElement(AZ::Edit::UIHandlers::Slider, &HoudiniTerrainComponent::m_lod, "LOD", "How much resolution do you want? 0 = full, 1 = 1/2, 2 = 1/4th, etc.")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ_CRC("RefreshAttributesAndValues", 0xcbc2147c))
                        ->Attribute(AZ::Edit::Attributes::Min, 0)
                        ->Attribute(AZ::Edit::Attributes::Max, 6)
                        ->Attribute(AZ::Edit::Attributes::SoftMin, 0)
                        ->Attribute(AZ::Edit::Attributes::SoftMax, 6)
                        ->Attribute(AZ::Edit::Attributes::Step, 1)
                    
                    ->DataElement(AZ::Edit::UIHandlers::Default, &HoudiniTerrainComponent::m_config, "Houdini", "")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ_CRC("RefreshAttributesAndValues", 0xcbc2147c))

                    ->DataElement(AZ::Edit::UIHandlers::Button, &HoudiniTerrainComponent::m_cmdUpdateSource, "", "Updates the Houdini Terrain object")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniTerrainComponent::OnUpdateSource)
                        ->Attribute(AZ::Edit::Attributes::ButtonText, "Update Houdini Terrain from LY")

                    ->DataElement(AZ::Edit::UIHandlers::Button, &HoudiniTerrainComponent::m_cmdApplyTerrainChanges, "", "Deforms the terrain.")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniTerrainComponent::OnApplyTerrainChanges)
                        ->Attribute(AZ::Edit::Attributes::ButtonText, "Apply Terrain Changes")

                    ;
            }
        }
    }

    void HoudiniTerrainComponent::DisplayEntityViewport(const AzFramework::ViewportInfo& /*viewportInfo*/, AzFramework::DebugDisplayRequests& debugDisplay)
    {
        AZ_PROFILE_FUNCTION(Editor);

        auto* dc = &debugDisplay;
        AZ_Assert(dc, "Invalid display context.");        

        if (m_config.m_node != nullptr) 
        {
            
        }
    }

    void HoudiniTerrainComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("HoudiniTerrainComponentService", 0x448bbb82));
    }

    void HoudiniTerrainComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC("HoudiniTerrainComponentService", 0x448bbb82));
    }

    void HoudiniTerrainComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {    
        required.push_back(AZ_CRC("TransformService", 0x8ee22c50));
    }

    void HoudiniTerrainComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        (void)dependent;
    }

    void HoudiniTerrainComponent::SaveToFbx()
    {
    }

    void HoudiniTerrainComponent::Init()
    {
        m_config.m_selectionMode = OperatorMode::Terrain;
    }

    void HoudiniTerrainComponent::Activate()
    {
        AZ::TickBus::Handler::BusConnect();
        AzFramework::EntityDebugDisplayEventBus::Handler::BusConnect(GetEntityId()); 
        //LegacyTerrain::LegacyTerrainNotificationBus::Handler::BusConnect(); //O3DECONVERT

        Init();
        m_config.m_nodeName = m_entity->GetName();
        m_config.m_entityId = GetEntityId();
    }

    void HoudiniTerrainComponent::Deactivate()
    {
        //LegacyTerrain::LegacyTerrainNotificationBus::Handler::BusDisconnect(); //O3DECONVERT
        AZ::TickBus::Handler::BusDisconnect();
        AzFramework::EntityDebugDisplayEventBus::Handler::BusDisconnect();
    }

    void HoudiniTerrainComponent::OnTick(float /*deltaTime*/, AZ::ScriptTimePoint /*time*/)
    {
    }

    void HoudiniTerrainComponent::LoadHoudiniInstance()
    {
        if (!m_loaded)
        {
            //Delayed load because we need to make sure other components are powered up.
            m_config.OnLoadHoudiniInstance();
            m_loaded = true;

            HoudiniEngineRequestBus::Broadcast(&HoudiniEngineRequestBus::Events::JoinProcessorThread);

            //Must be identity:
            AZ::Transform transform = AZ::Transform::CreateIdentity();
            AZ::TransformBus::Event(GetEntityId(), &AZ::TransformBus::Events::SetWorldTM, transform);
            m_config.UpdateWorldTransformData(transform);
        }
    }

    void HoudiniTerrainComponent::OnUpdateSource()
    {
        LoadHoudiniInstance();

        if (m_config.m_node != nullptr)
        {
            //Must be identity:
            AZ::Transform transform = AZ::Transform::CreateIdentity();
            AZ::TransformBus::Event(GetEntityId(), &AZ::TransformBus::Events::SetWorldTM, transform);
            m_config.UpdateWorldTransformData(transform);

            m_dirty = true;
            m_config.m_node->GetHou()->GetInputNodeManager()->GetNodeIdFromEntity(GetEntityId());
            m_config.UpdateNode();
        }
    }

    void HoudiniTerrainComponent::OnApplyTerrainChanges()
    {
        LoadHoudiniInstance();
        HoudiniEngineRequestBus::Broadcast(&HoudiniEngineRequestBus::Events::JoinProcessorThread);

        if (m_config.m_node != nullptr)
        {
            m_config.UpdateNode();
            HoudiniEngineRequestBus::Broadcast(&HoudiniEngineRequestBus::Events::JoinProcessorThread);

            const HAPI_Session* session = &(m_config.m_node->GetHou()->GetSession());

            // Get the volume info.
            HAPI_VolumeInfo volume_info = HAPI_VolumeInfo_Create();
            HAPI_GeoInfo geo_info = m_config.m_node->GetGeometryInfo();

            if (geo_info.partCount > 0)
            {
                //Houdini will crash if we ask for a volume info when there is no geometry, so first check that we have geometry:
                HAPI_AttributeInfo p_info = HAPI_AttributeInfo_Create();
                HAPI_GetAttributeInfo(session, geo_info.nodeId, 0, "P", HAPI_ATTROWNER_POINT, &p_info);

                if (p_info.exists)
                {
                    HAPI_GetVolumeInfo(session, geo_info.nodeId, 0, &volume_info);
                }
            }

            if (volume_info.zLength == 1 && volume_info.tupleSize == 1)
            {
                //Came back as a height field volume.
                int totalsize = (volume_info.xLength * volume_info.yLength);

                // Get the height field data.
                AZStd::vector<float> heightfieldData(totalsize);
                HAPI_GetHeightFieldData(&m_config.m_node->GetHou()->GetSession(), geo_info.nodeId, 0, heightfieldData.data(), 0, totalsize);
                
                HoudiniTerrain terrain(volume_info.xLength, volume_info.yLength, volume_info.tileSize, volume_info.tileSize);
                terrain.LoadData(heightfieldData);
                terrain.TransferToHeightmap();
            }
            else
            {
                //Came back as geometry:
                auto points = m_config.m_node->GetGeometryPoints();
                if (!points.empty())
                {
                    HoudiniTerrain terrain(points);
                    terrain.TransferToHeightmap();
                }
            }
        }
    }

    //void HoudiniTerrainComponent::UpdateTile(const AZ::u32 /*tileX*/, const AZ::u32 /*tileY*/, const AZ::u16* /*heightMap*/, const float /*heightMin*/, const float /*heightScale*/, const AZ::u32 /*tileSize*/, const AZ::u32 /*heightMapUnitSize*/)
    //{
        //m_dirty = true;
    //}

    void HoudiniTerrainComponent::GetTerrainData(int& /*width*/, int& /*height*/, AZStd::vector<AZ::Vector3>& /*terrain*/)
    {
        /*CHeightmap* heightmap = GetIEditor()->GetTerrainManager()->GetHeightmap();
        // If the level doesn't use terrain, there's nothing to export
        if (!heightmap->GetUseTerrain()) 
        {
            return;
        }        

        int worldScale = pow(2, m_lod);

        width = heightmap->GetWidth();
        height = heightmap->GetHeight();
        
        terrain.resize(width * height);
        int terrainScale = heightmap->GetUnitSize();
        if (terrainScale < 1)
        {
            terrainScale = 1;
        }

        width /= (worldScale);
        height /= (worldScale);
        
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {                                
                int xScaledIndex = x * worldScale;
                int yScaledIndex = y * worldScale;
             
                int xScaled = x * worldScale * terrainScale;
                int yScaled = y * worldScale * terrainScale;

                //Lumberyard stores the height map as y, x.
                float value = heightmap->GetXY(yScaledIndex, xScaledIndex);
                int idx = y * width + x;

                AZ::Vector3 pos(static_cast<float>(xScaled), static_cast<float>(yScaled), value);
                terrain[idx] = pos;
            }
        }*/
    }
}
