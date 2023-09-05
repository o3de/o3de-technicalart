#pragma once

#if !defined(Q_MOC_RUN)
#include <AzToolsFramework/UI/PropertyEditor/PropertyEditorAPI.h>
#include "HoudiniPropertyGroup.h"
#include <QVBoxLayout>
#include <QTabWidget>
#endif

#define HE_TOGGLE 1

namespace HoudiniEngine
{
    class PropertyFolderListCtrl : public QWidget
    {
        Q_OBJECT

    public:
        AZ_CLASS_ALLOCATOR(PropertyFolderListCtrl, AZ::SystemAllocator);

        PropertyFolderListCtrl(QWidget* pParent = nullptr);

#if HE_TOGGLE
		void SetFolderList(const AZStd::vector<HoudiniPropertyGroup>& groups);
        AZStd::vector<HoudiniPropertyGroup> GetFolderList() const;
#else
        void SetFolderList(const AZStd::vector<AZStd::string>& groups);
        AZStd::vector<AZStd::string> GetFolderList() const;
#endif
		

    protected:
        QVBoxLayout* Layout;
        QTabWidget* FolderContainer;

        //std::vector<HE_ParameterWidget_Folder*> Folders;
#if HE_TOGGLE
        AZStd::vector<HoudiniPropertyGroup> m_folders;
#else
        AZStd::vector<AZStd::string> m_folders;
#endif
    };

#if HE_TOGGLE
    class PropertyFolderListHandler : public AzToolsFramework::PropertyHandler<AZStd::vector<HoudiniPropertyGroup>, PropertyFolderListCtrl>
#else
    class PropertyFolderListHandler : public AzToolsFramework::PropertyHandler<AZStd::vector<AZStd::string>, PropertyFolderListCtrl>
#endif
    {
        AZ_CLASS_ALLOCATOR(PropertyFolderListHandler, AZ::SystemAllocator);

    public:
        PropertyFolderListHandler();

        AZ::u32 GetHandlerName(void) const override;
        //! Need to unregister ourselves
        bool AutoDelete() const override
        {
            return false;
        }

        QWidget* CreateGUI(QWidget* pParent) override;
        void ConsumeAttribute(PropertyFolderListCtrl* GUI, AZ::u32 attrib, AzToolsFramework::PropertyAttributeReader* attrValue, const char* debugName) override;
        void WriteGUIValuesIntoProperty(size_t index, PropertyFolderListCtrl* GUI, property_t& instance, AzToolsFramework::InstanceDataNode* node) override;
        bool ReadValuesIntoGUI(size_t index, PropertyFolderListCtrl* GUI, const property_t& instance, AzToolsFramework::InstanceDataNode* node) override;
        static void Register();

    };

}