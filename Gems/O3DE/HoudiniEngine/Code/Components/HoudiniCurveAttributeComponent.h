#pragma once

#include <HoudiniEngine/HoudiniEngineBus.h>
#include <HoudiniEngine/HoudiniCommonForwards.h>
#include <Components/HoudiniNodeComponentConfig.h>

#include <LmbrCentral/Shape/SplineAttribute.h>

namespace HoudiniEngine
{
    class HoudiniNode;   

    class HoudiniCurveAttributeComponent
        : public AzToolsFramework::Components::EditorComponentBase
        , public HoudiniCurveAttributeRequestBus::Handler
        , public AzFramework::EntityDebugDisplayEventBus::Handler
        , public LmbrCentral::SplineComponentNotificationBus::MultiHandler
        , public LmbrCentral::SplineAttributeNotificationBus::Handler
    {
    public:
        AZ_EDITOR_COMPONENT(HoudiniCurveAttributeComponent, HOUDINI_CURVE_ATTRIBUTE_COMPONENT_GUID, AzToolsFramework::Components::EditorComponentBase)
        
        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        HoudiniCurveAttributeComponent() = default;
        ~HoudiniCurveAttributeComponent() override = default;
        
        ////////////////////////////////////////////////////////////////////////
        // LyFactorRequestBus interface implementation
        ////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////


        //////////////////////////////////////////////////////////////////////////
        // AzFramework::EntityDebugDisplayEventBus interface implementation
        void DisplayEntityViewport(const AzFramework::ViewportInfo& viewportInfo, AzFramework::DebugDisplayRequests& debugDisplay) override;
        //////////////////////////////////////////////////////////////////////////

        void OnEnterPaintMode();

        AZStd::string GetName() override;
        void SetName(const AZ::ComponentId& id, const AZStd::string& value) override;
        void SetValue(const AZStd::string& name, int index, float value) override;
        bool GetValue(const AZStd::string& name, int index, float& value) override;
        bool GetValueCount(const AZStd::string& name, int& count) override;
        bool GetValueRange(const AZStd::string& name, float& minValue, float& maxValue) override;
        bool GetPaintValue(const AZStd::string& name, float& value) override;
        bool GetFloatAttribute(const AZStd::string& name, LmbrCentral::SplineAttribute<float>*& splineAttribute) override;
        void CommitChanges() override;

        //Spline Attribute Changes:
        void OnAttributeAdded(size_t index) override;
        void OnAttributeRemoved(size_t index) override;
        void OnAttributesSet(size_t size) override;
        void OnAttributesCleared() override;

        bool m_liveEdit = false;
        bool m_cmdPaintAttrib = false;
        bool m_cmdCopyValues = false;
        bool m_cmdPasteValues = false;

        void OnCopyValues();
        void OnPasteValues();

        float m_paintValue = 0;
        AZ::SplinePtr m_spline;
        AZStd::string m_name;        
        LmbrCentral::SplineAttribute<float> m_attribute;

        bool m_dirty = true;

    };
}
