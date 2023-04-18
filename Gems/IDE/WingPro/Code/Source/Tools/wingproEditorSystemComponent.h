
#pragma once
#include <AzCore/Component/Component.h>
#include <wingpro/wingproBus.h>


namespace wingpro
{
    /// System component for wingpro editor
    class wingproEditorSystemComponent
        : public wingproRequestBus::Handler
        , public AZ::Component
    {
    public:
        AZ_COMPONENT(wingproEditorSystemComponent, "{70F38C41-B94A-4127-AB8E-B3B90641F680}");
        static void Reflect(AZ::ReflectContext* context);

        wingproEditorSystemComponent();
        ~wingproEditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate();
        void Deactivate();
    };
} // namespace wingpro
