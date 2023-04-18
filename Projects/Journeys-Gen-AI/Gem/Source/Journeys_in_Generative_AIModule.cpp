
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>

#include "Journeys_in_Generative_AISystemComponent.h"

namespace Journeys_in_Generative_AI
{
    class Journeys_in_Generative_AIModule
        : public AZ::Module
    {
    public:
        AZ_RTTI(Journeys_in_Generative_AIModule, "{53486877-E748-4DC0-9C4D-BE10A96D8CE2}", AZ::Module);
        AZ_CLASS_ALLOCATOR(Journeys_in_Generative_AIModule, AZ::SystemAllocator, 0);

        Journeys_in_Generative_AIModule()
            : AZ::Module()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            m_descriptors.insert(m_descriptors.end(), {
                Journeys_in_Generative_AISystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<Journeys_in_Generative_AISystemComponent>(),
            };
        }
    };
}// namespace Journeys_in_Generative_AI

AZ_DECLARE_MODULE_CLASS(Gem_Journeys_in_Generative_AI, Journeys_in_Generative_AI::Journeys_in_Generative_AIModule)
