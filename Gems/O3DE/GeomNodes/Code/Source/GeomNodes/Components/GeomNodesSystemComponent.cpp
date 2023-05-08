/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "GeomNodesSystemComponent.h"

#include <GeomNodes/GeomNodesTypeIds.h>

#include <AzCore/Serialization/SerializeContext.h>

namespace GeomNodes
{
    void GeomNodesSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GeomNodesSystemComponent, AZ::Component>()->Version(0);
        }
    }

    void GeomNodesSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("GeomNodesService"));
    }

    void GeomNodesSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("GeomNodesService"));
    }

    void GeomNodesSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void GeomNodesSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    GeomNodesSystemComponent::GeomNodesSystemComponent()
    {
        if (GeomNodesInterface::Get() == nullptr)
        {
            GeomNodesInterface::Register(this);
        }
    }

    GeomNodesSystemComponent::~GeomNodesSystemComponent()
    {
        if (GeomNodesInterface::Get() == this)
        {
            GeomNodesInterface::Unregister(this);
        }
    }

    void GeomNodesSystemComponent::Init()
    {
    }

    void GeomNodesSystemComponent::Activate()
    {
        GeomNodesRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
    }

    void GeomNodesSystemComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        GeomNodesRequestBus::Handler::BusDisconnect();
    }

    void GeomNodesSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
    }

} // namespace GeomNodes
