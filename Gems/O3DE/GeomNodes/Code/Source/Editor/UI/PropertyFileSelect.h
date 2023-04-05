#pragma once

#if !defined(Q_MOC_RUN)
#include "PropertyFuncValBrowseEdit.h"
#endif

// Forward declare
class QPushButton;

namespace GeomNodes
{
    // Forward declare
    class ValidationHandler;

    class PropertyFileSelectCtrl
        : public PropertyFuncValBrowseEditCtrl
    {
        Q_OBJECT

    public:
        AZ_CLASS_ALLOCATOR(PropertyFileSelectCtrl, AZ::SystemAllocator)
        typedef QString(* FileSelectFuncType)(const QString&);

        PropertyFileSelectCtrl(QWidget* pParent = nullptr);

        virtual void ConsumeAttribute(AZ::u32 attrib, AzToolsFramework::PropertyAttributeReader* attrValue, const char* debugName) override;

    protected:
        void SelectFile();

        QPushButton* m_selectButton;
        FileSelectFuncType m_selectFunctor;
    };

    class PropertyFileSelectHandler
        : public AzToolsFramework::PropertyHandler<AZStd::string, PropertyFileSelectCtrl>
    {
        AZ_CLASS_ALLOCATOR(PropertyFileSelectHandler, AZ::SystemAllocator);

    public:
        PropertyFileSelectHandler(ValidationHandler* valHdlr);

        AZ::u32 GetHandlerName(void) const override;
        // Need to unregister ourselves
        bool AutoDelete() const override { return false; }

        QWidget* CreateGUI(QWidget* pParent) override;
        void ConsumeAttribute(PropertyFileSelectCtrl* GUI, AZ::u32 attrib, AzToolsFramework::PropertyAttributeReader* attrValue, const char* debugName) override;
        void WriteGUIValuesIntoProperty(size_t index, PropertyFileSelectCtrl* GUI, property_t& instance, AzToolsFramework::InstanceDataNode* node) override;
        bool ReadValuesIntoGUI(size_t index, PropertyFileSelectCtrl* GUI, const property_t& instance, AzToolsFramework::InstanceDataNode* node)  override;
        static PropertyFileSelectHandler* Register(ValidationHandler* valHdlr);

    private:
        ValidationHandler* m_validationHandler;
    };
} // namespace GeomNodes
