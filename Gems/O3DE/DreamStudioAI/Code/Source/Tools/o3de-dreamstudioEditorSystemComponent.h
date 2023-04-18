
#pragma once
#include <AzCore/Component/Component.h>
#include <o3de-dreamstudio/o3de-dreamstudioBus.h>


namespace o3de_dreamstudio
{
    /// System component for o3de_dreamstudio editor
    class o3de_dreamstudioEditorSystemComponent
        : public o3de_dreamstudioRequestBus::Handler
        , public AZ::Component
    {
    public:
        AZ_COMPONENT(o3de_dreamstudioEditorSystemComponent, "{27D6E094-B1B1-4DBD-B555-41BAF4BEB1D2}");
        static void Reflect(AZ::ReflectContext* context);

        o3de_dreamstudioEditorSystemComponent();
        ~o3de_dreamstudioEditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate();
        void Deactivate();
    };
} // namespace o3de_dreamstudio
