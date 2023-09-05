#include "StdAfx.h"

#include <AzCore/std/containers/stack.h>

#include <HoudiniCommon.h>

#include <IIndexedMesh.h>
#include <AzCore/Math/MathUtils.h>

#include <AzCore/std/algorithm.h>  // FL[FD-11761] Add support for multiparm blocks (lists) to lumberyard houdini engine

namespace HoudiniEngine
{
    HoudiniNode::HoudiniNode(IHoudini* hou, HoudiniNodePtr parent, HAPI_NodeId id, AZStd::string operatorName, AZStd::string nodeName) :
        m_hou(hou)
        , m_nodeId(id)
        , m_parent(parent)
        , m_operatorName(operatorName)
        , m_nodeName(nodeName)
    {
        m_session = const_cast<HAPI_Session*>(&hou->GetSession());
        m_nodeInfo = HAPI_NodeInfo_Create();
        m_geoInfo = HAPI_GeoInfo_Create();
        m_assetInfo = HAPI_AssetInfo_Create();

        // FL[FD-11761] Add support for multiparm blocks (lists) to lumberyard houdini engine
        UpdateParamInfoFromEngine();
        UpdateEditableNodeFromEngine();
		m_hasEditableGeometryBuilt = false;
    }    

    // FL[FD-11761] Add support for multiparm blocks (lists) to lumberyard houdini engine
    void HoudiniNode::UpdateParamInfoFromEngine()
    {
        if (m_nodeId >= 0)
        {
            HAPI_GetNodeInfo(m_session, m_nodeId, &m_nodeInfo);
            HAPI_AssetInfo assetInfo = GetAssetInfo();
            
            if (assetInfo.helpTextSH > 0)
            {
                m_helpText = m_hou->GetString(assetInfo.helpTextSH);
            }

            if (m_nodeInfo.isValid)
            {
                if (m_nodeInfo.parmCount > 0)
                {
                    AZStd::vector<HAPI_ParmInfo> parms(m_nodeInfo.parmCount);

                    HAPI_GetParameters(m_session, m_nodeId, &parms[0], 0, m_nodeInfo.parmCount);

                    // Skip existing parameters
                    // Removing no more existing params
                    AZStd::set<HAPI_ParmId> existingParamIds;
                    for (HoudiniParameterPtr param : m_parameters)
                    {
                        existingParamIds.insert(param->GetId());
                    }

                    AZStd::set<HAPI_ParmId> newParamIds;
                    for (HAPI_ParmInfo parmInfo : parms)
                    {
                        newParamIds.insert(parmInfo.id);
                    }

                    m_parameters.erase(
                        AZStd::remove_if(m_parameters.begin(), m_parameters.end(), [&](HoudiniParameterPtr param) {
                            return newParamIds.find(param->GetId()) == newParamIds.end();
                        }),
                        m_parameters.end()
                    );

                    for (int i = 0; i < m_nodeInfo.parmCount; i++)
                    {
                        if (existingParamIds.find(parms[i].id) != existingParamIds.end())
                        {
                            HoudiniParameterPtr param{ nullptr };
                            for (auto exParam : m_parameters)
                            {
                                if (exParam->GetId() == parms[i].id)
                                {
                                    exParam->SetParmInfo(parms[i]);
                                }
                            }
                            continue;
                        }
                            
                        // Adding new params
                        HoudiniParameterPtr parm(new HoudiniParameter(m_hou, parms[i], this));
                        m_parameters.push_back(parm);
                    }
                }
            }
            else
            {
                AZ_Warning("HOUDINI", false, "[Operator: %s][Node: %s] - Node was not valid"
                    , m_operatorName.c_str()
                    , m_nodeName.c_str()
                );
            }
        }
    }

