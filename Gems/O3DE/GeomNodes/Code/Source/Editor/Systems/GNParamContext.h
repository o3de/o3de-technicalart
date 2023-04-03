#pragma once

#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/JSON/document.h>
#include <AzCore/JSON/rapidjson.h>
#include <Editor/Common/GNConstants.h>

namespace GeomNodes
{
    class GNProperty;
    class GNParamContextImpl;
    class GNParamContext;

    enum class ParamType : AZ::u8
    {
        Bool,
        Int,
        Value,
        String,
        StringComboBox,

        Unknown
    };

    const char* GetEnumString(ParamType value);
    ParamType GetTypeFromString(const char* value);

    template<class T>
    struct GNValue
    {
        typedef typename AZStd::remove_const<typename AZStd::remove_reference<typename AZStd::remove_pointer<T>::type>::type>::type ValueType;
    };

    template<>
    struct GNValue<bool>
    {
        static const bool isNativeValueType = true; // We use native type for internal representation
        static bool Read(const rapidjson::Value& val);
    };

    template<>
    struct GNValue<const char*>
    {
        static const bool isNativeValueType = true; // We use native type for internal representation
        static const char* Read(const rapidjson::Value& val);
    };

    template<>
    struct GNValue<int>
    {
        static const bool isNativeValueType = true; // We use native type for internal representation
        static int Read(const rapidjson::Value& val);
    };

    template<>
    struct GNValue<double>
    {
        static const bool isNativeValueType = true; // We use native type for internal representation
        static double Read(const rapidjson::Value& val);
    };

    struct GNPropertyGroup
    {
        AZ_TYPE_INFO(GNPropertyGroup, "{439E8395-77B5-4BC6-94D6-5A0F51DBE9FD}");
        AZStd::string m_name;
        AZStd::vector<GNProperty*> m_properties;
        AZStd::vector<GNPropertyGroup> m_groups;

        // Get the pointer to the specified group in m_groups. Returns nullptr if not found.
        GNPropertyGroup* GetGroup(const char* groupName);
        // Get the pointer to the specified property in m_properties. Returns nullptr if not found.
        GNProperty* GetProperty(const char* propertyName);
        // Generate JSON string of all properties/parameters. NOTE: only select details are included.
        AZStd::string GetProperties();

        // Remove all properties and groups
        void Clear();

        GNPropertyGroup() = default;
        ~GNPropertyGroup();

        GNPropertyGroup(const GNPropertyGroup& rhs) = delete;
        GNPropertyGroup& operator=(GNPropertyGroup&) = delete;

    public:
        GNPropertyGroup(GNPropertyGroup&& rhs)
        {
            *this = AZStd::move(rhs);
        }
        GNPropertyGroup& operator=(GNPropertyGroup&& rhs);
    };

    class GNParamDataContext
    {
        friend GNParamContext;
    public:
        AZ_TYPE_INFO(GNParamDataContext, "{61ED88BA-210A-458B-A5E5-C71C05C05411}");

        GNParamDataContext()
        {
        }

        bool IsNil(int index) const;
        bool IsBoolean(int index) const;
        bool IsInt(int index) const;
        bool IsValue(int index) const;
        bool IsString(int index) const;

        template<class T>
        bool ReadValue(T& valueRef, const char* key) const;

        void SetParamObject(const rapidjson::Value* value)
        {
            m_curParamObj = value;
        }
        void ClearParamObject()
        {
            m_curParamObj = nullptr;
        }

        void SetReadOnlyPointer(bool* pReadOnly)
        {
            m_pReadOnly = pReadOnly;
        }

        bool* GetReadOnlyPointer()
        {
            return m_pReadOnly;
        }

        const char* GetParamName();
        ParamType GetParamType();

    protected:
        const rapidjson::Value* m_curParamObj;
        bool* m_pReadOnly;
    };

    template<class T>
    inline bool GNParamDataContext::ReadValue(T& valueRef, const char* key) const
    {
        if (m_curParamObj == nullptr || !(*m_curParamObj).HasMember(key))
            return false;

        valueRef = GNValue<T>::Read((*m_curParamObj)[key]);
        
        return true;
    }

    class GNParamContextImpl
    {
    public:
        typedef GNProperty* (*ParamTypeFactory)(GNParamDataContext& context, int valueIndex, const char* name);

        AZ_CLASS_ALLOCATOR(GNParamContextImpl, AZ::SystemAllocator, 0);

        GNParamContextImpl();
        ~GNParamContextImpl() = default;

        GNProperty* ConstructGNParam(GNParamDataContext& gndc, ParamType pType, const char* name);
        AZStd::vector<ParamTypeFactory> m_paramFactories;
    };

    class GNParamContext
    {
        friend class EditorGeomNodesComponent;

    public:
        AZ_CLASS_ALLOCATOR(GNParamContext, AZ::SystemAllocator, 0);

        AZ_TYPE_INFO(GeomNodes::GNParamContext, "{AA9713B7-70F1-43CB-9F95-5BEC9F44F556}");

        GNParamContext();
        ~GNParamContext();

        static void Reflect(AZ::ReflectContext* reflection);

        GNProperty* ConstructGNParam(GNParamDataContext& gndc, ParamType pType, const char* name);

    protected:
        GNPropertyGroup m_group;

        GNParamContextImpl* m_impl;
    };
} // namespace GeomNodes
