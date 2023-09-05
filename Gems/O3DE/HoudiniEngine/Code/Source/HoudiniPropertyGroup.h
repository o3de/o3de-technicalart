#pragma once

namespace HoudiniEngine
{
	class HoudiniScriptProperty;

    class HoudiniPropertyGroup
	{
    public:
		//AZ_TYPE_INFO(HoudiniPropertyGroup, "{4F2F3EB4-ED96-4981-B427-FED76D966889}");
		AZ_RTTI(HoudiniPropertyGroup, "{4F2F3EB4-ED96-4981-B427-FED76D966889}");
		AZ_CLASS_ALLOCATOR(HoudiniPropertyGroup, AZ::SystemAllocator);
		static void Reflect(AZ::ReflectContext* reflection);
		AZStd::string m_name;
		AZStd::vector<HoudiniScriptProperty*> m_properties;
		AZStd::vector<HoudiniPropertyGroup> m_groups;
		AZStd::vector<AZStd::string> m_folders;

		//! Get the pointer to the specified group in m_groups. Returns nullptr if not found.
        HoudiniPropertyGroup* GetGroup(const char* groupName);
		//! Get the pointer to the specified property in m_properties. Returns nullptr if not found.
        HoudiniScriptProperty* GetProperty(const char* propertyName);
		
		//! Remove all properties and groups
		void Clear();

        HoudiniPropertyGroup();
		virtual ~HoudiniPropertyGroup();

        HoudiniPropertyGroup(const HoudiniPropertyGroup& rhs) = default;
        HoudiniPropertyGroup& operator=(HoudiniPropertyGroup&) = default;

	
        HoudiniPropertyGroup(HoudiniPropertyGroup&& rhs)
		{
			*this = AZStd::move(rhs);
		}
        HoudiniPropertyGroup& operator=(HoudiniPropertyGroup&& rhs);
	};

	class HoudiniPropertyFolderList
	{
	public:
		AZ_TYPE_INFO(HoudiniPropertyFolderList, "{9A191F97-B286-47BB-9A87-C98FCEDCD0CB}");
		static void Reflect(AZ::ReflectContext* reflection);
		AZStd::string m_name;
		AZStd::vector<HoudiniScriptProperty*> m_properties;
		AZStd::vector<HoudiniPropertyFolderList> m_groups;

		//! Get the pointer to the specified group in m_groups. Returns nullptr if not found.
		HoudiniPropertyFolderList* GetGroup(const char* groupName);
		//! Get the pointer to the specified property in m_properties. Returns nullptr if not found.
		HoudiniScriptProperty* GetProperty(const char* propertyName);

		//! Remove all properties and groups
		void Clear();

		HoudiniPropertyFolderList() = default;
		~HoudiniPropertyFolderList();

		HoudiniPropertyFolderList(const HoudiniPropertyFolderList& rhs) = delete;
		HoudiniPropertyFolderList& operator=(HoudiniPropertyFolderList&) = delete;


		HoudiniPropertyFolderList(HoudiniPropertyFolderList&& rhs)
		{
			*this = AZStd::move(rhs);
		}
		HoudiniPropertyFolderList& operator=(HoudiniPropertyFolderList&& rhs);
	};
}