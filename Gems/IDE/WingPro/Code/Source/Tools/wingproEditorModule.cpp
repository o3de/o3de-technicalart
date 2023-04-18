
#include <wingproModuleInterface.h>
#include "wingproEditorSystemComponent.h"
#include <AzToolsFramework/API/PythonLoader.h>

#include <QtGlobal>

void InitwingproResources()
{
    // We must register our Qt resources (.qrc file) since this is being loaded from a separate module (gem)
    Q_INIT_RESOURCE(wingpro);
}

namespace wingpro
{
    class wingproEditorModule
        : public wingproModuleInterface
        , public AzToolsFramework::EmbeddedPython::PythonLoader
    {
    public:
        AZ_RTTI(wingproEditorModule, "{1D8ED570-EAC6-4E09-A8F6-6DC3EA9DD334}", wingproModuleInterface);
        AZ_CLASS_ALLOCATOR(wingproEditorModule, AZ::SystemAllocator, 0);

        wingproEditorModule()
        {
            InitwingproResources();

            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                wingproEditorSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<wingproEditorSystemComponent>(),
            };
        }
    };
}// namespace wingpro

AZ_DECLARE_MODULE_CLASS(Gem_wingpro, wingpro::wingproEditorModule)
