
#pragma once
#include <AzCore/Component/Component.h>
#include <urdf-exporter-gem/urdf-exporter-gemBus.h>


namespace urdf_exporter_gem
{
    /// System component for urdf_exporter_gem editor
    class urdf_exporter_gemEditorSystemComponent
        : public urdf_exporter_gemRequestBus::Handler
        , public AZ::Component
    {
    public:
        AZ_COMPONENT(urdf_exporter_gemEditorSystemComponent, "{096B91AE-ED04-4289-877D-C69F56A33A95}");
        static void Reflect(AZ::ReflectContext* context);

        urdf_exporter_gemEditorSystemComponent();
        ~urdf_exporter_gemEditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate();
        void Deactivate();
    };
} // namespace urdf_exporter_gem
