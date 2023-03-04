
#include <GeomNodesModuleInterface.h>
#include "Editor/Components/EditorGeomNodesSystemComponent.h"
#include "Editor/Components/EditorGeomNodesComponent.h"
#include "Editor/Components/EditorGeomNodesMeshComponent.h"

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
                    EditorGeomNodesSystemComponent::CreateDescriptor(),
                    EditorGeomNodesComponent::CreateDescriptor(),
                    EditorGeomNodesMeshComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<EditorGeomNodesSystemComponent>(),
                azrtti_typeid<EditorGeomNodesComponent>(),
                azrtti_typeid<EditorGeomNodesMeshComponent>(),
            };
        }
    };
}// namespace GeomNodes

AZ_DECLARE_MODULE_CLASS(Gem_GeomNodes, GeomNodes::GeomNodesEditorModule)
