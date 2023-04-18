
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>

namespace UrdfExporter
{
    class UrdfExporterModuleInterface
        : public AZ::Module
    {
    public:
        AZ_RTTI(UrdfExporterModuleInterface, "{7E550FCD-AB54-4BB0-9B8F-77E1FF07D387}", AZ::Module);
        AZ_CLASS_ALLOCATOR(UrdfExporterModuleInterface, AZ::SystemAllocator, 0);

        UrdfExporterModuleInterface()
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
}// namespace UrdfExporter
