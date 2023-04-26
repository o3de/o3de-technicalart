/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/EntityId.h>
#include <AzCore/Math/Crc.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/RTTI/TypeInfoSimple.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <Editor/EBus/EditorGeomNodesComponentBus.h>
#include <Editor/Systems/GNParamContext.h>
#include <GeomNodes/GeomNodesTypeIds.h>

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
        static void Reflect(AZ::ReflectContext* reflection);
        static bool VersionConverter(AZ::SerializeContext& context, AZ::SerializeContext::DataElementNode& classElement);

        virtual ~GNProperty()
        {
        }
        AZ_RTTI(GeomNodes::GNProperty, GNPropertyTypeId);

        GNProperty()
        {
        }
        GNProperty(const char* name, bool* pReadOnly, AZ::EntityId entityId)
            : m_id(AZ::Crc32(name))
            , m_name(name)
            , m_pReadOnly(pReadOnly)
            , m_entityId(entityId)
        {
        }

        virtual const void* GetDataAddress() const = 0;
        virtual AZ::TypeId GetDataTypeUuid() const = 0;
        virtual AZStd::string ToJSONString() const = 0;

        virtual void ReadSetGNId(GNParamDataContext& context);

        virtual bool IsReadOnly()
        {
            return m_pReadOnly == nullptr ? false : *m_pReadOnly;
        }

        void OnParamChange()
        {
            EditorGeomNodesComponentRequestBus::Event(m_entityId, &EditorGeomNodesComponentRequests::OnParamChange);
        }

        AZ::u64 m_id;
        //! Geometry Node Param Id
        AZStd::string m_gnId;
        //! Geometry Node Param Name
        AZStd::string m_name;
        //! Geometry Node Param Type
        AZStd::string m_type = "UNKNOWN";
        bool* m_pReadOnly = nullptr;
        bool m_isMaxSet = false;
        bool m_isMinSet = false;
        AZ::EntityId m_entityId;
    };

    class GNParamNil : public GNProperty
    {
    public:
        AZ_CLASS_ALLOCATOR(GNParamNil, AZ::SystemAllocator);
        AZ_RTTI(GeomNodes::GNParamNil, GNParamNilTypeId, GNProperty);

        static void Reflect(AZ::ReflectContext* reflection);
        static GNProperty* TryCreateProperty(GNParamDataContext& context, int valueIndex, const char* name);

        GNParamNil()
        {
        }
        GNParamNil(const char* name, bool* pReadOnly, AZ::EntityId entityId)
            : GNProperty(name, pReadOnly, entityId)
        {
        }

        const void* GetDataAddress() const override;
        AZ::TypeId GetDataTypeUuid() const override;
        AZStd::string ToJSONString() const override;
    };

    class GNParamBoolean : public GNProperty
    {
    public:
        AZ_CLASS_ALLOCATOR(GNParamBoolean, AZ::SystemAllocator);
        AZ_RTTI(GeomNodes::GNParamBoolean, GNParamBooleanTypeId, GNProperty);

        static void Reflect(AZ::ReflectContext* reflection);
        static GNProperty* TryCreateProperty(GNParamDataContext& context, int valueIndex, const char* name);

        GNParamBoolean()
            : m_value(false)
        {
            m_type = GetEnumString(ParamType::Bool);
        }
        GNParamBoolean(const char* name, bool value, bool* pReadOnly, AZ::EntityId entityId)
            : GNProperty(name, pReadOnly, entityId)
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

        bool m_value;
    };

    class GNParamInt : public GNProperty
    {
    public:
        AZ_CLASS_ALLOCATOR(GNParamInt, AZ::SystemAllocator);
        AZ_RTTI(GeomNodes::GNParamInt, GNParamIntTypeId, GNProperty);

        static void Reflect(AZ::ReflectContext* reflection);
        static GNProperty* TryCreateProperty(GNParamDataContext& context, int valueIndex, const char* name);

        GNParamInt()
            : m_value(0)
        {
            m_type = GetEnumString(ParamType::Int);
        }
        GNParamInt(const char* name, int value, bool* pReadOnly, AZ::EntityId entityId)
            : GNProperty(name, pReadOnly, entityId)
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
        //! Geometry Node Param Max(int)
        int m_max;
        //! Geometry Node Param Min(int)
        int m_min;
    };

    class GNParamValue : public GNProperty
    {
    public:
        AZ_CLASS_ALLOCATOR(GNParamValue, AZ::SystemAllocator);
        AZ_RTTI(GeomNodes::GNParamValue, GNParamValueTypeId, GNProperty);

        static void Reflect(AZ::ReflectContext* reflection);
        static GNProperty* TryCreateProperty(GNParamDataContext& context, int valueIndex, const char* name);

        GNParamValue()
            : m_value(0.0f)
        {
            m_type = GetEnumString(ParamType::Value);
        }
        GNParamValue(const char* name, double value, bool* pReadOnly, AZ::EntityId entityId)
            : GNProperty(name, pReadOnly, entityId)
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
        //! Geometry Node Param Max(double)
        double m_max;
        //! Geometry Node Param Min(double)
        double m_min;
    };

    class GNParamString : public GNProperty
    {
    public:
        AZ_CLASS_ALLOCATOR(GNParamString, AZ::SystemAllocator);
        AZ_RTTI(GeomNodes::GNParamString, GNParamStringTypeId, GNProperty);

        static void Reflect(AZ::ReflectContext* reflection);
        static GNProperty* TryCreateProperty(GNParamDataContext& context, int valueIndex, const char* name);

        GNParamString()
        {
            m_type = GetEnumString(ParamType::String);
        }
        GNParamString(const char* name, const char* value, bool* pReadOnly, AZ::EntityId entityId)
            : GNProperty(name, pReadOnly, entityId)
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

        AZStd::string m_value;
    };
} // namespace GeomNodes
