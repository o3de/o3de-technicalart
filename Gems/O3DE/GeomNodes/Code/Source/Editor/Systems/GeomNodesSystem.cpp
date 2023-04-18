/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "GeomNodesSystem.h"
#include "Editor/UI/PropertyFileSelect.h"
#include "Editor/UI/ValidationHandler.h"
#include "Editor/UI/GeomNodesValidator.h"
#include <Editor/Common/GNAPI.h>
#include <Editor/EBus/IpcHandlerBus.h>

namespace GeomNodes
{
    long IpcHandlerCB(AZ::u64 id, const char* data, AZ::u64 length)
    {
        /*AZStd::string msg;
        msg.assign(data, length);
        AZ_Printf("GeomNodesSystem", "id: %llu data: %s length: %llu", id, msg.c_str(), length);*/
        Ipc::IpcHandlerNotificationBus::Event(
            AZ::EntityId(id), &Ipc::IpcHandlerNotificationBus::Events::OnMessageReceived, reinterpret_cast<const AZ::u8*>(data), length);

        return 0;
    }

    GeomNodesSystem::GeomNodesSystem(AZStd::unique_ptr<GNSettingsRegistryManager> registryManager)
        : m_registryManager(AZStd::move(registryManager))
        , m_propertyHandlers()
        , m_validator(AZStd::make_unique<Validator>())
        , m_validationHandler(AZStd::make_unique<ValidationHandler>())
        {
        }

    GeomNodesSystem::~GeomNodesSystem()
    {
        Shutdown();
    }

    void GeomNodesSystem::Initialize(const GNConfiguration* config)
    {
		if (m_state == State::Initialized)
		{
			AZ_Warning("GeomNodesSystem", false, "GeomNodes system already initialized, Shutdown must be called first");
			return;
		}

		m_systemConfig = *config;
		
		API::Init(SERVER_ID, IpcHandlerCB);

		RegisterHandlersAndBuses();

		m_state = State::Initialized;
		m_initializeEvent.Signal(&m_systemConfig);
    }

    void GeomNodesSystem::Shutdown()
    {
		if (m_state != State::Initialized)
		{
			return;
		}

		UnregisterHandlersAndBuses();
		API::Uninitialize();

		m_state = State::Shutdown;
    }

    AZStd::string_view GeomNodesSystem::GetBlenderPath()
    {
        return m_systemConfig.m_blenderPath;
    }

    const GNConfiguration* GeomNodesSystem::GetConfiguration() const
    {
        return &m_systemConfig;
    }

	const GNConfiguration& GeomNodesSystem::GetSystemConfiguration() const
	{
		return m_systemConfig;
	}

    void GeomNodesSystem::SetLastPath(const AZStd::string& lastPath)
    {
        m_systemConfig.m_lastFilePath = lastPath;
        const GNSettingsRegistryManager& settingsRegManager = GetSettingsRegistryManager();
		auto saveCallback = [](const GNConfiguration& config, GNSettingsRegistryManager::Result result)
		{
			AZ_Warning("GeomNodes", result == GNSettingsRegistryManager::Result::Success, "Unable to save the GeomNodes configuration. Any changes have not been applied.");
			if (result == GNSettingsRegistryManager::Result::Success)
			{
				if (auto* gnSystem = GetGNSystem())
				{
					gnSystem->UpdateConfiguration(&config);
				}
			}
		};
		settingsRegManager.SaveSystemConfiguration(m_systemConfig, saveCallback);
    }

    AZStd::string GeomNodesSystem::GetLastPath()
    {
        return m_systemConfig.m_lastFilePath;
    }

    void GeomNodesSystem::UpdateConfiguration(const GNConfiguration* newConfig)
    {
		if(m_systemConfig != *newConfig)
		{
			m_systemConfig = (*newConfig);
			m_configChangeEvent.Signal(newConfig);
		}
    }

    const GNSettingsRegistryManager& GeomNodesSystem::GetSettingsRegistryManager() const
    {
        return *m_registryManager;
    }

    FunctorValidator* GeomNodesSystem::GetValidator(FunctorValidator::FunctorType functor)
    {
        return m_validator->GetQValidator(functor);
    }

    void GeomNodesSystem::TrackValidator(FunctorValidator* validator)
    {
        m_validator->TrackThisValidator(validator);
    }

    void GeomNodesSystem::RegisterHandlersAndBuses()
    {
        m_propertyHandlers.push_back(PropertyFuncValBrowseEditHandler::Register(m_validationHandler.get()));
        m_propertyHandlers.push_back(PropertyFileSelectHandler::Register(m_validationHandler.get()));
        ValidatorBus::Handler::BusConnect();

        //TODO: setup our IPC system. Since this is the gem it will be the server.
        // only one handler. Then messages will have the sender id(EntityID) which will be used when sending
        // the messages via the EBUS. when a component is registered as an EBUS handler they will receive
        // the correct messages.
    }

    void GeomNodesSystem::UnregisterHandlersAndBuses()
    {
        ValidatorBus::Handler::BusDisconnect();

        for (AzToolsFramework::PropertyHandlerBase* handler : m_propertyHandlers)
        {
            AzToolsFramework::PropertyTypeRegistrationMessages::Bus::Broadcast(
                &AzToolsFramework::PropertyTypeRegistrationMessages::Bus::Handler::UnregisterPropertyType,
                handler);
            delete handler;
        }
    }

    GeomNodesSystem* GetGNSystem()
    {
        return azdynamic_cast<GeomNodesSystem*>(AZ::Interface<GeomNodes::GNSystemInterface>::Get());
    }
}
