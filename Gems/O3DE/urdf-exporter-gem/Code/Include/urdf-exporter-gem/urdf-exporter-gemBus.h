
#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace urdf_exporter_gem
{
    class urdf_exporter_gemRequests
    {
    public:
        AZ_RTTI(urdf_exporter_gemRequests, "{A222C953-EA93-4377-95C3-1CEE04C1BA13}");
        virtual ~urdf_exporter_gemRequests() = default;
        // Put your public methods here
    };
    
    class urdf_exporter_gemBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using urdf_exporter_gemRequestBus = AZ::EBus<urdf_exporter_gemRequests, urdf_exporter_gemBusTraits>;
    using urdf_exporter_gemInterface = AZ::Interface<urdf_exporter_gemRequests>;

} // namespace urdf_exporter_gem
