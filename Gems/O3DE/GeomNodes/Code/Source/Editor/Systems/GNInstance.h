#pragma once

#include "Editor/Commons.h"
#include <AzFramework/Process/ProcessWatcher.h>
#include <AzCore/Component/EntityId.h>
#include <AzCore/Component/TickBus.h>

namespace GeomNodes
{
    class GNInstance
        : public AZ::TickBus::Handler
    {
    public:
        GNInstance() = default;
        virtual ~GNInstance();

        bool Init(const AZStd::string& filePath, const AZStd::string& scriptPath, const AZStd::string& exePath, AZ::EntityId entityId);
        void Cleanup();
        bool IsValid();
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        bool IsSamePath(const AZStd::string& path);
        void SendIPCMsg(const AZStd::string& content);
        bool RestartProcess();

        // Ipc messaging
        void SendParamUpdates(const AZStd::string& params, const AZStd::string& objectName);
        void SendHeartbeat();
        void RequestObjectParams();
        void CloseMap(AZ::u64 mapId);
        void RequestExport(const AZStd::string& fbxPath, const AZStd::string& objectName);

    private:
        AZStd::unique_ptr<AzFramework::ProcessWatcher> m_blenderProcessWatcher = nullptr;

        AZ::EntityId m_entityId;
        AZStd::string m_path = "";
        AZStd::string m_scriptPath = "";
        AZStd::string m_exePath = "";
    };
} // namespace GeomNodes
