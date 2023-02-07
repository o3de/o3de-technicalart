
#include <AzCore/Serialization/SerializeContext.h>
#include "urdf-exporter-gemEditorSystemComponent.h"

namespace urdf_exporter_gem
{
    void urdf_exporter_gemEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<urdf_exporter_gemEditorSystemComponent, AZ::Component>();
        }
    }

    urdf_exporter_gemEditorSystemComponent::urdf_exporter_gemEditorSystemComponent()
    {
        if (urdf_exporter_gemInterface::Get() == nullptr)
        {
            urdf_exporter_gemInterface::Register(this);
        }
    }

    urdf_exporter_gemEditorSystemComponent::~urdf_exporter_gemEditorSystemComponent()
    {
        if (urdf_exporter_gemInterface::Get() == this)
        {
            urdf_exporter_gemInterface::Unregister(this);
        }
    }

    void urdf_exporter_gemEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("urdf_exporter_gemEditorService"));
    }

    void urdf_exporter_gemEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("urdf_exporter_gemEditorService"));
    }

    void urdf_exporter_gemEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void urdf_exporter_gemEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void urdf_exporter_gemEditorSystemComponent::Activate()
    {
        urdf_exporter_gemRequestBus::Handler::BusConnect();
    }

    void urdf_exporter_gemEditorSystemComponent::Deactivate()
    {
        urdf_exporter_gemRequestBus::Handler::BusDisconnect();
    }

} // namespace urdf_exporter_gem
