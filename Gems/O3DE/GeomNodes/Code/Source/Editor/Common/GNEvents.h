#pragma once
#include <AzCore/EBus/Event.h>

namespace GeomNodes
{
    struct GNConfiguration;

    namespace SystemEvents
    {
        //! Event that triggers when the GeomNodes system configuration has been changed.
        //! When triggered the event will send the newly applied GNConfiguration object.
        using OnConfigurationChangedEvent = AZ::Event<const GNConfiguration*>;

        //! Event triggers when the GeomNodes system has completed initialization.
        //! When triggered the event will send the GNConfiguration used to initialize the system.
        using OnInitializedEvent = AZ::Event<const GNConfiguration*>;
    }
} // namespace GeomNodes