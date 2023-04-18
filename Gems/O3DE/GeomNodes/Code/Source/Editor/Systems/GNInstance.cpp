/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "GNInstance.h"
#include <Editor/Common/GNAPI.h>
#include <Editor/Systems/GeomNodesSystem.h>
#include <Editor/Common/GNConstants.h>

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

        return RestartProcess();
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
        API::SendMsg(content.c_str(), content.size(), (AZ::u64)m_entityId);
    }

    bool GNInstance::RestartProcess()
    {
        AzFramework::ProcessLauncher::ProcessLaunchInfo processLaunchInfo;
        AZStd::string blenderPath = GetGNSystem()->GetBlenderPath();
        if (blenderPath.empty())
        {
			AZ_Error("GNInstance", false, "Blender instance failed to launch! Blender path is not set or valid.");
			return false;
        }

        processLaunchInfo.m_commandlineParameters = AZStd::string::format(
            R"(%s --factory-startup -b "%s" -P "%s" -- "%s" %llu)",
            blenderPath.c_str(),
            m_path.c_str(),
            m_scriptPath.c_str(),
            m_exePath.c_str(),
            (AZ::u64)m_entityId);
        // NOTE: if you want to debug and see the console print outs set m_showWindow to true
        processLaunchInfo.m_showWindow = false;
        processLaunchInfo.m_processPriority = AzFramework::ProcessPriority::PROCESSPRIORITY_NORMAL;

        AzFramework::ProcessWatcher* outProcess = AzFramework::ProcessWatcher::LaunchProcess(
            processLaunchInfo, AzFramework::ProcessCommunicationType::COMMUNICATOR_TYPE_NONE);

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

    AZStd::string GNInstance::SendParamUpdates(const AZStd::string& params, const AZStd::string& objectName)
    {
		auto msg = AZStd::string::format(
			R"JSON(
                {
                    "%s": [ %s ],
                    "%s": "%s",
                    "ParamUpdate": true
                }
            )JSON"
			, Field::Params
			, params.c_str()
			, Field::Object
			, objectName.c_str()
		);

		SendIPCMsg(msg);

        return msg;
    }

    void GNInstance::SendHeartbeat()
    {
		SendIPCMsg(R"JSON(
                        {
                            "Alive": true 
                        }
                    )JSON");
    }

    void GNInstance::RequestObjectParams()
    {
		SendIPCMsg(R"JSON(
                        {
                            "FetchObjectParams": true 
                        }
                    )JSON");
    }

    void GNInstance::CloseMap(AZ::u64 mapId)
    {
		auto msg = AZStd::string::format(
			R"JSON(
                    {
                        "%s": true,
                        "%s": %llu
                    }
                    )JSON",
			Field::SHMClose,
			Field::MapId,
			mapId);
		SendIPCMsg(msg);
    }

    void GNInstance::RequestExport(const AZStd::string& params, const AZStd::string& objectName, const AZStd::string& fbxPath)
    {
		auto msg = AZStd::string::format(
			R"JSON(
                    {
                        "%s": true,
                        "%s": [ %s ],
                        "%s": "%s",
                        "%s": "%s"
                    }
                    )JSON"
			, Field::Export
			, Field::Params
			, params.c_str()
            , Field::Object
            , objectName.c_str()
            , Field::FBXPath
            , fbxPath.c_str()
			);
		SendIPCMsg(msg);
    }

} // namespace GeomNodes
