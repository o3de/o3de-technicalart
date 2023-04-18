
#include <AzCore/Serialization/SerializeContext.h>
#include "o3de-dreamstudioEditorSystemComponent.h"

namespace o3de_dreamstudio
{
    void o3de_dreamstudioEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<o3de_dreamstudioEditorSystemComponent, AZ::Component>();
        }
    }

    o3de_dreamstudioEditorSystemComponent::o3de_dreamstudioEditorSystemComponent()
    {
        if (o3de_dreamstudioInterface::Get() == nullptr)
        {
            o3de_dreamstudioInterface::Register(this);
        }
    }

    o3de_dreamstudioEditorSystemComponent::~o3de_dreamstudioEditorSystemComponent()
    {
        if (o3de_dreamstudioInterface::Get() == this)
        {
            o3de_dreamstudioInterface::Unregister(this);
        }
    }

    void o3de_dreamstudioEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("o3de_dreamstudioEditorService"));
    }

    void o3de_dreamstudioEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("o3de_dreamstudioEditorService"));
    }

    void o3de_dreamstudioEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void o3de_dreamstudioEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void o3de_dreamstudioEditorSystemComponent::Activate()
    {
        o3de_dreamstudioRequestBus::Handler::BusConnect();
    }

    void o3de_dreamstudioEditorSystemComponent::Deactivate()
    {
        o3de_dreamstudioRequestBus::Handler::BusDisconnect();
    }

} // namespace o3de_dreamstudio
