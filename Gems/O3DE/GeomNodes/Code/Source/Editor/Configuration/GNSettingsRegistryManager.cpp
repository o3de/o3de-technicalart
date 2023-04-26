/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Editor/Configuration/GNSettingsRegistryManager.h>

#include <AzCore/Settings/SettingsRegistry.h>
#include <AzCore/Settings/SettingsRegistryMergeUtils.h>

namespace GeomNodes
{
    GNSettingsRegistryManager::GNSettingsRegistryManager()
    {
        m_settingsRegistryPath =
            AZStd::string::format("%s/Gems/GeomNodes/GNConfiguration", AZ::SettingsRegistryMergeUtils::OrganizationRootKey);
    }

    AZStd::optional<GNConfiguration> GNSettingsRegistryManager::LoadSystemConfiguration() const
    {
        GNConfiguration systemConfig;

        bool configurationRead = false;

        AZ::SettingsRegistryInterface* settingsRegistry = AZ::SettingsRegistry::Get();
        if (settingsRegistry)
        {
            configurationRead = settingsRegistry->GetObject(systemConfig, m_settingsRegistryPath);
        }

        if (configurationRead)
        {
            AZ_TracePrintf(
                "GeomNodesSystem",
                R"(GNConfiguration was read from settings registry at pointer path)"
                R"( "%s)"
                "\n",
                m_settingsRegistryPath.c_str());
            return systemConfig;
        }
        return AZStd::nullopt;
    }

    void GNSettingsRegistryManager::SaveSystemConfiguration(
        [[maybe_unused]] const GNConfiguration& config, const OnGNConfigSaveComplete& saveCallback) const
    {
        if (saveCallback)
        {
            saveCallback(config, Result::Failed);
        }
    }
} // namespace GeomNodes