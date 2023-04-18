/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Editor/Components/EditorGeomNodesComponent.h>
#include <Editor/UI/UI_common.h>
#include <Editor/UI/Validators.h>
#include <Editor/UI/Utils.h>
#include <Editor/Systems/GNProperty.h>
#include <Editor/Rendering/GNMeshController.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzToolsFramework/API/EntityCompositionRequestBus.h>
#include <AzCore/Utils/Utils.h>

#include <AzCore/JSON/prettywriter.h>
#include <AzCore/JSON/stringbuffer.h>
#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
#include <Atom/RPI.Public/Scene.h>
#include <AzCore/Component/NonUniformScaleBus.h>
#include <AzCore/Settings/SettingsRegistryMergeUtils.h>

namespace GeomNodes
{
    static void* blendFunctor = reinterpret_cast<void*>(&SelectBlendFromFileDialog);

    EditorGeomNodesComponent::EditorGeomNodesComponent()
    {
    }

    EditorGeomNodesComponent::~EditorGeomNodesComponent()
    {
    }

    void EditorGeomNodesComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<EditorGeomNodesComponent, EditorComponentBase>()
                ->Version(1)
                ->Field("GNParamContext", &EditorGeomNodesComponent::m_paramContext)
                ->Field("BlenderFile", &EditorGeomNodesComponent::m_blenderFile)
				->Field("ObjectNameList", &EditorGeomNodesComponent::m_enumValues)
				->Field("ObjectInfos", &EditorGeomNodesComponent::m_defaultObjectInfos)
				//->Field("CurrentObject", &EditorGeomNodesComponent::m_currentObject)
                ->Field("CurrentObjectInfo", &EditorGeomNodesComponent::m_currentObjectInfo)
                ->Field("IsInitialized", &EditorGeomNodesComponent::m_initialized)
                ;
            
            GNParamContext::Reflect(context);
            

