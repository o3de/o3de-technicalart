
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>
#include <Clients/DreamStudioAISystemComponent.h>

namespace DreamStudioAI
{
    class DreamStudioAIModuleInterface
        : public AZ::Module
    {
    public:
        AZ_RTTI(DreamStudioAIModuleInterface, "{B01FF798-87E5-4AD6-AFC4-17E94FBA0B78}", AZ::Module);
        AZ_CLASS_ALLOCATOR(DreamStudioAIModuleInterface, AZ::SystemAllocator, 0);

        DreamStudioAIModuleInterface()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                DreamStudioAISystemComponent::CreateDescriptor(),
                });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<DreamStudioAISystemComponent>(),
            };
        }
    };
}// namespace DreamStudioAI
