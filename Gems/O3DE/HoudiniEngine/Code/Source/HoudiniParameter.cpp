#include "StdAfx.h"

#include <HoudiniCommon.h>

namespace HoudiniEngine
{
    HoudiniParameter::HoudiniParameter(IHoudini* hou, HAPI_ParmInfo info, IHoudiniNode* node):
        m_hou(hou)
        , m_id(info.id)
        , m_node(node)
        , m_info(info)
    {
        m_session = const_cast<HAPI_Session*>(&hou->GetSession());

        if (m_id >= 0)
        {
            m_name = hou->GetString( info.nameSH);
            m_label = hou->GetString(info.labelSH);
            m_typeInfo = hou->GetString(info.typeInfoSH);
            m_help = hou->GetString(info.helpSH);
            m_type = info.type;
            m_size = info.size;
            m_choiceCount = info.choiceCount;
            m_parentId = info.parentId; // FL[FD-11761] Add support for multiparm blocks (lists) to lumberyard houdini engine
        }
    }

    bool HoudiniParameter::GetValueBoolean()
    {
        int values[1];
        HAPI_GetParmIntValue(m_session, GetNode()->GetId(), GetName().c_str(), 0, values);
        m_valueBool = values[0] == 1 ? true : false;
        return m_valueBool;
    }

    const int HoudiniParameter::GetValueInt()
    {
        HAPI_GetParmIntValue(m_session, GetNode()->GetId(), GetName().c_str(), 0, &m_valueInt);
        return m_valueInt;
    }

    AZStd::vector<int> HoudiniParameter::GetValuesInt(int count)
    {
        AZStd::vector<int> values(count);
        HAPI_GetParmIntValues(m_session, GetNode()->GetId(), &values.front(), GetInfo().intValuesIndex, count);
        return values;
    }

    const float HoudiniParameter::GetValueFloat()
    {
        HAPI_GetParmFloatValue(m_session, GetNode()->GetId(), GetName().c_str(), 0, &m_valueFloat);
        return m_valueFloat;
    }

    const AZ::Vector2& HoudiniParameter::GetValueVec2()
    {
        float values[2];
        HAPI_GetParmFloatValues(m_session, GetNode()->GetId(), values, GetInfo().floatValuesIndex, 2);
        m_valueVec2.Set(values[0], values[1]); 
        
        return m_valueVec2;
    }

    const AZ::Vector3& HoudiniParameter::GetValueVec3()
    {
        float values[3];
        HAPI_GetParmFloatValues(m_session, GetNode()->GetId(), values, GetInfo().floatValuesIndex, 3);
        m_valueVec3.Set(values);
        
        return m_valueVec3;
    }

    const AZ::Vector4& HoudiniParameter::GetValueVec4()
    {
        float values[4];
        HAPI_GetParmFloatValues(m_session, GetNode()->GetId(), values, GetInfo().floatValuesIndex, 4);
        m_valueVec4.Set(values);
        return m_valueVec4;
    }

