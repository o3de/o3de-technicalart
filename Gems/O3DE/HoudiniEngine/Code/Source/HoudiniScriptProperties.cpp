#include "StdAfx.h"

#include <HoudiniCommon.h>

#include <AzFramework/Asset/AssetSystemBus.h>
#include <AzToolsFramework/API/EditorAssetSystemAPI.h>
#include <AzToolsFramework/Undo/UndoSystem.h> 

namespace HoudiniEngine
{
    HoudiniScriptProperty::HoudiniScriptProperty(HoudiniParameterPtr parameter) : ScriptProperty(parameter->GetName().c_str()),
        m_hou(parameter->GetHou())
        , m_parameter(parameter)
        , m_node(nullptr)
    {
        m_session = const_cast<HAPI_Session*>(&m_hou->GetSession());
        m_name = parameter->GetName();
        m_label = parameter->GetLabel();
        m_help = parameter->GetHelp();
        m_desc = m_name + ": " + m_help;

        m_parameter = parameter;
        m_min_int = (int)lroundf(m_min = parameter->GetInfo().min);
        m_max_int = (int)lroundf(m_max = parameter->GetInfo().max);        
        m_step_int = m_step = 1.0;
    }

    HoudiniScriptProperty::HoudiniScriptProperty(HoudiniNodePtr node, int index) : ScriptProperty((node->GetOperatorName() + AZStd::string::format("%d", index)).c_str()),
        m_hou(node->GetHou())
        , m_parameter(nullptr)
        , m_node(node)
    {
        m_session = const_cast<HAPI_Session*>(&m_hou->GetSession());
        m_help = "An input parameter to the node itself";
        m_desc = node->GetOperatorName() + ": " + m_help;

        HAPI_StringHandle nameHandle;
        HAPI_GetNodeInputName(m_session, node->GetId(), index, &nameHandle);
        m_name = m_hou->GetString(nameHandle);
        m_label = m_name;

        m_min_int = m_min = 0;
        m_max_int = m_max = 0;
        m_step_int = m_step = 1.0;
    }

