
#include "GeomNodesSystemComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>

namespace GeomNodes
{
    void GeomNodesSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<GeomNodesSystemComponent, AZ::Component>()
                ->Version(0)
                ;

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<GeomNodesSystemComponent>("GeomNodes", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }

    void GeomNodesSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("GeomNodesService"));
    }

    void GeomNodesSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("GeomNodesService"));
    }

    void GeomNodesSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void GeomNodesSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    GeomNodesSystemComponent::GeomNodesSystemComponent()
    {
        if (GeomNodesInterface::Get() == nullptr)
        {
            GeomNodesInterface::Register(this);
        }
    }

    GeomNodesSystemComponent::~GeomNodesSystemComponent()
    {
        if (GeomNodesInterface::Get() == this)
        {
            GeomNodesInterface::Unregister(this);
        }
    }

    void GeomNodesSystemComponent::Init()
    {
    }

    void GeomNodesSystemComponent::Activate()
    {
        GeomNodesRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
    }

    void GeomNodesSystemComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        GeomNodesRequestBus::Handler::BusDisconnect();
    }

    void GeomNodesSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
    }

} // namespace GeomNodes
