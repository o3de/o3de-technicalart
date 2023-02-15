
#include <GeomNodesModuleInterface.h>
#include "Editor/Components/GeomNodesEditorSystemComponent.h"
#include "Editor/Components/GeomNodesEditorComponent.h"

namespace GeomNodes
{
    class GeomNodesEditorModule
        : public GeomNodesModuleInterface
    {
    public:
        AZ_RTTI(GeomNodesEditorModule, "{49C42A73-EF4E-4D42-8ECF-0ADE7F942CCD}", GeomNodesModuleInterface);
        AZ_CLASS_ALLOCATOR(GeomNodesEditorModule, AZ::SystemAllocator, 0);

        GeomNodesEditorModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                    GeomNodesEditorSystemComponent::CreateDescriptor(),
                    GeomNodesEditorComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<GeomNodesEditorSystemComponent>(),
                azrtti_typeid<GeomNodesEditorComponent>(),
            };
        }
    };
}// namespace GeomNodes

AZ_DECLARE_MODULE_CLASS(Gem_GeomNodes, GeomNodes::GeomNodesEditorModule)
