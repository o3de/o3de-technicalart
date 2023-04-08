
#include <AzCore/Serialization/SerializeContext.h>
#include <Editor/Systems/GeomNodesSystem.h>
#include "EditorGeomNodesSystemComponent.h"
#include <Editor/UI/EditorWindow.h>

#include <GeomNodes/GeomNodesTypeIds.h>

AZ_DEFINE_BUDGET(GeomNodes);

namespace GeomNodes
{
    AZ_COMPONENT_IMPL(EditorGeomNodesSystemComponent, "EditorGeomNodesSystemComponent",
        GeomNodesEditorSystemComponentTypeId, BaseSystemComponent);

    void EditorGeomNodesSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        GNConfiguration::Reflect(context);
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<EditorGeomNodesSystemComponent, GeomNodesSystemComponent>()
                ->Version(0);
        }
    }

    EditorGeomNodesSystemComponent::EditorGeomNodesSystemComponent()
		: m_onSystemInitializedHandler(
			[](const GNConfiguration* /*config*/)
	        {
		        
	        })
		, m_onSystemConfigChangedHandler(
			[](const GNConfiguration* /*config*/)
	        {
	        })
    {

    }

    EditorGeomNodesSystemComponent::~EditorGeomNodesSystemComponent() = default;

    void EditorGeomNodesSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("EditorGeomNodesSystemService"));
    }

    void EditorGeomNodesSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("EditorGeomNodesSystemService"));
    }

    void EditorGeomNodesSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void EditorGeomNodesSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void EditorGeomNodesSystemComponent::ActivateGeomNodesSystem()
    {
        m_system = GetGNSystem();
        if (m_system)
        {
			m_system->RegisterSystemInitializedEvent(m_onSystemInitializedHandler);
			m_system->RegisterSystemConfigurationChangedEvent(m_onSystemConfigChangedHandler);
			const GNSettingsRegistryManager& registryManager = m_system->GetSettingsRegistryManager();
			if (AZStd::optional<GNConfiguration> config = registryManager.LoadSystemConfiguration();
				config.has_value())
			{
				m_system->Initialize(&(*config));
			}
			else //load defaults if there is no config
			{
				const GNConfiguration defaultConfig = GNConfiguration::CreateDefault();
				m_system->Initialize(&defaultConfig);

				auto saveCallback = []([[maybe_unused]] const GNConfiguration& config, [[maybe_unused]] GNSettingsRegistryManager::Result result)
				{
					AZ_Warning("GeomNodes", result == GNSettingsRegistryManager::Result::Success,
						"Unable to save the default GeomNodes configuration.");
				};
				registryManager.SaveSystemConfiguration(defaultConfig, saveCallback);
			}
        }
    }

    void EditorGeomNodesSystemComponent::Activate()
    {
        GeomNodesSystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
        ActivateGeomNodesSystem();
    }

    void EditorGeomNodesSystemComponent::Deactivate()
    {
		m_onSystemInitializedHandler.Disconnect();
		m_onSystemConfigChangedHandler.Disconnect();
		if (m_system != nullptr)
		{
			m_system->Shutdown();
			m_system = nullptr;
		}

        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        GeomNodesSystemComponent::Deactivate();
    }

    void EditorGeomNodesSystemComponent::NotifyRegisterViews()
    {
        GeomNodes::Editor::EditorWindow::RegisterViewClass();
    }

} // namespace GeomNodes
