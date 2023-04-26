/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzFramework/Physics/Collision/CollisionGroups.h>
#include <AzFramework/Physics/Collision/CollisionLayers.h>
#include <AzFramework/Physics/SystemBus.h>
#include <AzToolsFramework/UI/PropertyEditor/InstanceDataHierarchy.h>
#include <AzToolsFramework/UI/PropertyEditor/ReflectedPropertyEditor.hxx>
#include <Editor/UI/ConfigurationWidget.h>
#include <Editor/UI/SettingsWidget.h>
#include <QBoxLayout>


namespace GeomNodes
{
    namespace Editor
    {
        ConfigurationWidget::ConfigurationWidget(QWidget* parent)
            : QWidget(parent)
        {
            QVBoxLayout* verticalLayout = new QVBoxLayout(this);
            verticalLayout->setContentsMargins(0, 5, 0, 0);
            verticalLayout->setSpacing(0);

            m_settings = new SettingsWidget();

            verticalLayout->addWidget(m_settings);

            connect(
                m_settings,
                &SettingsWidget::onValueChanged,
                this,
                [this](const GNConfiguration& gnSystemConfiguration)
                {
                    m_gnSystemConfiguration = gnSystemConfiguration;
                    emit onConfigurationChanged(m_gnSystemConfiguration);
                });
        }

        ConfigurationWidget::~ConfigurationWidget()
        {
        }

        void ConfigurationWidget::SetConfiguration(const GNConfiguration& gnSystemConfiguration)
        {
            m_gnSystemConfiguration = gnSystemConfiguration;
            m_settings->SetValue(m_gnSystemConfiguration);
        }
    } // namespace Editor
} // namespace GeomNodes

#include <Editor/UI/moc_ConfigurationWidget.cpp>