    void HoudiniNode::UpdateEditableNodeFromEngine()
    {
        m_editableGeoInfo.clear();
        if (m_nodeId >= 0)
        {
            if (!m_nodeInfo.isValid)
            {
                HAPI_GetNodeInfo(m_session, m_nodeId, &m_nodeInfo);
            }
            
            const bool bAssetHasChildren = !(m_nodeInfo.type == HAPI_NODETYPE_SOP && m_nodeInfo.childNodeCount == 0);
			AZ::s32 EditableNodeCount = 0;
			if (bAssetHasChildren)
			{
				HAPI_ComposeChildNodeList(m_session, m_nodeId, HAPI_NODETYPE_SOP, HAPI_NODEFLAGS_EDITABLE, true, &EditableNodeCount);
			}

			if (EditableNodeCount > 0)
			{
				AZStd::vector<HAPI_NodeId> EditableNodeIds;
				//EditableNodeIds.reserve(EditableNodeCount);
				EditableNodeIds.resize(EditableNodeCount);
				HAPI_GetComposedChildNodeList(m_session, m_nodeId, &EditableNodeIds[0], EditableNodeCount);

				for (int32 nEditable = 0; nEditable < EditableNodeCount; nEditable++)
				{
					HAPI_GeoInfo CurrentEditableGeoInfo;
					HAPI_GeoInfo_Init(&CurrentEditableGeoInfo);
					HAPI_GetGeoInfo(m_session, EditableNodeIds[nEditable], &CurrentEditableGeoInfo);

					// TODO: Check whether this display geo is actually being output
					//       Just because this is a display node doesn't mean that it will be output (it
					//       might be in a hidden subnet)

					// Do not process the main display geo twice!
					if (CurrentEditableGeoInfo.isDisplayGeo)
						continue;

					// We only handle editable curves for now
					if (CurrentEditableGeoInfo.type != HAPI_GEOTYPE_CURVE)
						continue;

                    //HACK: 
                    if (CurrentEditableGeoInfo.partCount <= 0)
                    {
                        m_hou->CookNode(CurrentEditableGeoInfo.nodeId, "");
                        HAPI_GetGeoInfo(m_session, CurrentEditableGeoInfo.nodeId, &CurrentEditableGeoInfo);
                    }


					// Add this geo to the geo info array
					m_editableGeoInfo.push_back(CurrentEditableGeoInfo);
				}
			}
        }
    }

    HoudiniNode::~HoudiniNode()
    {
    }

    HoudiniNodePtr HoudiniNode::CreateNode(const AZStd::string& operatorName, const AZStd::string& nodeName)
    {
        HoudiniNodePtr node = m_hou->CreateNode(operatorName, nodeName, this);
        return node;
    }

    HoudiniNodePtr HoudiniNode::CreateCurve(const AZStd::string& nodeName)
    {
        HoudiniNodePtr node = m_hou->CreateNode("sop/curve", nodeName, this);
        return node;
    }

    AZ::Transform HoudiniNode::GetObjectTransform()
    {
        HAPI_ObjectInfo info = HAPI_ObjectInfo_Create();
        HAPI_NodeInfo nodeInfo = GetNodeInfo();

        HAPI_GetObjectInfo(m_session, nodeInfo.id, &info);

        //There is a bug in HARS that returns invalid info for this object, so we check for 0 or -1 here.
        if (info.nodeId <= HOUDINI_ZERO_ID)
        {
            HAPI_GetObjectInfo(m_session, nodeInfo.parentId, &info);
        }


        HAPI_Transform transformHapi = HAPI_Transform_Create();
        HAPI_GetObjectTransform(m_session, info.nodeId, HOUDINI_INVALID_ID, HAPI_RSTOrder::HAPI_RSTORDER_DEFAULT, &transformHapi);

        float* quat = transformHapi.rotationQuaternion;

        AZ::Quaternion qRot(quat[0], quat[1], quat[2], quat[3]);
        AZ::Vector3 pos(transformHapi.position[0], transformHapi.position[1], transformHapi.position[2]);
        AZ::Vector3 scale(transformHapi.scale[0], transformHapi.scale[1], transformHapi.scale[2]);

        //TODO: convert this to Matrix4x4 to Transform
        /*AZ::Transform transform = AZ::Transform::CreateFromQuaternionAndTranslation(qRot, pos);
        AZ::Transform scaleMat = AZ::Matrix3x3::CreateScale(scale);

        return transform * scaleMat;*/
        return AZ::Transform();
    }

