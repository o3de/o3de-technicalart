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