
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>

namespace wingpro
{
    class wingproModuleInterface
        : public AZ::Module
    {
    public:
        AZ_RTTI(wingproModuleInterface, "{57BC8700-4497-4053-8496-BD8B563BA1F7}", AZ::Module);
        AZ_CLASS_ALLOCATOR(wingproModuleInterface, AZ::SystemAllocator, 0);

        wingproModuleInterface()
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
}// namespace wingpro
