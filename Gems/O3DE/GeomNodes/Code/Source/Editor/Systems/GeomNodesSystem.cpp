#include "GeomNodesSystem.h"
#include "Editor/UI/PropertyFileSelect.h"
#include "Editor/UI/ValidationHandler.h"
#include "Editor/UI/GeomNodesValidator.h"
#include "Bridge.h"
#include <Editor/EBus/IpcHandlerBus.h>

namespace GeomNodes
{
    long IpcHandlerCB(AZ::u64 id, const char* data, AZ::u64 length)
    {
        AZ_Printf("GeomNodesSystem", "id: %llu data: %s length: %llu", id, data, length);
        Ipc::IpcHandlerNotificationBus::Event(
            AZ::EntityId(id), &Ipc::IpcHandlerNotificationBus::Events::OnMessageReceived, reinterpret_cast<const AZ::u8*>(data), length);

        return 0;
    }

    GeomNodesSystem::GeomNodesSystem()
        : m_propertyHandlers()
        , m_validator(AZStd::make_unique<Validator>())
        , m_validationHandler(AZStd::make_unique<ValidationHandler>())
        {
        }

    GeomNodesSystem::~GeomNodesSystem()
    {
    }

    void GeomNodesSystem::OnActivate()
    {
        Init(SERVER_ID, IpcHandlerCB); 

        RegisterHandlersAndBuses();
    }

    void GeomNodesSystem::OnDeactivate()
    {
        UnregisterHandlersAndBuses();
        Uninitialize();
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
}
