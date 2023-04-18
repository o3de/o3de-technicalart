
#pragma once
#include <AzCore/Component/Component.h>
#include <StudioTools/StudioToolsBus.h>


namespace StudioTools
{
    /// System component for StudioTools editor
    class StudioToolsEditorSystemComponent
        : public StudioToolsRequestBus::Handler
        , public AZ::Component
    {
    public:
        AZ_COMPONENT(StudioToolsEditorSystemComponent, "{DB6B46EB-CAC0-4BD9-B33C-84F841542C58}");
        static void Reflect(AZ::ReflectContext* context);

        StudioToolsEditorSystemComponent();
        ~StudioToolsEditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate();
        void Deactivate();
    };
} // namespace StudioTools
