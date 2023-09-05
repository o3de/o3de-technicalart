#include "StdAfx.h"

#include <HoudiniEngine/HoudiniApi.h>
#include <HoudiniEngine/HoudiniEngineBus.h>
#include "HoudiniCurveAttributeComponent.h"
#include <HoudiniCommon.h>
#include <LmbrCentral/Shape/SplineComponentBus.h>

#include "HoudiniPaintAttribTool.h"

#include <ISystem.h>
#include <Windows.h>

#include <QApplication>
#include <QClipboard>
#include <QMimeData>


namespace HoudiniEngine
{    
    /*static*/ void HoudiniCurveAttributeComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<HoudiniCurveAttributeComponent, AzToolsFramework::Components::EditorComponentBase>()
                ->Version(1)
                ->Field("LiveEdit", &HoudiniCurveAttributeComponent::m_liveEdit)
                ->Field("Name", &HoudiniCurveAttributeComponent::m_name)                
                ->Field("CurveAttribute", &HoudiniCurveAttributeComponent::m_attribute)
                ->Field("Paint Value", &HoudiniCurveAttributeComponent::m_paintValue)
                ->Field("On Copy Values", &HoudiniCurveAttributeComponent::m_cmdCopyValues)
                ->Field("On Paste Values", &HoudiniCurveAttributeComponent::m_cmdPasteValues)
                ->Field("On Paint Command", &HoudiniCurveAttributeComponent::m_cmdPaintAttrib)
                ;

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<HoudiniCurveAttributeComponent>("Houdini Attribute Data", "Per point data passed to Houdini")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Houdini")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                    ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Houdini.png")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Houdini.png")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)

                    ->DataElement(AZ::Edit::UIHandlers::Default, &HoudiniCurveAttributeComponent::m_liveEdit, "Live Edit", "")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ_CRC("RefreshAttributesAndValues", 0xcbc2147c))

                    ->DataElement(AZ::Edit::UIHandlers::Default, &HoudiniCurveAttributeComponent::m_name, "Name", "")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ_CRC("RefreshAttributesAndValues", 0xcbc2147c))
                    
                    ->DataElement(AZ::Edit::UIHandlers::Slider, &HoudiniCurveAttributeComponent::m_paintValue, "Value", "")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ_CRC("RefreshAttributesAndValues", 0xcbc2147c))
                    ->Attribute(AZ::Edit::Attributes::Min, -10000.0)
                    ->Attribute(AZ::Edit::Attributes::Max, 10000.0)
                    ->Attribute(AZ::Edit::Attributes::SoftMin, -100.0)
                    ->Attribute(AZ::Edit::Attributes::SoftMax, 100.0)
                    ->Attribute(AZ::Edit::Attributes::Step, 1.0)

                    ->DataElement(AZ::Edit::UIHandlers::Button, &HoudiniCurveAttributeComponent::m_cmdCopyValues, "", "Copies values to the clipboard")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniCurveAttributeComponent::OnCopyValues)
                    ->Attribute(AZ::Edit::Attributes::ButtonText, "Copy Values")

                    ->DataElement(AZ::Edit::UIHandlers::Button, &HoudiniCurveAttributeComponent::m_cmdPasteValues, "", "Paste values from the clipboard")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniCurveAttributeComponent::OnPasteValues)
                    ->Attribute(AZ::Edit::Attributes::ButtonText, "Paste Values")

                    ->DataElement(AZ::Edit::UIHandlers::Button, &HoudiniCurveAttributeComponent::m_cmdPaintAttrib, "", "Enters a painting mode for this value, using the value above.")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniCurveAttributeComponent::OnEnterPaintMode)
                    ->Attribute(AZ::Edit::Attributes::ButtonText, "Paint Attribute")

                    //Too slow
                    //->DataElement(AZ::Edit::UIHandlers::Default, &HoudiniCurveAttributeComponent::m_attribute, "Curve Attribute", "")
                    //->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ_CRC("RefreshAttributesAndValues", 0xcbc2147c)) 
                    
                    ;                   
            }
        }
    }

    void HoudiniCurveAttributeComponent::OnEnterPaintMode()
    {
        AZStd::vector<AZ::EntityId> splineIds;
        splineIds.push_back(GetEntityId());
        
        auto* tool = new HoudiniPaintAttribTool();
        tool->SetSplines(splineIds);
        tool->SetAttributeName(m_name);
        //GetIEditor()->SetEditTool(tool); //O3DECONVERT
    }



    void HoudiniCurveAttributeComponent::Init()
    {
        EditorComponentBase::Init();
    }

    void HoudiniCurveAttributeComponent::Activate()
    {
        EditorComponentBase::Activate();        
        AzFramework::EntityDebugDisplayEventBus::Handler::BusConnect(GetEntityId());
        HoudiniCurveAttributeRequestBus::Handler::BusConnect(GetEntityId());
        LmbrCentral::SplineAttributeNotificationBus::Handler::BusConnect(GetEntityId());

        m_attribute.Activate(GetEntityId());

        //TODO: Spline change
    }

    void HoudiniCurveAttributeComponent::Deactivate()
    {
        //TODO: Spline change
        m_attribute.Deactivate();
        LmbrCentral::SplineAttributeNotificationBus::Handler::BusDisconnect();
        HoudiniCurveAttributeRequestBus::Handler::BusDisconnect();
        AzFramework::EntityDebugDisplayEventBus::Handler::BusDisconnect();
        EditorComponentBase::Deactivate();
    }
    
    void HoudiniCurveAttributeComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("HoudiniCurveAttributeComponent", 0xa490fa4b));
    }

    void HoudiniCurveAttributeComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& /*incompatible*/)
    {
        
    }

    void HoudiniCurveAttributeComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC("TransformService", 0x8ee22c50));
        required.push_back(AZ_CRC("SplineService", 0x2b674d3c));
    }

    void HoudiniCurveAttributeComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        (void)dependent;
    }

    void HoudiniCurveAttributeComponent::DisplayEntityViewport(const AzFramework::ViewportInfo& /*viewportInfo*/, AzFramework::DebugDisplayRequests& /*debugDisplay*/)
    {
        AZ_PROFILE_FUNCTION(Editor);
        
        /*
        auto* dc = &debugDisplay;
        AZ_Assert(dc, "Invalid display context.");

        if (m_node != nullptr)
        {
            auto points = m_node->GetGeometryPoints();
            AZ::Transform transform;
            AZ::TransformBus::EventResult(transform, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);

            for (AZ::Vector3 point : points)
            {
                auto tPoint = transform * point;
                dc->DrawBall(tPoint, 0.001f);
            }
        }
        */
    }

    AZStd::string HoudiniCurveAttributeComponent::GetName()
    {
        return m_name;
    }

    void HoudiniCurveAttributeComponent::SetName(const AZ::ComponentId& id, const AZStd::string& value)
    {
        if (id == this->GetId())
        {
            //Only settable if we are initializing.
            m_name = value;
        }
    }

    void HoudiniCurveAttributeComponent::SetValue(const AZStd::string& name, int index, float value)
    {
        if (name == m_name) 
        {
            m_attribute.SetElement(index, value);

            if (m_liveEdit)
            {
                LmbrCentral::SplineComponentNotificationBus::Event(GetEntityId(), &LmbrCentral::SplineComponentNotificationBus::Events::OnSplineChanged);
            }
        }
    }

    void HoudiniCurveAttributeComponent::CommitChanges()
    {
        LmbrCentral::SplineComponentNotificationBus::Event(GetEntityId(), &LmbrCentral::SplineComponentNotificationBus::Events::OnSplineChanged);
    }

    void HoudiniCurveAttributeComponent::OnAttributeAdded([[maybe_unused]] size_t index)
    {
        //TODO: check this "index - 1 < 0" expecting a negative value but size_t is unsigned
        /*int iPre = index - 1 < 0 ? index : index - 1;
        int iPost = index + 1 >= m_attribute.Size() ? index : index + 1;

        float vPre = m_attribute.GetElement(iPre);
        float vPost = m_attribute.GetElement(iPost);
        float vAvg = (vPre + vPost) * 0.5f;
        m_attribute.SetElement(index, vAvg);
        LmbrCentral::SplineComponentNotificationBus::Event(GetEntityId(), &LmbrCentral::SplineComponentNotificationBus::Events::OnSplineChanged);*/
    }

    void HoudiniCurveAttributeComponent::OnAttributeRemoved(size_t /*index*/)
    {

    }

    void HoudiniCurveAttributeComponent::OnAttributesSet(size_t /*size*/)
    {

    }

    void HoudiniCurveAttributeComponent::OnAttributesCleared()
    {

    }

    void HoudiniCurveAttributeComponent::OnCopyValues()
    {
        QClipboard *clipboard = QApplication::clipboard();

        QString output = "";
        for (int i = 0; i < m_attribute.Size(); i++)
        {
            output += QString::number(m_attribute.GetElement(i)) + " ";
        }

        clipboard->setText(output);
    }

    void HoudiniCurveAttributeComponent::OnPasteValues()
    {
        const QClipboard *clipboard = QApplication::clipboard();
        const QMimeData *mimeData = clipboard->mimeData();

        if (mimeData->hasText()) 
        {
            const QString& dataString = clipboard->text();

            QRegExp rx("(\\ |\\,|\\t)");
            auto parts = dataString.split(rx, Qt::SkipEmptyParts);

            for (int i = 0; i < parts.size(); i++)
            {
                bool okay = false;
                float value = parts[i].toFloat(&okay);
                
                if (okay && i < m_attribute.Size())
                {
                    m_attribute.SetElement(i, value);
                }
            }
        }

        LmbrCentral::SplineComponentNotificationBus::Event(GetEntityId(), &LmbrCentral::SplineComponentNotificationBus::Events::OnSplineChanged);
    }

    bool HoudiniCurveAttributeComponent::GetValue(const AZStd::string& name, int index, float& value)
    {
        if (name == m_name)
        {
            value = m_attribute.GetElement(index);
            return true;
        }

        return false;
    }

    bool HoudiniCurveAttributeComponent::GetValueRange(const AZStd::string& name, float& minValue, float& maxValue)
    {
        if (name == m_name)
        {
            float maxVal = -1e10f;
            float minVal = 1e10f;


            for (int i = 0; i < m_attribute.Size(); i++)
            {
                float iVal = m_attribute.GetElement(i);
                if (iVal > maxVal)
                {
                    maxVal = iVal;
                }
                if (iVal < minVal)
                {
                    minVal = iVal;
                }
            }
            
            minValue = minVal;
            maxValue = maxVal;
            return true;
        }

        return false;
    }


    bool HoudiniCurveAttributeComponent::GetValueCount(const AZStd::string& name, int& count)
    {
        if (name == m_name)
        {
            count = aznumeric_cast<int>(m_attribute.Size());
            return true;
        }

        return false;
    }

    bool HoudiniCurveAttributeComponent::GetPaintValue(const AZStd::string& name, float& value)
    {
        if (name == m_name)
        {
            value = m_paintValue;
            return true;
        }

        return false;
    }

    bool HoudiniCurveAttributeComponent::GetFloatAttribute(const AZStd::string& name, LmbrCentral::SplineAttribute<float>*& splineAttrib)
    {
        if (name == m_name)
        {
            splineAttrib = &m_attribute;
            return true;
        }

        return false;
    }

}
