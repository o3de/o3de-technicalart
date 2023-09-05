#include "StdAfx.h"

#include <EditorDefs.h>
#include "HoudiniScatterComponent.h"

#include <HoudiniEngine/HoudiniApi.h>
#include <HoudiniEngine/HoudiniEngineBus.h>
#include <Components/HoudiniAssetComponent.h>
#include <HoudiniCommon.h>

#include <HAPI/HAPI.h>
#include <ISystem.h>
#include <Windows.h>


namespace HoudiniEngine
{
    /*static*/ void HoudiniScatterElement::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<HoudiniScatterElement>()
                ->Version(1)
                ->Field("Asset", &HoudiniScatterElement::m_asset)
                ->Field("Weight", &HoudiniScatterElement::m_weight)
                ->Field("Radius", &HoudiniScatterElement::m_radius);

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<HoudiniScatterElement>("HoudiniScatterElement", "Houdini Scatter System")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Houdini")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)

                    ->DataElement(AZ::Edit::UIHandlers::Default, &HoudiniScatterElement::m_asset, "Slice", "The slice that you want to instance at every point")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ_CRC("RefreshAttributesAndValues", 0xcbc2147c))

                    //This is in the asset at the moment, in case we change our minds:
                    //->DataElement(AZ::Edit::UIHandlers::Default, &HoudiniScatterElement::m_weight, "Distribution weight", "The Sum is added up and this number becomes a percentage of the total.")
                    //->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ_CRC("RefreshAttributesAndValues", 0xcbc2147c))

                    //->DataElement(AZ::Edit::UIHandlers::Default, &HoudiniScatterElement::m_radius, "Radius", "The estimated size of this asset for distribution")
                    //->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ_CRC("RefreshAttributesAndValues", 0xcbc2147c))
                    
                    ;
            }
        }
    }

    /*static*/ void HoudiniScatterComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            HoudiniScatterElement::Reflect(context);

            serialize->Class<HoudiniScatterComponent, AzToolsFramework::Components::EditorComponentBase>()
                ->Version(2)
                ->Field("Distribution", &HoudiniScatterComponent::m_distribution)
                ->Field("Houdini", &HoudiniScatterComponent::m_config)
                ->Field("ScatterElements", &HoudiniScatterComponent::m_cmdScatter)
                ->Field("LiveUpdate", &HoudiniScatterComponent::m_liveUpdate);

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<HoudiniScatterComponent>("HoudiniScatterComponent", "Houdini Scatter System")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Houdini")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                    ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Houdini.png")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Houdini.png")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)

                    ->DataElement(AZ::Edit::UIHandlers::Default, &HoudiniScatterComponent::m_distribution, "Distribution Elements", "Previews are Blue, Yellow, Green, Purple, Lime")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ_CRC("RefreshAttributesAndValues", 0xcbc2147c))

                    ->DataElement(AZ::Edit::UIHandlers::Default, &HoudiniScatterComponent::m_config, "Houdini", "")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ_CRC("RefreshAttributesAndValues", 0xcbc2147c))

                    ->DataElement(AZ::Edit::UIHandlers::Button, &HoudiniScatterComponent::m_cmdScatter, "", "Scatters the elements as children below this node.")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniScatterComponent::OnScatterElements)
                    ->Attribute(AZ::Edit::Attributes::ButtonText, "Scatter Elements")

                    ->DataElement(AZ::Edit::UIHandlers::CheckBox, &HoudiniScatterComponent::m_liveUpdate, "Live Update", "EXPERIMENTAL!")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniScatterComponent::OnLiveUpdateChanged);
            }
        }
    }

    void HoudiniScatterComponent::DisplayEntityViewport(const AzFramework::ViewportInfo& /*viewportInfo*/, AzFramework::DebugDisplayRequests& debugDisplay)
    {
        AZ_PROFILE_FUNCTION(Editor);
        
        bool selected = false;
        AzToolsFramework::EditorEntityInfoRequestBus::EventResult(selected, GetEntityId(), &AzToolsFramework::EditorEntityInfoRequestBus::Events::IsSelected);

        if (selected == false)
        {
            return;
        }

        auto* dc = &debugDisplay;
        AZ_Assert(dc, "Invalid display context.");
        
        IHoudiniNode* node = m_config.m_node.get();

        if (node != nullptr)
        {
            auto points = node->GetGeometryPoints();
            AZ::Transform entityTransform;
            AZ::TransformBus::EventResult(entityTransform, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);
         
            AZStd::vector<AZ::Color> colors = 
            {
                AZ::Color(30, 144, 255, (AZ::u8)255),   //Dodger blue
                AZ::Color(236, 219, 84, (AZ::u8)255),   //Murky Yellow
                AZ::Color(0, 165, 145, (AZ::u8)255),    //Arcadia Green
                AZ::Color(188, 112, 164, (AZ::u8)255),  //Spring Purple
                AZ::Color(191, 214, 65, (AZ::u8)255)    //Dirty Lime
            };

            const auto& assetId = node->GetIntPoints("assetID");
            const auto& transforms = node->GetMatrix3Points("rotation");
                        
            for (int i = 0; i < points.size(); i++)
            {
                AZ::Vector3 point = points[i];
                point = entityTransform.TransformPoint(point); //O3DECONVERT check

                AZ::Vector3 target = AZ::Vector3(0, 0, 3);

                AZ::Transform transform = AZ::Transform::CreateIdentity();
                transform = AZ::Transform::CreateTranslation(point);

                int spawnIndex = 0;

                if (i < transforms.size())
                {
                    auto rotMat = AZ::Transform::CreateFromMatrix3x3(transforms[i]);
                    //target *= rotMat;
                    target = rotMat.TransformPoint(target); // O3DECONVERT
                }

                if (i < assetId.size())
                {
                    int newIndex = assetId[i] - 1;

                    if (newIndex >= 0 && newIndex < colors.size())
                    {
                        spawnIndex = newIndex;
                    }
                }

                dc->SetColor(colors[spawnIndex]);
                dc->DrawArrow(point, point + target, 2.0f);

                float boxSize = 0.25f;
                AZ::Vector3 tPoint = point;
                AZ::Vector3 minP = tPoint + AZ::Vector3(-boxSize, -boxSize, -boxSize);
                AZ::Vector3 maxP = tPoint + AZ::Vector3(boxSize, boxSize, boxSize);
                dc->DrawSolidBox(minP, maxP);
            }
        }
    }

    void HoudiniScatterComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("HoudiniScatterComponentService", 0x47624ae0));
    }

    void HoudiniScatterComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& /*incompatible*/)
    {

    }

    void HoudiniScatterComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC("TransformService", 0x8ee22c50));
    }

    void HoudiniScatterComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        (void)dependent;
    }

    void HoudiniScatterComponent::Init()
    {
        m_config.m_selectionMode = OperatorMode::Scatter;
    }

    void HoudiniScatterComponent::Activate()
    {
        EditorComponentBase::Activate();
        HoudiniAssetRequestBus::Handler::BusConnect(GetEntityId());

        AZ::TickBus::Handler::BusConnect();
        AZ::TransformNotificationBus::Handler::BusConnect(GetEntityId());
        AzFramework::EntityDebugDisplayEventBus::Handler::BusConnect(GetEntityId());
        AzToolsFramework::EditorEntityContextNotificationBus::Handler::BusConnect();
        AzToolsFramework::EditorComponentSelectionRequestsBus::Handler::BusConnect(GetEntityId());

        Init();
        m_config.m_nodeName = m_entity->GetName();
        m_config.m_entityId = GetEntityId();
    }

    void HoudiniScatterComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        AZ::TransformNotificationBus::Handler::BusDisconnect();
        AzFramework::EntityDebugDisplayEventBus::Handler::BusDisconnect();
        AzToolsFramework::EditorEntityContextNotificationBus::Handler::BusDisconnect();
        AzToolsFramework::EditorComponentSelectionRequestsBus::Handler::BusDisconnect();

        HoudiniAssetRequestBus::Handler::BusDisconnect();
        EditorComponentBase::Deactivate();
    }


    void HoudiniScatterComponent::LoadHoudiniInstance()
    {
        if (!m_loaded)
        {
            //Delayed load because we need to make sure other components are powered up.
            m_config.OnLoadHoudiniInstance();
            m_loaded = true;            

            HoudiniEngineRequestBus::Broadcast(&HoudiniEngineRequestBus::Events::JoinProcessorThread);

            AZ::Transform transform = AZ::Transform::CreateIdentity();
            AZ::TransformBus::EventResult(transform, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);

            m_config.UpdateWorldTransformData(transform);
        }
    }

    void HoudiniScatterComponent::OnTick(float /*deltaTime*/, AZ::ScriptTimePoint /*time*/)
    {
        //We only need this loaded upfront if we have live update on, otherwise, it can be on-demand.
        if (m_liveUpdate)
        {
            LoadHoudiniInstance();
        }

        if (m_config.UpdateNode() && m_liveUpdate)
        {
            OnScatterElements();
        }
    }

    void HoudiniScatterComponent::OnTransformChanged(const AZ::Transform& /*localTM*/, const AZ::Transform& worldTM)
    {
        m_config.UpdateWorldTransformData(worldTM);
    }
    
    AZStd::vector<AZStd::queue<AZ::EntityId>> HoudiniScatterComponent::FindSlices( AZ::EntityId parentId)
    {
        AZStd::vector<AZStd::queue<AZ::EntityId>> output;

        // Get list of child entities (check if needed child already exists before creating one)
        AzToolsFramework::EntityIdList childEntityIds;
        AzToolsFramework::EditorEntityInfoRequestBus::EventResult(childEntityIds, parentId, &AzToolsFramework::EditorEntityInfoRequests::GetChildren);

        output.resize(m_distribution.size());
        
        for (int i = 0; i < m_distribution.size(); i++)
        {
            //determine slice asset name
            AZ::Data::AssetId spawnId = m_distribution[i].m_asset.GetId();

            if (spawnId.IsValid())
            {
                AZStd::string spawnSliceName;
                AZ::Data::AssetCatalogRequestBus::BroadcastResult(spawnSliceName, &AZ::Data::AssetCatalogRequests::GetAssetPathById, spawnId);
            }
        }        

        for (auto& childEntityId : childEntityIds)
        {
            bool isSliceRoot = false;
            AzToolsFramework::EditorEntityInfoRequestBus::EventResult(isSliceRoot, childEntityId, &AzToolsFramework::EditorEntityInfoRequestBus::Events::IsSliceRoot);

            if (isSliceRoot)
            {
                AZStd::string sliceName;
                AZ::Data::AssetId sliceId;
                AzToolsFramework::EditorEntityInfoRequestBus::EventResult(sliceName, childEntityId, &AzToolsFramework::EditorEntityInfoRequestBus::Events::GetSliceAssetName);
                AZ::Data::AssetCatalogRequestBus::BroadcastResult(sliceId, &AZ::Data::AssetCatalogRequests::GetAssetIdByPath, sliceName.c_str(), AZ::Data::s_invalidAssetType, false);

                //Find bucket:
                if (sliceId.IsValid()) 
                {
                    for (int i = 0; i < m_distribution.size(); i++)
                    {
                        //determine slice asset name
                        AZ::Data::AssetId spawnId = m_distribution[i].m_asset.GetId();
                        if (spawnId.IsValid() && spawnId == sliceId)
                        {
                            output[i].push(childEntityId);
                        }
                    }
                }

            }

        }

        return output;
    }

    AZ::Crc32 HoudiniScatterComponent::OnScatterElements()
    {
        LoadHoudiniInstance();        
        HoudiniEngine::HoudiniEngineRequestBus::Broadcast(&HoudiniEngine::HoudiniEngineRequestBus::Events::JoinProcessorThread);

        if (m_config.GetNode() != nullptr)
        {
            IHoudiniNode* node = m_config.GetNode();
            node->Cook();
            HoudiniEngine::HoudiniEngineRequestBus::Broadcast(&HoudiniEngine::HoudiniEngineRequestBus::Events::JoinProcessorThread);

            AZ::Transform entityTransform = AZ::Transform::CreateIdentity();
            AZ::TransformBus::EventResult(entityTransform, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);

            const auto& points = node->GetGeometryPoints();
            const auto& assetId = node->GetIntPoints("assetID");
            const auto& transforms = node->GetMatrix3Points("rotation");

            for (int i = 0; i < m_distribution.size(); i++)
            {
                m_distribution[i].m_asset.QueueLoad();
            }

            auto existingSlices = FindSlices(GetEntityId());
            
            for (int i = 0; i < points.size(); i++)
            {
                AZ::Vector3 point = points[i];
                point = entityTransform.TransformPoint(point); // O3DECONVERT check

                AZ::Transform transform = AZ::Transform::CreateIdentity();
                transform = AZ::Transform::CreateTranslation(point);
             
                int spawnIndex = 0;

                if (i < transforms.size())
                {
                    auto rotMat = AZ::Transform::CreateFromMatrix3x3(transforms[i]);
                    //rotMat.Transpose(); //O3DECONVERT
                    transform *= rotMat;
                }

                if (i < assetId.size()) 
                {
                    int newIndex = assetId[i] - 1;
                    
                    if ( newIndex >= 0 && newIndex < m_distribution.size())
                    {
                        spawnIndex = newIndex;
                    }
                    else
                    {
                        //Not a valid item, skip it!
                        continue;
                    }
                }
                
                if (spawnIndex < 0 || spawnIndex >= m_distribution.size())
                {
                    continue;
                }

                auto& existingQueue = existingSlices[spawnIndex];
                if (existingQueue.empty())
                {
                    //Create a new slice:
                    //O3DECONVERT
                    AzFramework::SliceInstantiationTicket ticket;
                    /*AzToolsFramework::EditorEntityContextRequestBus::BroadcastResult(ticket,
                        &AzToolsFramework::EditorEntityContextRequests::InstantiateEditorSlice,
                        m_distribution[spawnIndex].m_asset,
                        transform
                    );*/
                    
                    HoudiniScatterPoint scatterPoint;
                    scatterPoint.m_pos = point;
                    scatterPoint.m_transform = transform;
                    scatterPoint.m_distributionIndex = spawnIndex;
                    m_tickets[ticket] = scatterPoint;
                }
                else
                {
                    AZ::EntityId topId = existingQueue.front();
                    existingQueue.pop();

                    //Re-use an existing slice of this type:
                    auto qRot = transform.GetRotation();//AZ::Quaternion::CreateFromTransform(transform); // O3DECONVERT
                    AZ::TransformBus::Event(topId, &AZ::TransformInterface::SetWorldTranslation, transform.GetTranslation());
                    AZ::TransformBus::Event(topId, &AZ::TransformInterface::SetLocalRotationQuaternion, qRot);
                }
            }

            AZStd::vector<AZ::EntityId> destroyList;

            //Remove extras:
            for (auto& que : existingSlices)
            {
                while (que.empty() == false)
                {                    
                    auto& topId = que.front();
                    que.pop();
                    
                    if (topId.IsValid())
                    {
                        GatherAllEntities(topId, destroyList);
                    }
                }
            }

            for (auto& deadItem : destroyList)
            {
                AZ::ComponentApplicationBus::Broadcast(&AZ::ComponentApplicationRequests::DeleteEntity, deadItem);
            }

        }

        return AZ::Edit::PropertyRefreshLevels::AttributesAndValues;
    }

    void HoudiniScatterComponent::GatherAllEntities(const AZ::EntityId& rootId, AZStd::vector<AZ::EntityId>& listOfEntities)
    {
        listOfEntities.push_back(rootId);

        AzToolsFramework::EntityIdList childEntityIds;
        AzToolsFramework::EditorEntityInfoRequestBus::EventResult(childEntityIds, rootId, &AzToolsFramework::EditorEntityInfoRequests::GetChildren);

        for (auto& child : childEntityIds)
        {
            GatherAllEntities(child, listOfEntities);
        }
    }

    /*void HoudiniScatterComponent::OnSliceInstantiated(const AZ::Data::AssetId& sliceAssetId, AZ::SliceComponent::SliceInstanceAddress& sliceAddress, const AzFramework::SliceInstantiationTicket& ticket)
    {
        if (m_tickets.find(ticket) != m_tickets.end())
        {
            HoudiniScatterPoint& scatterPoint = m_tickets[ticket];

            const AZ::SliceComponent::EntityList& entities = sliceAddress.second->GetInstantiated()->m_entities;
            for (AZ::Entity* entity : entities)
            {
                AZ::TransformInterface* entityParent;
                AZ::TransformBus::EventResult(entityParent, entity->GetId(), &AZ::TransformInterface::GetParent);

                if (entityParent == nullptr)
                {
                    AZ::TransformBus::Event(entity->GetId(), &AZ::TransformInterface::SetParent, GetEntityId());
                                        
                    auto qRot = AZ::Quaternion::CreateFromTransform(scatterPoint.m_transform);
                    AZ::TransformBus::Event(entity->GetId(), &AZ::TransformInterface::SetWorldTranslation, scatterPoint.m_transform.GetTranslation());
                    AZ::TransformBus::Event(entity->GetId(), &AZ::TransformInterface::SetLocalRotationQuaternion, qRot);
                }
            }
        }
        else
        {
            AZ_Warning("HOUDINI", false, "Scatter could not find slice");
        }
    }*/

    AZ::Aabb HoudiniScatterComponent::GetEditorSelectionBoundsViewport(const AzFramework::ViewportInfo& /*viewportInfo*/)
    {
        return AZ::Aabb::CreateCenterRadius(AZ::Vector3(0, 0, 0), 8000);
    }

    bool HoudiniScatterComponent::EditorSelectionIntersectRayViewport(const AzFramework::ViewportInfo& /*viewportInfo*/, const AZ::Vector3& /*src*/, const AZ::Vector3& /*dir*/, float& /*distance*/)
    {
        return false;
    }

    void HoudiniScatterComponent::SaveToFbx()
    {
    }

    AZ::Crc32 HoudiniScatterComponent::OnLiveUpdateChanged()
    {
        if (m_config.GetNode() != nullptr)
        {
            m_config.GetNode()->SetDirty();
        }

        return AZ::Edit::PropertyRefreshLevels::AttributesAndValues;
    }

}
