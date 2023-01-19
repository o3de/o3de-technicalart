
#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace wingpro
{
    class wingproRequests
    {
    public:
        AZ_RTTI(wingproRequests, "{89A445E9-5FBB-4508-8FB2-1062FDF24B22}");
        virtual ~wingproRequests() = default;
        // Put your public methods here
    };
    
    class wingproBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using wingproRequestBus = AZ::EBus<wingproRequests, wingproBusTraits>;
    using wingproInterface = AZ::Interface<wingproRequests>;

} // namespace wingpro
