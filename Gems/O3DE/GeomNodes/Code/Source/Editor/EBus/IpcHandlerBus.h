#pragma once

#include <AzCore/EBus/EBus.h>

namespace Ipc
{
    class IpcHandlerNotifications : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::EntityId;
        //////////////////////////////////////////////////////////////////////////

        virtual ~IpcHandlerNotifications() {}

        virtual void OnMessageReceived(const AZ::u8* content, const AZ::u64 length) = 0;
    };

    using IpcHandlerNotificationBus = AZ::EBus<IpcHandlerNotifications>;
}
