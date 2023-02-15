#include "GNInstance.h"
#include "Bridge.h"

namespace GeomNodes
{
    GNInstance::~GNInstance()
    {
        Cleanup();
    }

    void GNInstance::Cleanup()
    {
        if (m_blenderProcessWatcher)
        {
            m_blenderProcessWatcher->TerminateProcess(0);

            m_blenderProcessWatcher = nullptr;
        }

        AZ::TickBus::Handler::BusDisconnect();
    }

    bool GNInstance::IsValid()
    {
        return m_blenderProcessWatcher && m_blenderProcessWatcher->IsProcessRunning();
    }

    bool GNInstance::Init(const AZStd::string& filePath, const AZStd::string& scriptPath, const AZStd::string& exePath, AZ::EntityId entityId)
    {
        m_entityId = entityId;
        m_path = filePath;
        m_scriptPath = scriptPath;
        m_exePath = exePath;

        AZ::TickBus::Handler::BusConnect();
        return RestartProcess();
    }

    void GNInstance::OnTick(float /*deltaTime*/, AZ::ScriptTimePoint /*time*/)
    {
        
    }

    bool GNInstance::IsSamePath(const AZStd::string& path)
    {
        if (m_path.empty())
            return false;

        if (path == m_path)
            return true;

        return false;
    }

    void GNInstance::SendIPCMsg(const AZStd::string& content)
    {
        //AZ::u64 entityId = 123456;
        AZ::u64 entityId = (AZ::u64)m_entityId;
        SendMsg(content.c_str(), content.size(), entityId);
    }

    bool GNInstance::RestartProcess()
    {
        AzFramework::ProcessLauncher::ProcessLaunchInfo processLaunchInfo;
        //TODO: don't hard code this. Add a way for the user to set this up or the gem detects it automatically.
        AZStd::string blenderPath = "C:\\Program Files\\Blender Foundation\\Blender 3.3\\blender.exe";
        processLaunchInfo.m_commandlineParameters = AZStd::string::format(
            R"(%s --factory-startup -b "%s" -P "%s" -- "%s" %llu)",
            blenderPath.c_str(),
            m_path.c_str(),
            m_scriptPath.c_str(),
            m_exePath.c_str(),
            (AZ::u64)m_entityId);
        processLaunchInfo.m_showWindow = false;
        processLaunchInfo.m_processPriority = AzFramework::ProcessPriority::PROCESSPRIORITY_NORMAL;

        AzFramework::ProcessWatcher* outProcess = AzFramework::ProcessWatcher::LaunchProcess(
            processLaunchInfo, AzFramework::ProcessCommunicationType::COMMUNICATOR_TYPE_STDINOUT);

        if (outProcess)
        {
            // Stop the previous server if one exists
            if (m_blenderProcessWatcher)
            {
                m_blenderProcessWatcher->TerminateProcess(0);
            }

            m_blenderProcessWatcher.reset(outProcess);
        }
        else
        {
            AZ_Error("GNInstance", outProcess, "Blender instance failed to launch! Unable to create AzFramework::ProcessWatcher.");
            return false;
        }

        return true;
    }
    
} // namespace GeomNodes
