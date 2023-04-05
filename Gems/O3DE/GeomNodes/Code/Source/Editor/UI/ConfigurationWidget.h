#pragma once

#if !defined(Q_MOC_RUN)
#include <Editor/Configuration/GNConfiguration.h>
#include <AzCore/Memory/SystemAllocator.h>
#endif

namespace GeomNodes
{
    namespace Editor
    {
        class SettingsWidget;
        
        /// Widget for editing GeomNodes configuration and settings.
        ///
        class ConfigurationWidget
            : public QWidget
        {
            Q_OBJECT

        public:
            AZ_CLASS_ALLOCATOR(ConfigurationWidget, AZ::SystemAllocator);

            explicit ConfigurationWidget(QWidget* parent = nullptr);
            ~ConfigurationWidget() override;

            void SetConfiguration(const GNConfiguration& gnSystemConfiguration);

        signals:
            void onConfigurationChanged(const GNConfiguration& gnSystemConfiguration);

        private:
            GNConfiguration m_gnSystemConfiguration;
            
            SettingsWidget* m_settings;
        };
    } // namespace Editor
} // namespace GeomNodes
