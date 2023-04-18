
#pragma once
#include <AzCore/Component/Component.h>
#include <UrdfExporter/UrdfExporterBus.h>


namespace UrdfExporter
{
    /// System component for UrdfExporter editor
    class UrdfExporterEditorSystemComponent
        : public UrdfExporterRequestBus::Handler
        , public AZ::Component
    {
    public:
        AZ_COMPONENT(UrdfExporterEditorSystemComponent, "{7F4B1D9D-2346-4836-A197-837860FCC9D0}");
        static void Reflect(AZ::ReflectContext* context);

        UrdfExporterEditorSystemComponent();
        ~UrdfExporterEditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate();
        void Deactivate();
    };
} // namespace UrdfExporter