    void HoudiniNode::SetObjectTransform(const AZ::Transform& transform)
    {
        HAPI_ObjectInfo info = HAPI_ObjectInfo_Create();
        HAPI_NodeInfo nodeInfo = GetNodeInfo();

        HAPI_GetObjectInfo(m_session, nodeInfo.id, &info);

        //There is a bug in HARS that returns invalid info for this object, so we check for 0 or -1 here.
        if (info.nodeId <= HOUDINI_ZERO_ID)
        {
            HAPI_GetObjectInfo(m_session, nodeInfo.parentId, &info);
        }

        const AZ::Vector3& pos = transform.GetTranslation();
        //TODO: convert
        //AZ::Vector3 scale = transform.GetScale();

        ////Lumberyard returns scaled Euler degrees so we must first remove the scale:
        //if ((float)scale.GetLength() < AZ::Constants::FloatEpsilon)
        //{
        //    scale = AZ::Vector3::CreateOne();
        //}

        //AZ::Transform uniformTransform = transform * AZ::Transform::CreateScale(AZ::Vector3::CreateOne() / scale);
        //AZ::Vector3 angles = uniformTransform.GetEulerDegrees();
        AZ::Vector3 scale;
        AZ::Vector3 angles;

        HAPI_TransformEuler transformHapi = HAPI_TransformEuler_Create();
        transformHapi.position[0] = (float)pos.GetX();
        transformHapi.position[1] = (float)pos.GetY();
        transformHapi.position[2] = (float)pos.GetZ();

        transformHapi.rotationEuler[0] = angles.GetX();
        transformHapi.rotationEuler[1] = angles.GetY();
        transformHapi.rotationEuler[2] = angles.GetZ();

        transformHapi.scale[0] = scale.GetX();
        transformHapi.scale[1] = scale.GetY();
        transformHapi.scale[2] = scale.GetZ();

        float previousMatrix[16];
        float currentMatrix[16];

        HAPI_Transform previousTransform = HAPI_Transform_Create();        
        HAPI_GetObjectTransform(m_session, info.nodeId, -1, HAPI_RSTOrder::HAPI_RSTORDER_DEFAULT, &previousTransform);
        
        HAPI_ConvertTransformQuatToMatrix(m_session, &previousTransform, previousMatrix);
        HAPI_ConvertTransformEulerToMatrix(m_session, &transformHapi, currentMatrix);
        
        bool different = false;
        for (int i = 0; i < 16; i++)
        {
            float a = previousMatrix[i];
            float b = currentMatrix[i];            
            if (fabs(a - b) > 0.0001f)
            {
                different = true;
                break;
            }
        }

        if (different)
        {
            transformHapi.rotationOrder = HAPI_XYZOrder::HAPI_ZYX; 
            transformHapi.rstOrder = HAPI_RSTOrder::HAPI_RSTORDER_DEFAULT;
            HAPI_SetObjectTransform(m_session, info.nodeId, &transformHapi);
        }
    }

    const AZStd::string HoudiniNode::GetNodePath()
    {
        auto info = GetNodeInfo();
        
        AZStd::stack<HAPI_NodeInfo> stack;
        stack.push(info);

        AZStd::string path;

        while (!stack.empty())
        {
            HAPI_NodeInfo& top = stack.top();
            stack.pop();

            if (top.type != HAPI_NODETYPE_NONE)
            {
                path = "/" + GetHou()->GetString(top.nameSH) + path;
            }

            if (top.parentId != HOUDINI_ROOT_NODE_ID)
            {
                HAPI_NodeInfo parent;
                HAPI_GetNodeInfo(&m_hou->GetSession(), top.parentId, &parent);
                stack.push(parent);
            }
        }

        return path;
    }

