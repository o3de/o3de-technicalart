
#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace Journeys_in_Generative_AI
{
    class Journeys_in_Generative_AIRequests
    {
    public:
        AZ_RTTI(Journeys_in_Generative_AIRequests, "{22D353B4-AD8C-4E3C-849F-F2525EA8B654}");
        virtual ~Journeys_in_Generative_AIRequests() = default;
        // Put your public methods here
    };

    class Journeys_in_Generative_AIBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using Journeys_in_Generative_AIRequestBus = AZ::EBus<Journeys_in_Generative_AIRequests, Journeys_in_Generative_AIBusTraits>;
    using Journeys_in_Generative_AIInterface = AZ::Interface<Journeys_in_Generative_AIRequests>;

} // namespace Journeys_in_Generative_AI
