
#pragma once

#include <AzCore/Component/Component.h>

#include <Journeys_in_Generative_AI/Journeys_in_Generative_AIBus.h>

namespace Journeys_in_Generative_AI
{
    class Journeys_in_Generative_AISystemComponent
        : public AZ::Component
        , protected Journeys_in_Generative_AIRequestBus::Handler
    {
    public:
        AZ_COMPONENT(Journeys_in_Generative_AISystemComponent, "{852A1FF0-DB8A-46C6-84C4-32EEF9B918EB}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        Journeys_in_Generative_AISystemComponent();
        ~Journeys_in_Generative_AISystemComponent();

    protected:
        ////////////////////////////////////////////////////////////////////////
        // Journeys_in_Generative_AIRequestBus interface implementation

        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////
    };
}
