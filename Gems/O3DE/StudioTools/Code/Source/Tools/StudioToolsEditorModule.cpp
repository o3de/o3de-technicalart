
#include <StudioToolsModuleInterface.h>
#include "StudioToolsEditorSystemComponent.h"
#include <AzToolsFramework/API/PythonLoader.h>

#include <QtGlobal>

void InitStudioToolsResources()
{
    // We must register our Qt resources (.qrc file) since this is being loaded from a separate module (gem)
    Q_INIT_RESOURCE(StudioTools);
}

namespace StudioTools
{
    class StudioToolsEditorModule
        : public StudioToolsModuleInterface
        , public AzToolsFramework::EmbeddedPython::PythonLoader
    {
    public:
        AZ_RTTI(StudioToolsEditorModule, "{223E8CD4-FE90-4D25-B928-401255AC3B32}", StudioToolsModuleInterface);
        AZ_CLASS_ALLOCATOR(StudioToolsEditorModule, AZ::SystemAllocator, 0);

        StudioToolsEditorModule()
        {
            InitStudioToolsResources();

            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                StudioToolsEditorSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<StudioToolsEditorSystemComponent>(),
            };
        }
    };
}// namespace StudioTools

AZ_DECLARE_MODULE_CLASS(Gem_StudioTools, StudioTools::StudioToolsEditorModule)
