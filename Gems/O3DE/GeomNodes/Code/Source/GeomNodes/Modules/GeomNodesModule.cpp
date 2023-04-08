
#include <GeomNodes/GeomNodesTypeIds.h>
#include <GeomNodesModuleInterface.h>

namespace GeomNodes
{
    class GeomNodesModule
        : public GeomNodesModuleInterface
    {
    public:
        AZ_RTTI(GeomNodesModule, GeomNodesModuleTypeId, GeomNodesModuleInterface);
        AZ_CLASS_ALLOCATOR(GeomNodesModule, AZ::SystemAllocator);
    };
}// namespace GeomNodes

AZ_DECLARE_MODULE_CLASS(Gem_GeomNodes, GeomNodes::GeomNodesModule)
