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
    class GNInstance
    {
    public:
        GNInstance() = default;
        virtual ~GNInstance();

        bool Init(const AZStd::string& filePath, const AZStd::string& scriptPath, const AZStd::string& exePath, AZ::EntityId entityId);
        void Cleanup();
        bool IsValid();
        bool IsSamePath(const AZStd::string& path);
        void SendIPCMsg(const AZStd::string& content);
        bool RestartProcess();

        // Ipc messaging
        AZStd::string SendParamUpdates(const AZStd::string& params, const AZStd::string& objectName);
        void SendHeartbeat();
        void RequestObjectParams();
        void CloseMap(AZ::u64 mapId);
        void RequestExport(const AZStd::string& params, const AZStd::string& objectName, const AZStd::string& fbxPath);

    private:
        AZStd::unique_ptr<AzFramework::ProcessWatcher> m_blenderProcessWatcher = nullptr;

        AZ::EntityId m_entityId;
        AZStd::string m_path = "";
        AZStd::string m_scriptPath = "";
        AZStd::string m_exePath = "";
    };
} // namespace GeomNodes
