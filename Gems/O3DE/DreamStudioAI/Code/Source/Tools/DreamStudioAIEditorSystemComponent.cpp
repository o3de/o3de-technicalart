
#include <AzCore/Serialization/SerializeContext.h>
#include "DreamStudioAIEditorSystemComponent.h"

namespace DreamStudioAI
{
    void DreamStudioAIEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<DreamStudioAIEditorSystemComponent, DreamStudioAISystemComponent>()
                ->Version(0);
        }
    }

    DreamStudioAIEditorSystemComponent::DreamStudioAIEditorSystemComponent() = default;

    DreamStudioAIEditorSystemComponent::~DreamStudioAIEditorSystemComponent() = default;

    void DreamStudioAIEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("DreamStudioAIEditorService"));
    }

    void DreamStudioAIEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("DreamStudioAIEditorService"));
    }

    void DreamStudioAIEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void DreamStudioAIEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void DreamStudioAIEditorSystemComponent::Activate()
    {
        DreamStudioAISystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
    }

    void DreamStudioAIEditorSystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        DreamStudioAISystemComponent::Deactivate();
    }

} // namespace DreamStudioAI