    /////////////////////////
    // HoudiniScriptProperty
    /////////////////////////
    void HoudiniScriptProperty::Reflect(AZ::ReflectContext* reflection)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);

        if (serializeContext)
        {
            serializeContext->Class<HoudiniScriptProperty, AZ::ScriptProperty>()->
                Version(3, HoudiniScriptProperty::VersionConverter)
                ->Field("label", &HoudiniScriptProperty::m_label)
                ->Field("groupName", &HoudiniScriptProperty::m_groupName);

            AZ::EditContext* ec = serializeContext->GetEditContext();
            ec->Class<HoudiniScriptProperty>("Script Property (string)", "A script string property")->
                ClassElement(AZ::Edit::ClassElements::EditorData, "ScriptPropertyGroup's class attributes.")->
                Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                ;
        }
    }

    bool HoudiniScriptProperty::VersionConverter(AZ::SerializeContext& context, AZ::SerializeContext::DataElementNode& classElement)
    {
        if (classElement.GetVersion() == 1)
        {
            //Use the old name as the new label:
            AZStd::string label;
            
            auto baseClassNode = classElement.FindSubElement(AZ::Crc32("BaseClass1"));
            if (!baseClassNode)
            {
                return false;
            }

            if (!baseClassNode->GetChildData(AZ::Crc32("name"), label))
            {
                return false;
            }
            
            classElement.AddElementWithData<AZStd::string>(context, "label", label);
        }
        return true;
    }

    AZ::u32 HoudiniScriptProperty::ScriptHasChanged()
    {
        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }


    //////////////////////////
    ////  Boolean
    /////////////////////////
    HoudiniScriptPropertyBoolean::HoudiniScriptPropertyBoolean(HoudiniParameterPtr parameter) : HoudiniScriptProperty(parameter)
    {
        m_value = parameter->GetValueBoolean();
    }

    void HoudiniScriptPropertyBoolean::Reflect(AZ::ReflectContext* reflection)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);

        if (serializeContext)
        {
            serializeContext->Class<HoudiniScriptPropertyBoolean, HoudiniScriptProperty>()->
                Version(1)->
                Field("value", &HoudiniScriptPropertyBoolean::m_value);

            AZ::EditContext* ec = serializeContext->GetEditContext();
            ec->Class<HoudiniScriptPropertyBoolean>("Script Property (string)", "A script string property")->                                                
                ClassElement(AZ::Edit::ClassElements::EditorData, "ScriptPropertyGroup's class attributes.")->                
                Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)->                
                DataElement(AZ::Edit::UIHandlers::CheckBox, &HoudiniScriptPropertyBoolean::m_value, "m_value", "Boolean")->
                Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniScriptProperty::ScriptHasChanged)->
                Attribute(AZ::Edit::Attributes::Suffix, " ")->
                Attribute(AZ::Edit::Attributes::DescriptionTextOverride, &HoudiniScriptProperty::m_desc)->
                //Attribute(AZ::Edit::Attributes::GroupDynamic, &HoudiniScriptProperty::GetGroupName)-> //Requires group dynamic support. //O3DECONVERT
                Attribute(AZ::Edit::Attributes::NameLabelOverride, &HoudiniScriptProperty::m_label);
        }
    }

    AZ::u32 HoudiniScriptPropertyBoolean::ScriptHasChanged()
    {
        if (m_parameter != nullptr)
        {
            m_parameter->SetValueBool(m_value);
        }

        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }

    //////////////////////////
    ////  STRING
    /////////////////////////
    HoudiniScriptPropertyString::HoudiniScriptPropertyString(HoudiniParameterPtr parameter) : HoudiniScriptProperty(parameter)
    {
        if (parameter->GetInfo().size == 1)
        {
            m_value = parameter->GetValueString();
        }
    }

    void HoudiniScriptPropertyString::Reflect(AZ::ReflectContext* reflection)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);

        if (serializeContext)
        {
            serializeContext->Class<HoudiniScriptPropertyString, HoudiniScriptProperty>()->
                Version(1)->
                Field("value", &HoudiniScriptPropertyString::m_value);

            AZ::EditContext* ec = serializeContext->GetEditContext();
            ec->Class<HoudiniScriptPropertyString>("HDA String", "A string property")->
                ClassElement(AZ::Edit::ClassElements::EditorData, "HDA String class attributes")->
                Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)->
                DataElement(AZ::Edit::UIHandlers::LineEdit, &HoudiniScriptPropertyString::m_value, "m_value", "A string")->
                Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniScriptProperty::ScriptHasChanged)->
                Attribute(AZ::Edit::Attributes::Min, -100000000.0)->
                Attribute(AZ::Edit::Attributes::Max, 100000000.0)->
                Attribute(AZ::Edit::Attributes::SoftMin, &HoudiniScriptProperty::m_min)->
                Attribute(AZ::Edit::Attributes::SoftMax, &HoudiniScriptProperty::m_max)->
                Attribute(AZ::Edit::Attributes::Step, &HoudiniScriptProperty::m_step)->
                Attribute(AZ::Edit::Attributes::Suffix, " ")->
                Attribute(AZ::Edit::Attributes::DescriptionTextOverride, &HoudiniScriptProperty::m_desc)->
                //Attribute(AZ::Edit::Attributes::GroupDynamic, &HoudiniScriptProperty::GetGroupName)-> //O3DECONVERT
                Attribute(AZ::Edit::Attributes::NameLabelOverride, &HoudiniScriptProperty::m_label);
        }
    }

    AZ::u32 HoudiniScriptPropertyString::ScriptHasChanged()
    {
        if (m_parameter != nullptr)
        {
            m_parameter->SetValueString(m_value);
        }

        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }

    //////////////////////////
    ////  INT
    /////////////////////////
    HoudiniScriptPropertyInt::HoudiniScriptPropertyInt(HoudiniParameterPtr parameter) : HoudiniScriptProperty(parameter)
    {
        if (parameter->GetInfo().size == 1)
        {
            m_value = parameter->GetValueInt();
        }
    }

    void HoudiniScriptPropertyInt::Reflect(AZ::ReflectContext* reflection)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);

        if (serializeContext)
        {
            serializeContext->Class<HoudiniScriptPropertyInt, HoudiniScriptProperty>()->
                Version(1)->
                Field("value", &HoudiniScriptPropertyInt::m_value);

            AZ::EditContext* ec = serializeContext->GetEditContext();
            ec->Class<HoudiniScriptPropertyInt>("Integer", "An HDA int property")->
                ClassElement(AZ::Edit::ClassElements::EditorData, "HDA Class attributes")->
                Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)->
                DataElement(AZ::Edit::UIHandlers::Slider, &HoudiniScriptPropertyInt::m_value, "m_value", "An integer")->
                Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniScriptProperty::ScriptHasChanged)->
                Attribute(AZ::Edit::Attributes::Min, -100000000.0)->
                Attribute(AZ::Edit::Attributes::Max, 100000000.0)->
                Attribute(AZ::Edit::Attributes::SoftMin, &HoudiniScriptProperty::m_min_int)->
                Attribute(AZ::Edit::Attributes::SoftMax, &HoudiniScriptProperty::m_max_int)->
                Attribute(AZ::Edit::Attributes::Step, &HoudiniScriptProperty::m_step_int)->
                Attribute(AZ::Edit::Attributes::Suffix, " ")->
                Attribute(AZ::Edit::Attributes::DescriptionTextOverride, &HoudiniScriptProperty::m_desc)->
                //Attribute(AZ::Edit::Attributes::GroupDynamic, &HoudiniScriptProperty::GetGroupName)-> //O3DECONVERT
                Attribute(AZ::Edit::Attributes::NameLabelOverride, &HoudiniScriptProperty::m_label);
        }
    }

    AZ::u32 HoudiniScriptPropertyInt::ScriptHasChanged()
    {
        if (m_parameter != nullptr)
        {
            m_parameter->SetValueInt((int)m_value);
        }

        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }

	//////////////////////////
	////  CHOICE INT
	/////////////////////////
	void HoudiniScriptPropertyIntChoice::Reflect(AZ::ReflectContext* reflection)
	{
		AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);

		if (serializeContext)
		{
			serializeContext->Class<HoudiniScriptPropertyIntChoice, HoudiniScriptProperty>()
                ->Version(1)
                ->Field("value", &HoudiniScriptPropertyIntChoice::m_value)
                ->Field("choice", &HoudiniScriptPropertyIntChoice::m_choice)
                ->Field("choices", &HoudiniScriptPropertyIntChoice::m_choices)
                ;


			AZ::EditContext* ec = serializeContext->GetEditContext();
			ec->Class<HoudiniScriptPropertyIntChoice>("Integer Choices", "An HDA int choices property")->
				ClassElement(AZ::Edit::ClassElements::EditorData, "HDA Class attributes")->
				Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)->
				DataElement(AZ::Edit::UIHandlers::ComboBox, &HoudiniScriptPropertyIntChoice::m_choice, "", "")->
				Attribute(AZ::Edit::Attributes::StringList, &HoudiniScriptPropertyIntChoice::GetChoices)->
                Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniScriptProperty::ScriptHasChanged)->
				Attribute(AZ::Edit::Attributes::DescriptionTextOverride, &HoudiniScriptProperty::m_desc)->
				//Attribute(AZ::Edit::Attributes::GroupDynamic, &HoudiniScriptProperty::GetGroupName)-> //O3DECONVERT
				Attribute(AZ::Edit::Attributes::NameLabelOverride, &HoudiniScriptProperty::m_label);
		}
	}

	HoudiniScriptPropertyIntChoice::HoudiniScriptPropertyIntChoice(HoudiniParameterPtr parameter) : HoudiniScriptProperty(parameter)
	{
		if (parameter->GetInfo().size == 1 && parameter->GetInfo().choiceCount > 0)
		{
            m_choices = parameter->GetValueChoices();
			m_value = parameter->GetValuesInt(1)[0];
            if (!m_choices.empty())
            {
                m_choice = m_choices[m_value];
            }
		}
	}

	AZ::u32 HoudiniScriptPropertyIntChoice::ScriptHasChanged()
	{
		if (m_parameter != nullptr)
		{
            m_value = GetChoiceIndex(m_choice);
			m_parameter->SetValueInt((int)m_value);
		}

		return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
	}

	AZStd::vector<AZStd::string> HoudiniScriptPropertyIntChoice::GetChoices()
	{
		return m_choices;
	}

    int HoudiniScriptPropertyIntChoice::GetChoiceIndex(const AZStd::string& choice)
    {
        int index = 0;
        if (m_choices.empty())
            return index;

        auto it = AZStd::find(m_choices.begin(), m_choices.end(), choice);
        if (it != m_choices.end())
        {
            index = it - m_choices.begin();
        }
        return index;
    }

    //////////////////////////
    ////  FLOAT
    /////////////////////////
    HoudiniScriptPropertyFloat::HoudiniScriptPropertyFloat(HoudiniParameterPtr parameter) : HoudiniScriptProperty(parameter)
    {
        if (parameter->GetInfo().size == 1)
        {
            m_value = parameter->GetValueFloat();
        }
    }

    void HoudiniScriptPropertyFloat::Reflect(AZ::ReflectContext* reflection)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);

        if (serializeContext)
        {
            serializeContext->Class<HoudiniScriptPropertyFloat, HoudiniScriptProperty>()->
                Version(1)->
                Field("value", &HoudiniScriptPropertyFloat::m_value);

            AZ::EditContext* ec = serializeContext->GetEditContext();
            ec->Class<HoudiniScriptPropertyFloat>("Floats", "A script string property")->
                ClassElement(AZ::Edit::ClassElements::EditorData, "ScriptPropertyGroup's class attributes.")->
                Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)->
                DataElement(AZ::Edit::UIHandlers::Slider, &HoudiniScriptPropertyFloat::m_value, "m_value", "Floating point number")->
                Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniScriptProperty::ScriptHasChanged)->
                Attribute(AZ::Edit::Attributes::Min, -100000000.0)->
                Attribute(AZ::Edit::Attributes::Max, 100000000.0)->
                Attribute(AZ::Edit::Attributes::SoftMin, &HoudiniScriptProperty::m_min)->
                Attribute(AZ::Edit::Attributes::SoftMax, &HoudiniScriptProperty::m_max)->
                Attribute(AZ::Edit::Attributes::Step, &HoudiniScriptProperty::m_step)->
                Attribute(AZ::Edit::Attributes::Suffix, " ")->
                Attribute(AZ::Edit::Attributes::DescriptionTextOverride, &HoudiniScriptProperty::m_desc)->
                //Attribute(AZ::Edit::Attributes::GroupDynamic, &HoudiniScriptProperty::GetGroupName)-> //O3DECONVERT
                Attribute(AZ::Edit::Attributes::NameLabelOverride, &HoudiniScriptProperty::m_label);
        }
    }

    AZ::u32 HoudiniScriptPropertyFloat::ScriptHasChanged()
    {
        if (m_parameter != nullptr)
        {
            m_parameter->SetValueFloat((float)m_value);
        }

        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }




    //////////////////////////
    ////  Vector 2
    /////////////////////////
    HoudiniScriptPropertyVector2::HoudiniScriptPropertyVector2(HoudiniParameterPtr parameter) : HoudiniScriptProperty(parameter)
    {
        if (parameter->GetInfo().size == 2)
        {
            m_value = parameter->GetValueVec2();
        }
    }

    void HoudiniScriptPropertyVector2::Reflect(AZ::ReflectContext* reflection)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);

        if (serializeContext)
        {
            serializeContext->Class<HoudiniScriptPropertyVector2, HoudiniScriptProperty>()->
                Version(1)->
                Field("value", &HoudiniScriptPropertyVector2::m_value);

            AZ::EditContext* ec = serializeContext->GetEditContext();
            ec->Class<HoudiniScriptPropertyVector2>("Script Property (string)", "A script string property")->
                ClassElement(AZ::Edit::ClassElements::EditorData, "ScriptPropertyGroup's class attributes.")->
                Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)->
                DataElement(AZ::Edit::UIHandlers::Vector2, &HoudiniScriptPropertyVector2::m_value, "m_value", "Vector2 (x,y)")->
                Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniScriptProperty::ScriptHasChanged)->
                Attribute(AZ::Edit::Attributes::Min, -100000000.0)->
                Attribute(AZ::Edit::Attributes::Max, 100000000.0)->
                Attribute(AZ::Edit::Attributes::SoftMin, &HoudiniScriptProperty::m_min)->
                Attribute(AZ::Edit::Attributes::SoftMax, &HoudiniScriptProperty::m_max)->
                Attribute(AZ::Edit::Attributes::Step, &HoudiniScriptProperty::m_step)->
                Attribute(AZ::Edit::Attributes::Suffix, " ")->
                Attribute(AZ::Edit::Attributes::DescriptionTextOverride, &HoudiniScriptProperty::m_desc)->
                //Attribute(AZ::Edit::Attributes::GroupDynamic, &HoudiniScriptProperty::GetGroupName)-> //O3DECONVERT
                Attribute(AZ::Edit::Attributes::NameLabelOverride, &HoudiniScriptProperty::m_label);
        }
    }

    AZ::u32 HoudiniScriptPropertyVector2::ScriptHasChanged()
    {
        if (m_parameter != nullptr)
        {
            m_parameter->SetValueVec2(m_value);
        }

        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }




    //////////////////////////
    ////  Vector 3
    /////////////////////////
    HoudiniScriptPropertyVector3::HoudiniScriptPropertyVector3(HoudiniParameterPtr parameter) : HoudiniScriptProperty(parameter)
    {
        if (parameter->GetInfo().size == 3)
        {
            m_value = parameter->GetValueVec3();
        }
    }

    void HoudiniScriptPropertyVector3::Reflect(AZ::ReflectContext* reflection)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);

        if (serializeContext)
        {
            serializeContext->Class<HoudiniScriptPropertyVector3, HoudiniScriptProperty>()->
                Version(1)->
                Field("value", &HoudiniScriptPropertyVector3::m_value);

            AZ::EditContext* ec = serializeContext->GetEditContext();
            ec->Class<HoudiniScriptPropertyVector3>("Script Property (string)", "A script string property")->
                ClassElement(AZ::Edit::ClassElements::EditorData, "ScriptPropertyGroup's class attributes.")->
                Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)->
                DataElement(AZ::Edit::UIHandlers::Vector3, &HoudiniScriptPropertyVector3::m_value, "m_value", "Vector3 (x,y,z)")->
                Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniScriptProperty::ScriptHasChanged)->
                Attribute(AZ::Edit::Attributes::Min, -100000000.0)->
                Attribute(AZ::Edit::Attributes::Max, 100000000.0)->
                Attribute(AZ::Edit::Attributes::SoftMin, &HoudiniScriptProperty::m_min)->
                Attribute(AZ::Edit::Attributes::SoftMax, &HoudiniScriptProperty::m_max)->
                Attribute(AZ::Edit::Attributes::Step, &HoudiniScriptProperty::m_step)->
                Attribute(AZ::Edit::Attributes::Suffix, " ")->
                Attribute(AZ::Edit::Attributes::DescriptionTextOverride, &HoudiniScriptProperty::m_desc)->
                //Attribute(AZ::Edit::Attributes::GroupDynamic, &HoudiniScriptProperty::GetGroupName)-> //O3DECONVERT
                Attribute(AZ::Edit::Attributes::NameLabelOverride, &HoudiniScriptProperty::m_label);
        }
    }

    AZ::u32 HoudiniScriptPropertyVector3::ScriptHasChanged()
    {
        if (m_parameter != nullptr)
        {
            m_parameter->SetValueVec3(m_value);
        }

        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }




    //////////////////////////
    ////  Vector 4
    /////////////////////////
    HoudiniScriptPropertyVector4::HoudiniScriptPropertyVector4(HoudiniParameterPtr parameter) : HoudiniScriptProperty(parameter)
    {
        if (parameter->GetInfo().size == 4)
        {
            m_value = parameter->GetValueVec4();
        }
    }

    void HoudiniScriptPropertyVector4::Reflect(AZ::ReflectContext* reflection)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);

        if (serializeContext)
        {
            serializeContext->Class<HoudiniScriptPropertyVector4, HoudiniScriptProperty>()->
                Version(1)->
                Field("value", &HoudiniScriptPropertyVector4::m_value);

            AZ::EditContext* ec = serializeContext->GetEditContext();
            ec->Class<HoudiniScriptPropertyVector4>("Script Property (string)", "A script string property")->
                ClassElement(AZ::Edit::ClassElements::EditorData, "ScriptPropertyGroup's class attributes.")->
                Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)->
                DataElement(AZ::Edit::UIHandlers::Vector4, &HoudiniScriptPropertyVector4::m_value, "m_value", "Vector4 (x,y,z,w)")->
                Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniScriptProperty::ScriptHasChanged)->
                Attribute(AZ::Edit::Attributes::Min, -100000000.0)->
                Attribute(AZ::Edit::Attributes::Max, 100000000.0)->
                Attribute(AZ::Edit::Attributes::SoftMin, &HoudiniScriptProperty::m_min)->
                Attribute(AZ::Edit::Attributes::SoftMax, &HoudiniScriptProperty::m_max)->
                Attribute(AZ::Edit::Attributes::Step, &HoudiniScriptProperty::m_step)->
                Attribute(AZ::Edit::Attributes::Suffix, " ")->
                Attribute(AZ::Edit::Attributes::DescriptionTextOverride, &HoudiniScriptProperty::m_desc)->
                //Attribute(AZ::Edit::Attributes::GroupDynamic, &HoudiniScriptProperty::GetGroupName)-> //O3DECONVERT
                Attribute(AZ::Edit::Attributes::NameLabelOverride, &HoudiniScriptProperty::m_label);
        }
    }

    AZ::u32 HoudiniScriptPropertyVector4::ScriptHasChanged()
    {
        if (m_parameter != nullptr)
        {
            m_parameter->SetValueVec4(m_value);
        }

        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }





    //////////////////////////
    ////  EntityId
    /////////////////////////
    HoudiniScriptPropertyEntity::HoudiniScriptPropertyEntity(HoudiniParameterPtr parameter) : HoudiniScriptProperty(parameter)
    {
        m_value = parameter->GetValueEntity();
    }

    void HoudiniScriptPropertyEntity::Reflect(AZ::ReflectContext* reflection)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);

        if (serializeContext)
        {
            serializeContext->Class<HoudiniScriptPropertyEntity, HoudiniScriptProperty>()->
                Version(1)->
                Field("value", &HoudiniScriptPropertyEntity::m_value);

            AZ::EditContext* ec = serializeContext->GetEditContext();
            ec->Class<HoudiniScriptPropertyEntity>("Script Property (string)", "A script string property")->
                ClassElement(AZ::Edit::ClassElements::EditorData, "ScriptPropertyGroup's class attributes.")->
                Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)->
                DataElement(AZ::Edit::UIHandlers::EntityId, &HoudiniScriptPropertyEntity::m_value, "m_value", "Entity ID")->
                Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniScriptProperty::ScriptHasChanged)->
                Attribute(AZ::Edit::Attributes::Suffix, " ")->
                Attribute(AZ::Edit::Attributes::DescriptionTextOverride, &HoudiniScriptProperty::m_desc)->
                //Attribute(AZ::Edit::Attributes::GroupDynamic, &HoudiniScriptProperty::GetGroupName)-> //O3DECONVERT
                Attribute(AZ::Edit::Attributes::NameLabelOverride, &HoudiniScriptProperty::m_label);
        }
    }

    AZ::u32 HoudiniScriptPropertyEntity::ScriptHasChanged()
    {
        if (m_parameter != nullptr)
        {
            m_dirty = m_parameter->SetValueEntity(m_value) == false;
        }

        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }

    void HoudiniScriptPropertyEntity::Update()
    {
        if ( m_dirty && m_parameter != nullptr)
        {
            if (m_parameter->SetValueEntity(m_value))
            {
                m_dirty = false;
            }
        }
    }


    //////////////////////////
    ////  EntityId
    /////////////////////////
    HoudiniScriptPropertyInput::HoudiniScriptPropertyInput(HoudiniNodePtr parameter, int index) : HoudiniScriptProperty(parameter, index)
    {
        m_index = index;
        m_value = AZ::EntityId();
    }

    void HoudiniScriptPropertyInput::Reflect(AZ::ReflectContext* reflection)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);

        if (serializeContext)
        {
            serializeContext->Class<HoudiniScriptPropertyInput, HoudiniScriptProperty>()->
                Version(1)->
                Field("value", &HoudiniScriptPropertyInput::m_value);

            AZ::EditContext* ec = serializeContext->GetEditContext();
            ec->Class<HoudiniScriptPropertyInput>("Script Property (string)", "A script string property")->
                ClassElement(AZ::Edit::ClassElements::EditorData, "ScriptPropertyGroup's class attributes.")->
                Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)->
                DataElement(AZ::Edit::UIHandlers::EntityId, &HoudiniScriptPropertyInput::m_value, "m_value", "Entity ID")->
                Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniScriptProperty::ScriptHasChanged)->
                Attribute(AZ::Edit::Attributes::Suffix, " ")->
                Attribute(AZ::Edit::Attributes::NameLabelOverride, &HoudiniScriptProperty::m_label)->
                //Attribute(AZ::Edit::Attributes::GroupDynamic, &HoudiniScriptProperty::GetGroupName)-> //O3DECONVERT
                Attribute(AZ::Edit::Attributes::DescriptionTextOverride, &HoudiniScriptProperty::m_desc);
        }
    }

    AZ::u32 HoudiniScriptPropertyInput::ScriptHasChanged()
    {
        if (m_node != nullptr)
        {
            m_node->SetInputEntity(m_index, m_value);
        }

        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }


    //////////////////////////
    ////  File
    /////////////////////////
    HoudiniScriptPropertyFile::HoudiniScriptPropertyFile(HoudiniParameterPtr parameter) : HoudiniScriptProperty(parameter)
    {
        AZStd::string filePath = parameter->GetValueString();
        AZStd::string relPath;
        AZ::Data::AssetId assetId;

        AzToolsFramework::AssetSystemRequestBus::Broadcast(&AzToolsFramework::AssetSystemRequestBus::Events::GetRelativeProductPathFromFullSourceOrProductPath, filePath, relPath);
        AZ::Data::AssetCatalogRequestBus::BroadcastResult(assetId, &AZ::Data::AssetCatalogRequestBus::Events::GetAssetIdByPath, relPath.c_str(), AZ::Data::s_invalidAssetType, false);

        //ATOMCONVERT
        if (assetId.IsValid())
        {
            //m_value.Create(assetId);
        }
    }

    void HoudiniScriptPropertyFile::Reflect(AZ::ReflectContext* reflection)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);

        if (serializeContext)
        {
            serializeContext->Class<HoudiniScriptPropertyFile, HoudiniScriptProperty>()->
                Version(1);//->
                //Field("value", &HoudiniScriptPropertyFile::m_value); //ATOMCONVERT

            AZ::EditContext* ec = serializeContext->GetEditContext();
            ec->Class<HoudiniScriptPropertyFile>("Script Property (string)", "A script string property")->
                ClassElement(AZ::Edit::ClassElements::EditorData, "ScriptPropertyGroup's class attributes.")->
                Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)->
                //DataElement(AZ::Edit::UIHandlers::Default, &HoudiniScriptPropertyFile::m_value, "m_value", "Geometry")-> //ATOMCONVERT
                Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniScriptPropertyFile::ScriptHasChanged)->
                Attribute(AZ::Edit::Attributes::Suffix, " ")->
                Attribute(AZ::Edit::Attributes::DescriptionTextOverride, &HoudiniScriptPropertyFile::m_desc)->
                //Attribute(AZ::Edit::Attributes::GroupDynamic, &HoudiniScriptProperty::GetGroupName)-> //O3DECONVERT
                Attribute(AZ::Edit::Attributes::NameLabelOverride, &HoudiniScriptPropertyFile::m_name);
        }
    }

    AZ::u32 HoudiniScriptPropertyFile::ScriptHasChanged()
    {
        if (m_parameter != nullptr)
        {
            AZStd::string path;// = m_value.GetHint(); //O3DECONVERT
            AZStd::string fullPath;

            //Only check if we are expecting something:
            if (path.empty() == false)
            {                
                AzToolsFramework::AssetSystemRequestBus::Broadcast(&AzToolsFramework::AssetSystemRequestBus::Events::GetFullSourcePathFromRelativeProductPath, path, fullPath);
            }

            if (fullPath.empty() == false && AZ::IO::FileIOBase::GetInstance()->Exists(fullPath.c_str()))
            {
                m_parameter->SetValueString(fullPath);
            }
            else
            {                
                if (path.empty() == false)
                {
                    //Only warn if things went wrong:
                    AZStd::string nodeName = "Input Param";
                    if (m_node != nullptr)
                    {
                        m_node->GetNodeName();
                    }

                    AZStd::string warning = nodeName + ":" + m_name + ": '" + path + "' could not find full source!  Check this file exists.";
                    AZ_Warning("[HOUDINI]", false, warning.c_str());
                }

                m_parameter->SetValueString("");

                // O3DECONVERT
                //m_value = AZ::Data::Asset<LmbrCentral::MeshAsset>(nullptr);
                return AZ::Edit::PropertyRefreshLevels::AttributesAndValues;
            }
        }

        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }

    void HoudiniScriptPropertyFile::Update()
    {
    }


    //////////////////////////
    ////  File IMAGES
    /////////////////////////
    HoudiniScriptPropertyFileImage::HoudiniScriptPropertyFileImage(HoudiniParameterPtr parameter) : HoudiniScriptProperty(parameter)
    {
        AZStd::string filePath = parameter->GetValueString();
        AZStd::string relPath;
        AZ::Data::AssetId assetId;

        AzToolsFramework::AssetSystemRequestBus::Broadcast(&AzToolsFramework::AssetSystemRequestBus::Events::GetRelativeProductPathFromFullSourceOrProductPath, filePath, relPath);
        AZ::Data::AssetCatalogRequestBus::BroadcastResult(assetId, &AZ::Data::AssetCatalogRequestBus::Events::GetAssetIdByPath, relPath.c_str(), AZ::Data::s_invalidAssetType, false);

        if (assetId.IsValid())
        {
            //m_value.SetAssetPath(relPath.c_str()); //ATOMCONVERT
        }
    }

    void HoudiniScriptPropertyFileImage::Reflect(AZ::ReflectContext* reflection)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);

        if (serializeContext)
        {
            serializeContext->Class<HoudiniScriptPropertyFileImage, HoudiniScriptProperty>()->
                Version(1);//->
                //Field("value", &HoudiniScriptPropertyFileImage::m_value); //ATOMCONVERT

            AZ::EditContext* ec = serializeContext->GetEditContext();
            ec->Class<HoudiniScriptPropertyFileImage>("Script Property (string)", "A script string property")->
                ClassElement(AZ::Edit::ClassElements::EditorData, "ScriptPropertyGroup's class attributes.")->
                Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)->
                //DataElement(AZ::Edit::UIHandlers::Default, &HoudiniScriptPropertyFileImage::m_value, "m_value", "Geometry")-> //ATOMCONVERT
                Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniScriptPropertyFileImage::ScriptHasChanged)->
                Attribute(AZ::Edit::Attributes::Suffix, " ")->
                Attribute(AZ::Edit::Attributes::DescriptionTextOverride, &HoudiniScriptPropertyFileImage::m_desc)->
                //Attribute(AZ::Edit::Attributes::GroupDynamic, &HoudiniScriptProperty::GetGroupName)-> //O3DECONVERT
                Attribute(AZ::Edit::Attributes::NameLabelOverride, &HoudiniScriptPropertyFileImage::m_name);
        }
    }

    AZ::u32 HoudiniScriptPropertyFileImage::ScriptHasChanged()
    {
        if (m_parameter != nullptr)
        {
            AZStd::string path;// = m_value.GetAssetPath(); // ATOMCONVERT
            AZStd::string fullPath;

            if (path.empty() == false)
            {
                AzToolsFramework::AssetSystemRequestBus::Broadcast(&AzToolsFramework::AssetSystemRequestBus::Events::GetFullSourcePathFromRelativeProductPath, path, fullPath);
            }

            if (fullPath.empty() == false && AZ::IO::FileIOBase::GetInstance()->Exists(fullPath.c_str()))
            {
                m_parameter->SetValueString(fullPath);
            }
            else
            {
                if (path.empty() == false)
                {
                    AZ_Warning("[HOUDINI]", false, (path + " could not find full source!  Check this file exists.").c_str());
                }

                m_parameter->SetValueString("");
            }
        }

        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }

    void HoudiniScriptPropertyFileImage::Update()
    {
    }


    //////////////////////////
    ////  File ANYTHING - Doesn't work properly yet - defaults to textures.
    /////////////////////////
    HoudiniScriptPropertyFileAny::HoudiniScriptPropertyFileAny(HoudiniParameterPtr parameter) : HoudiniScriptProperty(parameter)
    {
        AZStd::string filePath = parameter->GetValueString();
        AZStd::string relPath;
        AZ::Data::AssetId assetId;

        if (!filePath.empty())
        {
			AzToolsFramework::AssetSystemRequestBus::Broadcast(&AzToolsFramework::AssetSystemRequestBus::Events::GetRelativeProductPathFromFullSourceOrProductPath, filePath, relPath);
			AZ::Data::AssetCatalogRequestBus::BroadcastResult(assetId, &AZ::Data::AssetCatalogRequestBus::Events::GetAssetIdByPath, relPath.c_str(), AZ::Data::s_invalidAssetType, false);

			if (assetId.IsValid())
			{
				//m_value.SetAssetPath(relPath.c_str()); //ATOMCONVERT
			}
        }
    }

    void HoudiniScriptPropertyFileAny::Reflect(AZ::ReflectContext* reflection)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);

        if (serializeContext)
        {
            AzFramework::SimpleAssetReference<AnyAsset>::Register(*serializeContext);

            serializeContext->Class<HoudiniScriptPropertyFileAny, HoudiniScriptProperty>()->
                Version(1);//->
                //Field("value", &HoudiniScriptPropertyFileAny::m_value); //ATOMCONVERT

            AZ::EditContext* ec = serializeContext->GetEditContext();
            ec->Class<HoudiniScriptPropertyFileAny>("Script Property (string)", "A script string property")->
                ClassElement(AZ::Edit::ClassElements::EditorData, "ScriptPropertyGroup's class attributes.")->
                Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)->
                //DataElement(AZ::Edit::UIHandlers::Default, &HoudiniScriptPropertyFileAny::m_value, "m_value", "Geometry")-> //ATOMCONVERT
                Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniScriptPropertyFileAny::ScriptHasChanged)->
                Attribute(AZ::Edit::Attributes::Suffix, " ")->
                Attribute(AZ::Edit::Attributes::DescriptionTextOverride, &HoudiniScriptPropertyFileAny::m_desc)->
                //Attribute(AZ::Edit::Attributes::GroupDynamic, &HoudiniScriptProperty::GetGroupName)-> //O3DECONVERT
                Attribute(AZ::Edit::Attributes::NameLabelOverride, &HoudiniScriptPropertyFileAny::m_name);
        }
    }

    AZ::u32 HoudiniScriptPropertyFileAny::ScriptHasChanged()
    {
        if (m_parameter != nullptr)
        {
            AZStd::string path;// = m_value.GetAssetPath(); //ATOMCONVERT
            AZStd::string fullPath;
            
            if (path.empty() == false)
            {
                AzToolsFramework::AssetSystemRequestBus::Broadcast(&AzToolsFramework::AssetSystemRequestBus::Events::GetFullSourcePathFromRelativeProductPath, path, fullPath);
            }

            if (fullPath.empty() == false && AZ::IO::FileIOBase::GetInstance()->Exists(fullPath.c_str()))
            {
                m_parameter->SetValueString(fullPath);
            }
            else
            {
                if (path.empty() == false)
                {
                    AZ_Warning("[HOUDINI]", false, (path + " could not find full source!  Check this file exists.").c_str());
                }

                m_parameter->SetValueString("");
            }
        }

        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }

    void HoudiniScriptPropertyFileAny::Update()
    {
    }

    HoudiniEngine::HoudiniScriptProperty* HoudiniScriptPropertyFactory::CreateNew(HoudiniParameterPtr param)
    {
        HoudiniScriptProperty* prop = nullptr;

        //Make a new one!
        if (param->GetType() == HAPI_PARMTYPE_INT)
        {
            if (param->GetInfo().size == 1)
            {
                if (param->GetInfo().choiceCount > 0)
                {
                    prop = aznew HoudiniScriptPropertyIntChoice(param);
                }
                else
                {
                    prop = aznew HoudiniScriptPropertyInt(param);
                }
            }
        }
        else if (param->GetType() == HAPI_PARMTYPE_STRING)
        {
            if (param->GetInfo().size == 1)
            {
                prop = aznew HoudiniScriptPropertyString(param);
            }
        }
        else if (param->GetType() == HAPI_PARMTYPE_FLOAT)
        {
            if (param->GetInfo().size == 1)
            {
                prop = aznew HoudiniScriptPropertyFloat(param);
            }
            else if (param->GetInfo().size == 2)
            {
                prop = aznew HoudiniScriptPropertyVector2(param);
            }
            else if (param->GetInfo().size == 3)
            {
                prop = aznew HoudiniScriptPropertyVector3(param);
            }
            else if (param->GetInfo().size == 4)
            {
                prop = aznew HoudiniScriptPropertyVector4(param);
            }
        }
        else if (param->GetType() == HAPI_PARMTYPE_TOGGLE)
        {
            prop = aznew HoudiniScriptPropertyBoolean(param);
        }
        else if (param->GetType() == HAPI_PARMTYPE_NODE)
        {
            prop = aznew HoudiniScriptPropertyEntity(param);
        }
        else if (param->GetType() == HAPI_PARMTYPE_PATH_FILE)
        {
            prop = aznew HoudiniScriptPropertyFileAny(param);
        }
        else if (param->GetType() == HAPI_PARMTYPE_PATH_FILE_GEO)
        {
            prop = aznew HoudiniScriptPropertyFile(param);
        }
        else if (param->GetType() == HAPI_PARMTYPE_PATH_FILE_IMAGE)
        {
            prop = aznew HoudiniScriptPropertyFileImage(param);
        }
        // FL[FD-11758] Add support for color parameter to lumberyard houdini engine
        else if (param->GetType() == HAPI_PARMTYPE_COLOR)
        {
            prop = aznew HoudiniScriptPropertyColor(param);
        }
        // FL[FD-11761] Add support for multiparm blocks (lists) to lumberyard houdini engine
        else if (param->GetType() == HAPI_PARMTYPE_MULTIPARMLIST)
        {
            prop = aznew HoudiniScriptPropertyMultiparm(param);
        }
        // FL[FD-12463] Implement button interface parm in houdini engine for lumberyard
        else if (param->GetType() == HAPI_PARMTYPE_BUTTON)
        {
            prop = aznew HoudiniScriptPropertyButton(param);
        }

        return prop;
    }

    // FL[FD-11758] Add support for color parameter to lumberyard houdini engine
    //////////////////////////
    ////  Color
    /////////////////////////
    HoudiniScriptPropertyColor::HoudiniScriptPropertyColor(HoudiniParameterPtr parameter) : HoudiniScriptProperty(parameter)
    {
        if (parameter->GetInfo().size >= 1 && parameter->GetInfo().size <= 4)
        {
            m_value = parameter->GetValueColor();
        }
    }

    void HoudiniScriptPropertyColor::Reflect(AZ::ReflectContext* reflection)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);

        if (serializeContext)
        {
            serializeContext->Class<HoudiniScriptPropertyColor, HoudiniScriptProperty>()->
                Version(0)->
                Field("value", &HoudiniScriptPropertyColor::m_value);

            AZ::EditContext* ec = serializeContext->GetEditContext();
            ec->Class<HoudiniScriptPropertyColor>("Script Property (color)", "A color property")->
                ClassElement(AZ::Edit::ClassElements::EditorData, "ScriptPropertyGroup's class attributes.")->
                Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)->
                DataElement(AZ::Edit::UIHandlers::Color, &HoudiniScriptPropertyColor::m_value, "m_value", "Vector4 (r,g,b,a)")->
                Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniScriptPropertyColor::ScriptHasChanged)->
                Attribute(AZ::Edit::Attributes::Suffix, " ")->
                Attribute(AZ::Edit::Attributes::DescriptionTextOverride, &HoudiniScriptPropertyColor::m_desc)->
                //Attribute(AZ::Edit::Attributes::GroupDynamic, &HoudiniScriptProperty::GetGroupName)-> //O3DECONVERT
                Attribute(AZ::Edit::Attributes::NameLabelOverride, &HoudiniScriptPropertyColor::m_label);
        }
    }

    AZ::u32 HoudiniScriptPropertyColor::ScriptHasChanged()
    {
        if (m_parameter != nullptr)
        {
            m_parameter->SetValueColor(m_value);
        }

        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }

    // FL[FD-11761] Add support for multiparm blocks (lists) to lumberyard houdini engine
    //////////////////////////
    ////  Multiparam instance
    //////////////////////////
    HoudiniMultiparamInstance::HoudiniMultiparamInstance(HoudiniParameterPtr parameter) : HoudiniScriptProperty(parameter)
    {
    }

    void HoudiniMultiparamInstance::Reflect(AZ::ReflectContext* reflection)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);

        if (serializeContext)
        {
            serializeContext->Class<HoudiniMultiparamInstance, HoudiniScriptProperty>()->
                Field("instanceNum", &HoudiniMultiparamInstance::m_instanceNum)->
                Field("params", &HoudiniMultiparamInstance::m_params)->
                Version(0);

            if (AZ::EditContext* ec = serializeContext->GetEditContext())
            {
                ec->Class<HoudiniMultiparamInstance>("Multiparam Instance", "Multiparam Instance")->
                    ClassElement(AZ::Edit::ClassElements::EditorData, "Attributes")->
                    Attribute(AZ::Edit::Attributes::NameLabelOverride, &HoudiniMultiparamInstance::GetLabelOverride)->
                DataElement(0, &HoudiniMultiparamInstance::m_params, "", "")->
                    Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)->
                    Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniMultiparamInstance::ScriptHasChanged)->
                    Attribute(AZ::Edit::Attributes::Suffix, " ")->
                    Attribute("ContainerCanBeModified", false);
            }
        }
    }

    AZStd::string HoudiniMultiparamInstance::GetLabelOverride()
    {
        return AZStd::to_string(m_instanceNum);
    }

    AZ::u32 HoudiniMultiparamInstance::ScriptHasChanged()
    {
        return AZ::Edit::PropertyRefreshLevels::AttributesAndValues;
    }

    void HoudiniMultiparamInstance::SetParentParam(HoudiniParameterPtr param)
    {
        m_parameter = param;
    }

    void HoudiniMultiparamInstance::SetNode(HoudiniNodePtr node)
    {
        for (auto param : m_params)
        {
            param->SetNode(node);
        }
    }

    //////////////////////////
    ////  Multiparm
    /////////////////////////
    HoudiniScriptPropertyMultiparm::HoudiniScriptPropertyMultiparm(HoudiniParameterPtr parameter) : HoudiniScriptProperty(parameter)
    {
        m_instances.clear();
        if (m_parameter != nullptr)
        {
            m_value = m_parameter->GetValueMultiparm();
            m_instances.resize(static_cast<size_t>(m_value));

            for (int i = 0; i < m_instances.size(); ++i)
            {
                m_instances[i] = aznew HoudiniMultiparamInstance(m_parameter);

                AZStd::vector<HoudiniParameterPtr> instanceChildParams = m_parameter->GetChildren(i);

                for (auto param : instanceChildParams)
                {
                    HoudiniScriptProperty* prop = HoudiniScriptPropertyFactory::CreateNew(param);
                    m_instances[i]->m_params.push_back(prop);
                }
            }
        }
    }

    void HoudiniScriptPropertyMultiparm::Reflect(AZ::ReflectContext* reflection)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);

        if (serializeContext)
        {
            serializeContext->Class<HoudiniScriptPropertyMultiparm, HoudiniScriptProperty>()->
                Field("value", &HoudiniScriptPropertyMultiparm::m_value)->
                Field("instances", &HoudiniScriptPropertyMultiparm::m_instances)->
                Version(0);

            AZ::EditContext* ec = serializeContext->GetEditContext();
            ec->Class<HoudiniScriptPropertyMultiparm>("Script Property (multiparm list)", "A multiparm list property")->
                ClassElement(AZ::Edit::ClassElements::EditorData, "ScriptPropertyGroup's class attributes.")->
                    Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)->
                DataElement(AZ::Edit::UIHandlers::SpinBox, &HoudiniScriptPropertyMultiparm::m_value, "Multiparam", "Multiparam description")->
                    Attribute(AZ::Edit::Attributes::NameLabelOverride, &HoudiniScriptPropertyMultiparm::m_label)->
                    Attribute(AZ::Edit::Attributes::DescriptionTextOverride, &HoudiniScriptPropertyMultiparm::m_desc)->
                    Attribute(AZ::Edit::Attributes::Step, 1u)->
                    Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniScriptPropertyMultiparm::InstanceNumHasChanged)->
                DataElement(AZ::Edit::UIHandlers::Default, &HoudiniScriptPropertyMultiparm::m_instances, "", "")->
                    Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)->
                    Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniScriptPropertyMultiparm::ScriptHasChanged)->
                    Attribute(AZ::Edit::Attributes::Suffix, " ")->
                    //Attribute(AZ::Edit::Attributes::GroupDynamic, &HoudiniScriptProperty::GetGroupName)-> //O3DECONVERT
                    Attribute(AZ::Edit::Attributes::IndexedChildNameLabelOverride, &HoudiniScriptPropertyMultiparm::GetInstanceSlotLabel)->
                    Attribute("ContainerCanBeModified", false);
        }
    }

    AZStd::string HoudiniScriptPropertyMultiparm::GetInstanceSlotLabel(int index)
    {
        return AZStd::to_string(index);
    }

    AZ::u32 HoudiniScriptPropertyMultiparm::ScriptHasChanged()
    {
        return AZ::Edit::PropertyRefreshLevels::AttributesAndValues;
    }

    AZ::u32 HoudiniScriptPropertyMultiparm::InstanceNumHasChanged()
    {
        if (m_parameter != nullptr)
        {
            AzToolsFramework::ScopedUndoBatch undoBatch("Houdini Digital Asset Multiparm Instance Number Change");

            AZ::u32 oldSize = aznumeric_cast<AZ::u32>(m_instances.size());

            // Update parameters themselves
            if (oldSize < m_value)
            {
                for (AZ::u32 i = oldSize; i < m_value; ++i)
                {
                    m_parameter->InsertInstance(i);
                }
            }

            if (oldSize > m_value)
            {
                for (AZ::u32 i = oldSize; i > m_value; --i)
                {
                    m_parameter->RemoveInstance(i - 1);
                }
            }

            m_parameter->GetNode()->UpdateParamInfoFromEngine();

            // Updat User Interface according to parameters change
            if (oldSize < m_value)
            {
                m_instances.resize(m_value);

                for (AZ::u32 i = oldSize; i < m_value; ++i)
                {
                    m_instances[i] = aznew HoudiniMultiparamInstance(m_parameter);
                    m_instances[i]->m_instanceNum = i;

                    AZStd::vector<HoudiniParameterPtr> instanceChildParams = m_parameter->GetChildren(i);

                    for (auto param : instanceChildParams)
                    {
                        HoudiniScriptProperty* prop = HoudiniScriptPropertyFactory::CreateNew(param);
                        m_instances[i]->m_params.push_back(prop);
                    }
                }

                undoBatch.MarkEntityDirty(this->m_parameter->GetNode()->GetEntityId());
            }
            else if (oldSize > m_value)
            {
                for (AZ::u32 i = oldSize; i > m_value; --i)
                {
                    delete m_instances[i-1];
                }
                m_instances.resize(m_value);

                undoBatch.MarkEntityDirty(this->m_parameter->GetNode()->GetEntityId());
            }
            else
            {
                return AZ::Edit::PropertyRefreshLevels::None;
            }

            return AZ::Edit::PropertyRefreshLevels::EntireTree;
        }

        return AZ::Edit::PropertyRefreshLevels::None;
    }

    void HoudiniScriptPropertyMultiparm::SetNode(HoudiniNodePtr node)
    {
        m_hou = node->GetHou();
        m_node = node;

        for (auto instance : m_instances)
        {
            instance->SetNode(node);
        }
    }

    // FL[FD-12463] Implement button interface parm in houdini engine for lumberyard
    HoudiniScriptPropertyButton::HoudiniScriptPropertyButton(HoudiniParameterPtr parameter) : HoudiniScriptProperty(parameter)
    {
    }

    void HoudiniScriptPropertyButton::Reflect(AZ::ReflectContext* reflection)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);

        if (serializeContext)
        {
            serializeContext->Class<HoudiniScriptPropertyButton, HoudiniScriptProperty>()->
                Field("Callback", &HoudiniScriptPropertyButton::m_callbackButton)->
                Version(0);

            if (AZ::EditContext* ec = serializeContext->GetEditContext())
            {
                ec->Class<HoudiniScriptPropertyButton>("Script Property (Button)", "Button script property")->
                    ClassElement(AZ::Edit::ClassElements::EditorData, "Attributes")->
                    Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)->
                    DataElement(AZ::Edit::UIHandlers::Button, &HoudiniScriptPropertyButton::m_callbackButton, "", "")->
                    Attribute(AZ::Edit::Attributes::ButtonText, &HoudiniScriptPropertyButton::m_label)->
                    Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniScriptPropertyButton::ScriptHasChanged);
            }
        }
    }

    AZ::u32 HoudiniScriptPropertyButton::ScriptHasChanged()
    {
        if (m_parameter != nullptr)
        {
            if (m_parameter->GetValueInt() != 0)
            {
                m_parameter->SetValueInt(0);
            }
            else
            {
                m_parameter->SetValueInt(1);
            }
        }

        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }
}
