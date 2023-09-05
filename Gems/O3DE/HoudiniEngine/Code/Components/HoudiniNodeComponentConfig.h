#pragma once

#include <HoudiniEngine/HoudiniEngineBus.h>
#include <HoudiniEngine/HoudiniCommonForwards.h>
#include "HoudiniPropertyGroup.h"
#include <AzToolsFramework/ToolsComponents/ScriptEditorComponent.h>

namespace HoudiniEngine
{
    class HoudiniNode;
    class HoudiniPropertyGroup;

    class HoudiniNodeComponentConfig
        : public AZ::ComponentConfig
        , public IHoudiniNodeComponentConfig
    {
    public:
        AZ_CLASS_ALLOCATOR(HoudiniNodeComponentConfig, AZ::SystemAllocator);
        AZ_RTTI(HoudiniNodeComponentConfig, HOUDINI_CONFIG_GUID, AZ::ComponentConfig);
        static void Reflect(AZ::ReflectContext* context);
        static bool VersionConverter(AZ::SerializeContext& context, AZ::SerializeContext::DataElementNode& classElement);

        HoudiniNodeComponentConfig() = default;
        virtual ~HoudiniNodeComponentConfig() = default;

        bool IsInitialized() override
        {
            return m_initialized;
        }

        //HoudiniAssetRequestBus
        IHoudiniNode* LoadHda(const AZStd::string& operatorName, const AZStd::string& nodeName, AZStd::function<void(IHoudiniNode*)> onLoad = {}) override;

        AZ::ScriptProperty* GetProperty(const AZStd::string& name) override;

        bool SetPropertyValueBool(const AZStd::string& name, bool value) override;
        bool SetPropertyValueInt(const AZStd::string& name, int value) override;
        bool SetPropertyValueFloat(const AZStd::string& name, float value) override;
        bool SetPropertyValueVec2(const AZStd::string& name, const AZ::Vector2& value) override;
        bool SetPropertyValueVec3(const AZStd::string& name, const AZ::Vector3& value) override;
        bool SetPropertyValueVec4(const AZStd::string& name, const AZ::Vector4& value) override;
        bool SetPropertyValueEntityId(const AZStd::string& name, const AZ::EntityId & value) override;
        bool SetPropertyValueString(const AZStd::string& name, const AZStd::string & value) override;

        bool SetInputEntityIdByName(const AZStd::string& name, const AZ::EntityId & value) override;
        bool SetInputEntityId(int index, const AZ::EntityId & value) override;

        int GetPropertyValueInt(const AZStd::string& name) override;
        float GetPropertyValueFloat(const AZStd::string& name) override;
        AZ::Vector2 GetPropertyValueVec2(const AZStd::string& name) override;
        AZ::Vector3 GetPropertyValueVec3(const AZStd::string& name) override;
        AZ::Vector4 GetPropertyValueVec4(const AZStd::string& name) override;
        AZStd::string GetPropertyValueString(const AZStd::string& name) override;
        AZ::EntityId GetPropertyValueEntityId(const AZStd::string& name) override;

        AZ::EntityId GetInputEntityIdByName(const AZStd::string& name) override;
        AZ::EntityId GetInputEntityId(int index) override;

        void UpdateWorldTransformData(const AZ::Transform& transform);

        IHoudiniNode* GetNode() override { return m_node.get(); }
        virtual const AZStd::string& GetNodeName() { return m_nodeName; }
        virtual const AZStd::string& GetOperatorName() { return m_operatorName; }

        bool UpdateNode() override;
        void RenameNode(const AZStd::string& newName) override;
        void FixEntityPointers() override;

        AZStd::string GetHelpText();
        
        bool m_creating = false;
        bool m_initialized = false;
        bool m_locked = false;

        AZ::EntityId m_entityId;
        AZStd::string m_nodeName;

        AZStd::shared_ptr<IHoudiniNode> m_node;
        HoudiniPropertyGroup m_properties;
        
        AZStd::string m_operatorName;
        AZStd::vector<AZStd::string> getOperatorNames() override;

        AZ::EntityId CreateSplineEntity(const AZStd::string& name, AZ::EntityId parent);

        //Presets:
        AZStd::unordered_map<AZStd::string, AZ::EntityId> m_defaultEntities;

        //BUTTONS:

        AZ::Crc32 OnSelectOperator();

        AZ::Crc32 OnNodeNameChanged();

        bool m_viewButton = false;
        AZ::Crc32 OnLoadHoudiniInstance();

        bool m_viewReloadButton = false;
        AZ::Crc32 OnReloadHoudiniInstance();
        
        bool m_viewExtractGroup = false;
        AZ::Crc32 OnExtractGroupPoints();

        bool m_viewDebugButton = false;
        AZ::Crc32 OnSaveDebugAsset();

        OperatorMode m_selectionMode = OperatorMode::Assets;

        AZStd::string m_helpTextField;
        AZStd::string m_lastError;
        const AZStd::string& GetLastError();

        // FL[FD-15480] Update parm values from Houdini side
        AZStd::map<AZStd::string, AZ::EntityId> m_inputTypePropertiesBackup;

        void BackupInputTypeProperties();
        void ReloadProperties();

        bool m_viewReloadPropertiesButton = false;
        AZ::Crc32 OnReloadProperties();

        bool m_hasSpline = false;
    };

}
