
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>

namespace urdf_exporter_gem
{
    class urdf_exporter_gemModuleInterface
        : public AZ::Module
    {
    public:
        AZ_RTTI(urdf_exporter_gemModuleInterface, "{68A4F935-0CAD-4A27-A7FD-1674BC8C0DF0}", AZ::Module);
        AZ_CLASS_ALLOCATOR(urdf_exporter_gemModuleInterface, AZ::SystemAllocator, 0);

        urdf_exporter_gemModuleInterface()
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
}// namespace urdf_exporter_gem