    void HoudiniNode::SetInputEntity(const AZStd::string& name, const AZ::EntityId& entityId)
    {
        m_hou->ExecuteCommand(m_entityId, [this, name, entityId]
        {
            for (int i = 0; i < GetNodeInfo().inputCount; i++)
            {
                HAPI_StringHandle nameHandle;
                HAPI_GetNodeInputName(m_session, GetId(), i, &nameHandle);
                AZStd::string inputName = m_hou->GetString(nameHandle);

                if (inputName == name)
                {
                    SetInputEntity(i, entityId);
                    return true;
                }
            }

            return false;
        });
    }

    void HoudiniNode::SetInputEntity(int index, const AZ::EntityId& entityId)
    {
        m_hou->ExecuteCommand(m_entityId, [this, index, entityId]
        {
            if (entityId.IsValid() == false)
            {
                HAPI_DisconnectNodeInput(m_session, GetId(), index);
                SetDirty();
                return false;
            }

            HAPI_NodeId valueId = m_hou->GetInputNodeManager()->GetNodeIdFromEntity(entityId);
            if (valueId == HOUDINI_NOT_READY_ID)
            {
                //In this case, the node is valid, but its just not ready yet.  Return true to signal to try again later.
                valueId = HOUDINI_INVALID_ID;
                m_retryDependentInput[index] = entityId;
            }

            if (valueId != HOUDINI_INVALID_ID)
            {
                HAPI_ConnectNodeInput(m_session, GetId(), index, valueId, 0);
            }
            else
            {
                HAPI_DisconnectNodeInput(m_session, GetId(), index);
            }

            if (m_hou->CheckForErrors(true, false /*NOT IncludeCookingErrors*/))
            {
                AZStd::string entityName = m_hou->LookupEntityName(entityId);

                AZ_Warning("HOUDINI", false, "[Entity: %s][Node: %s] - Error setting input to %s"
                    , (entityName + " " + entityId.ToString()).c_str()
                    , GetNodeName().c_str()
                    , entityId.ToString().c_str()
                );

                //TODO: This seems to be okay even though it gives an error...
                //HAPI_ConnectNodeInput(m_session, GetId(), index, HOUDINI_INVALID_ID);
                return false;
            }

            SetDirty();
            return true;
        });
    }

    void HoudiniNode::Cook()
    {
        if (m_hou != nullptr)
        {

            //Check to see if we have any inputs that were missing earlier that might be ready now!
            if (m_retryDependentInput.empty() == false)
            {
                AZStd::vector<int> keysFound;

                for (auto& retryEntity : m_retryDependentInput)
                {
                    HAPI_NodeId valueId = m_hou->GetInputNodeManager()->GetNodeIdFromEntity(retryEntity.second);
                    if (valueId != HOUDINI_NOT_READY_ID)
                    {
                        keysFound.push_back(retryEntity.first);
                        SetInputEntity(retryEntity.first, retryEntity.second);
                    }
                }

                for (auto& key : keysFound)
                {
                    m_retryDependentInput.erase(key);
                }
            }

            AZStd::string entityName = m_hou->LookupEntityName(m_entityId);

            m_hasCookingError = false;
            m_lastCookError = "";

            AZStd::string error = m_hou->GetLastHoudiniCookError();
            m_hou->CookNode(GetNodeInfo().id, entityName);
            m_hou->CheckForErrors(false);

            error = m_hou->GetLastHoudiniCookError();
            if (error.length() > 0)
            {
                m_hasCookingError = true;
                m_lastCookError = error;
            }
        }
    }

    bool HoudiniNode::UpdateData()
    {        
        Cook();

        if (m_hasCookingError)
        {
            //Print only once when the error begins.
            if (m_hasCookingErrorBefore == false || m_previousError != m_lastCookError)
            {
                //TODO: The error is already printed, but maybe one day we want something else here?
            }
        }

        m_previousError = m_lastCookError;
        m_hasCookingErrorBefore = m_hasCookingError;

        HAPI_GeoInfo geometryInfo = GetGeometryInfo();
        return geometryInfo.hasGeoChanged;
    }

    AZStd::vector<AZStd::string> HoudiniNode::GetInputs()
    {
        AZStd::vector<AZStd::string> output;

        for (int i = 0; i < GetNodeInfo().inputCount; i++)
        {
            HAPI_StringHandle nameHandle;
            HAPI_GetNodeInputName(m_session, GetId(), i, &nameHandle);
            AZStd::string inputName = m_hou->GetString(nameHandle);

            output.push_back(inputName);
        }

        return output;
    }

