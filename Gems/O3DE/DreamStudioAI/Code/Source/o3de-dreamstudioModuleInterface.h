
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>

namespace o3de_dreamstudio
{
    class o3de_dreamstudioModuleInterface
        : public AZ::Module
    {
    public:
        AZ_RTTI(o3de_dreamstudioModuleInterface, "{D7FF9F0A-6ADC-4023-89F4-735C493B0E24}", AZ::Module);
        AZ_CLASS_ALLOCATOR(o3de_dreamstudioModuleInterface, AZ::SystemAllocator, 0);

        o3de_dreamstudioModuleInterface()
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
}// namespace o3de_dreamstudio
