/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

// This is similar to the WhiteBox implementation
namespace GeomNodes
{
    class GNRenderMeshInterface;

    //! Function object alias for creating a GNRenderMeshInterface.
    //! @note Used by SetGNRenderMeshInterfaceBuilder in GeomNodesRequests.
    using GNRenderMeshInterfaceBuilderFn = AZStd::function<AZStd::unique_ptr<GNRenderMeshInterface>(AZ::EntityId)>;

    //! system level requests.
    class GeomNodesRequests : public AZ::EBusTraits
    {
    public:
        // EBusTraits overrides ...
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

        //! Create a render mesh for use with geometry nodes data.
        virtual AZStd::unique_ptr<GNRenderMeshInterface> CreateGNRenderMeshInterface(AZ::EntityId) = 0;
        //! Control what concrete implementation of GNRenderMeshInterface CreateGNRenderMeshInterface returns.
        virtual void SetGNRenderMeshInterfaceBuilder(GNRenderMeshInterfaceBuilderFn builder) = 0;

    protected:
        ~GeomNodesRequests() = default;
    };

    using WhiteBoxRequestBus = AZ::EBus<GeomNodesRequests>;
} // namespace GeomNodes