    HoudiniParameterPtr HoudiniNode::GetParameter(AZStd::string name)
    {
        for (auto param : m_parameters)
        {
            if (param->GetName() == name)
            {
                return param;
            }
        }

        return nullptr;
    }

    AZStd::vector<int> HoudiniNode::GetIntPoints(const AZStd::string& attributeName)
    {
        AZStd::vector<int> output;
        //Refresh the info:
        HAPI_NodeId node_id = GetGeometryInfo().nodeId;

        const AZStd::vector<HAPI_PartInfo>& parts = GetGeometryParts();
        for (auto partInfo : parts)
        {
            HAPI_AttributeInfo attr_info;
            attr_info.exists = false;
            HAPI_GetAttributeInfo(m_session, node_id, partInfo.id, attributeName.c_str(), HAPI_ATTROWNER_POINT, &attr_info);
            m_hou->CheckForErrors();

            if (attr_info.exists && attr_info.tupleSize == 1)
            {
                output.reserve(output.size() + attr_info.count);

                AZStd::vector<int> v = AZStd::vector<int>(attr_info.count * attr_info.tupleSize);
                HAPI_GetAttributeIntData(m_session, node_id, partInfo.id, attributeName.c_str(), &attr_info, attr_info.tupleSize, &v[0], 0, attr_info.count);
                m_hou->CheckForErrors();

                for (int i = 0; i < attr_info.count; ++i)
                {                    
                    output.push_back(v[i]);
                }
            }
        }

        return output;
    }

    AZStd::vector<float> HoudiniNode::GetFloatPoints(const AZStd::string& attributeName)
    {
        AZStd::vector<float> output;
        //Refresh the info:
        HAPI_NodeId node_id = GetGeometryInfo().nodeId;

        const AZStd::vector<HAPI_PartInfo>& parts = GetGeometryParts();
        for (auto partInfo : parts)
        {
            HAPI_AttributeInfo attr_info;
            attr_info.exists = false;
            HAPI_GetAttributeInfo(m_session, node_id, partInfo.id, attributeName.c_str(), HAPI_ATTROWNER_POINT, &attr_info);
            m_hou->CheckForErrors();

            if (attr_info.exists && attr_info.tupleSize == 1)
            {
                output.reserve(output.size() + attr_info.count);

                AZStd::vector<float> v = AZStd::vector<float>(attr_info.count * attr_info.tupleSize);
                HAPI_GetAttributeFloatData(m_session, node_id, partInfo.id, attributeName.c_str(), &attr_info, attr_info.tupleSize, &v[0], 0, attr_info.count);
                m_hou->CheckForErrors();

                for (int i = 0; i < attr_info.count; ++i)
                {
                    output.push_back(v[i]);
                }
            }
        }

        return output;
    }

    AZStd::vector<AZ::Matrix3x3> HoudiniNode::GetMatrix3Points(const AZStd::string& attributeName)
    {
        AZStd::vector<AZ::Matrix3x3> output;
        //Refresh the info:
        HAPI_NodeId node_id = GetGeometryInfo().nodeId;

        const AZStd::vector<HAPI_PartInfo>& parts = GetGeometryParts();
        for (auto partInfo : parts)
        {
            HAPI_AttributeInfo attr_info;
            attr_info.exists = false;
            HAPI_GetAttributeInfo(m_session, node_id, partInfo.id, attributeName.c_str(), HAPI_ATTROWNER_POINT, &attr_info);
            m_hou->CheckForErrors();

            if (attr_info.exists && attr_info.tupleSize == 9)
            {
                output.reserve(output.size() + attr_info.count);

                AZStd::vector<float> v = AZStd::vector<float>(attr_info.count * attr_info.tupleSize);
                HAPI_GetAttributeFloatData(m_session, node_id, partInfo.id, attributeName.c_str(), &attr_info, attr_info.tupleSize, &v[0], 0, attr_info.count);
                m_hou->CheckForErrors();

                for (int i = 0; i < attr_info.count; ++i)
                {
                    AZ::Vector3 r0(v[i * 9 + 0], v[i * 9 + 1], v[i * 9 + 2]);
                    AZ::Vector3 r1(v[i * 9 + 3], v[i * 9 + 4], v[i * 9 + 5]);
                    AZ::Vector3 r2(v[i * 9 + 6], v[i * 9 + 7], v[i * 9 + 8]);

                    AZ::Matrix3x3 point;
                    point.SetRows(r0, r1, r2);
                    output.push_back(point);
                }
            }
        }

        return output;
    }

