/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Editor/Configuration/GNConfiguration.h>
#include <Editor/Systems/GeomNodesSystem.h>
#include <Editor/UI/ConfigurationWidget.h>
#include <Editor/UI/EditorWindow.h>
#include <Editor/UI/ui_EditorWindow.h>


#include <AzCore/Interface/Interface.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzToolsFramework/API/ViewPaneOptions.h>


namespace GeomNodes
{
    namespace Editor
    {
        static const char* const CategoryTools = "Tools";
        static const char* const GeomNodesConfigurationEditor = "GeomNodes Configuration (PREVIEW)";

        EditorWindow::EditorWindow(QWidget* parent)
            : QWidget(parent)
            , m_ui(new Ui::EditorWindowClass())
        {
            m_ui->setupUi(this);

            auto* gnSystem = AZ::Interface<GNSystemInterface>::Get();
            const auto* gnSystemConfiguration = azdynamic_cast<const GNConfiguration*>(gnSystem->GetConfiguration());

            m_ui->m_GeomNodesConfigurationWidget->SetConfiguration(*gnSystemConfiguration);
            connect(
                m_ui->m_GeomNodesConfigurationWidget,
                &Editor::ConfigurationWidget::onConfigurationChanged,
                this,
                &EditorWindow::SaveConfiguration);
        }

        void EditorWindow::RegisterViewClass()
        {
            AzToolsFramework::ViewPaneOptions options;
            options.preferedDockingArea = Qt::LeftDockWidgetArea;
            options.saveKeyName = "GeomNodesConfiguration";
            options.isPreview = true;
            AzToolsFramework::RegisterViewPane<EditorWindow>(GeomNodesConfigurationEditor, CategoryTools, options);
        }

        void EditorWindow::SaveConfiguration(const GNConfiguration& gnSystemConfiguration)
        {
            auto* gnSystem = GetGNSystem();
            if (gnSystem == nullptr)
            {
                AZ_Error(
                    "GeomNodes",
                    false,
                    "Unable to save the GeomNodes configuration. The GeomNodesSystem not initialized. Any changes have not been applied.");
                return;
            }

            // update the GeomNodes system config if it has changed
            const GNSettingsRegistryManager& settingsRegManager = gnSystem->GetSettingsRegistryManager();
            if (gnSystem->GetSystemConfiguration() != gnSystemConfiguration)
            {
                auto saveCallback = [](const GNConfiguration& config, GNSettingsRegistryManager::Result result)
                {
                    AZ_Warning(
                        "GeomNodes",
                        result == GNSettingsRegistryManager::Result::Success,
                        "Unable to save the GeomNodes configuration. Any changes have not been applied.");
                    if (result == GNSettingsRegistryManager::Result::Success)
                    {
                        if (auto* gnSystem = GetGNSystem())
                        {
                            gnSystem->UpdateConfiguration(&config);
                        }
                    }
                };
                settingsRegManager.SaveSystemConfiguration(gnSystemConfiguration, saveCallback);
            }
        }
    } // namespace Editor
} // namespace GeomNodes
#include <Editor/UI/moc_EditorWindow.cpp>
