
#include <o3de-dreamstudioModuleInterface.h>
#include "o3de-dreamstudioEditorSystemComponent.h"
#include <AzToolsFramework/API/PythonLoader.h>

#include <QtGlobal>

void Inito3de_dreamstudioResources()
{
    // We must register our Qt resources (.qrc file) since this is being loaded from a separate module (gem)
    Q_INIT_RESOURCE(o3de_dreamstudio);
}

namespace o3de_dreamstudio
{
    class o3de_dreamstudioEditorModule
        : public o3de_dreamstudioModuleInterface
        , public AzToolsFramework::EmbeddedPython::PythonLoader
    {
    public:
        AZ_RTTI(o3de_dreamstudioEditorModule, "{51DDFC63-7B9D-4027-9986-D370A05602E7}", o3de_dreamstudioModuleInterface);
        AZ_CLASS_ALLOCATOR(o3de_dreamstudioEditorModule, AZ::SystemAllocator, 0);

        o3de_dreamstudioEditorModule()
        {
            Inito3de_dreamstudioResources();

            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                o3de_dreamstudioEditorSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<o3de_dreamstudioEditorSystemComponent>(),
            };
        }
    };
}// namespace o3de_dreamstudio

AZ_DECLARE_MODULE_CLASS(Gem_o3de_dreamstudio, o3de_dreamstudio::o3de_dreamstudioEditorModule)
