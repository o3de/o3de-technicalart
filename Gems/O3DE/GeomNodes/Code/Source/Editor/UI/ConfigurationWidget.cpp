#include <AzFramework/Physics/SystemBus.h>
#include <AzFramework/Physics/Collision/CollisionGroups.h>
#include <AzFramework/Physics/Collision/CollisionLayers.h>
#include <AzToolsFramework/UI/PropertyEditor/ReflectedPropertyEditor.hxx>
#include <AzToolsFramework/UI/PropertyEditor/InstanceDataHierarchy.h>
#include <QBoxLayout>
#include <Editor/UI/ConfigurationWidget.h>
#include <Editor/UI/SettingsWidget.h>

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

            connect(m_settings, &SettingsWidget::onValueChanged,
                this, [this](const GNConfiguration& gnSystemConfiguration)
            {
                m_gnSystemConfiguration = gnSystemConfiguration;
                emit onConfigurationChanged(m_gnSystemConfiguration);
            });
        }

        ConfigurationWidget::~ConfigurationWidget()
        {
        }

        void ConfigurationWidget::SetConfiguration(
            const GNConfiguration& gnSystemConfiguration)
        {
            m_gnSystemConfiguration = gnSystemConfiguration;
            m_settings->SetValue(m_gnSystemConfiguration);
        }
    } // namespace Editor
} // namespace GeomNodes

#include <Editor/UI/moc_ConfigurationWidget.cpp>