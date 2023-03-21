#pragma once

#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Math/Crc.h>
#include <AzCore/RTTI/TypeInfoSimple.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <Editor/Systems/GNParamContext.h>
#include <AzCore/Serialization/EditContext.h>

namespace AZ
{
    class ReflectContext;
}

namespace GeomNodes
{
    
    class GNProperties
    {
    public:
        static void Reflect(AZ::ReflectContext* reflection);
    };

    /**
     * Base class for all script properties.
     */
    class GNProperty
    {
    public:
        //static void UpdateScriptProperty(GNParamDataContext& sdc, int valueIndex, GNProperty** targetProperty);

        static void Reflect(AZ::ReflectContext* reflection);
        static bool VersionConverter(AZ::SerializeContext& context, AZ::SerializeContext::DataElementNode& classElement);

        virtual ~GNProperty()
        {
        }
        AZ_RTTI(GeomNodes::GNProperty, "{71904E43-F0A1-45EA-B87F-4CC5234E1E52}");

        GNProperty()
        {
        }
        GNProperty(const char* name, bool* pReadOnly)
            : m_id(AZ::Crc32(name))
            , m_name(name)
            , m_pReadOnly(pReadOnly)
        {
        }

        virtual const void* GetDataAddress() const = 0;
        virtual AZ::TypeId GetDataTypeUuid() const = 0;
        virtual AZStd::string ToJSONString() const = 0;
        /**
         * Test if the value at the index valueIndex is of the same type as that of the instance of GNProperty's subclass.
         */
        virtual bool DoesTypeMatch(GNParamDataContext& /*context*/, int /*valueIndex*/) const
        {
            return false;
        }

        virtual void ReadSetGNId(GNParamDataContext& context);
        
        /*virtual bool Write(AZ::ScriptContext& context) = 0;
        virtual bool TryRead(GNParamDataContext& context, int valueIndex)
        {
            (void)context;
            (void)valueIndex;
            return false;
        };*/
        bool TryUpdate(const GNProperty* gnProperty)
        {
            bool allowUpdate = azrtti_typeid(gnProperty) == azrtti_typeid(this);

            if (allowUpdate)
            {
                CloneDataFrom(gnProperty);
            }

            return allowUpdate;
        }

        virtual bool IsReadOnly()
        {
            return m_pReadOnly == nullptr ? false : *m_pReadOnly;
        }

        AZ::u64         m_id;
        AZStd::string   m_gnId;                     // Geometry Node Param Id
        AZStd::string   m_name;                     // Geometry Node Param Name
        AZStd::string   m_type = "UNKNOWN";         // Geometry Node Param Type
        bool*           m_pReadOnly = nullptr;
        bool            m_isMaxSet = false;
        bool            m_isMinSet = false;
    protected:
        virtual void CloneDataFrom(const GNProperty* gnProperty) = 0;
    };

    class GNParamNil : public GNProperty
    {
    public:
        AZ_CLASS_ALLOCATOR(GNParamNil, AZ::SystemAllocator, 0);
        AZ_RTTI(GeomNodes::GNParamNil, "{519D98C7-054A-4047-BCEB-28DCD38CFCD4}", GNProperty);

        static void Reflect(AZ::ReflectContext* reflection);
        static GNProperty* TryCreateProperty(GNParamDataContext& context, int valueIndex, const char* name);

        GNParamNil()
        {
        }
		GNParamNil(const char* name, bool* pReadOnly)
			: GNProperty(name, pReadOnly)
        {
        }

        const void* GetDataAddress() const override;
        AZ::TypeId GetDataTypeUuid() const override;
        AZStd::string ToJSONString() const override;

        /*bool Write(AZ::ScriptContext& context) override;
        bool TryRead(GNParamDataContext& context, int valueIndex) override;*/

    protected:
        void CloneDataFrom(const GNProperty* gnProperty) override;
    };

    class GNParamBoolean : public GNProperty
    {
    public:
        AZ_CLASS_ALLOCATOR(GNParamBoolean, AZ::SystemAllocator, 0);
        AZ_RTTI(GeomNodes::GNParamBoolean, "{6A05BCAB-50F7-4988-96E1-0EDB6B76C3A3}", GNProperty);

        static void Reflect(AZ::ReflectContext* reflection);
        static GNProperty* TryCreateProperty(GNParamDataContext& context, int valueIndex, const char* name);

        GNParamBoolean()
            : m_value(false)
        {
            m_type = GetEnumString(ParamType::Bool);
        }
        GNParamBoolean(const char* name, bool value, bool* pReadOnly)
            : GNProperty(name, pReadOnly)
            , m_value(value)
        {
            m_type = GetEnumString(ParamType::Bool);
        }

        const void* GetDataAddress() const override
        {
            return &m_value;
        }

        AZStd::string ToJSONString() const override;

        AZ::TypeId GetDataTypeUuid() const override;

