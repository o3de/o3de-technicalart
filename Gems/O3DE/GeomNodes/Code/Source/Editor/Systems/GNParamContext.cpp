/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "GNParamContext.h"

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <Editor/Systems/GNProperty.h>

namespace GeomNodes
{
    // templates
    bool GNValue<bool>::Read(const rapidjson::Value& val)
    {
        return val.GetInt();
    }

    int GNValue<int>::Read(const rapidjson::Value& val)
    {
        return val.GetInt();
    }

    double GNValue<double>::Read(const rapidjson::Value& val)
    {
        return val.GetDouble();
    }

    const char* GNValue<const char*>::Read(const rapidjson::Value& val)
    {
        return val.GetString();
    }

    // GNParamContext
    GNParamContext::GNParamContext()
    {
        m_impl = aznew GNParamContextImpl();
        m_group.m_name = "Geometry Nodes Parameters";
    }

    GNParamContext::~GNParamContext()
    {
        m_group.Clear();
        delete m_impl;
    }

    void GNParamContext::Reflect(AZ::ReflectContext* reflection)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);
        if (serializeContext)
        {
            // we may have been reflected by EditorGeomNodesComponent already, so check first
            if (serializeContext->FindClassData(AZ::Uuid("{AA9713B7-70F1-43CB-9F95-5BEC9F44F556}")) == nullptr)
            {
                serializeContext->Class<GNParamContext>()->Version(2)->Field("Properties", &GNParamContext::m_group);

                serializeContext->Class<GNPropertyGroup>()
                    ->Field("Name", &GNPropertyGroup::m_name)
                    ->Field("Properties", &GNPropertyGroup::m_properties)
                    ->Field("Groups", &GNPropertyGroup::m_groups);

                // reflect all properties
                GNProperties::Reflect(reflection);
            }
        }
    }

    GNProperty* GNParamContext::ConstructGNParam(GNParamDataContext& gndc, ParamType pType, const char* name)
    {
        return m_impl->ConstructGNParam(gndc, pType, name);
    }

    //=========================================================================
    // GetGroup
    //=========================================================================
    GNPropertyGroup* GNPropertyGroup::GetGroup(const char* groupName)
    {
        for (GNPropertyGroup& subGroup : m_groups)
        {
            if (subGroup.m_name == groupName)
            {
                return &subGroup;
            }
        }
        return nullptr;
    }

    //=========================================================================
    // GetProperty
    //=========================================================================
    GNProperty* GNPropertyGroup::GetProperty(const char* propertyName)
    {
        for (GNProperty* prop : m_properties)
        {
            if (prop->m_name == propertyName)
            {
                return prop;
            }
        }
        return nullptr;
    }

    AZStd::string GNPropertyGroup::GetProperties()
    {
        AZStd::string jsonString = "";

        AZ::u64 uCtr = m_properties.size();
        for (GNProperty* prop : m_properties)
        {
            jsonString += prop->ToJSONString();
            uCtr--;
            if (uCtr > 0)
                jsonString += ", ";
        }

        return jsonString;
    }

    //=========================================================================
    // Clear
    //=========================================================================
    void GNPropertyGroup::Clear()
    {
        for (GNProperty* prop : m_properties)
        {
            delete prop;
        }
        m_properties.clear();
        m_groups.clear();
    }

    //=========================================================================
    // ~GNPropertyGroup
    //=========================================================================
    GNPropertyGroup::~GNPropertyGroup()
    {
        Clear();
    }

    //=========================================================================
    // operator=
    //=========================================================================
    GNPropertyGroup& GNPropertyGroup::operator=(GNPropertyGroup&& rhs)
    {
        m_name.swap(rhs.m_name);
        m_properties.swap(rhs.m_properties);
        m_groups.swap(rhs.m_groups);

        return *this;
    }

    const char* GetEnumString(ParamType value)
    {
        switch (value)
        {
        case ParamType::Bool:
            return "BOOLEAN";
        case ParamType::Int:
            return "INT";
        case ParamType::Value:
            return "VALUE";
        case ParamType::String:
            return "STRING";
        }

        return "UNKNOWN";
    }

    ParamType GetTypeFromString(const char* value)
    {
        if (strcmp("BOOLEAN", value) == 0)
        {
            return ParamType::Bool;
        }
        else if (strcmp("INT", value) == 0)
        {
            return ParamType::Int;
        }
        else if (strcmp("VALUE", value) == 0)
        {
            return ParamType::Value;
        }
        else if (strcmp("STRING", value) == 0)
        {
            return ParamType::String;
        }

        return ParamType::Unknown;
    }

    // GNParamDataContext

    bool GNParamDataContext::IsNil(int index) const
    {
        return index == (int)ParamType::Unknown;
    }

    bool GNParamDataContext::IsBoolean(int index) const
    {
        return index == (int)ParamType::Bool;
    }

    bool GNParamDataContext::IsInt(int index) const
    {
        return index == (int)ParamType::Int;
    }

    bool GNParamDataContext::IsValue(int index) const
    {
        return index == (int)ParamType::Value;
    }

    bool GNParamDataContext::IsString(int index) const
    {
        return index == (int)ParamType::String;
    }

    const char* GNParamDataContext::GetParamName()
    {
        if (m_curParamObj == nullptr)
            return nullptr;

        return (*m_curParamObj)[Field::Name].GetString();
    }

    ParamType GNParamDataContext::GetParamType()
    {
        if (m_curParamObj == nullptr)
            return ParamType::Unknown;

        return GetTypeFromString((*m_curParamObj)[Field::Type].GetString());
    }

    // GNParamContextImpl
    GNParamContextImpl::GNParamContextImpl()
        : m_paramFactories({
              // Nil
              &GNParamNil::TryCreateProperty,

              // Values
              &GNParamBoolean::TryCreateProperty,
              &GNParamInt::TryCreateProperty,
              &GNParamValue::TryCreateProperty,
              &GNParamString::TryCreateProperty,
          })
    {
    }

    GNProperty* GNParamContextImpl::ConstructGNParam(GNParamDataContext& gndc, ParamType pType, const char* name)
    {
        GNProperty* gnParam = nullptr;

        for (ParamTypeFactory factory : m_paramFactories)
        {
            gnParam = factory(gndc, (int)pType, name);

            if (gnParam != nullptr)
            {
                break;
            }
        }

        return gnParam;
    }
} // namespace GeomNodes
