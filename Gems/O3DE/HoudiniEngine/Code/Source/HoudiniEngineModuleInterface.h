
#include "StdAfx.h"

#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>
#include <Clients/HoudiniEngineSystemComponent.h>
#include <Game/HoudiniMeshComponent.h>

namespace HoudiniEngine
{
    class HoudiniEngineModuleInterface
        : public CryHooksModule
    {
    public:
        AZ_RTTI(HoudiniEngineModuleInterface, "{85AF1A64-B987-4CED-94FD-20002A73992E}", CryHooksModule);
        AZ_CLASS_ALLOCATOR(HoudiniEngineModuleInterface, AZ::SystemAllocator, 0);

        HoudiniEngineModuleInterface()
            : CryHooksModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                HoudiniMeshComponent::CreateDescriptor(),
                HoudiniEngineSystemComponent::CreateDescriptor(),
                });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<HoudiniEngineSystemComponent>(),
            };
        }
    };
}// namespace HoudiniEngine
