
#include <UrdfExporterModuleInterface.h>
#include "UrdfExporterEditorSystemComponent.h"
#include <AzToolsFramework/API/PythonLoader.h>

#include <QtGlobal>

void InitUrdfExporterResources()
{
    // We must register our Qt resources (.qrc file) since this is being loaded from a separate module (gem)
    Q_INIT_RESOURCE(UrdfExporter);
}

namespace UrdfExporter
{
    class UrdfExporterEditorModule
        : public UrdfExporterModuleInterface
        , public AzToolsFramework::EmbeddedPython::PythonLoader
    {
    public:
        AZ_RTTI(UrdfExporterEditorModule, "{0C23F845-1369-42FC-95EE-5079CFC1E133}", UrdfExporterModuleInterface);
        AZ_CLASS_ALLOCATOR(UrdfExporterEditorModule, AZ::SystemAllocator, 0);

        UrdfExporterEditorModule()
        {
            InitUrdfExporterResources();

            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                UrdfExporterEditorSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<UrdfExporterEditorSystemComponent>(),
            };
        }
    };
}// namespace UrdfExporter

AZ_DECLARE_MODULE_CLASS(Gem_UrdfExporter, UrdfExporter::UrdfExporterEditorModule)