    AZStd::vector<AZ::Vector3> HoudiniNode::GetGeometryPoints()
    {
        AZStd::vector<AZ::Vector3> output;

        //Refresh the info:
        HAPI_NodeId node_id = GetGeometryInfo().nodeId;

        const AZStd::vector<HAPI_PartInfo>& parts = GetGeometryParts();
        for (auto partInfo : parts)
        {
            HAPI_AttributeInfo attr_info;
            attr_info.exists = false;
            HAPI_GetAttributeInfo(m_session, node_id, partInfo.id, "P", HAPI_ATTROWNER_POINT, &attr_info);
            m_hou->CheckForErrors();

            if (attr_info.exists)
            {
                output.reserve(output.size() + attr_info.count);

                AZStd::vector<float> v = AZStd::vector<float>(attr_info.count * attr_info.tupleSize);
                HAPI_GetAttributeFloatData(m_session, node_id, partInfo.id, "P", &attr_info, attr_info.tupleSize, &v[0], 0, attr_info.count);
                m_hou->CheckForErrors();

                for (int i = 0; i < attr_info.count; ++i)
                {
                    AZ::Vector3 point(v[i * 3 + 0], v[i * 3 + 1], v[i * 3 + 2]);
                    output.push_back(point);
                }
            }
        }

        return output;
    }

    AZStd::vector<AZ::Vector3> HoudiniNode::GetGeometryPointGroup(const AZStd::vector<AZStd::string>& groupNames)
    {
        AZStd::vector<AZ::Vector3> output;

        //Refresh the info:
        HAPI_NodeId node_id = GetGeometryInfo().nodeId;

        const AZStd::vector<HAPI_PartInfo>& parts = GetGeometryParts();
        for (auto partInfo : parts)
        {
            HAPI_AttributeInfo attr_info;
            attr_info.exists = false;
            HAPI_GetAttributeInfo(m_session, node_id, partInfo.id, "P", HAPI_ATTROWNER_POINT, &attr_info);
            m_hou->CheckForErrors();

            if (attr_info.exists)
            {
                output.reserve(output.size() + attr_info.count);

                AZStd::vector<AZStd::vector<int>> membershipMaps;
                for (auto& groupName : groupNames)
                {
                    membershipMaps.push_back(AZStd::vector<int>());

                    AZStd::vector<int>& membershipData = membershipMaps.back();
                    membershipData.resize(attr_info.count);

                    bool allEqual = true;
                    HAPI_GetGroupMembership(m_session, node_id, partInfo.id, HAPI_GROUPTYPE_POINT, groupName.c_str(), &allEqual, membershipData.data(), 0, attr_info.count);
                }
                
                AZStd::vector<float> v = AZStd::vector<float>(attr_info.count * attr_info.tupleSize);
                HAPI_GetAttributeFloatData(m_session, node_id, partInfo.id, "P", &attr_info, attr_info.tupleSize, &v[0], 0, attr_info.count);
                m_hou->CheckForErrors();

                for (int i = 0; i < attr_info.count; ++i)
                {
                    bool inAllGroups = true;
                    for (auto& membershipData : membershipMaps)
                    {
                        inAllGroups = inAllGroups && membershipData[i];
                    }

                    if (inAllGroups)
                    {
                        AZ::Vector3 point(v[i * 3 + 0], v[i * 3 + 1], v[i * 3 + 2]);
                        output.push_back(point);
                    }
                }
            }
        }

        return output;
    }

}
