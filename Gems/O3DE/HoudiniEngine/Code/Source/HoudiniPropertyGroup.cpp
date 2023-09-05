#include "HoudiniCommon.h"
#include "HoudiniPropertyGroup.h"

namespace HoudiniEngine
{
	void HoudiniPropertyGroup::Reflect(AZ::ReflectContext* reflection)
	{
		AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);
		if (serializeContext)
		{
			serializeContext->Class<HoudiniPropertyGroup>()
				->Field("Name", &HoudiniPropertyGroup::m_name)
				->Field("Folders", &HoudiniPropertyGroup::m_folders)
				->Field("Properties", &HoudiniPropertyGroup::m_properties)
				->Field("Groups", &HoudiniPropertyGroup::m_groups)
				;

			if (AZ::EditContext* ec = serializeContext->GetEditContext())
			{
				ec->Class<HoudiniPropertyGroup>("Houdini Property group", "This is a  property group")
					->ClassElement(AZ::Edit::ClassElements::EditorData, "HoudiniPropertyGroup's class attributes.")
					->Attribute(AZ::Edit::Attributes::NameLabelOverride, &HoudiniPropertyGroup::m_name)
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)
					/*->DataElement(Handlers::FolderList, &HoudiniPropertyGroup::m_folders, "", "")
					->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::HideChildren)
					->Attribute(AZ::Edit::Attributes::ContainerCanBeModified, false)*/
					->DataElement(nullptr, &HoudiniPropertyGroup::m_properties, "", "Properties in this property group")
					->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
					->Attribute(AZ::Edit::Attributes::ContainerCanBeModified, false)
					->DataElement(/*nullptr*/Handlers::FolderList, &HoudiniPropertyGroup::m_groups, "", "Subgroups in this property group")
					->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
					->Attribute(AZ::Edit::Attributes::ContainerCanBeModified, false)
					//AZ_CRC("ForbidExpansion", 0x966ad11a)
					
					;
			}
		}
	}

	HoudiniPropertyGroup* HoudiniPropertyGroup::GetGroup(const char* groupName)
    {
		for (HoudiniPropertyGroup& subGroup : m_groups)
		{
			if (subGroup.m_name == groupName)
			{
				return &subGroup;
			}
		}
		return nullptr;
    }

    HoudiniScriptProperty* HoudiniPropertyGroup::GetProperty(const char* propertyName)
    {
		for (HoudiniScriptProperty* prop : m_properties)
		{
			if (prop->m_name == propertyName)
			{
				return prop;
			}
		}
		return nullptr;
    }

    void HoudiniPropertyGroup::Clear()
    {
		for (HoudiniScriptProperty* prop : m_properties)
		{
			delete prop;
		}
		m_properties.clear();
		m_groups.clear();
    }

	HoudiniPropertyGroup::HoudiniPropertyGroup()
	{
		m_folders.push_back("Branches");
		m_folders.push_back("Leaves");
	}

	HoudiniPropertyGroup::~HoudiniPropertyGroup()
    {
        Clear();
    }
    
	HoudiniPropertyGroup& HoudiniPropertyGroup::operator=(HoudiniPropertyGroup&& rhs)
	{
		m_name.swap(rhs.m_name);
		m_properties.swap(rhs.m_properties);
		m_groups.swap(rhs.m_groups);

		return *this;
	}
    
	// HoudiniPropertyFolderList
	void HoudiniPropertyFolderList::Reflect(AZ::ReflectContext* reflection)
	{
		AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);
		if (serializeContext)
		{
			serializeContext->Class<HoudiniPropertyFolderList>()
				->Field("Name", &HoudiniPropertyFolderList::m_name)
				->Field("Properties", &HoudiniPropertyFolderList::m_properties)
				->Field("Folders", &HoudiniPropertyFolderList::m_groups);

			if (AZ::EditContext* ec = serializeContext->GetEditContext())
			{
				ec->Class<HoudiniPropertyFolderList>("Houdini Property group", "This is a  property group")
					->ClassElement(AZ::Edit::ClassElements::EditorData, "HoudiniPropertyFolderList's class attributes.")
					->Attribute(AZ::Edit::Attributes::NameLabelOverride, &HoudiniPropertyFolderList::m_name)
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)
					->DataElement(nullptr, &HoudiniPropertyFolderList::m_properties, "", "Properties in this property group")
					->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
					->Attribute(AZ::Edit::Attributes::ContainerCanBeModified, false)
					->DataElement(nullptr/*Handlers::FolderList*/, &HoudiniPropertyFolderList::m_groups, "", "Subgroups in this property group")
					->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
					->Attribute(AZ::Edit::Attributes::ContainerCanBeModified, false)
					//AZ_CRC("ForbidExpansion", 0x966ad11a)
					;
			}
		}
	}

	HoudiniPropertyFolderList* HoudiniPropertyFolderList::GetGroup(const char* groupName)
	{
		for (HoudiniPropertyFolderList& subGroup : m_groups)
		{
			if (subGroup.m_name == groupName)
			{
				return &subGroup;
			}
		}
		return nullptr;
	}

	HoudiniScriptProperty* HoudiniPropertyFolderList::GetProperty(const char* propertyName)
	{
		for (HoudiniScriptProperty* prop : m_properties)
		{
			if (prop->m_name == propertyName)
			{
				return prop;
			}
		}
		return nullptr;
	}

	void HoudiniPropertyFolderList::Clear()
	{
		for (HoudiniScriptProperty* prop : m_properties)
		{
			delete prop;
		}
		m_properties.clear();
		m_groups.clear();
	}

	HoudiniPropertyFolderList::~HoudiniPropertyFolderList()
	{
		Clear();
	}

	HoudiniPropertyFolderList& HoudiniPropertyFolderList::operator=(HoudiniPropertyFolderList&& rhs)
	{
		m_name.swap(rhs.m_name);
		m_properties.swap(rhs.m_properties);
		m_groups.swap(rhs.m_groups);

		return *this;
	}
}