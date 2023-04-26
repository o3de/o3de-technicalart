/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Editor/Configuration/GNSettingsRegistryManager.h>

namespace GeomNodes
{
    class GNEditorSettingsRegistryManager : public GNSettingsRegistryManager
    {
    public:
        GNEditorSettingsRegistryManager();

        // GNSystemSettingsRegistry ...
        void SaveSystemConfiguration(const GNConfiguration& config, const OnGNConfigSaveComplete& saveCallback) const override;

    private:
        AZ::IO::FixedMaxPath m_gnConfigurationFilePath = "Registry/geomnodesconfiguration.setreg";

        bool m_initialized = false;
    };
} // namespace GeomNodes