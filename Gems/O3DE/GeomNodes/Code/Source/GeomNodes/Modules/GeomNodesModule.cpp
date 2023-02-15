

#include <GeomNodesModuleInterface.h>
#include "GeomNodes/Components/GeomNodesSystemComponent.h"

namespace GeomNodes
{
    class GeomNodesModule
        : public GeomNodesModuleInterface
    {
    public:
        AZ_RTTI(GeomNodesModule, "{49C42A73-EF4E-4D42-8ECF-0ADE7F942CCD}", GeomNodesModuleInterface);
        AZ_CLASS_ALLOCATOR(GeomNodesModule, AZ::SystemAllocator, 0);
    };
}// namespace GeomNodes

AZ_DECLARE_MODULE_CLASS(Gem_GeomNodes, GeomNodes::GeomNodesModule)
