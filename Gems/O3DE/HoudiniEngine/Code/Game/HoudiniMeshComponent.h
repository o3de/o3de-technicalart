#pragma once
#include <AzCore/Component/Component.h>
#include <AzFramework/Asset/SimpleAsset.h>
// #include <LmbrCentral/Rendering/MaterialAsset.h>

#include <Game/HoudiniMeshData.h>
#include <HoudiniEngine/HoudiniEngineBus.h>

namespace HoudiniEngine
{
    class HoudiniMeshComponent : public AZ::Component
        //, public HoudiniMeshRequestBus::Handler
    {
        public:
            // AZStd::vector<_smart_ptr<IMaterial>> m_materials;
            AZStd::vector<HoudiniMaterialSettings> m_materialNames;            
            AZStd::vector<HoudiniMeshData> m_meshData;

            AZ_COMPONENT(HoudiniMeshComponent, HOUDINI_MESH_COMPONENT_GUID);
            static void Reflect(AZ::ReflectContext* context);
            
            HoudiniMeshComponent() = default;
            virtual ~HoudiniMeshComponent();
            
            void Activate() override;
            void Deactivate() override;

            //AZStd::vector<HoudiniMeshStatObject> GetStatObjects() override;

    };
}
