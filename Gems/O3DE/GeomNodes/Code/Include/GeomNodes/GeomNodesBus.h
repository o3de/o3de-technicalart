
#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace GeomNodes
{
    class GeomNodesRequests
    {
    public:
        AZ_RTTI(GeomNodesRequests, "{B09157F9-452C-41AE-91F5-578D6EB2C425}");
        virtual ~GeomNodesRequests() = default;
        // Put your public methods here
    };
    
    class GeomNodesBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using GeomNodesRequestBus = AZ::EBus<GeomNodesRequests, GeomNodesBusTraits>;
    using GeomNodesInterface = AZ::Interface<GeomNodesRequests>;

} // namespace GeomNodes
