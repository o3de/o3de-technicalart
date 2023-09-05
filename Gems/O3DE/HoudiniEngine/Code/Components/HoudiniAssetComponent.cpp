#include "StdAfx.h"

#include <HoudiniEngine/HoudiniApi.h>
#include <HoudiniEngine/HoudiniEngineBus.h>
#include <Components/HoudiniAssetComponent.h>
#include <HoudiniCommon.h>
#include <Game/HoudiniMeshComponent.h>

#include <ISystem.h>
#include <IEditor.h>
#include <Settings.h>
#include <Windows.h>

#include <AzToolsFramework/Entity/EditorEntityInfoBus.h>

#define HDA_MAX_MB_SIZE 20.0

namespace HoudiniEngine
{    
    /*static*/ void HoudiniAssetComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<HoudiniAssetComponent, AzToolsFramework::Components::EditorComponentBase>()
                ->Version(5, HoudiniAssetComponent::VersionConverter)
                ->Field("MaterialSettings", &HoudiniAssetComponent::m_nodeExporter)
                ->Field("Houdini", &HoudiniAssetComponent::m_config)
                ->Field("FbxConfig", &HoudiniAssetComponent::m_fbxConfig)
                ;

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                //Reflect all the custom types we need for houdini property editing.
                HoudiniNodeComponentConfig::Reflect(context);
                HoudiniFbxConfig::Reflect(context);
                HoudiniNodeExporter::Reflect(context);

                HoudiniScriptProperty::Reflect(context);
                HoudiniScriptPropertyBoolean::Reflect(context);
                HoudiniScriptPropertyInt::Reflect(context);
                HoudiniScriptPropertyIntChoice::Reflect(context);
                HoudiniScriptPropertyFloat::Reflect(context);
                HoudiniScriptPropertyVector2::Reflect(context);
                HoudiniScriptPropertyVector3::Reflect(context);
                HoudiniScriptPropertyVector4::Reflect(context);
                HoudiniScriptPropertyEntity::Reflect(context);
                HoudiniScriptPropertyFile::Reflect(context);
                HoudiniScriptPropertyFileAny::Reflect(context);
                HoudiniScriptPropertyFileImage::Reflect(context);
                HoudiniScriptPropertyString::Reflect(context);
                HoudiniScriptPropertyInput::Reflect(context);
                HoudiniScriptPropertyColor::Reflect(context); // FL[FD-11758] Add support for color parameter to lumberyard houdini engine
                // FL[FD-11761] Add support for multiparm blocks (lists) to lumberyard houdini engine
                HoudiniMultiparamInstance::Reflect(context);
                HoudiniScriptPropertyMultiparm::Reflect(context);
                HoudiniScriptPropertyButton::Reflect(context);  // FL[FD-12463] Implement button interface parm in houdini engine for lumberyard
                HoudiniPropertyGroup::Reflect(context);
				

