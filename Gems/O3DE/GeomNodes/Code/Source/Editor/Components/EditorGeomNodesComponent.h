/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <AzCore/Serialization/EditContext.h>
#include "Editor/Systems/GNInstance.h"
#include "Editor/Systems/GNParamContext.h"
#include "Editor/Rendering/GNModelData.h"
#include <Editor/EBus/IpcHandlerBus.h>
#include <Editor/EBus/EditorGeomNodesComponentBus.h>
#include <Editor/Common/GNConstants.h>
#include <AzToolsFramework/Entity/EntityTypes.h>

namespace GeomNodes
{
    class GNMeshController;
    class EditorGeomNodesComponent
        : public AzToolsFramework::Components::EditorComponentBase
        , private Ipc::IpcHandlerNotificationBus::Handler
        , private EditorGeomNodesComponentRequestBus::Handler
    {
    public:
        AZ_EDITOR_COMPONENT(EditorGeomNodesComponent, "{E59507EF-9EBB-4F6C-8D89-92DCA57722E5}", EditorComponentBase);

        static void Reflect(AZ::ReflectContext* context);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        EditorGeomNodesComponent();
        virtual ~EditorGeomNodesComponent();

        void Init() override;
        void Activate() override;
        void Deactivate() override;

		// EditorGeomNodesComponentRequestBus overrides ...
        void SetWorkInProgress(bool flag) override;
        bool GetWorkInProgress() override;
        void SendIPCMsg(const AZStd::string& msg) override;
		void OnParamChange() override;

    private:
		
    protected:
        //got this from ScriptEditorComponent
        struct ElementInfo
        {
            AZ::Uuid m_uuid;                    // Type uuid for the class field that should use this edit data.
            AZ::Edit::ElementData m_editData;   // Edit metadata (name, description, attribs, etc).
            bool m_isAttributeOwner;            // True if this ElementInfo owns the internal attributes. We can use a single
                                                // ElementInfo for more than one class field, but only one owns the Attributes.
            float m_sortOrder;                  // Sort order of the property as defined by using the "order" attribute, by default the order is FLT_MAX
                                                // which means alphabetical sort will be used
        };
        
		void Clear();
        void OnPathChange(const AZStd::string& path);
        // IpcHandlerNotificationBus overrides...
        void OnMessageReceived(const AZ::u8* content, const AZ::u64 length) override;

        void ExportToStaticMesh();
        bool IsBlenderFileLoaded();
        
        AZStd::string ExportButtonText();

        void LoadObjects(const rapidjson::Value& objectNameArray, const rapidjson::Value& objectArray);
        void LoadObjectNames(const rapidjson::Value& objectNames);
        void LoadParams(const rapidjson::Value& objectArray);
        void CreateDataElements(GNPropertyGroup& group);
        void CreateObjectNames(const AZStd::string& objectName, const StringVector& enumValues, GNPropertyGroup& group);
        void CreateParam(const AZStd::string& objectName, GNPropertyGroup& group);
        bool LoadProperties(const rapidjson::Value& paramVal, GNPropertyGroup& group);
        void LoadAttribute(ParamType type, AZ::Edit::ElementData& ed, GNProperty* prop);
        
        void ClearDataElements();

        const AZ::Edit::ElementData* GetDataElement(const void* element, const AZ::Uuid& typeUuid) const;

        static const AZ::Edit::ElementData* GetParamsEditData(
            const void* handlerPtr, const void* elementPtr, const AZ::Uuid& elementType);

        void AddDataElement(GNProperty* gnParam, ElementInfo& ei);

        const char* CacheString(const char* str);
        AZStd::unordered_map<const void*, AZStd::string> m_cachedStrings;
        AZStd::unordered_map<const void*, ElementInfo> m_dataElements;
        AZStd::unordered_map<AZStd::string, AZStd::string> m_defaultObjectInfos;

        StringVector m_enumValues;

        GNParamContext m_paramContext;
        GNModelData m_modelData;
        AZStd::unique_ptr<GNMeshController> m_controller;

        AZStd::string m_blenderFile;
        AZStd::string m_currentObject;
        AZStd::string m_currentObjectInfo; //!< in JSON form. This is the representation of the current selected object along with the current parameters.

        GNInstance* m_instance = nullptr;
        AzToolsFramework::EntityIdList m_entityIdList;

        bool m_initialized = false;
        bool m_workInProgress = false;
        bool m_fromActivate = false;
    };
}
