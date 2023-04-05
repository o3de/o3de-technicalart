
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>
#include <GeomNodes/Components/GeomNodesSystemComponent.h>

namespace GeomNodes
{
    class GeomNodesModuleInterface
        : public AZ::Module
    {
    public:
        AZ_RTTI(GeomNodesModuleInterface, "{2D38307D-709D-4B17-9878-AF59F988FC54}", AZ::Module);
        AZ_CLASS_ALLOCATOR(GeomNodesModuleInterface, AZ::SystemAllocator);

        GeomNodesModuleInterface()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                GeomNodesSystemComponent::CreateDescriptor(),
                });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<GeomNodesSystemComponent>(),
            };
        }
    };
}// namespace GeomNodes
