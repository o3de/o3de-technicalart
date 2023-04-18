
#include <DreamStudioAIModuleInterface.h>
#include "DreamStudioAIEditorSystemComponent.h"

namespace DreamStudioAI
{
    class DreamStudioAIEditorModule
        : public DreamStudioAIModuleInterface
    {
    public:
        AZ_RTTI(DreamStudioAIEditorModule, "{5DC31AAC-489A-4F20-9D54-72EA11B0179F}", DreamStudioAIModuleInterface);
        AZ_CLASS_ALLOCATOR(DreamStudioAIEditorModule, AZ::SystemAllocator, 0);

        DreamStudioAIEditorModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                DreamStudioAIEditorSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<DreamStudioAIEditorSystemComponent>(),
            };
        }
    };
}// namespace DreamStudioAI

AZ_DECLARE_MODULE_CLASS(Gem_DreamStudioAI, DreamStudioAI::DreamStudioAIEditorModule)
