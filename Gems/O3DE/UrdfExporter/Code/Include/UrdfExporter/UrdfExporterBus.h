
#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace UrdfExporter
{
    class UrdfExporterRequests
    {
    public:
        AZ_RTTI(UrdfExporterRequests, "{6268B4A9-6A4E-49AE-9AA7-69DF622CE4E9}");
        virtual ~UrdfExporterRequests() = default;
        // Put your public methods here
    };
    
    class UrdfExporterBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using UrdfExporterRequestBus = AZ::EBus<UrdfExporterRequests, UrdfExporterBusTraits>;
    using UrdfExporterInterface = AZ::Interface<UrdfExporterRequests>;

} // namespace UrdfExporter