    // FL[FD-11758] Add support for color parameter to lumberyard houdini engine
    const AZ::Color& HoudiniParameter::GetValueColor()
    {
        float values[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        HAPI_GetParmFloatValues(m_session, GetNode()->GetId(), values, GetInfo().floatValuesIndex, GetInfo().size);
        m_valueColor.Set(values);
        return m_valueColor;
    }

    AZStd::vector<AZStd::string> HoudiniParameter::GetValueChoices()
    {
        int count = GetInfo().choiceCount;
        HAPI_ParmChoiceInfo* ChoiceInfos = new HAPI_ParmChoiceInfo[count];
        AZStd::vector<AZStd::string> choiceLabels(count);
        if (HAPI_GetParmChoiceLists(m_session, GetNode()->GetId(), ChoiceInfos, GetInfo().choiceIndex, count) == HAPI_RESULT_SUCCESS)
        {
			for (int i = 0; i < count; i++)
			{
                choiceLabels[i] = m_hou->GetString(ChoiceInfos[i].labelSH);
			}
        }

        delete[] ChoiceInfos;
        return choiceLabels;
    }

    const AZStd::string& HoudiniParameter::GetValueString()
    {        
        HAPI_StringHandle handle;
        HAPI_GetParmStringValue(m_session, GetNode()->GetId(), GetName().c_str(), 0, true, &handle);
        m_valueString = m_hou->GetString(handle);

        return m_valueString;
    }

    const AZ::EntityId& HoudiniParameter::GetValueEntity()
    {
        return m_valueEntity;
    }

    void HoudiniParameter::SetValueBool(bool value)
    {
        int values[1];
        HAPI_GetParmIntValues(m_session, GetNode()->GetId(), values, GetInfo().intValuesIndex, 1);

        if ((values[0] == 1) != value)
        {
            m_valueBool = value;
            values[0] = m_valueBool;
            HAPI_SetParmIntValues(m_session, GetNode()->GetId(), values, GetInfo().intValuesIndex, 1);
        }
    }

    void HoudiniParameter::SetValueInt(int value)
    {
        int previousValue = 0;
        HAPI_GetParmIntValue(m_session, GetNode()->GetId(), GetName().c_str(), 0, &previousValue);

        if (previousValue != value)
        {
            m_valueInt = value;
            HAPI_SetParmIntValue(m_session, GetNode()->GetId(), GetName().c_str(), 0, value);
        }
    }

    void HoudiniParameter::SetValueFloat(float value)
    {
        float previousValue = 0;
        HAPI_GetParmFloatValue(m_session, GetNode()->GetId(), GetName().c_str(), 0, &previousValue);

        if (previousValue != value)
        {
            m_valueFloat = value;
            HAPI_SetParmFloatValue(m_session, GetNode()->GetId(), GetName().c_str(), 0, value);
        }
    }

    void HoudiniParameter::SetValueVec2(const AZ::Vector2& value)
    {        
        float values[2];
        HAPI_GetParmFloatValues(m_session, GetNode()->GetId(), values, GetInfo().floatValuesIndex, 2);

        if (values[0] != value.GetX() || values[1] != value.GetY())
        {
            m_valueVec2 = value;
            values[0] = m_valueVec2.GetX();
            values[1] = m_valueVec2.GetY();
            HAPI_SetParmFloatValues(m_session, GetNode()->GetId(), values, GetInfo().floatValuesIndex, 2);
        }
    }

    void HoudiniParameter::SetValueVec3(const AZ::Vector3& value)
    {
        float values[3];
        HAPI_GetParmFloatValues(m_session, GetNode()->GetId(), values, GetInfo().floatValuesIndex, 3);

        if (values[0] != value.GetX() || values[1] != value.GetY() || values[2] != value.GetZ())
        {
            m_valueVec3 = value;
            values[0] = m_valueVec3.GetX();
            values[1] = m_valueVec3.GetY();
            values[2] = m_valueVec3.GetZ();
            HAPI_SetParmFloatValues(m_session, GetNode()->GetId(), values, GetInfo().floatValuesIndex, 3);
        }
    }

    void HoudiniParameter::SetValueVec4(const AZ::Vector4& value)
    {
        float values[4];
        HAPI_GetParmFloatValues(m_session, GetNode()->GetId(), values, GetInfo().floatValuesIndex, 4);

        if (values[0] != value.GetX() || values[1] != value.GetY() || values[2] != value.GetZ() || values[3] != value.GetW())
        {
            m_valueVec4 = value;
            values[0] = m_valueVec4.GetX();
            values[1] = m_valueVec4.GetY();
            values[2] = m_valueVec4.GetZ();
            values[3] = m_valueVec4.GetW();
            HAPI_SetParmFloatValues(m_session, GetNode()->GetId(), values, GetInfo().floatValuesIndex, 4);
        }
    }

    void HoudiniParameter::SetValueString(const AZStd::string& value)
    {
        HAPI_ParmId startNodeParamId;
        HAPI_GetParmIdFromName(m_session, GetNode()->GetId(), m_name.c_str(), &startNodeParamId);
        m_valueString = value;

        if (startNodeParamId != HOUDINI_INVALID_ID)
        {
            HAPI_StringHandle handle;
            HAPI_GetParmStringValue(m_session, GetNode()->GetId(), GetName().c_str(), 0, true, &handle);
            AZStd::string previousStringData = m_hou->GetString(handle);

            if (value != previousStringData)
            {
                HAPI_SetParmStringValue(m_session, GetNode()->GetId(), value.c_str(), startNodeParamId, 0);
            }
        }
    }

    bool HoudiniParameter::SetValueEntity(const AZ::EntityId& value)
    {
        bool output = true;

        m_valueEntity = value;

        if (value.IsValid() == false) 
        {
            HAPI_SetParmNodeValue(m_session, GetNode()->GetId(), GetName().c_str(), HOUDINI_INVALID_ID);
            return true;
        }

        HAPI_NodeId nodeId = m_hou->GetInputNodeManager()->GetNodeIdFromEntity(value);
        if (nodeId == HOUDINI_NOT_READY_ID)
        {
            //In this case, the node is valid, but its just not ready yet.  Return false to signal to try again later.
            nodeId = HOUDINI_INVALID_ID;
            output = false;
        }

        HAPI_NodeId previousId = -1;
        HAPI_GetParmNodeValue(m_session, GetNode()->GetId(), GetName().c_str(), &previousId);

        if (previousId != nodeId)
        {
            HAPI_SetParmNodeValue(m_session, GetNode()->GetId(), GetName().c_str(), nodeId);
            if (m_hou->CheckForErrors())
            {
                AZStd::string entityName = m_hou->LookupEntityName(m_node->GetEntityId());

                AZ_Warning("HOUDINI", false, "[Entity: %s][Node: %s][Prop: %s] - Error setting entity to %s"
                    , (entityName + " " + m_node->GetEntityId().ToString()).c_str()
                    , GetNode()->GetNodeName().c_str()
                    , GetName().c_str()
                    , value.ToString().c_str()
                );

                HAPI_SetParmNodeValue(m_session, GetNode()->GetId(), GetName().c_str(), HOUDINI_INVALID_ID);
                return true;
            }
        }

        return output;
    }    

    // FL[FD-11758] Add support for color parameter to lumberyard houdini engine
    void HoudiniParameter::SetValueColor(const AZ::Color& value)
    {
        float values[4] = { -1.0f, 0.0f, 0.0f, 0.0f };
        HAPI_GetParmFloatValues(m_session, GetNode()->GetId(), values, GetInfo().floatValuesIndex, GetInfo().size);

        if (AZ::Color(values[0], values[1], values[2], values[3]) != value)
        {
            m_valueColor = value;
            values[0] = m_valueColor.GetR();
            values[1] = m_valueColor.GetG();
            values[2] = m_valueColor.GetB();
            values[3] = m_valueColor.GetA();
            HAPI_SetParmFloatValues(m_session, GetNode()->GetId(), values, GetInfo().floatValuesIndex, 4);
        }
    }

    // FL[FD-11761] Add support for multiparm blocks (lists) to lumberyard houdini engine
    void HoudiniParameter::SetParmInfo(HAPI_ParmInfo info)
    {
        m_info = info;
    }

    void HoudiniParameter::SetValueMultiparm(int value)
    {
        int previousValue = 0;
        HAPI_GetParmIntValue(m_session, GetNode()->GetId(), GetName().c_str(), 0, &previousValue);

        if (previousValue != value)
        {
            m_multiParamValue = value;
        }
    }

    const int HoudiniParameter::GetValueMultiparm()
    {
        HAPI_GetParmIntValue(m_session, GetNode()->GetId(), GetName().c_str(), 0, &m_multiParamValue);
        return m_multiParamValue;
    }

    void HoudiniParameter::InsertInstance(const int position)
    {
        int finalPosition = GetInfo().instanceStartOffset + position;

        HAPI_InsertMultiparmInstance(m_session, GetNode()->GetId(), GetId(), finalPosition);
        GetNode()->UpdateParamInfoFromEngine();
    }

    void HoudiniParameter::RemoveInstance(const int position)
    {
        const int finalPosition = GetInfo().instanceStartOffset + position;
        AZ_Assert(finalPosition < GetInfo().instanceCount + GetInfo().instanceStartOffset, "Wrong position to remove child parameter to MULTIPARMLIST");
        
        HAPI_RemoveMultiparmInstance(m_session, GetNode()->GetId(), GetId(), finalPosition);
        GetNode()->UpdateParamInfoFromEngine();
    }

    HoudiniParameterPtr HoudiniParameter::GetParent()
    {
        for (auto param : GetNode()->GetParameters())
        {
            if (param->GetInfo().id == GetInfo().parentId && param->GetInfo().id != HOUDINI_INVALID_ID)
            {
                return param;
            }
        }
        return nullptr;
    }

    AZStd::vector<HoudiniParameterPtr> HoudiniParameter::GetChildren(int instanceNum)
    {
        AZStd::vector<HoudiniParameterPtr> result;
        for (auto param : GetNode()->GetParameters())
        {
            if (param->GetInfo().parentId == GetInfo().id)
            {
                if (instanceNum == -1 || (instanceNum >= 0 && instanceNum + GetInfo().instanceStartOffset == param->GetInfo().instanceNum))
                {
                    result.push_back(param);
                }
            }
        }

        return result;
    }
}