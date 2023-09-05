
#pragma once

#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <Clients/HoudiniEngineSystemComponent.h>

#include <HoudiniEngine/HoudiniEngineBus.h>

#include <AzCore/XML/rapidxml.h>
#include <ISystem.h>
#include <AzCore/Component/EntityBus.h>

namespace HoudiniEngine
{
	class Houdini;
    class HoudiniStatusPanel;

    /// System component for HoudiniEngine editor
    class HoudiniEngineEditorSystemComponent
		: public AZ::Component
		//, protected AzToolsFramework::EditorEvents::Bus::Handler
		, protected HoudiniEngineRequestBus::Handler
		, public CrySystemEventBus::Handler
		, public ISystemEventListener
		, public AZ::TickBus::Handler
		, public AzToolsFramework::EditorEvents::Bus::Handler
		, public AZ::EntitySystemBus::Handler
    {
    public:
        AZ_COMPONENT(HoudiniEngineEditorSystemComponent, "{EA171C26-65F3-4CBF-88A8-7198442A3ED1}");
        static void Reflect(AZ::ReflectContext* context);

		static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
		static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
		static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
		static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        bool IsActive() override;
		HoudiniPtr GetHoudiniEngine() override;
		void CancelProcessorThread() override;
		void CancelProcessorJob(AZ::EntityId entityToRemove) override;
		void JoinProcessorThread() override;
		virtual AZStd::string GetHoudiniResultByCode(int code) override;  // FL[FD-10714] Houdini integration to 1.21

		HoudiniPtr m_houdiniInstance;
		HoudiniStatusPanel* m_houdiniStatusPanel;
	protected:
		////////////////////////////////////////////////////////////////////////
		// HoudiniEngineRequestBus interface implementation        
		////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// AzToolsFramework::EditorEvents
		void PopulateEditorGlobalContextMenu_SliceSection(QMenu* menu, const AZ::Vector2& point, int flags) override;
		void NotifyRegisterViews() override;
		//////////////////////////////////////////////////////////////////////////

		// Entity System Bus:
		void OnEntityActivated(const AZ::EntityId&) override;
		void OnEntityDeactivated(const AZ::EntityId&) override;

		////////////////////////////////////////////////////////////////////////
		// AZ::Component interface implementation
		void Init() override;
		void Activate() override;
		void Deactivate() override;
		////////////////////////////////////////////////////////////////////////

		bool LoadHoudiniEngine();
		void OnSystemEvent(ESystemEvent /*event*/, UINT_PTR /*wparam*/, UINT_PTR /*lparam*/) override;
		void OnCrySystemInitialized(ISystem& system, const SSystemInitParams& systemInitParams) override;
		void OnCrySystemShutdown(ISystem&) override;

		void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

		struct XmlData
		{
			bool m_okay = false;
			AZStd::string m_docSrc;
			AZ::rapidxml::xml_document<> m_doc;

			XmlData(const AZStd::string& docSource)
			{
				m_docSrc = docSource;
				m_okay = m_doc.parse<0>((char*)m_docSrc.c_str());
			}
		};

		AZ::Entity* FindTerrain();
		bool FindMatchingIds(const AzToolsFramework::EntityIdList& selectedEntityList, AZ::rapidxml::xml_node<>* contextMenuItemNode, AZStd::vector<AZ::EntityId>& matchingIds);

		void AddDynamicContextMenus(QMenu* menu, const AzToolsFramework::EntityIdList& selectedEntityList);
		void AddDynamicContextMenu(QMenu* menu, const AzToolsFramework::EntityIdList& selectedEntityList, AZStd::shared_ptr<XmlData> xml, AZ::rapidxml::xml_node<>* contextMenuNode);
		AZ::Entity* CreateNewHoudiniDigitalAsset(const AZStd::string& name, const AZ::EntityId& parent);
    };
} // namespace HoudiniEngine
