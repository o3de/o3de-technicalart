
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>

#include "Journeys_in_Generative_AISystemComponent.h"

namespace Journeys_in_Generative_AI
{
    void Journeys_in_Generative_AISystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<Journeys_in_Generative_AISystemComponent, AZ::Component>()
                ->Version(0)
                ;

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<Journeys_in_Generative_AISystemComponent>("Journeys_in_Generative_AI", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }

    void Journeys_in_Generative_AISystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("Journeys_in_Generative_AIService"));
    }

    void Journeys_in_Generative_AISystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC("Journeys_in_Generative_AIService"));
    }

    void Journeys_in_Generative_AISystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void Journeys_in_Generative_AISystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    Journeys_in_Generative_AISystemComponent::Journeys_in_Generative_AISystemComponent()
    {
        if (Journeys_in_Generative_AIInterface::Get() == nullptr)
        {
            Journeys_in_Generative_AIInterface::Register(this);
        }
    }

    Journeys_in_Generative_AISystemComponent::~Journeys_in_Generative_AISystemComponent()
    {
        if (Journeys_in_Generative_AIInterface::Get() == this)
        {
            Journeys_in_Generative_AIInterface::Unregister(this);
        }
    }

    void Journeys_in_Generative_AISystemComponent::Init()
    {
    }

    void Journeys_in_Generative_AISystemComponent::Activate()
    {
        Journeys_in_Generative_AIRequestBus::Handler::BusConnect();
    }

    void Journeys_in_Generative_AISystemComponent::Deactivate()
    {
        Journeys_in_Generative_AIRequestBus::Handler::BusDisconnect();
    }
}
