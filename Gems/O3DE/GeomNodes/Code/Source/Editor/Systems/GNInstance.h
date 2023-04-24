/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include "Editor/Commons.h"
#include <AzFramework/Process/ProcessWatcher.h>
#include <AzCore/Component/EntityId.h>
#include <AzCore/Component/TickBus.h>

namespace GeomNodes
{
    //! Handles the Blender instance and communication between Gem and Client Script
    class GNInstance
    {
    public:
        GNInstance() = default;
        virtual ~GNInstance();

        //! Standard initialization function.
        bool Init(const AZStd::string& filePath, const AZStd::string& scriptPath, const AZStd::string& exePath, AZ::EntityId entityId);
        //! Do some cleanup. Like terminating the running process.
        void Cleanup();
        //! Check if the Blender instance is still valid or running.
        bool IsValid();
        //! Checks if the path provided is the same one as the Blender instance is currently using.
        bool IsSamePath(const AZStd::string& path);
        //! Send a generic IPC message.
        void SendIPCMsg(const AZStd::string& content);
        //! Start or Restart the Blender instance.
        bool RestartProcess();

        //! IPC messaging
        //! Send Parameter updates so the client can update the Geometry Node in the Blender side.
        AZStd::string SendParamUpdates(const AZStd::string& params, const AZStd::string& objectName);
        //! Send a Heartbeat message to the client so it knows the Gem is still running.
        void SendHeartbeat();
        //! Request for Object Parameter info from the client script.
        void RequestObjectParams();
        //! Tells the client script to close the SHM map on the client side as the gem will manage it.
        void CloseMap(AZ::u64 mapId);
        //! Tells the client script that we need to export and write it to an FBX file.
        void RequestExport(const AZStd::string& params, const AZStd::string& objectName, const AZStd::string& fbxPath);

    private:
        //! Blender instance's Process Watcher
        AZStd::unique_ptr<AzFramework::ProcessWatcher> m_blenderProcessWatcher = nullptr;

        //! Stores assigned EntityId
        AZ::EntityId m_entityId;
        //! Stores the blender file path that has a Geometry Nodes Modifier
        AZStd::string m_path = "";
        //! Stores the script path. i.e. <GeomNodes Gem Path>/External/Scripts/__init__.py
        AZStd::string m_scriptPath = "";
        //! Stores the path where Bridge.dll is located
        AZStd::string m_exePath = "";
    };
} // namespace GeomNodes
