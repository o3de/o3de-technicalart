/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <GeomNodes/GeomNodesTypeIds.h>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace GeomNodes
{
    class GeomNodesRequests
    {
    public:
        AZ_RTTI(GeomNodesRequests, GeomNodesRequestsTypeId);
        virtual ~GeomNodesRequests() = default;
        // Put your public methods here
    };

    class GeomNodesBusTraits : public AZ::EBusTraits
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