                ec->Class<HoudiniAssetComponent>("HoudiniDigitalAsset", "Houdini Digital Asset")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Houdini")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Houdini.png")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Houdini.png")
                    ->Attribute(AZ::Edit::Attributes::RuntimeExportCallback, &HoudiniAssetComponent::ExportMeshComponent)

                    ->DataElement(AZ::Edit::UIHandlers::Default, &HoudiniAssetComponent::m_nodeExporter, "Material Settings", "")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ_CRC("RefreshAttributesAndValues", 0xcbc2147c))

                    ->DataElement(AZ::Edit::UIHandlers::Default, &HoudiniAssetComponent::m_config, "Houdini", "")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ_CRC("RefreshAttributesAndValues", 0xcbc2147c))

					->DataElement(0, &HoudiniAssetComponent::m_fbxConfig, "Bake Settings", "")
					//->Attribute(AZ::Edit::Attributes::ChangeNotify, &EditorDebugDrawLineComponent::OnPropertyUpdate)
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }

    AZ::ExportedComponent HoudiniAssetComponent::ExportMeshComponent(AZ::Component* /*thisComponent*/, const AZ::PlatformTagSet& /*platformTags*/)
    {
        //TODO: what does this function do exactly???

        //AZ_PROFILE_FUNCTION(Editor);

        //HoudiniAssetComponent* thisCompo = static_cast<HoudiniAssetComponent*>(thisComponent);

        auto* meshCompo = aznew HoudiniMeshComponent();
        //auto& meshData = thisCompo->m_nodeExporter.GetModelData();
        //
        //meshCompo->m_meshData.assign(meshData.begin(), meshData.end());
        //meshCompo->m_materialNames.assign(m_nodeExporter.m_materialSettings.begin(), m_nodeExporter.m_materialSettings.end());

        //size_t totalBytes = 0;

        ////Don't share render nodes:
        //for (auto& meshElement : meshCompo->m_meshData)
        //{
        //    //meshElement.m_renderNode = nullptr;
        //    totalBytes += meshElement.GetBytesEstimate();
        //}
        //
        //double totalMegaBytes = (double)totalBytes / 1024.0 / 1024.0;
        //if (totalMegaBytes > HDA_MAX_MB_SIZE)
        //{
        //    AZ_Warning("HOUDINI", false, "Potential Error: Entity HDA %s is taking up a large amount of memory during export: %f MB", thisCompo->GetEntity()->GetName().c_str(), totalMegaBytes);
        //}
        //else
        //{
        //    AZ::Debug::Trace::Instance().Printf("HOUDINI", "Entity %s HDA using: %f MB", thisCompo->GetEntity()->GetName().c_str(), totalMegaBytes);
        //}

        return AZ::ExportedComponent(meshCompo, true);
    }

    bool HoudiniAssetComponent::VersionConverter(AZ::SerializeContext& context, AZ::SerializeContext::DataElementNode& classElement)
    {
        if (classElement.GetVersion() == 1)
        {          
            //Convert the base class up.
            for (int i = 0; i < classElement.GetNumSubElements(); ++i)
            {
                AZ::SerializeContext::DataElementNode& elementNode = classElement.GetSubElement(i);
                if (elementNode.GetName() == AZ_CRC("BaseClass1", 0xd4925735))
                {
                    elementNode.Convert(context, AzToolsFramework::Components::EditorComponentBase::TYPEINFO_Uuid());
                }
            }

            HoudiniNodeComponentConfig newConfig;
            classElement.GetChildData(AZ_CRC("OperatorType", 0xe5e6918a), newConfig.m_operatorName);
            classElement.GetChildData(AZ_CRC("Props", 0x5b7eaaa8), newConfig.m_properties);
            classElement.GetChildData(AZ_CRC("ViewButton", 0xd21de7b2), newConfig.m_viewButton);
            classElement.GetChildData(AZ_CRC("ViewReloadButton", 0xd276126b), newConfig.m_viewReloadButton);
            classElement.GetChildData(AZ_CRC("ViewDebugButton", 0x16c31f4b), newConfig.m_viewDebugButton);

            classElement.RemoveElementByName(AZ_CRC("Node", 0x857fe845));
            classElement.RemoveElementByName(AZ_CRC("OperatorType", 0xe5e6918a));
            classElement.RemoveElementByName(AZ_CRC("Props", 0x5b7eaaa8));
            classElement.RemoveElementByName(AZ_CRC("ViewButton", 0xd21de7b2));
            classElement.RemoveElementByName(AZ_CRC("ViewReloadButton", 0xd276126b));
            classElement.RemoveElementByName(AZ_CRC("ViewDebugButton", 0x16c31f4b));

            classElement.AddElementWithData<HoudiniNodeComponentConfig>(context, "Houdini", newConfig);
        }     
        else if (classElement.GetVersion() == 3)
        {                        
            AZStd::vector<HoudiniMaterialSettings> newMaterials;
            newMaterials.push_back(HoudiniMaterialSettings());

            HoudiniNodeExporter exporter;
            exporter.m_materialSettings = newMaterials;

            //Materials became an array now move to the first position in the array:
            // TODO:
            //classElement.GetChildData(AZ_CRC("Material", 0x7cbe7595), exporter.m_materialSettings[0].m_materialAsset);
            classElement.GetChildData(AZ_CRC("Visible", 0x7ab0e859), exporter.m_materialSettings[0].m_visible);

            classElement.RemoveElementByName(AZ_CRC("Material", 0x7cbe7595));
            classElement.RemoveElementByName(AZ_CRC("Visible", 0x7ab0e859));
            classElement.RemoveElementByName(AZ_CRC("LiveUpdate", 0x42c7d4f9));

            classElement.AddElementWithData<HoudiniNodeExporter>(context, "MaterialSettings", exporter);
        }
        else if (classElement.GetVersion() == 4)
        {
            classElement.RemoveElementByName(AZ_CRC("LiveUpdate", 0x42c7d4f9));
        }

        return true;
    }
    
    void HoudiniAssetComponent::Init()
    {
        EditorComponentBase::Init();
    }

    void HoudiniAssetComponent::Activate()
    {
        EditorComponentBase::Activate();
        HoudiniAssetRequestBus::Handler::BusConnect(GetEntityId());
        //HoudiniMeshRequestBus::Handler::BusConnect(GetEntityId());
        HoudiniMaterialRequestBus::Handler::BusConnect(GetEntityId());

        AZ::TickBus::Handler::BusConnect();
        AZ::TransformNotificationBus::Handler::BusConnect(GetEntityId());
        AzFramework::EntityDebugDisplayEventBus::Handler::BusConnect(GetEntityId());
        //AzToolsFramework::EditorComponentSelectionRequestsBus::Handler::BusConnect(GetEntityId());
        AzToolsFramework::EditorComponentSelectionNotificationsBus::Handler::BusConnect(GetEntityId());
        AzToolsFramework::EditorVisibilityNotificationBus::Handler::BusConnect(GetEntityId());
        LmbrCentral::SplineComponentNotificationBus::Handler::BusConnect(GetEntityId());

        if (AZ::EntityBus::Handler::BusIsConnectedId(GetEntityId()) == false)
        {
            AZ::EntityBus::Handler::BusConnect(GetEntityId());
        }
        
        m_config.m_entityId = GetEntityId();        
		m_nodeExporter.Initialize(GetEntityId(), &m_fbxConfig);
		m_nodeExporter.SetDirty(true);
        m_nodeExporter.GenerateMeshMaterials();

        AZ::Transform transform = AZ::Transform::CreateIdentity();
        AZ::TransformBus::EventResult(transform, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);

        m_config.UpdateWorldTransformData(transform);
        m_nodeExporter.UpdateWorldTransformData(transform);
        GetIEditor()->RegisterNotifyListener(this);
    }

    void HoudiniAssetComponent::Deactivate()
    {
        m_nodeExporter.RemoveMeshData();
        GetIEditor()->UnregisterNotifyListener(this);
        
        //HoudiniMeshRequestBus::Handler::BusDisconnect();

        AZ::TickBus::Handler::BusDisconnect();
        AZ::TransformNotificationBus::Handler::BusDisconnect(GetEntityId());
        AzFramework::EntityDebugDisplayEventBus::Handler::BusDisconnect();
        //AzToolsFramework::EditorComponentSelectionRequestsBus::Handler::BusDisconnect();
        AzToolsFramework::EditorComponentSelectionNotificationsBus::Handler::BusDisconnect();
        AzToolsFramework::EditorVisibilityNotificationBus::Handler::BusDisconnect();
        LmbrCentral::SplineComponentNotificationBus::Handler::BusDisconnect();

        // If I disconnect... I can't get the destruction event...
        //AZ::EntityBus::Handler::BusDisconnect();

        HoudiniAssetRequestBus::Handler::BusDisconnect();
        EditorComponentBase::Deactivate();
    }
    
    void HoudiniAssetComponent::OnEntityVisibilityChanged(bool visibility)
    {
        m_nodeExporter.SetVisibleInEditor(visibility);
    }

    void HoudiniAssetComponent::OnEntityNameChanged(const AZStd::string& name)
    {
        if (m_config.GetNode() != nullptr)
        {
            m_config.RenameNode(name);
        }
    }

    void HoudiniAssetComponent::OnEntityDestruction(const AZ::EntityId& /*id*/)
    {
        //The entity was destroyed, clean up things here        
    }

    void HoudiniAssetComponent::OnTransformChanged(const AZ::Transform& /*localTM*/, const AZ::Transform& worldTM)
    {
        m_config.UpdateWorldTransformData(worldTM);
        m_nodeExporter.UpdateWorldTransformData(worldTM);
    }

    void HoudiniAssetComponent::SaveToFbx()
    {
        AZ_PROFILE_FUNCTION(Editor);
        if (m_config.GetNode() != nullptr)
        {
            m_config.UpdateNode();
            m_nodeExporter.SetNode(m_config.GetNode());
            m_nodeExporter.SaveToFbx();
        }
    }

    void HoudiniAssetComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("HoudiniAssetComponentService", 0xf22d18e6));
    }

    void HoudiniAssetComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC("HoudiniAssetComponentService", 0xf22d18e6));
    }

    void HoudiniAssetComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC("TransformService", 0x8ee22c50));
    }

    void HoudiniAssetComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        (void)dependent;
    }

    HoudiniAssetComponent::~HoudiniAssetComponent()
    {
        if (GetEntity() != nullptr)
        {
            HoudiniEngineRequestBus::Broadcast(&HoudiniEngineRequestBus::Events::CancelProcessorJob, GetEntityId());
        }
    }

    void HoudiniAssetComponent::OnTick(float deltaTime, AZ::ScriptTimePoint /*time*/)
    {
        AZ_PROFILE_FUNCTION(Editor);
        HoudiniPtr hou;
        HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);
        if (hou != nullptr)
        {
            //Update the lookup tables:
            hou->LookupId(GetEntityId());
        }

        if (!m_loaded)
        {
            //Delayed load because we need to make sure other components are powered up.
            m_config.OnLoadHoudiniInstance();
            m_loaded = true;

            AZ::Transform transform = AZ::Transform::CreateIdentity();
            AZ::TransformBus::EventResult(transform, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);
            
            m_config.UpdateWorldTransformData(transform);
            m_nodeExporter.UpdateWorldTransformData(transform);

            bool currentVisibility = true;
            AzToolsFramework::EditorEntityInfoRequestBus::EventResult(currentVisibility, GetEntityId(), &AzToolsFramework::EditorEntityInfoRequestBus::Events::IsVisible);
            m_nodeExporter.SetVisibleInEditor(currentVisibility);
        }

        // FL[FD-10789] Support Mesh as Input to Houdini Digital Asset
        float fUpdatePeriod{ 0.25f };
        if (gEnv && gEnv->pConsole)
        {
            fUpdatePeriod = gEnv->pConsole->GetCVar("hou_update_period")->GetFVal();
        }
        fUpdatePeriod = (fUpdatePeriod < 0.01f || fUpdatePeriod >= 1.0f) ? 0.25f : fUpdatePeriod;

        m_updateTime += deltaTime;
        if (m_updateTime < fUpdatePeriod)  // FL[FD-10789] Support Mesh as Input to Houdini Digital Asset
        {
            //Update only once every 250 ms.
            return;
        }
        m_updateTime = 0;

        if (m_config.GetNode() != nullptr)
        {            
            //Handle any non-threaded functions:
            m_nodeExporter.OnTick();

            AZ::EntityId myId = GetEntityId();

            bool isSelected = false;
            AzToolsFramework::ToolsApplicationRequests::Bus::BroadcastResult(isSelected, &AzToolsFramework::ToolsApplicationRequests::IsSelected, myId);

            if ( m_updating == false )
            {
                *m_config.GetNode()->GetHou() << "==========Update " << GetNodeName() << " ========" << "";
                m_updating = true;                                

                m_config.GetNode()->GetHou()->ExecuteCommand(myId, [this]()
                {
                    bool updated = m_config.UpdateNode();

                    if (m_config.m_locked == false)
                    {
                        m_nodeExporter.SetNode(m_config.GetNode());
                        //m_nodeExporter.GenerateMeshMaterials();
                        if(m_nodeExporter.GenerateMeshData())
                            m_nodeExporter.GenerateEditableMeshData();
                    }

                    if (updated && m_nodeExporter.IsDirty())
                    {
                        AZ::Transform transform = m_config.GetNode()->GetHou()->LookupTransform(GetEntityId());
                        m_nodeExporter.UpdateWorldTransformData(transform);
                    }

                    m_updating = false;
                    return updated;
                });

                if (isSelected)
                {
                    m_config.GetNode()->GetHou()->RaiseCommandPriority(myId);
                }
            }
        }

        if (m_nodeExporter.IsDirty())
        {
            m_nodeExporter.RebuildRenderMesh();
        }

        //TODO:
        ////Update any nodes that need to display.
        //for (auto& data : m_nodeExporter.GetMeshData())
        //{
        //    if (data.IsRenderNodeDirty())
        //    {
        //        //TODO: inform mesh component that new data is available for rebuilding.

        //    }
        //}
    }        

    void HoudiniAssetComponent::OnEditorNotifyEvent(EEditorNotifyEvent event)
    {
        switch (event)
        {
            case eNotify_OnBeginSceneSave:
                
                break;
        }
    }

    //AZ::Aabb HoudiniAssetComponent::GetEditorSelectionBoundsViewport(const AzFramework::ViewportInfo& /*viewportInfo*/)
    //{
    //    AABB output;
    //    output.Reset();

    //    //ATOMCONVERT
    //    /*auto& statObjects = m_nodeExporter.GetStatObjects();
    //    for (auto* geometry : statObjects)
    //    {
    //        output.Add(geometry->GetAABB());
    //    }*/

    //    float size = output.GetSize().len();

    //    AZ::Transform transform = AZ::Transform::CreateIdentity();
    //    AZ::TransformBus::EventResult(transform, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);

    //    if (size < 0.0001 || size > 8000)
    //    {            
    //        return AZ::Aabb::CreateCenterRadius(transform.GetTranslation(), 8000);
    //    }

    //    AZ::Aabb outputAz = LyAABBToAZAabb(output);
    //    outputAz.ApplyTransform(transform);

    //    return outputAz;
    //}

    //bool HoudiniAssetComponent::EditorSelectionIntersectRayViewport(const AzFramework::ViewportInfo& /*viewportInfo*/, const AZ::Vector3& /*src*/, const AZ::Vector3& /*dir*/, float& /*distance*/)
    //{
    //    AZ_PROFILE_FUNCTION(Editor);
    //    //O3DECONVERT
    //    /*
    //    auto statObjects = m_nodeExporter.GetStatObjects();
    //    for ( auto* geometry : statObjects)        
    //    {
    //        AZ::Transform transform = AZ::Transform::CreateIdentity();
    //        AZ::TransformBus::EventResult(transform, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);
    //        auto legacyTransform = AZTransformToLYTransform(transform);
    //        const auto legacySrc = AZVec3ToLYVec3(src);
    //        const auto legacyDir = AZVec3ToLYVec3(dir);

    //        const Matrix34 inverseTM = legacyTransform.GetInverted();
    //        const Vec3 raySrcLocal = inverseTM.TransformPoint(legacySrc);
    //        const Vec3 rayDirLocal = inverseTM.TransformVector(legacyDir).GetNormalized();

    //        //O3DEConvert
    //        SRayHitInfo hi;
    //        hi.inReferencePoint = raySrcLocal;
    //        hi.inRay = Ray(raySrcLocal, rayDirLocal);
    //        if (geometry->RayIntersection(hi))
    //        {
    //            const Vec3 worldHitPos = legacyTransform.TransformPoint(hi.vHitPos);
    //            distance = legacySrc.GetDistance(worldHitPos);
    //            return true;
    //        }
    //    }
    //    */

    //    return false;
    //}

    void HoudiniAssetComponent::OnAccentTypeChanged(AzToolsFramework::EntityAccentType accent)
    {
        m_accentType = accent;
    }

    void HoudiniAssetComponent::OnSplineChanged()
    {
        m_nodeExporter.OnSplineChanged();
    }

    void HoudiniAssetComponent::OnVerticesSet(const AZStd::vector<AZ::Vector3>& /*vertices*/)
    {
        m_config.m_hasSpline = true;
    }

    void HoudiniAssetComponent::OnVerticesCleared()
    {
        m_config.m_hasSpline = false;
    }


    //AZStd::vector<HoudiniMeshStatObject> HoudiniAssetComponent::GetStatObjects()
    //{
    //    AZ_PROFILE_FUNCTION(Editor);

    //    AZStd::vector<HoudiniMeshStatObject> output;
    //    auto& statObjects = m_nodeExporter.GetMeshData();
    //    for (auto& meshData: statObjects)
    //    {
    //        HoudiniMeshStatObject obj;
    //        obj.MaterialIndex = meshData.m_materialIndex;
    //        //obj.StatObject = meshData.GetStatObject(); //ATOMCONVERT
    //        output.push_back(obj);
    //    }

    //    return output;
    //}

    void HoudiniAssetComponent::SetMaterialPath(const AZStd::string& materialName, const AZStd::string& materialPath)
    {
        m_nodeExporter.SetMaterialPath(materialName, materialPath);
    }

    //TODO:
    //static void DecideColor(int index, const bool selected, const bool mouseHovered, const bool /*visible*/, ColorB& triangleColor, ColorB& lineColor)
    //{
    //    
    //    static AZStd::vector<ColorB> LotsOfLineColors =
    //    {
    //        ColorB(100, 149, 237, 60), //BLUE!
    //        ColorB(0x09, 0x7A, 0xFF, 60),
    //        ColorB(0xFA, 0x33, 0x9A, 60),
    //        ColorB(0xFF, 0x8A, 0x00, 60),
    //        ColorB(0x6A, 0xE8, 0x3A, 60),
    //    };

    //    static AZStd::vector<ColorB> LotsOfTriColors =
    //    {
    //        ColorB(198,226,255, 90), //LightBlue
    //        ColorB(0xF9, 0xED, 0x3A, 90),
    //        ColorB(0x4C, 0xB2, 0xD4, 90),
    //        ColorB(0x84, 0x4D, 0x9E, 90),
    //        ColorB(0xEB, 0x7B, 0x2D, 90),
    //    };

    //    const ColorB shadeOfLightBlue = ColorB(100, 149, 237, 60);
    //    const ColorB shadeOfLightBlueSelected = ColorB(100, 149, 237, 90);
    //    const ColorB shadeOfBlue = ColorB(30, 144, 255, 25);

    //    // default both colors to hidden
    //    triangleColor = ColorB(AZ::u32(0));
    //    lineColor = ColorB(AZ::u32(0));

    //    if (selected)
    //    {                                    
    //        lineColor = LotsOfLineColors[index % LotsOfLineColors.size()];

    //        if (mouseHovered)
    //        {
    //            triangleColor = LotsOfTriColors[index%LotsOfTriColors.size()];
    //        }
    //    }
    //    else
    //    {
    //        if (mouseHovered)
    //        {
    //            triangleColor = LotsOfTriColors[index%LotsOfTriColors.size()];
    //            lineColor = LotsOfLineColors[index%LotsOfLineColors.size()];
    //        }
    //    }
    //}

    void HoudiniAssetComponent::DisplayEntityViewport(const AzFramework::ViewportInfo& /*viewportInfo*/, AzFramework::DebugDisplayRequests& /*debugDisplay*/)
    {
        AZ_PROFILE_FUNCTION(Editor);

        bool selected = false;
        AzToolsFramework::EditorEntityInfoRequestBus::EventResult(selected, GetEntityId(), &AzToolsFramework::EditorEntityInfoRequestBus::Events::IsSelected);

        const bool mouseHovered = m_accentType == AzToolsFramework::EntityAccentType::Hover;

        IEditor* editor = nullptr;
        AzToolsFramework::EditorRequests::Bus::BroadcastResult(editor, &AzToolsFramework::EditorRequests::GetEditor);

        const bool highlightGeometryOnMouseHover = editor->GetEditorSettings()->viewports.bHighlightMouseOverGeometry;
        // if the mesh component is not visible, when selected we still draw the wireframe to indicate the shapes extent and position
        const bool highlightGeometryWhenSelected = editor->GetEditorSettings()->viewports.bHighlightSelectedGeometry;

        if ((!selected && mouseHovered && highlightGeometryOnMouseHover) || (selected && highlightGeometryWhenSelected))
        {
            AZ::Transform transform = AZ::Transform::CreateIdentity();
            AZ::TransformBus::EventResult(transform, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);

            //auto statObjects = m_nodeExporter.GetStatObjects();
            //for (int i = 0; i < statObjects.size(); i++)
            //{
            //    //auto* geometry = statObjects[i];
            //    ColorB triangleColor, lineColor;
            //    DecideColor(i, IsSelected(), mouseHovered, true, triangleColor, lineColor);

            //    //O3DECONVERT - debug draw gem exists?
            //    //ATOMCONVERT
            //    /*SGeometryDebugDrawInfo dd;
            //    dd.tm = AZTransformToLYTransform(transform);
            //    dd.bExtrude = true;
            //    dd.color = triangleColor;
            //    dd.lineColor = lineColor;
            //
            //    //Must lock to protect against changes from the houdini thread.
            //    AZStd::unique_lock<AZStd::mutex> theLock(HoudiniMeshData::m_dataLock);
            //    geometry->DebugDraw(dd);*/
            //}
        }
    }
}
