/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once
#include <AzCore/std/optional.h>
#include <Editor/Configuration/GNConfiguration.h>

namespace GeomNodes
{
    //! Handles loading ad saving the settings registry
    class GNSettingsRegistryManager
    {
    public:
        enum class Result : AZ::u8
        {
            Success,
            Failed
        };

        using OnGNConfigSaveComplete = AZStd::function<void(const GNConfiguration&, Result)>;

        GNSettingsRegistryManager();
        virtual ~GNSettingsRegistryManager() = default;

        //! Load the GeomNodes Configuration from the Settings Registry
        //! @return Returns true if successful.
        virtual AZStd::optional<GNConfiguration> LoadSystemConfiguration() const;

        //! Save the GeomNodes Configuration from the Settings Registry
        //! @return Returns true if successful. When not in Editor, always returns false.
        virtual void SaveSystemConfiguration(const GNConfiguration& config, const OnGNConfigSaveComplete& saveCallback) const;

    protected:
        AZStd::string m_settingsRegistryPath;
    };
} // namespace GeomNodes