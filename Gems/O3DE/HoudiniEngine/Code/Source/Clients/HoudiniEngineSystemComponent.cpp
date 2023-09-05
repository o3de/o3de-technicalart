
#include "HoudiniEngineSystemComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>

namespace HoudiniEngine
{
    void HoudiniEngineSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<HoudiniEngineSystemComponent, AZ::Component>()
                ->Version(0)
                ;

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<HoudiniEngineSystemComponent>("HoudiniEngine", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }

    void HoudiniEngineSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("HoudiniEngineService"));
    }

    void HoudiniEngineSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("HoudiniEngineService"));
    }

    void HoudiniEngineSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void HoudiniEngineSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    HoudiniEngineSystemComponent::HoudiniEngineSystemComponent()
    {
        /*if (HoudiniEngineInterface::Get() == nullptr)
        {
            HoudiniEngineInterface::Register(this);
        }*/
    }

    HoudiniEngineSystemComponent::~HoudiniEngineSystemComponent()
    {
        /*if (HoudiniEngineInterface::Get() == this)
        {
            HoudiniEngineInterface::Unregister(this);
        }*/
    }

    void HoudiniEngineSystemComponent::Init()
    {
    }

    void HoudiniEngineSystemComponent::Activate()
    {
        //HoudiniEngineRequestBus::Handler::BusConnect();
        
        AZ::TickBus::Handler::BusConnect();
    }

    void HoudiniEngineSystemComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        //HoudiniEngineRequestBus::Handler::BusDisconnect();
    }

    void HoudiniEngineSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
    }

} // namespace HoudiniEngine
