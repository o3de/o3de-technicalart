/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/EBus/EBus.h>

namespace Ipc
{
    //! IPC related notifications
    class IpcHandlerNotifications : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::EntityId;
        //////////////////////////////////////////////////////////////////////////

        virtual ~IpcHandlerNotifications()
        {
        }

        //! Triggered when an IPC message is sent from the client(s) to the gem.
        virtual void OnMessageReceived(const AZ::u8* content, const AZ::u64 length) = 0;
    };

    using IpcHandlerNotificationBus = AZ::EBus<IpcHandlerNotifications>;
} // namespace Ipc