            AZ::EditContext* ec = serializeContext->GetEditContext();
            if (ec)
            {
                ec->Class<EditorGeomNodesComponent>(
                    "Geometry Node",
                    "The Geometry Node component allows you to load a blend file with geometry node and tweak exposed parameters. ")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Blender")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("UI", 0x27ff46b0))
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Level", 0x9aeacc13))
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Layer", 0xe4db211a))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(
                        Handlers::FileSelect,
                        &EditorGeomNodesComponent::m_blenderFile,
                        "Blender File",
                        "Blender file with Geometry Nodes")
                        ->Attribute(Attributes::FuncValidator, ConvertFunctorToVoid(&Validators::ValidBlenderOrEmpty))
                        ->Attribute(Attributes::SelectFunction, blendFunctor)
                        ->Attribute(Attributes::ValidationChange, &EditorGeomNodesComponent::OnPathChange)
                        ->Attribute(AZ::Edit::Attributes::ReadOnly, &EditorGeomNodesComponent::GetWorkInProgress)
					->DataElement(nullptr, &EditorGeomNodesComponent::m_paramContext, "Geom Nodes Parameters", "Parameter template")
                    ->SetDynamicEditDataProvider(&EditorGeomNodesComponent::GetParamsEditData)
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
					->UIElement(AZ::Edit::UIHandlers::Button, "", "Export to static mesh")
					->Attribute(AZ::Edit::Attributes::ChangeNotify, &EditorGeomNodesComponent::ExportToStaticMesh)
					->Attribute(AZ::Edit::Attributes::ButtonText, &EditorGeomNodesComponent::ExportButtonText)
                    ->Attribute(AZ::Edit::Attributes::Visibility, &EditorGeomNodesComponent::IsBlenderFileLoaded)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::EntireTree)
                    ->Attribute(AZ::Edit::Attributes::ReadOnly, &EditorGeomNodesComponent::GetWorkInProgress)
                    ;

                ec->Class<GNParamContext>("Geom Nodes Parameter Context", "Adding exposed Geometry Nodes parameters to the entity!")
                    ->DataElement(nullptr, &GNParamContext::m_group, "Properties", "Geometry Nodes properties")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;

                ec->Class<GNPropertyGroup>("Geom Nodes Property group", "This is a  property group")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "GNPropertyGroup's class attributes.")
                    ->Attribute(AZ::Edit::Attributes::NameLabelOverride, &GNPropertyGroup::m_name)
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(nullptr, &GNPropertyGroup::m_properties, "m_properties", "Properties in this property group")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->DataElement(nullptr, &GNPropertyGroup::m_groups, "m_groups", "Subgroups in this property group")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly);

                ec->Class<GNProperty>("GeomNodes Property", "Base class for Geometry Nodes properties")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "GNPropertyGroup's class attributes.")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly);

                ec->Class<GNParamBoolean>("Geom Nodes Property (bool)", "A Geom Nodes boolean property")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "GNPropertyGroup's class attributes.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GNParamBoolean::m_value, "m_value", "A boolean")
                    ->Attribute(AZ::Edit::Attributes::ReadOnly, &GNProperty::IsReadOnly)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &GNProperty::OnParamChange)
                    ->Attribute(AZ::Edit::Attributes::NameLabelOverride, &GNParamBoolean::m_name);

                ec->Class<GNParamInt>("Geom Nodes Property (int)", "A Geom Nodes int property")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "GNPropertyGroup's class attributes.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GNParamInt::m_value, "m_value", "An int")
                    ->Attribute(AZ::Edit::Attributes::ReadOnly, &GNProperty::IsReadOnly)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &GNProperty::OnParamChange)
                    ->Attribute(AZ::Edit::Attributes::NameLabelOverride, &GNParamInt::m_name);

                ec->Class<GNParamValue>("Geom Nodes Property (double)", "A Geom Nodes double property")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "GNPropertyGroup's class attributes.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GNParamValue::m_value, "m_value", "A double/value")
                    ->Attribute(AZ::Edit::Attributes::ReadOnly, &GNProperty::IsReadOnly)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &GNProperty::OnParamChange)
                        ->Attribute(AZ::Edit::Attributes::NameLabelOverride, &GNParamValue::m_name);

                ec->Class<GNParamString>("Geom Nodes Property (string)", "A Geom Nodes string property")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "GNPropertyGroup's class attributes.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GNParamString::m_value, "m_value", "A string")
                    ->Attribute(AZ::Edit::Attributes::ReadOnly, &GNProperty::IsReadOnly)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &GNProperty::OnParamChange)
                        ->Attribute(AZ::Edit::Attributes::NameLabelOverride, &GNParamString::m_name);
            }
        }
    }

    void EditorGeomNodesComponent::OnPathChange(const AZStd::string& path)
    {
        if (!path.empty())
        {
            bool bClearParams = (m_instance && !m_instance->IsSamePath(path));
            if (bClearParams)
            {
                m_paramContext.m_group.Clear();
                ClearDataElements();
                m_initialized = false;
                m_currentObjectInfo.clear();
                SetWorkInProgress(true);
            }

            if (!m_instance || bClearParams || (m_instance && !m_instance->IsValid()))
            {
                if (m_instance)
                    delete m_instance;
                else
                    SetWorkInProgress(true);

                const AZ::IO::FixedMaxPath gemPath = AZ::Utils::GetGemPath("GeomNodes");
                const AZ::IO::FixedMaxPath exePath = AZ::Utils::GetExecutableDirectory();
                AZ::IO::FixedMaxPath bridgePath = exePath / "Bridge.dll"; //TODO: make this platform agnostic
                if (!AZ::IO::SystemFile::Exists(bridgePath.c_str()))
                {
                    auto registry = AZ::SettingsRegistry::Get();
                    AZ::IO::FixedMaxPath projectBuildPath;
                    if (!registry->Get(projectBuildPath.Native(), AZ::SettingsRegistryMergeUtils::ProjectBuildPath))
                    {
                        AZ_Error("GeomNodes", false, "No project build path setting was found in the user registry folder");
                        return;
                    }

                    bridgePath = projectBuildPath / "bin/profile/Bridge.dll"; //TODO: check if there is a way to get "bin/profile" in the registry or somewhere and not hard coded.
                    if (!AZ::IO::SystemFile::Exists(bridgePath.c_str()))
                    {
                        AZ_Error("GeomNodes", false, "Can't find Bridge.dll");
                        return;
                    }

                    bridgePath = projectBuildPath / "bin/profile";
                }
                else
                {
                    bridgePath = exePath.c_str();
                }

                AZStd::string scriptPath = AZStd::string::format(R"(%s\External\Scripts\__init__.py)", gemPath.c_str());

                m_instance = new GNInstance;
                m_instance->Init(path, scriptPath, bridgePath.c_str(), GetEntityId());
                if (m_instance->IsValid())
                {
                    Ipc::IpcHandlerNotificationBus::Handler::BusConnect(GetEntityId());
                }

                m_controller->SetFileName(path);
            }
        }
    }

    void EditorGeomNodesComponent::OnParamChange()
    {
        SetWorkInProgress(true);
        
        if (m_paramContext.m_group.m_properties.size() > 0)
        {
            // this checks if the user chooses another object.
			auto gnParam = reinterpret_cast<GNParamString*>(m_paramContext.m_group.GetProperty(Field::Objects));
			if (gnParam->m_value != m_currentObject)
			{
				m_currentObject = gnParam->m_value;
				m_paramContext.m_group.Clear(); // clear the group/properties
                CreateDataElements(m_paramContext.m_group);
			}
        }
        
        if(m_instance)
        {
            if (!m_instance->IsValid())
            {
                m_instance->RestartProcess();
            }
            
			m_currentObjectInfo = m_instance->SendParamUpdates(m_paramContext.m_group
				.GetGroup(m_currentObject.c_str())->GetProperties(), m_currentObject);
        }

        AZ_Printf("EditorGeomNodesComponent", "%llu: Parameter has changed", (AZ::u64)GetEntityId());
    }

    void EditorGeomNodesComponent::OnMessageReceived(const AZ::u8* content, const AZ::u64 length)
    {
        rapidjson::Document jsonDocument;
        jsonDocument.Parse((const char*)content, length);
        if (!jsonDocument.HasParseError())
        {
            // send back an "Alive" message to the client when it asks for a heartbeat. 
            if (jsonDocument.HasMember(Field::Heartbeat))
            {
                m_instance->SendHeartbeat();
            }
            else if (jsonDocument.HasMember(Field::Initialized))
            {
                if (!m_initialized)
                {
                    m_instance->RequestObjectParams();
                    m_initialized = true;
                }
                else if(m_fromActivate)
                {
                    OnParamChange();
                }
                m_fromActivate = false;
            }
            else if (jsonDocument.HasMember(Field::ObjectNames) && jsonDocument.HasMember(Field::Objects) && jsonDocument.HasMember(Field::Materials))
            {
                LoadObjects(jsonDocument[Field::ObjectNames], jsonDocument[Field::Objects]);
                CreateDataElements(m_paramContext.m_group);

                // Send message for fetching the 3D data
                // Will just call OnParamChange since it's basically the same request
                OnParamChange();

                // Handle the materials as well
                m_controller->LoadMaterials(jsonDocument[Field::Materials]);
                SetWorkInProgress(false);
            }
            else if (jsonDocument.HasMember(Field::SHMOpen) && jsonDocument.HasMember(Field::MapId))
            {
                SetWorkInProgress(true);
                AZ::u64 mapId = jsonDocument[Field::MapId].GetInt64();
                m_controller->ReadData(mapId);
                m_instance->CloseMap(mapId);
                m_controller->RebuildRenderMesh();
            }
            else if (jsonDocument.HasMember(Field::Export) && jsonDocument.HasMember(Field::Error))
            {
                AZStd::string errorMsg = jsonDocument[Field::Error].GetString();
                if (!errorMsg.empty())
                {
                    AZ_Warning("EditorGeomNodesComponent", false, errorMsg.c_str());
                    SetWorkInProgress(false);
                }
            }
        }
        else
        {
            AZ_Warning("EditorGeomNodesComponent", false, "Message is not in JSON format!");
        }
    }

    void EditorGeomNodesComponent::ExportToStaticMesh()
    {
        if (!m_workInProgress)
        {
			m_instance->RequestExport(m_paramContext.m_group
				.GetGroup(m_currentObject.c_str())->GetProperties()
                , m_currentObject, m_controller->GenerateFBXPath());
            SetWorkInProgress(true);
        }
    }

    bool EditorGeomNodesComponent::IsBlenderFileLoaded()
    {
        return m_initialized;
    }

    void EditorGeomNodesComponent::SetWorkInProgress(bool flag)
    {
		AZ::SystemTickBus::QueueFunction(
            [=]() {
				if (m_workInProgress != flag)
				{
    				m_workInProgress = flag;
                    EBUS_EVENT(AzToolsFramework::ToolsApplicationEvents::Bus, InvalidatePropertyDisplay, AzToolsFramework::Refresh_EntireTree);
                }
            });
    }

    bool EditorGeomNodesComponent::GetWorkInProgress()
    {
        return m_workInProgress;
    }

    void EditorGeomNodesComponent::SendIPCMsg(const AZStd::string& msg)
    {
        if (m_instance != nullptr)
        {
            m_instance->SendIPCMsg(msg);
        }
    }

    AZStd::string EditorGeomNodesComponent::ExportButtonText()
    {
        return m_workInProgress ? "Working on your request" : "Export";
    }

    void EditorGeomNodesComponent::LoadObjects(const rapidjson::Value& objectNameArray, const rapidjson::Value& objectArray)
    {
        
        // Populate m_enumValues that will store the list of object names
        LoadObjectNames(objectNameArray);
        
        // Load and save our param list object from json. Need this so it's faster to switch between objects and not need to send a request via IPC.
        LoadParams(objectArray);

        m_currentObject = m_enumValues[0];
    }

    void EditorGeomNodesComponent::LoadObjectNames(const rapidjson::Value& objectNames)
    {
        AZ_Assert(objectNames.IsArray(), "Passed JSON Value is not an array!");

        uint32_t idx = 0;
        m_enumValues.clear();
        for (rapidjson::Value::ConstValueIterator itr = objectNames.Begin(); itr != objectNames.End(); ++itr, idx++)
        {
            m_enumValues.push_back(itr->GetString());
        }

        AZ_Assert(!m_enumValues.empty(), "No Object found! There should be at least one.");
    }

    void EditorGeomNodesComponent::LoadParams(const rapidjson::Value& objectArray)
    {
        for (rapidjson::Value::ConstValueIterator itr = objectArray.Begin(); itr != objectArray.End(); ++itr)
        {
            const char* objectName = (*itr)[Field::Object].GetString();

            rapidjson::StringBuffer jsonDataBuffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> jsonDatawriter(jsonDataBuffer);
            (*itr).Accept(jsonDatawriter);

            m_defaultObjectInfos.insert(AZStd::make_pair(objectName, jsonDataBuffer.GetString()));
        }
    }

    void EditorGeomNodesComponent::CreateDataElements(GNPropertyGroup& group)
    {
        ClearDataElements();

        // Create the combo box that will show the object names.
        CreateObjectNames(m_currentObject, m_enumValues, group);

        // Create the currently selected Object parameters and attributes. Load only the first or saved object.
        CreateParam(m_currentObject, group);

        AZ::SystemTickBus::QueueFunction(
            [=]() {
                EBUS_EVENT(AzToolsFramework::ToolsApplicationEvents::Bus, InvalidatePropertyDisplay, AzToolsFramework::Refresh_EntireTree);
                AzToolsFramework::ToolsApplicationRequests::Bus::Broadcast(
                    &AzToolsFramework::ToolsApplicationRequests::Bus::Events::AddDirtyEntity, GetEntityId());
            });
    }

    void EditorGeomNodesComponent::CreateObjectNames(const AZStd::string& objectName, const StringVector& enumValues, GNPropertyGroup& group)
    {
        ElementInfo ei;
        ei.m_editData.m_name = CacheString(Field::Objects);
        ei.m_editData.m_description = "";
        ei.m_editData.m_elementId = AZ::Edit::UIHandlers::ComboBox;
        ei.m_sortOrder = FLT_MAX;

        auto gnParam = aznew GNParamString(Field::Objects, "", &m_workInProgress, GetEntityId());
        gnParam->m_value = objectName;
        
        ei.m_editData.m_attributes.push_back(
            AZ::Edit::AttributePair(AZ::Edit::Attributes::StringList, aznew AZ::AttributeContainerType<StringVector>(enumValues)));

        group.m_properties.emplace_back(gnParam);

        AddDataElement(gnParam, ei);
    }

    void EditorGeomNodesComponent::CreateParam(const AZStd::string& objectName, GNPropertyGroup& group)
    {
        AZStd::string jsonBuffer;

        if (m_currentObjectInfo.empty())
        {
			auto it = m_defaultObjectInfos.find(objectName);
			if (it != m_defaultObjectInfos.end())
			{
				jsonBuffer = it->second;
			}
        }
        else {
			jsonBuffer = m_currentObjectInfo;
        }
        

        if (!jsonBuffer.empty())
        {
			rapidjson::Document jsonDocument;
			jsonDocument.Parse(jsonBuffer.c_str(), jsonBuffer.size());
			if (!jsonDocument.HasParseError())
			{
				GNPropertyGroup* subGroup = group.GetGroup(objectName.c_str());
				if (subGroup == nullptr)
				{
					group.m_groups.emplace_back();
					subGroup = &group.m_groups.back();
					subGroup->m_name = objectName;
				}
				LoadProperties(jsonDocument[Field::Params], *subGroup);
			}
        }
    }

    bool EditorGeomNodesComponent::LoadProperties(const rapidjson::Value& paramVal, GNPropertyGroup& group)
    {
        // parse params
        for (rapidjson::Value::ConstValueIterator itr = paramVal.Begin(); itr != paramVal.End(); ++itr)
        {
            //set this up so the context can do it's own parsing of the current GN param JSON object.
            GNParamDataContext gndc;
            gndc.SetParamObject(itr);
            gndc.SetReadOnlyPointer(&m_workInProgress);
            gndc.SetEntityId(GetEntityId());
            auto propertyName = gndc.GetParamName();
            auto paramType = gndc.GetParamType();
                
            // default value will differ based on the type

            if (GNProperty* gnParam = m_paramContext.ConstructGNParam(gndc, paramType, propertyName))
            {
                group.m_properties.emplace_back(gnParam);

                ElementInfo ei;
                ei.m_editData.m_name = CacheString(propertyName);
                ei.m_editData.m_description = "";
                ei.m_editData.m_elementId = AZ::Edit::UIHandlers::Default;
                ei.m_sortOrder = FLT_MAX;

                // Load any attributes
                LoadAttribute(paramType, ei.m_editData, group.m_properties.back());

                AddDataElement(gnParam, ei);
            }
            else
            {
                AZ_Warning("GeomNodes", false, "We support only boolean, number, and string as properties %s!", propertyName);
            }
        }

        return true;
    }

    void EditorGeomNodesComponent::LoadAttribute(ParamType type, AZ::Edit::ElementData& ed, GNProperty* prop)
    {
        switch (type)
        {
        case ParamType::Int:
            if (prop->m_isMinSet)
            {
                int value = ((GNParamInt*)prop)->m_min;
                ed.m_attributes.push_back(AZ::Edit::AttributePair(AZ::Crc32("min"), aznew AZ::Edit::AttributeData<int>(value)));
            }

            if (prop->m_isMaxSet)
            {
                int value = ((GNParamInt*)prop)->m_max;
                ed.m_attributes.push_back(AZ::Edit::AttributePair(AZ::Crc32("max"), aznew AZ::Edit::AttributeData<int>(value)));
            }
            break;
        case ParamType::Value:
            if (prop->m_isMinSet)
            {
                double value = ((GNParamValue*)prop)->m_min;
                ed.m_attributes.push_back(AZ::Edit::AttributePair(AZ::Crc32("min"), aznew AZ::Edit::AttributeData<double>(value)));
            }

            if (prop->m_isMaxSet)
            {
                double value = ((GNParamValue*)prop)->m_max;
                ed.m_attributes.push_back(AZ::Edit::AttributePair(AZ::Crc32("max"), aznew AZ::Edit::AttributeData<double>(value)));
            }

            ed.m_attributes.push_back(AZ::Edit::AttributePair(AZ::Edit::Attributes::Step, aznew AZ::Edit::AttributeData<double>(0.1f)));
            
            break;
        }
    }

	void EditorGeomNodesComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
	{
		//required.push_back(AZ_CRC("TransformService", 0x8ee22c50));
	}

	void EditorGeomNodesComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
	{
		dependent.push_back(AZ_CRC("TransformService", 0x8ee22c50));
	}

    void EditorGeomNodesComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("EditorGeomNodesService"));
    }

    void EditorGeomNodesComponent::GetIncompatibleServices(
        [[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC("EditorGeomNodesService"));
    }

    void EditorGeomNodesComponent::Init()
    {
    }

    void EditorGeomNodesComponent::Activate()
    {
        AzToolsFramework::Components::EditorComponentBase::Activate();
        EditorGeomNodesComponentRequestBus::Handler::BusConnect(GetEntityId());
        
        m_controller = AZStd::make_unique<GNMeshController>(GetEntityId());

        m_fromActivate = true;

        OnPathChange(m_blenderFile);
    }

    void EditorGeomNodesComponent::Deactivate()
    {
        // BUG: this gets called when a component is added so deal with it properly as it destroys any current instance we have.
        Clear();
        AzToolsFramework::Components::EditorComponentBase::Deactivate();

        if (m_instance)
        {
            delete m_instance;
            m_instance = nullptr;
        }
    }

    void EditorGeomNodesComponent::Clear()
    {
        m_controller.reset();
        m_enumValues.clear();
        ClearDataElements();
        EditorGeomNodesComponentRequestBus::Handler::BusDisconnect();
        Ipc::IpcHandlerNotificationBus::Handler::BusDisconnect(GetEntityId());
    }

    const AZ::Edit::ElementData* EditorGeomNodesComponent::GetParamsEditData(
        const void* handlerPtr, const void* elementPtr, const AZ::Uuid& elementType)
    {
        const EditorGeomNodesComponent* owner = reinterpret_cast<const EditorGeomNodesComponent*>(handlerPtr);
        return owner->GetDataElement(elementPtr, elementType);
    }

    void EditorGeomNodesComponent::AddDataElement(GNProperty* gnParam, ElementInfo& ei)
    {
        // add the attributes to the map of default values
        ei.m_uuid = gnParam->GetDataTypeUuid();
        ei.m_isAttributeOwner = true;
        m_dataElements.insert(AZStd::make_pair(gnParam->GetDataAddress(), ei));

        // Also register to the script property itself, so friendly data can be displayed at its own level.
        ei.m_uuid = azrtti_typeid(gnParam);
        ei.m_isAttributeOwner = false;
        m_dataElements.insert(AZStd::make_pair(gnParam, ei));
    }

    const char* EditorGeomNodesComponent::CacheString(const char* str)
    {
        if (str == nullptr)
        {
            return nullptr;
        }

        return m_cachedStrings.insert(AZStd::make_pair(str, AZStd::string(str))).first->second.c_str();
    }

    void EditorGeomNodesComponent::ClearDataElements()
    {
        for (auto it = m_dataElements.begin(); it != m_dataElements.end(); ++it)
        {
            if (it->second.m_isAttributeOwner)
            {
                it->second.m_editData.ClearAttributes();
            }
        }

        m_dataElements.clear();

        // The display tree might still be holding onto pointers to our attributes that we just cleared above, so force a refresh to remove
        // them. However, only force the refresh if we have a valid entity.  If we don't have an entity, this component isn't currently
        // being shown or edited, so a refresh is at best superfluous, and at worst could cause a feedback loop of infinite refreshes.
        if (GetEntity())
        {
            AZ::SystemTickBus::QueueFunction(
                [=]() {
                    AzToolsFramework::ToolsApplicationEvents::Bus::Broadcast(
                        &AzToolsFramework::ToolsApplicationEvents::InvalidatePropertyDisplay, AzToolsFramework::Refresh_EntireTree);
                });
        }
    }

    const AZ::Edit::ElementData* EditorGeomNodesComponent::GetDataElement(const void* element, const AZ::Uuid& typeUuid) const
    {
        auto it = m_dataElements.find(element);
        if (it != m_dataElements.end())
        {
            if (it->second.m_uuid == typeUuid)
            {
                return &it->second.m_editData;
            }
        }
        return nullptr;
    }
}
