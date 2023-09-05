#pragma once

#include <HAPI/HAPI.h>
#include <HoudiniEngine/HoudiniApi.h>

#include <AzCore/std/containers/vector.h> // FL[FD-11761] Add support for multiparm blocks (lists) to lumberyard houdini engine

namespace HoudiniEngine
{
    class HoudiniParameter : public IHoudiniParameter
    {
        friend class Houdini;
        friend class HoudiniNode;

        protected:
            IHoudini* m_hou;
            HAPI_Session* m_session;
            IHoudiniNode* m_node;
            HAPI_ParmInfo m_info;
            
            //CachedInfo
            int m_size;
            int m_choiceCount;
            HAPI_ParmId m_id;
            // FL[FD-11761] Add support for multiparm blocks (lists) to lumberyard houdini engine
            HAPI_ParmId m_parentId = 0;
            int m_instanceId = -1;
            HAPI_ParmType m_type;
            AZStd::string m_name;
            AZStd::string m_label;
            AZStd::string m_typeInfo;
            AZStd::string m_help;

            bool m_isProcessed; // needed for O3DE parameter processing

            bool m_valueBool = false;
            int m_valueInt = 0;
            float m_valueFloat = 0;
            AZ::EntityId m_valueEntity;
            AZ::Vector2 m_valueVec2 = AZ::Vector2(0,0);
            AZ::Vector3 m_valueVec3 = AZ::Vector3(0,0,0);
            AZ::Vector4 m_valueVec4 = AZ::Vector4(0,0,0,0);
            AZStd::string m_valueString;
            AZ::Color m_valueColor = AZ::Color::CreateZero();  // FL[FD-11758] Add support for color parameter to lumberyard houdini engine
            int m_multiParamValue{ 0 };  // FL[FD-11761] Add support for multiparm blocks (lists) to lumberyard houdini engine

            HoudiniParameter(IHoudini* hou, HAPI_ParmInfo info, IHoudiniNode* node);
            
        public:
            virtual ~HoudiniParameter() = default;

            AZStd::string GetName() override
            {
                return m_name;
            }

            AZStd::string GetLabel() override
            {
                return m_label;
            }

            AZStd::string GetTypeName() override
            {
                switch (GetType())
                {
                case HAPI_PARMTYPE_INT: return "INT";
                case HAPI_PARMTYPE_MULTIPARMLIST: return "MULTIPARMLIST";
                case HAPI_PARMTYPE_TOGGLE: return "TOGGLE";
                case HAPI_PARMTYPE_BUTTON: return "BUTTON";

                case HAPI_PARMTYPE_FLOAT: return "FLOAT";
                case HAPI_PARMTYPE_COLOR: return "COLOR";

                case HAPI_PARMTYPE_STRING: return "STRING";
                case HAPI_PARMTYPE_PATH_FILE: return "PATH_FILE";
                case HAPI_PARMTYPE_PATH_FILE_GEO: return "PATH_FILE_GEO";
                case HAPI_PARMTYPE_PATH_FILE_IMAGE: return "PATH_FILE_IMAGE";

                case HAPI_PARMTYPE_NODE: return "NODE";

                case HAPI_PARMTYPE_FOLDERLIST: return "FOLDERLIST";
                case HAPI_PARMTYPE_FOLDERLIST_RADIO: return "FOLDERLIST_RADIO";

                case HAPI_PARMTYPE_FOLDER: return "FOLDER";
                case HAPI_PARMTYPE_LABEL: return "LABEL";
                case HAPI_PARMTYPE_SEPARATOR: return "SEPARATOR";
                }

                return "Unknown";
            }

            HAPI_ParmType GetType() override
            {
                return m_type;
            }

            int GetSize() override
            {
                return m_size;
            }

            IHoudini* GetHou() override
            {
                return m_hou;
            }

            HAPI_ParmId GetId() override
            {
                return m_id;
            }

            HAPI_ParmId GetParentId() override
            {
                return m_parentId;
            }

            const AZStd::string& GetTypeInfo() override
            {
                return m_typeInfo;
            }

            const AZStd::string& GetHelp() override
            {
                return m_help;
            }

            IHoudiniNode* GetNode() override
            {
                return m_node;
            }

            const HAPI_ParmInfo& GetInfo() override
            {
                return m_info;
            }

            void SetName(const AZStd::string & value) override
            {
                m_name = value;
            }
            
            void SetTypeInfo(const AZStd::string & value) override
            {
                m_typeInfo = value;
            }

            void SetValueBool(bool value) override;
            void SetValueInt(int value) override;
            void SetValueFloat(float value) override;
            void SetValueVec2(const AZ::Vector2& value) override;
            void SetValueVec3(const AZ::Vector3& value) override;
            void SetValueVec4(const AZ::Vector4& value) override;
            bool SetValueEntity(const AZ::EntityId & value) override;
            void SetValueString(const AZStd::string & value) override;
            void SetValueColor(const AZ::Color& value) override;  // FL[FD-11758] Add support for color parameter to lumberyard houdini engine
            // FL[FD-11761] Add support for multiparm blocks (lists) to lumberyard houdini engine
            void SetValueMultiparm(int value) override;
            void SetParmInfo(HAPI_ParmInfo info) override;

            bool GetValueBoolean() override;
            const int GetValueInt() override;
            AZStd::vector<int> GetValuesInt(int count) override;
            const float GetValueFloat() override;
            const AZ::Vector2& GetValueVec2() override;
            const AZ::Vector3& GetValueVec3() override;
            const AZ::Vector4& GetValueVec4() override;
            const AZ::EntityId& GetValueEntity() override;
            const AZStd::string& GetValueString() override;
            const AZ::Color& GetValueColor() override;  // FL[FD-11758] Add support for color parameter to lumberyard houdini engine
            AZStd::vector<AZStd::string> GetValueChoices() override;
            const int GetValueMultiparm() override;  // FL[FD-11761] Add support for multiparm blocks (lists) to lumberyard houdini engine

            // FL[FD-11761] Add support for multiparm blocks (lists) to lumberyard houdini engine
            HoudiniParameterPtr GetParent() override;
            AZStd::vector<HoudiniParameterPtr> GetChildren(int instanceNum) override;
            void InsertInstance(const int position) override;
            void RemoveInstance(const int position) override;

            void SetProcessed(bool flag) override
            {
                m_isProcessed = flag;
            }

            bool IsProcessed() override
            {
                return m_isProcessed;
            }
    };
}
