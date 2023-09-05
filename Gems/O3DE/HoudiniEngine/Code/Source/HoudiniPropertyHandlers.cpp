#include "HoudiniPropertyHandlers.h"
#include "HoudiniConstants.h"

namespace HoudiniEngine
{
	PropertyFolderListCtrl::PropertyFolderListCtrl(QWidget* pParent)
        : QWidget(pParent)
	{
		Layout = new QVBoxLayout;
		FolderContainer = new QTabWidget;

		Layout->setAlignment(Qt::AlignTop);
		Layout->addWidget(FolderContainer);

		this->setLayout(Layout);
	}

#if HE_TOGGLE
    void PropertyFolderListCtrl::SetFolderList(const AZStd::vector<HoudiniPropertyGroup>& groups)
    {
        //m_folders = folderList;
        FolderContainer->clear();
        for (auto& group : groups)
        {
			auto tab = new QWidget();
			tab->setObjectName(QString::fromUtf8(group.m_name.c_str()));
            FolderContainer->addTab(tab, group.m_name.c_str());
            FolderContainer->show();
        }
    }

	AZStd::vector<HoudiniPropertyGroup> PropertyFolderListCtrl::GetFolderList() const
	{
		return m_folders;
	}
#else
    void PropertyFolderListCtrl::SetFolderList(const AZStd::vector<AZStd::string>& groups)
    {
		m_folders = groups;
		FolderContainer->clear();
		for (auto& group : m_folders)
		{
			auto tab = new QWidget();
			tab->setObjectName(QString::fromUtf8(group.c_str()));
			FolderContainer->addTab(tab, group.c_str());
			FolderContainer->show();
		}
    }

	AZStd::vector<AZStd::string> PropertyFolderListCtrl::GetFolderList() const
	{
		return m_folders;
	}
#endif

    PropertyFolderListHandler::PropertyFolderListHandler()
#if HE_TOGGLE
        : AzToolsFramework::PropertyHandler<AZStd::vector<HoudiniPropertyGroup>, PropertyFolderListCtrl>()
#else
        : AzToolsFramework::PropertyHandler<AZStd::vector<AZStd::string>, PropertyFolderListCtrl>()
#endif
    {

    }

    AZ::u32 PropertyFolderListHandler::GetHandlerName(void) const
    {
        return Handlers::FolderList;
    }

    QWidget* PropertyFolderListHandler::CreateGUI(QWidget* parent)
    {
        return aznew PropertyFolderListCtrl(parent);
    }

    void PropertyFolderListHandler::ConsumeAttribute(PropertyFolderListCtrl* /*GUI*/, AZ::u32 /*attrib*/, AzToolsFramework::PropertyAttributeReader* /*attrValue*/, const char* /*debugName*/)
    {
    }

    void PropertyFolderListHandler::WriteGUIValuesIntoProperty(size_t /*index*/, PropertyFolderListCtrl* GUI, property_t& instance, AzToolsFramework::InstanceDataNode* /*node*/)
    {
        instance = GUI->GetFolderList();
    }

    bool PropertyFolderListHandler::ReadValuesIntoGUI(size_t /*index*/, PropertyFolderListCtrl* GUI, const property_t& instance, AzToolsFramework::InstanceDataNode* /*node*/)
    {
        QSignalBlocker signalBlocker(GUI);
        GUI->SetFolderList(instance);
        return true;
    }

    void PropertyFolderListHandler::Register()
    {
        AzToolsFramework::PropertyTypeRegistrationMessages::Bus::Broadcast(
			&AzToolsFramework::PropertyTypeRegistrationMessages::Bus::Events::RegisterPropertyType, aznew PropertyFolderListHandler());
    }
}

#include "moc_HoudiniPropertyHandlers.cpp"