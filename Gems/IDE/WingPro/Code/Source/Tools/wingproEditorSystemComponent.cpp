
#include <AzCore/Serialization/SerializeContext.h>
#include "wingproEditorSystemComponent.h"

namespace wingpro
{
    void wingproEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<wingproEditorSystemComponent, AZ::Component>();
        }
    }

    wingproEditorSystemComponent::wingproEditorSystemComponent()
    {
        if (wingproInterface::Get() == nullptr)
        {
            wingproInterface::Register(this);
        }
    }

    wingproEditorSystemComponent::~wingproEditorSystemComponent()
    {
        if (wingproInterface::Get() == this)
        {
            wingproInterface::Unregister(this);
        }
    }

    void wingproEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("wingproEditorService"));
    }

    void wingproEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("wingproEditorService"));
    }

    void wingproEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void wingproEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void wingproEditorSystemComponent::Activate()
    {
        wingproRequestBus::Handler::BusConnect();
    }

    void wingproEditorSystemComponent::Deactivate()
    {
        wingproRequestBus::Handler::BusDisconnect();
    }

} // namespace wingpro