        bool DoesTypeMatch(GNParamDataContext& context, int valueIndex) const override;

        /*bool Write(AZ::ScriptContext& context) override;
        bool TryRead(GNParamDataContext& context, int valueIndex) override;*/

        bool m_value;

    protected:
        void CloneDataFrom(const GNProperty* gnProperty) override;
    };

    class GNParamInt : public GNProperty
    {
    public:
        AZ_CLASS_ALLOCATOR(GNParamInt, AZ::SystemAllocator, 0);
        AZ_RTTI(GNParamInt, "{B2457A3D-F30C-43F9-90F0-5BFAFFFD0F59}", GNProperty);

        static void Reflect(AZ::ReflectContext* reflection);
        static GNProperty* TryCreateProperty(GNParamDataContext& context, int valueIndex, const char* name);

        GNParamInt()
            : m_value(0)
        {
            m_type = GetEnumString(ParamType::Int);
        }
		GNParamInt(const char* name, int value, bool* pReadOnly)
			: GNProperty(name, pReadOnly)
            , m_value(value)
        {
            m_type = GetEnumString(ParamType::Int);
        }

        const void* GetDataAddress() const override
        {
            return &m_value;
        }
        AZ::TypeId GetDataTypeUuid() const override;
        AZStd::string ToJSONString() const override;

        bool DoesTypeMatch(GNParamDataContext& context, int valueIndex) const override;

        /*bool Write(AZ::ScriptContext& context) override;
        bool TryRead(GNParamDataContext& context, int valueIndex) override;*/

        void SetMinValue(int min)
        {
            m_min = min;
            m_isMinSet = true;
        }

        void SetMaxValue(int max)
        {
            m_max = max;
            m_isMaxSet = true;
        }

        int m_value;
        int m_max;      // Geometry Node Param Max(int)
        int m_min;      // Geometry Node Param Min(int)

    protected:
        void CloneDataFrom(const GNProperty* gnProperty) override;
    };

    class GNParamValue : public GNProperty
    {
    public:
        AZ_CLASS_ALLOCATOR(GNParamValue, AZ::SystemAllocator, 0);
        AZ_RTTI(GeomNodes::GNParamValue, "{4790660B-B942-4421-B942-AE27DF67BF4F}", GNProperty);

        static void Reflect(AZ::ReflectContext* reflection);
        static GNProperty* TryCreateProperty(GNParamDataContext& context, int valueIndex, const char* name);

        GNParamValue()
            : m_value(0.0f)
        {
            m_type = GetEnumString(ParamType::Value);
        }
		GNParamValue(const char* name, double value, bool* pReadOnly)
			: GNProperty(name, pReadOnly)
            , m_value(value)
        {
            m_type = GetEnumString(ParamType::Value);
        }

        const void* GetDataAddress() const override
        {
            return &m_value;
        }
        AZ::TypeId GetDataTypeUuid() const override;
        AZStd::string ToJSONString() const override;

        bool DoesTypeMatch(GNParamDataContext& context, int valueIndex) const override;

        /*bool Write(AZ::ScriptContext& context) override;
        bool TryRead(GNParamDataContext& context, int valueIndex) override;*/

        void SetMinValue(double min)
        {
            m_min = min;
            m_isMinSet = true;
        }

        void SetMaxValue(double max)
        {
            m_max = max;
            m_isMaxSet = true;
        }

        double m_value;
        double m_max;       // Geometry Node Param Max(double)
        double m_min;       // Geometry Node Param Min(double)

    protected:
        void CloneDataFrom(const GNProperty* gnProperty) override;
    };

    class GNParamString : public GNProperty
    {
    public:
        AZ_CLASS_ALLOCATOR(GNParamString, AZ::SystemAllocator, 0);
        AZ_RTTI(GeomNodes::GNParamString, "{9296C827-0281-4DBF-AC1A-B6636BCEC716}", GNProperty);

        static void Reflect(AZ::ReflectContext* reflection);
        static GNProperty* TryCreateProperty(GNParamDataContext& context, int valueIndex, const char* name);

        GNParamString()
        {
            m_type = GetEnumString(ParamType::String);
        }
		GNParamString(const char* name, const char* value, bool* pReadOnly)
			: GNProperty(name, pReadOnly)
            , m_value(value)
        {
            m_type = GetEnumString(ParamType::String);
        }

        const void* GetDataAddress() const override
        {
            return &m_value;
        }
        AZ::TypeId GetDataTypeUuid() const override;
        AZStd::string ToJSONString() const override;

        bool DoesTypeMatch(GNParamDataContext& context, int valueIndex) const override;

        /*bool Write(AZ::ScriptContext& context) override;
        bool TryRead(GNParamDataContext& context, int valueIndex) override;*/

        AZStd::string m_value;

    protected:
        void CloneDataFrom(const GNProperty* gnProperty) override;
    };
} // namespace GeomNodes
