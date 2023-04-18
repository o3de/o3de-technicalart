
#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace o3de_dreamstudio
{
    class o3de_dreamstudioRequests
    {
    public:
        AZ_RTTI(o3de_dreamstudioRequests, "{B6AED969-730D-430C-8E38-39A882415372}");
        virtual ~o3de_dreamstudioRequests() = default;
        // Put your public methods here
    };
    
    class o3de_dreamstudioBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using o3de_dreamstudioRequestBus = AZ::EBus<o3de_dreamstudioRequests, o3de_dreamstudioBusTraits>;
    using o3de_dreamstudioInterface = AZ::Interface<o3de_dreamstudioRequests>;

} // namespace o3de_dreamstudio
