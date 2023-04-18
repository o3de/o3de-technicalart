
#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace DreamStudioAI
{
    class DreamStudioAIRequests
    {
    public:
        AZ_RTTI(DreamStudioAIRequests, "{FE40DD24-2C5E-4A40-9998-F5AF74FD9E82}");
        virtual ~DreamStudioAIRequests() = default;
        // Put your public methods here
    };
    
    class DreamStudioAIBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using DreamStudioAIRequestBus = AZ::EBus<DreamStudioAIRequests, DreamStudioAIBusTraits>;
    using DreamStudioAIInterface = AZ::Interface<DreamStudioAIRequests>;

} // namespace DreamStudioAI
