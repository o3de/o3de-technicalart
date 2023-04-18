
#include "DreamStudioAISystemComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>

namespace DreamStudioAI
{
    void DreamStudioAISystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<DreamStudioAISystemComponent, AZ::Component>()
                ->Version(0)
                ;

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<DreamStudioAISystemComponent>("DreamStudioAI", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }

    void DreamStudioAISystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("DreamStudioAIService"));
    }

    void DreamStudioAISystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("DreamStudioAIService"));
    }

    void DreamStudioAISystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void DreamStudioAISystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    DreamStudioAISystemComponent::DreamStudioAISystemComponent()
    {
        if (DreamStudioAIInterface::Get() == nullptr)
        {
            DreamStudioAIInterface::Register(this);
        }
    }

    DreamStudioAISystemComponent::~DreamStudioAISystemComponent()
    {
        if (DreamStudioAIInterface::Get() == this)
        {
            DreamStudioAIInterface::Unregister(this);
        }
    }

    void DreamStudioAISystemComponent::Init()
    {
    }

    void DreamStudioAISystemComponent::Activate()
    {
        DreamStudioAIRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
    }

    void DreamStudioAISystemComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        DreamStudioAIRequestBus::Handler::BusDisconnect();
    }

    void DreamStudioAISystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
    }

} // namespace DreamStudioAI
