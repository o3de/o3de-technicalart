#include "StdAfx.h"

#include <AzCore/Component/Entity.h>
#include <AzCore/Serialization/EditContext.h>

#include <Game/HoudiniMeshComponent.h>
#include <Game/HoudiniMeshData.h>

#include <IIndexedMesh.h>

namespace HoudiniEngine
{
    void HoudiniMeshComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            HoudiniMeshData::Reflect(context);
            HoudiniMaterialSettings::Reflect(context);

            serializeContext->Class<HoudiniMeshComponent, AZ::Component>()
                ->Version(1)
                ->Field("MaterialNames", &HoudiniMeshComponent::m_materialNames)
                ->Field("MeshData", &HoudiniMeshComponent::m_meshData)
                ;

            // Expose properties of game-component for editing
            if (auto editContext = serializeContext->GetEditContext())
            {
                editContext->Class<HoudiniMeshComponent>("HoudiniMeshComponent", "Renders generated meshes to the screen.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Category, "HoudiniMeshComponent")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                    
                    ->DataElement(0, &HoudiniMeshComponent::m_meshData, "MeshData", "Data needed to draw things")
                    ;
            }
        }
    }

    HoudiniMeshComponent::~HoudiniMeshComponent()
    {
    }

    void HoudiniMeshComponent::Activate()
    {
        AZ_PROFILE_FUNCTION(Editor);

        //HoudiniMeshRequestBus::Handler::BusConnect(GetEntityId());

        /*m_materials.clear();

        for (auto& material : m_materialNames)
        {
            m_materials.push_back(material.LoadMaterial());
        }*/

        AZ::Transform transform = AZ::Transform::CreateIdentity();
        AZ::TransformBus::EventResult(transform, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);

        for (int i = 0; i < m_meshData.size(); i++)
        {
            auto& meshData = m_meshData[i];
            //meshData.CreateRenderNode(GetEntityId());
            //IStatObj * staticObject = meshData.GetStatObject();            //ATOMCONVERT
            //staticObject->SetGeoName(GetEntity()->GetName().c_str() );

            //Update Material:
            /*if (meshData.m_materialIndex < m_materials.size())
            {
                meshData.SetPhysics(m_materialNames[meshData.m_materialIndex].m_physics);
                meshData.SetVisible(m_materialNames[meshData.m_materialIndex].m_visible);
                meshData.SetMaterial(m_materials[meshData.m_materialIndex]);
                meshData.SetHighPriorityTextureStreaming(m_materialNames[meshData.m_materialIndex].m_highPriorityTextureStreaming);
            }
            else
            {
                meshData.SetPhysics(false);
                meshData.SetVisible(true);
                meshData.SetMaterial(nullptr);
                meshData.SetHighPriorityTextureStreaming(false);
            }*/
                        
            meshData.UpdateStatObject();
            meshData.UpdateWorldTransform(transform);
            
            //When running in game we don't need this data anymore:
            meshData.ClearSerializedMeshData();
        }
    }

    void HoudiniMeshComponent::Deactivate()
    {
        //HoudiniMeshRequestBus::Handler::BusDisconnect();
    }

    //AZStd::vector<HoudiniMeshStatObject> HoudiniMeshComponent::GetStatObjects()
    //{
    //    AZ_PROFILE_FUNCTION(Editor);

    //    AZStd::vector<HoudiniMeshStatObject> output;
    //    for (auto& geometry : m_meshData)
    //    {
    //        HoudiniMeshStatObject obj;
    //        obj.MaterialIndex = geometry.m_materialIndex;
    //        //obj.StatObject = geometry.GetStatObject(); //ATOMCONVERT
    //        output.push_back(obj);
    //    }        
    //    return output;
    //}
}
