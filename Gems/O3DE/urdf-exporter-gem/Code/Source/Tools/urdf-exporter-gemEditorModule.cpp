
#include <urdf-exporter-gemModuleInterface.h>
#include "urdf-exporter-gemEditorSystemComponent.h"
#include <AzToolsFramework/API/PythonLoader.h>

#include <QtGlobal>

void Initurdf_exporter_gemResources()
{
    // We must register our Qt resources (.qrc file) since this is being loaded from a separate module (gem)
    Q_INIT_RESOURCE(urdf_exporter_gem);
}

namespace urdf_exporter_gem
{
    class urdf_exporter_gemEditorModule
        : public urdf_exporter_gemModuleInterface
        , public AzToolsFramework::EmbeddedPython::PythonLoader
    {
    public:
        AZ_RTTI(urdf_exporter_gemEditorModule, "{24A2AE05-E717-4786-9857-4B54972CE012}", urdf_exporter_gemModuleInterface);
        AZ_CLASS_ALLOCATOR(urdf_exporter_gemEditorModule, AZ::SystemAllocator, 0);

        urdf_exporter_gemEditorModule()
        {
            Initurdf_exporter_gemResources();

            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                urdf_exporter_gemEditorSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<urdf_exporter_gemEditorSystemComponent>(),
            };
        }
    };
}// namespace urdf_exporter_gem

AZ_DECLARE_MODULE_CLASS(Gem_urdf_exporter_gem, urdf_exporter_gem::urdf_exporter_gemEditorModule)
