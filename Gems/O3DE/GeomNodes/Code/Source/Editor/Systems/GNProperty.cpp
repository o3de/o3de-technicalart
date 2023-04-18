/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "GNProperty.h"

namespace GeomNodes
{
    void GNProperties::Reflect(AZ::ReflectContext* reflection)
    {
        GNProperty::Reflect(reflection);

        GNParamBoolean::Reflect(reflection);
        GNParamInt::Reflect(reflection);
        GNParamValue::Reflect(reflection);
        GNParamString::Reflect(reflection);
    }

    void GNProperty::Reflect(AZ::ReflectContext* reflection)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);

        if (serializeContext)
        {
            serializeContext->Class<GNProperty>()
                ->Version(2, GNProperty::VersionConverter)
                ->PersistentId(
                    [](const void* instance) -> AZ::u64
                    {
                        return reinterpret_cast<const GNProperty*>(instance)->m_id;
                    })
                ->Field("id", &GNProperty::m_id)
                ->Field("name", &GNProperty::m_name)
                ->Field("gnId", &GNProperty::m_gnId)
                ;
        }
    }

    bool GNProperty::VersionConverter(AZ::SerializeContext& context, AZ::SerializeContext::DataElementNode& classElement)
    {
        if (classElement.GetVersion() == 1)
        {
            // Generate persistent Id field.
            for (int i = 0; i < classElement.GetNumSubElements(); ++i)
            {
                AZ::SerializeContext::DataElementNode& elementNode = classElement.GetSubElement(i);
                if (elementNode.GetName() == AZ_CRC("name", 0x5e237e06))
                {
                    AZStd::string name;
                    if (elementNode.GetData(name))
                    {
                        const int idx = classElement.AddElement<AZ::u64>(context, "id");
                        const AZ::u32 crc = AZ::Crc32(name.c_str());
                        classElement.GetSubElement(idx).SetData<AZ::u64>(context, crc);
                    }
                }
            }
        }

        return true;
    }

    void GNProperty::ReadSetGNId(GNParamDataContext& context)
    {
        const char* gnId;
        if (context.ReadValue(gnId, Field::Id))
        {
            m_gnId = gnId;
        }
    }

    //////////////////////
    // GNParamNil
    //////////////////////

    void GNParamNil::Reflect(AZ::ReflectContext* reflection)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);

        if (serializeContext)
        {
            serializeContext->Class<GNParamNil, GNProperty>()->Version(1)->SerializeWithNoData();
        }
    }

    GNProperty* GNParamNil::TryCreateProperty(GNParamDataContext& context, int valueIndex, const char* name)
    {
        GNProperty* retVal = nullptr;
        if (context.IsNil(valueIndex))
        {
            retVal = aznew GNParamNil(name, context.GetReadOnlyPointer(), context.GetEntityId());
        }

        return retVal;
    }

    const void* GNParamNil::GetDataAddress() const
    {
        return nullptr;
    }

    AZ::TypeId GNParamNil::GetDataTypeUuid() const
    {
        return AZ::SerializeTypeInfo<void*>::GetUuid();
    }

    AZStd::string GNParamNil::ToJSONString() const
    {
        return AZStd::string();
    }

    void GNParamNil::CloneDataFrom(const GNProperty* gnProperty)
    {
        (void)gnProperty;
    }

    //////////////////////////
    // GNParamBoolean
    //////////////////////////
    void GNParamBoolean::Reflect(AZ::ReflectContext* reflection)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);

        if (serializeContext)
        {
            serializeContext->Class<GNParamBoolean, GNProperty>()->Version(1)->Field("value", &GNParamBoolean::m_value);
        }
    }

    GNProperty* GNParamBoolean::TryCreateProperty(GNParamDataContext& context, int valueIndex, const char* name)
    {
        GNProperty* retVal = nullptr;

        if (context.IsBoolean(valueIndex))
        {
            bool value;
            if (!context.ReadValue(value, Field::DefaultValue))
            {
                if (!context.ReadValue(value, Field::Value))
                {
                    AZ_Error("GNParamBoolean", false, "Missing DefaultValue or Value keys in the JSON data\n");
                }
            }

			retVal = aznew GNParamBoolean(name, value, context.GetReadOnlyPointer(), context.GetEntityId());
			retVal->ReadSetGNId(context);
        }

        return retVal;
    }
    

    bool GNParamBoolean::DoesTypeMatch(GNParamDataContext& context, int valueIndex) const
    {
        return context.IsBoolean(valueIndex);
    }

    AZStd::string GNParamBoolean::ToJSONString() const
    {
        auto jsonString = AZStd::string::format(
            R"JSON(
                    {
                        "%s": "%s",
                        "%s": %i,
                        "%s": "%s",
                        "%s": "%s"
                    }
                )JSON"
            , Field::Id
            , m_gnId.c_str()
            , Field::Value
            , m_value ? 1 : 0
            , Field::Type
            , m_type.c_str()
			, Field::Name
			, m_name.c_str()
        );
        return jsonString;
    }

    AZ::TypeId GNParamBoolean::GetDataTypeUuid() const
    {
        return AZ::SerializeTypeInfo<bool>::GetUuid();
    }

    void GNParamBoolean::CloneDataFrom(const GNProperty* gnProperty)
    {
        const GNParamBoolean* booleanProperty = azrtti_cast<const GNParamBoolean*>(gnProperty);
        AZ_Error("GNParamBoolean", booleanProperty, "Invalid call to CloneData. Types must match before clone attempt is made.\n");

        if (booleanProperty)
        {
            m_value = booleanProperty->m_value;
        }
    }

    /////////////////////////
    // GNParamInt
    /////////////////////////
    void GNParamInt::Reflect(AZ::ReflectContext* reflection)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);

        if (serializeContext)
        {
            serializeContext->Class<GNParamInt, GNProperty>()->Version(1)->Field("value", &GNParamInt::m_value);
        }
    }

    GNProperty* GNParamInt::TryCreateProperty(GNParamDataContext& context, int valueIndex, const char* name)
    {
        GNProperty* retVal = nullptr;

        if (context.IsInt(valueIndex))
        {
            int value;
			if (!context.ReadValue(value, Field::DefaultValue))
			{
				if (!context.ReadValue(value, Field::Value))
				{
					AZ_Error("GNParamInt", false, "Missing DefaultValue or Value keys in the JSON data\n");
				}
			}
            auto paramInt = aznew GNParamInt(name, value, context.GetReadOnlyPointer(), context.GetEntityId());

            int min, max;
            if (context.ReadValue(min, Field::MinValue))
            {
                paramInt->SetMinValue(min);
            }

            if (context.ReadValue(max, Field::MaxValue))
            {
                paramInt->SetMaxValue(max);
            }

            retVal = paramInt;
            retVal->ReadSetGNId(context);
        }

        return retVal;
    }

    bool GNParamInt::DoesTypeMatch(GNParamDataContext& context, int valueIndex) const
    {
        return context.IsInt(valueIndex);
    }

 

    AZ::TypeId GNParamInt::GetDataTypeUuid() const
    {
        return AZ::SerializeTypeInfo<int>::GetUuid();
    }

    AZStd::string GNParamInt::ToJSONString() const
    {
        auto jsonString = AZStd::string::format(
            R"JSON(
                    {
                        "%s": "%s",
                        "%s": %i,
                        "%s": "%s",
                        "%s": "%s"
                    }
                )JSON"
            , Field::Id
            , m_gnId.c_str()
            , Field::Value
            , m_value
            , Field::Type
            , m_type.c_str()
			, Field::Name
			, m_name.c_str()
        );
        return jsonString;
    }

    void GNParamInt::CloneDataFrom(const GNProperty* gnProperty)
    {
        const GNParamInt* numberProperty = azrtti_cast<const GNParamInt*>(gnProperty);
        AZ_Error("GNParamValue", numberProperty, "Invalid call to CloneData. Types must match before clone attempt is made.\n");

        if (numberProperty)
        {
            m_value = numberProperty->m_value;
        }
    }

    /////////////////////////
    // GNParamValue
    /////////////////////////
    void GNParamValue::Reflect(AZ::ReflectContext* reflection)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);

        if (serializeContext)
        {
            serializeContext->Class<GNParamValue, GNProperty>()->Version(1)->Field(
                "value", &GNParamValue::m_value);
        }
    }

    GNProperty* GNParamValue::TryCreateProperty(GNParamDataContext& context, int valueIndex, const char* name)
    {
        GNProperty* retVal = nullptr;

        if (context.IsValue(valueIndex))
        {
            double value;
			if (!context.ReadValue(value, Field::DefaultValue))
			{
				if (!context.ReadValue(value, Field::Value))
				{
					AZ_Error("GNParamValue", false, "Missing DefaultValue or Value keys in the JSON data\n");
				}
			}
            auto paramValue = aznew GNParamValue(name, value, context.GetReadOnlyPointer(), context.GetEntityId());
            double min, max;
            if (context.ReadValue(min, Field::MinValue))
            {
                paramValue->SetMinValue(min);
            }

            if (context.ReadValue(max, Field::MaxValue))
            {
                paramValue->SetMaxValue(max);
            }

            retVal = paramValue;
            retVal->ReadSetGNId(context);
        }

        return retVal;
    }

    bool GNParamValue::DoesTypeMatch(GNParamDataContext& context, int valueIndex) const
    {
        return context.IsValue(valueIndex);
    }

    AZ::TypeId GNParamValue::GetDataTypeUuid() const
    {
        return AZ::SerializeTypeInfo<double>::GetUuid();
    }

    AZStd::string GNParamValue::ToJSONString() const
    {
        auto jsonString = AZStd::string::format(
            R"JSON(
                    {
                        "%s": "%s",
                        "%s": %.17f,
                        "%s": "%s",
                        "%s": "%s"
                    }
                )JSON"
            , Field::Id
            , m_gnId.c_str()
            , Field::Value
            , m_value
            , Field::Type
            , m_type.c_str()
			, Field::Name
			, m_name.c_str()
        );
        return jsonString;
    }

    void GNParamValue::CloneDataFrom(const GNProperty* gnProperty)
    {
        const GNParamValue* numberProperty = azrtti_cast<const GNParamValue*>(gnProperty);
        AZ_Error("GNParamValue", numberProperty, "Invalid call to CloneData. Types must match before clone attempt is made.\n");

        if (numberProperty)
        {
            m_value = numberProperty->m_value;
        }
    }

    /////////////////////////
    // GNParamString
    /////////////////////////
    void GNParamString::Reflect(AZ::ReflectContext* reflection)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection);

        if (serializeContext)
        {
            serializeContext->Class<GNParamString, GNProperty>()->Version(1)->Field("value", &GNParamString::m_value);
        }
    }

    GNProperty* GNParamString::TryCreateProperty(GNParamDataContext& context, int valueIndex, const char* name)
    {
        GNProperty* retVal = nullptr;

        if (context.IsString(valueIndex))
        {
            const char* value = nullptr;
			if (!context.ReadValue(value, Field::DefaultValue))
			{
				if (!context.ReadValue(value, Field::Value))
				{
					AZ_Error("GNParamString", false, "Missing DefaultValue or Value keys in the JSON data\n");
				}
			}
            retVal = aznew GNParamString(name, value, context.GetReadOnlyPointer(), context.GetEntityId());
            retVal->ReadSetGNId(context);
        }

        return retVal;
    }

    bool GNParamString::DoesTypeMatch(GNParamDataContext& context, int valueIndex) const
    {
        return context.IsString(valueIndex);
    }

    AZ::TypeId GNParamString::GetDataTypeUuid() const
    {
        return AZ::SerializeGenericTypeInfo<AZStd::string>::GetClassTypeId();
    }

    AZStd::string GNParamString::ToJSONString() const
    {
        auto jsonString = AZStd::string::format(
            R"JSON(
                    {
                        "%s": "%s",
                        "%s": "%s",
                        "%s": "%s",
                        "%s": "%s"
                    }
                )JSON"
            , Field::Id
            , m_gnId.c_str()
            , Field::Value
            , m_value.c_str()
            , Field::Type
            , m_type.c_str()
			, Field::Name
			, m_name.c_str()
        );
        return jsonString;
    }

    void GNParamString::CloneDataFrom(const GNProperty* gnProperty)
    {
        const GNParamString* stringProperty = azrtti_cast<const GNParamString*>(gnProperty);
        AZ_Error("GNParamString", stringProperty, "Invalid call to CloneData. Types must match before clone attempt is made.\n");

        if (stringProperty)
        {
            m_value = stringProperty->m_value;
        }
    }
} // namespace GeomNodes
