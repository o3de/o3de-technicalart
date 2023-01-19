
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>

namespace StudioTools
{
    class StudioToolsModuleInterface
        : public AZ::Module
    {
    public:
        AZ_RTTI(StudioToolsModuleInterface, "{F0F2B835-3C8A-4471-AF58-9EDF1CC33CA8}", AZ::Module);
        AZ_CLASS_ALLOCATOR(StudioToolsModuleInterface, AZ::SystemAllocator, 0);

        StudioToolsModuleInterface()
        {
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
            };
        }
    };
}// namespace StudioTools
