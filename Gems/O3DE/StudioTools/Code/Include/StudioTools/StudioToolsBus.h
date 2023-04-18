
#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace StudioTools
{
    class StudioToolsRequests
    {
    public:
        AZ_RTTI(StudioToolsRequests, "{189B0D29-45C3-4560-8136-D1560040C471}");
        virtual ~StudioToolsRequests() = default;
        // Put your public methods here
    };
    
    class StudioToolsBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using StudioToolsRequestBus = AZ::EBus<StudioToolsRequests, StudioToolsBusTraits>;
    using StudioToolsInterface = AZ::Interface<StudioToolsRequests>;

} // namespace StudioTools
