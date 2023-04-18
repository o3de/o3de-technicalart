
#include <AzCore/Serialization/SerializeContext.h>
#include "UrdfExporterEditorSystemComponent.h"

namespace UrdfExporter
{
    void UrdfExporterEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<UrdfExporterEditorSystemComponent, AZ::Component>();
        }
    }

    UrdfExporterEditorSystemComponent::UrdfExporterEditorSystemComponent()
    {
        if (UrdfExporterInterface::Get() == nullptr)
        {
            UrdfExporterInterface::Register(this);
        }
    }

    UrdfExporterEditorSystemComponent::~UrdfExporterEditorSystemComponent()
    {
        if (UrdfExporterInterface::Get() == this)
        {
            UrdfExporterInterface::Unregister(this);
        }
    }

    void UrdfExporterEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("UrdfExporterEditorService"));
    }

    void UrdfExporterEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("UrdfExporterEditorService"));
    }

    void UrdfExporterEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void UrdfExporterEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void UrdfExporterEditorSystemComponent::Activate()
    {
        UrdfExporterRequestBus::Handler::BusConnect();
    }

    void UrdfExporterEditorSystemComponent::Deactivate()
    {
        UrdfExporterRequestBus::Handler::BusDisconnect();
    }

} // namespace UrdfExporter
