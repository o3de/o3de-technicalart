#include "Editor/Components/EditorGeomNodesComponent.h"
#include "Editor/Components/EditorGeomNodesMeshComponent.h"
#include <Editor/EBus/EditorGeomNodesMeshComponentBus.h>
#include "Editor/UI/UI_common.h"
#include "Editor/UI/Validators.h"
#include "Editor/UI/Utils.h"

#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzToolsFramework/API/EntityCompositionRequestBus.h>
#include <AtomLyIntegration/CommonFeatures/Material/MaterialComponentConstants.h>
#include <AzCore/Utils/Utils.h>
#include <Editor/Systems/GNProperty.h>
#include <AzCore/JSON/prettywriter.h>
#include <AzCore/JSON/stringbuffer.h>
#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
#include <Atom/RPI.Public/Scene.h>
#include <AzCore/Component/NonUniformScaleBus.h>

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
                ->Field("BlenderFile", &EditorGeomNodesComponent::m_blenderFile);
            
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
                    ->DataElement(nullptr, &EditorGeomNodesComponent::m_paramContext, "Geom Nodes Parameters", "Parameter template")
                    ->SetDynamicEditDataProvider(&EditorGeomNodesComponent::GetParamsEditData)
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
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
                    ->DataElement(nullptr, &GNParamBoolean::m_value, "m_value", "A boolean")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &EditorGeomNodesComponent::OnParamChange)
                    ->Attribute(AZ::Edit::Attributes::NameLabelOverride, &GNParamBoolean::m_name);

                ec->Class<GNParamInt>("Geom Nodes Property (int)", "A Geom Nodes int property")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "GNPropertyGroup's class attributes.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->DataElement(nullptr, &GNParamInt::m_value, "m_value", "An int")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &EditorGeomNodesComponent::OnParamChange)
                    ->Attribute(AZ::Edit::Attributes::NameLabelOverride, &GNParamInt::m_name);

                ec->Class<GNParamValue>("Geom Nodes Property (double)", "A Geom Nodes double property")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "GNPropertyGroup's class attributes.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->DataElement(nullptr, &GNParamValue::m_value, "m_value", "A double/value")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &EditorGeomNodesComponent::OnParamChange)
                        ->Attribute(AZ::Edit::Attributes::NameLabelOverride, &GNParamValue::m_name);

                ec->Class<GNParamString>("Geom Nodes Property (string)", "A Geom Nodes string property")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "GNPropertyGroup's class attributes.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->DataElement(nullptr, &GNParamString::m_value, "m_value", "A string")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &EditorGeomNodesComponent::OnParamChange)
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
            }

            if (!m_instance || bClearParams || (m_instance && !m_instance->IsValid()))
            {
                if (m_instance)
                    delete m_instance;

                const AZ::IO::FixedMaxPath gemPath = AZ::Utils::GetGemPath("GeomNodes");
                const AZ::IO::FixedMaxPath exePath = AZ::Utils::GetExecutableDirectory();
                AZStd::string scriptPath = AZStd::string::format(R"(%s\External\Scripts\__init__.py)", gemPath.c_str());

                m_instance = new GNInstance;
                m_instance->Init(path, scriptPath, exePath.c_str(), GetEntityId());
                if (m_instance->IsValid())
                {
                    //AZ::EntityId entityId = AZ::EntityId(123456);
                    auto entityId = GetEntityId();
                    Ipc::IpcHandlerNotificationBus::Handler::BusConnect(entityId);
                }

                AzFramework::StringFunc::Path::GetFileName(path.c_str(), m_currentBlenderFileName);
            }
        }
    }

    void EditorGeomNodesComponent::OnParamChange()
    {
        auto gnParam = reinterpret_cast<GNParamString*>(m_paramContext.m_group.GetProperty(Field::Objects));
        if (gnParam->m_value != m_currentObject)
        {
            m_currentObject = gnParam->m_value;
            m_paramContext.m_group.Clear(); // clear the group/properties
            CreateDataElements(m_paramContext.m_group);

            // TODO: send a message to blender since we need to update the object rendered
        }
        else
        {
            if(m_instance && !m_instance->IsValid())
            {
                m_instance->RestartProcess();
            }

            auto msg = AZStd::string::format(
                R"JSON(
                    {
                        "%s": [ %s ],
                        "%s": "%s",
                        "Frame": 0,
                        "ParamUpdate": true
                    }
                )JSON"
                , Field::Params
                , m_paramContext.m_group
                    .GetGroup(m_currentObject.c_str())
                    ->GetProperties().c_str()
                , Field::Object
                , m_currentObject.c_str()
            );

            m_instance->SendIPCMsg(msg);
        }

        AZ_Printf("EditorGeomNodesComponent", "Parameter has changed");
    }

    void EditorGeomNodesComponent::OnMessageReceived(const AZ::u8* content, const AZ::u64 length)
    {
        rapidjson::Document jsonDocument;
        jsonDocument.Parse((const char*)content, length);
        if (!jsonDocument.HasParseError())
        {
            //TODO: need to put these hard coded text into one place
            if (jsonDocument.HasMember(Field::Initialized))
            {
                AZStd::string msg;
                if (!m_initialized)
                {
                    msg = R"JSON(
                        {
                            "FetchObjectParams": true 
                        }
                    )JSON";
                    m_initialized = true;
                }
                else
                {
                    // send messages that are queued.
                }
                m_instance->SendIPCMsg(msg);
            }
            else if (jsonDocument.HasMember(Field::ObjectNames) && jsonDocument.HasMember(Field::Objects) && jsonDocument.HasMember(Field::Materials))
            {
                LoadObjects(jsonDocument[Field::ObjectNames], jsonDocument[Field::Objects]);
                CreateDataElements(m_paramContext.m_group);

                // Send message for fetching the 3D data
                // Will just call OnParamChange since it's basically the same request
                OnParamChange();

                // Handle the materials as well
                LoadMaterials(jsonDocument[Field::Materials]);
            }
            else if (jsonDocument.HasMember(Field::SHMOpen) && jsonDocument.HasMember(Field::MapId))
            {
                
                AZ::u64 mapId = jsonDocument[Field::MapId].GetInt64();
                m_modelData.ReadData(mapId);
                auto msg = AZStd::string::format(
                    R"JSON(
                    {
                        "%s": true,
                        "%s": %llu
                    }
                    )JSON",
                    Field::SHMClose,
                    Field::MapId,
                    mapId);
                m_instance->SendIPCMsg(msg);

                m_manageChildEntities = true; // tell OnTick that we want to manage the child entities
            }
        }
        else
        {
            AZ_Warning("EditorGeomNodesComponent", false, "Message is not in JSON format!");
        }
    }

    void EditorGeomNodesComponent::LoadObjects(const rapidjson::Value& objectNameArray, const rapidjson::Value& objectArray)
    {
        
        // Populate m_enumValues that will store the list of object names
        LoadObjectNames(objectNameArray);
        
        // Load and save our param list object from json. Need this so it's faster to switch between objects and not need to send a request via IPC.
        LoadParams(objectArray);

        // TODO: load from save point if any
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

            m_objectInfos.insert(AZStd::make_pair(objectName, jsonDataBuffer.GetString()));
        }
    }

    void EditorGeomNodesComponent::CreateDataElements(GNPropertyGroup& group)
    {
        ClearDataElements();

        // Create the combo box that will show the object names.
        CreateObjectNames(m_enumValues, group);

        // Create the currently selected Object parameters and attributes. Load only the first or saved object.
        CreateParam(m_currentObject, group);

        EBUS_EVENT(AzToolsFramework::ToolsApplicationEvents::Bus, InvalidatePropertyDisplay, AzToolsFramework::Refresh_EntireTree);
        AzToolsFramework::ToolsApplicationRequests::Bus::Broadcast(
            &AzToolsFramework::ToolsApplicationRequests::Bus::Events::AddDirtyEntity, GetEntityId());
    }

    void EditorGeomNodesComponent::CreateObjectNames(const StringVector& enumValues, GNPropertyGroup& group)
    {
        ElementInfo ei;
        ei.m_editData.m_name = CacheString(Field::Objects);
        ei.m_editData.m_description = "";
        ei.m_editData.m_elementId = AZ::Edit::UIHandlers::ComboBox;
        ei.m_sortOrder = FLT_MAX;

        auto gnParam = aznew GNParamString(Field::Objects, "");
        gnParam->m_value = m_currentObject;
        
        ei.m_editData.m_attributes.push_back(
            AZ::Edit::AttributePair(AZ::Edit::Attributes::StringList, aznew AZ::AttributeContainerType<StringVector>(enumValues)));

        group.m_properties.emplace_back(gnParam);

        AddDataElement(gnParam, ei);
    }

    void EditorGeomNodesComponent::CreateParam(const AZStd::string& objectName, GNPropertyGroup& group)
    {
        auto it = m_objectInfos.find(objectName);
        if (it != m_objectInfos.end())
        {
            rapidjson::Document jsonDocument;
            jsonDocument.Parse((const char*)it->second.c_str(), it->second.size());
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

        //TODO: parse materials

        //// Remove all old properties, every confirmed property will have
        //// a corresponding Element data
        // RemovedOldProperties(m_scriptComponent.m_properties);

        // SortProperties(m_scriptComponent.m_properties);
        
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
            break;
        }
    }

    void EditorGeomNodesComponent::LoadMaterials(const rapidjson::Value& materialArray)
    {
        m_modelData.SetMaterialPathFormat(""); // reset the material path format
        // iterate through the material arrays and write them into files.
		for (rapidjson::Value::ConstValueIterator  itr = materialArray.Begin(); itr != materialArray.End(); ++itr)
		{
            const auto matItr = itr->MemberBegin();
            AZStd::string materialName = matItr->name.GetString();
            AZStd::string materialContent = matItr->value.GetString();
			
            AZStd::string fullFilePath = GetProjectRoot() + "/";
            AZStd::string materialFilePath = AZStd::string::format(MaterialFilePathFormat.data(), m_currentBlenderFileName.c_str());
            
            fullFilePath += materialFilePath + materialName + MaterialExtension.data();

            AZ::Utils::WriteFile(materialContent, fullFilePath.c_str());

            // re-use materialFilePath and just append the azmaterial extension
            m_modelData.SetMaterialPathFormat(materialFilePath + "%s" + AzMaterialExtension.data());
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
		AZ::TickBus::Handler::BusConnect();
    }

    void EditorGeomNodesComponent::Deactivate()
    {
        // BUG: this gets called when a component is added so deal with it properly as it destroys any current instance we have.
        Clear();
        AZ::TickBus::Handler::BusDisconnect();
        AzToolsFramework::Components::EditorComponentBase::Deactivate();

        if (m_instance)
        {
            delete m_instance;
            m_instance = nullptr;
        }
    }

    void EditorGeomNodesComponent::OnTick(float /*deltaTime*/, AZ::ScriptTimePoint /*time*/)
    {
        if (m_manageChildEntities)
        {
            ManageChildEntities();
            m_manageChildEntities = false;
        }
    }

    GNMeshData EditorGeomNodesComponent::GetMeshData(AZ::u64 entityId)
    {
        return m_modelData.GetMeshData(entityId);
    }

    void EditorGeomNodesComponent::Clear()
    {
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

    void EditorGeomNodesComponent::ManageChildEntities()
    {
		AzToolsFramework::EntityIdList entityIdList;

		AZ::s32 entityCount = m_modelData.MeshCount() - m_entityIdList.size();
		if (entityCount > 0)
		{
			for ([[maybe_unused]] AZ::s32 i = 0; i < entityCount; i++)
			{
				AZ::EntityId entityId;
				EBUS_EVENT_RESULT(entityId, AzToolsFramework::EditorRequests::Bus, CreateNewEntity, GetEntityId());

				entityIdList.push_back(entityId);
			}

			AzToolsFramework::EntityCompositionRequests::AddComponentsOutcome addedComponentsResult = AZ::Failure(AZStd::string("Failed to call AddComponentsToEntities on EntityCompositionRequestBus"));
			AzToolsFramework::EntityCompositionRequestBus::BroadcastResult(addedComponentsResult, &AzToolsFramework::EntityCompositionRequests::AddComponentsToEntities, entityIdList, AZ::ComponentTypeList{ /*AZ::Render::EditorMaterialComponentTypeId, */azrtti_typeid<EditorGeomNodesMeshComponent>() });

			if (addedComponentsResult.IsSuccess())
			{
				AzToolsFramework::ToolsApplicationEvents::Bus::Broadcast(&AzToolsFramework::ToolsApplicationEvents::InvalidatePropertyDisplay, AzToolsFramework::Refresh_EntireTree_NewContent);
			}

			m_entityIdList.insert(m_entityIdList.begin(), entityIdList.begin(), entityIdList.end());
		}
		else if (entityCount < 0)
		{
			entityCount *= -1; // flipping the sign so we can use it

			for ([[maybe_unused]] AZ::s32 i = 0; i < entityCount; i++) {
				entityIdList.insert(entityIdList.begin(), m_entityIdList.back());
				m_entityIdList.pop_back();
			}

			AzToolsFramework::ToolsApplicationRequestBus::Broadcast(&AzToolsFramework::ToolsApplicationRequests::DeleteEntities, entityIdList);
		}

		// assign the mesh data to the entityId
		for (auto entityId : m_entityIdList)
		{
			m_modelData.AssignMeshData((AZ::u64)entityId);
			EditorGeomNodesMeshComponentEventBus::Event(entityId, &EditorGeomNodesMeshComponentEvents::OnMeshDataAssigned, m_modelData.GetMeshData((AZ::u64)entityId));
		}

        
        AzToolsFramework::ToolsApplicationRequestBus::Broadcast(&AzToolsFramework::ToolsApplicationRequests::MarkEntitiesDeselected, m_entityIdList);
        AzToolsFramework::ToolsApplicationRequestBus::Broadcast(&AzToolsFramework::ToolsApplicationRequests::MarkEntitySelected, GetEntityId());
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
            AzToolsFramework::ToolsApplicationEvents::Bus::Broadcast(
                &AzToolsFramework::ToolsApplicationEvents::InvalidatePropertyDisplay, AzToolsFramework::Refresh_EntireTree);
        }
    }

    const AZ::Edit::ElementData* EditorGeomNodesComponent::GetDataElement(
        [[maybe_unused]] const void* element, [[maybe_unused]] const AZ::Uuid& typeUuid) const
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