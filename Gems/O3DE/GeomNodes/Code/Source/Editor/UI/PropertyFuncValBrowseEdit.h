#pragma once

#if !defined(Q_MOC_RUN)
#include "FunctorValidator.h"

#include <AzToolsFramework/UI/PropertyEditor/PropertyEditorAPI.h>
#include <AzQtComponents/Components/Widgets/BrowseEdit.h>
#endif

#include "UI_common.h"

namespace GeomNodes
{
    // Forward Declare
    class ValidationHandler;
    
    class PropertyFuncValBrowseEditCtrl
        : public QWidget
    {
        Q_OBJECT

    public:
        AZ_CLASS_ALLOCATOR(PropertyFuncValBrowseEditCtrl, AZ::SystemAllocator, 0);

        using ValidationCallbackType = AZ::Edit::AttributeFunction<void(const AZStd::string&)>;

        PropertyFuncValBrowseEditCtrl(QWidget* pParent = nullptr);

        virtual QString GetValue() const;
        // Sets value programmtically and triggers validation
        virtual void SetValue(const QString& value);
        // Sets value as if user set it
        void SetValueUser(const QString& value);
        // Returns pointer to the validator used
        FunctorValidator* GetValidator();
        // Sets the validator for the lineedit
        void SetValidator(FunctorValidator* validator);
        // Sets the validator for the linedit
        void SetValidator(FunctorValidator::FunctorType validator);
        // Returns false if invalid and returns shows error as tooltip
        bool ValidateAndShowErrors();
        // Forces the values to up validated and style updated
        void ForceValidate();
        virtual void SetNotifyTarget(void* notifyTarget);

        // Returns a pointer to the BrowseEdit object.
        AzQtComponents::BrowseEdit* browseEdit();

        virtual void ConsumeAttribute(AZ::u32 attrib, AzToolsFramework::PropertyAttributeReader* attrValue, const char* debugName);

    signals:
        void ValueChangedByUser();

    protected:
        // Keeps track of the validator so no const_casts must be done
        FunctorValidator* m_validator;
        ValidationCallbackType* m_validationResultHandler = nullptr;
        void* m_notifyTarget = nullptr;

        AzQtComponents::BrowseEdit* m_browseEdit = nullptr;
    };

    class PropertyFuncValBrowseEditHandler
        : public AzToolsFramework::PropertyHandler <AZStd::string, PropertyFuncValBrowseEditCtrl>
    {
        AZ_CLASS_ALLOCATOR(PropertyFuncValBrowseEditHandler, AZ::SystemAllocator, 0);

    public:
        PropertyFuncValBrowseEditHandler(ValidationHandler* valHdlr);

        AZ::u32 GetHandlerName(void) const override;
        // Need to unregister ourselves
        bool AutoDelete() const override { return false; }

        QWidget* CreateGUI(QWidget* pParent) override;
        void ConsumeAttribute(PropertyFuncValBrowseEditCtrl* GUI, AZ::u32 attrib, AzToolsFramework::PropertyAttributeReader* attrValue, const char* debugName) override;
        void WriteGUIValuesIntoProperty(size_t index, PropertyFuncValBrowseEditCtrl* GUI, property_t& instance, AzToolsFramework::InstanceDataNode* node) override;
        bool ReadValuesIntoGUI(size_t index, PropertyFuncValBrowseEditCtrl* GUI, const property_t& instance, AzToolsFramework::InstanceDataNode* node)  override;
        static PropertyFuncValBrowseEditHandler* Register(ValidationHandler* valHdlr);

    private:
        ValidationHandler* m_validationHandler;
    };
} // namespace GeomNodes