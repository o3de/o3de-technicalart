#pragma once

#include <AzCore/EBus/Event.h>
#include <Editor/Common/GNEvents.h>
#include <Editor/Configuration/GNConfiguration.h>

namespace GeomNodes
{
    class GNSystemInterface
    {
    public:
        AZ_RTTI(GNSystemInterface, "{83173679-DAF6-4496-BEFA-B0D252C40366}");

        GNSystemInterface() = default;
        virtual ~GNSystemInterface() = default;
        AZ_DISABLE_COPY_MOVE(GNSystemInterface);

        static void Reflect(AZ::ReflectContext* context);

        //! Initialize the system with the given configuration.
        //! @param config Contains the configuration options
        virtual void Initialize(const GNConfiguration* config) = 0;
        //! Teardown the whole  system.
        //! system will stop running.
        virtual void Shutdown() = 0;

        //! Retrieve the Blender Editor path
        //! @return the Blender Editor path
        virtual AZStd::string_view GetBlenderPath() = 0;

		//! Get the current GNConfiguration used to initialize the GeomNodes system.
		virtual const GNConfiguration* GetConfiguration() const = 0;

		//! Update the GNConfiguration.
		//! This will apply the new configuration
		//! @param newConfig The new configuration to apply.
		virtual void UpdateConfiguration(const GNConfiguration* newConfig) = 0;

        //! Register to receive notifications when the GeomNodes System is Initialized.
        //! @param handler The handler to receive the event.
        void RegisterSystemInitializedEvent(SystemEvents::OnInitializedEvent::Handler& handler) { handler.Connect(m_initializeEvent); }
        //! Register to receive notifications when the GNConfiguration changes.
        //! @param handler The handler to receive the event.
        void RegisterSystemConfigurationChangedEvent(SystemEvents::OnConfigurationChangedEvent::Handler& handler) { handler.Connect(m_configChangeEvent); }
    
    protected:
        SystemEvents::OnInitializedEvent m_initializeEvent;
        SystemEvents::OnConfigurationChangedEvent m_configChangeEvent;
    };
} // namespace GeomNodes