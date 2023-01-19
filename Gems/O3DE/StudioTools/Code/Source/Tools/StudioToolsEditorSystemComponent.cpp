
#include <AzCore/Serialization/SerializeContext.h>
#include "StudioToolsEditorSystemComponent.h"

namespace StudioTools
{
    void StudioToolsEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<StudioToolsEditorSystemComponent, AZ::Component>();
        }
    }

    StudioToolsEditorSystemComponent::StudioToolsEditorSystemComponent()
    {
        if (StudioToolsInterface::Get() == nullptr)
        {
            StudioToolsInterface::Register(this);
        }
    }

    StudioToolsEditorSystemComponent::~StudioToolsEditorSystemComponent()
    {
        if (StudioToolsInterface::Get() == this)
        {
            StudioToolsInterface::Unregister(this);
        }
    }

    void StudioToolsEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("StudioToolsEditorService"));
    }

    void StudioToolsEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("StudioToolsEditorService"));
    }

    void StudioToolsEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void StudioToolsEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void StudioToolsEditorSystemComponent::Activate()
    {
        StudioToolsRequestBus::Handler::BusConnect();
    }

    void StudioToolsEditorSystemComponent::Deactivate()
    {
        StudioToolsRequestBus::Handler::BusDisconnect();
    }

} // namespace StudioTools
