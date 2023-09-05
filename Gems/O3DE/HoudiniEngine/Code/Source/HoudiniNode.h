#pragma once

#include <HAPI/HAPI.h>

//Cry Includes:
#include <IIndexedMesh.h>
#include <ISystem.h>
#include <ILevelSystem.h>
//#include <IGame.h> // O3DECONVERT
//#include <IGameFramework.h>

//Houdini Includes:
#include <HoudiniEngine/HoudiniApi.h>

//#include <IEntityRenderState.h>

namespace HoudiniEngine
{
    class HoudiniNode
        : public IHoudiniNode        
    {
        friend class Houdini;
        friend class HoudiniParameter;
        friend class HoudiniGeometry;

        protected:
            int m_deleteCount = 0;
            IHoudini * m_hou;
            HAPI_Session* m_session;
            HAPI_NodeId m_nodeId;
            HAPI_NodeInfo m_nodeInfo;
            HAPI_GeoInfo m_geoInfo;
			HAPI_AssetInfo m_assetInfo;

            AZ::EntityId m_entityId;

            HoudiniNodePtr m_parent;
            AZStd::vector<HAPI_PartInfo> m_geomParts;

            bool m_isGeometryCached = false;
            bool m_hasCookingError = false;
            bool m_hasCookingErrorBefore = false;

            AZStd::string m_previousError;
            AZStd::string m_lastCookError;
            AZStd::vector<AZStd::string> m_geomPaths;
            AZStd::string m_operatorName;
            AZStd::string m_nodeName;
            AZStd::string m_helpText;

            AZStd::vector<HoudiniParameterPtr> m_parameters;
            AZStd::vector<HoudiniParameterPtr> m_parameterInputs;            
            AZStd::vector<HAPI_NodeId> m_children;        
            AZStd::vector<HAPI_GeoInfo> m_editableGeoInfo;
            bool m_hasEditableGeometryBuilt = false;
            AZStd::unordered_map<int, AZ::EntityId> m_retryDependentInput;

            HoudiniNode(IHoudini* hou, HoudiniNodePtr parent, HAPI_NodeId id, AZStd::string operatorName, AZStd::string nodeName);
                        
        public:
            
            virtual ~HoudiniNode();

            //Houdini API Stuff:
            void Cook() override;
            bool HasCookingError() override { return m_hasCookingError; }
            HoudiniNodePtr CreateNode( const AZStd::string& operatorName, const AZStd::string& nodeName) override;
            HoudiniNodePtr CreateCurve(const AZStd::string& nodeName) override;

            AZStd::vector<int> GetIntPoints(const AZStd::string& attributeName) override;
            AZStd::vector<float> GetFloatPoints(const AZStd::string& attributeName) override;
            AZStd::vector<AZ::Vector3> GetGeometryPoints() override;
            AZStd::vector<AZ::Vector3> GetGeometryPointGroup(const AZStd::vector<AZStd::string>& groupNames) override;
            AZStd::vector<AZ::Matrix3x3> GetMatrix3Points(const AZStd::string& attributeName) override;

            //Builds the data
            bool UpdateData() override;
            void UpdateParamInfoFromEngine() override;  // FL[FD-11761] Add support for multiparm blocks (lists) to lumberyard houdini engine
            void UpdateEditableNodeFromEngine() override;

            void DeleteNode() override
            {      
                if (m_hou != nullptr && m_hou->IsActive())
                {
                    HAPI_DeleteNode(m_session, GetNodeInfo().id);
                    m_hou->RemoveNode(m_nodeName, this);
                }

                m_nodeId = -1;
                m_deleteCount++;
				m_hasEditableGeometryBuilt = false;
            }
        
            HAPI_NodeInfo GetNodeInfo() override
            { 
                HAPI_GetNodeInfo(m_session, m_nodeId, &m_nodeInfo);
                if (m_nodeInfo.isValid && m_nodeName.empty()) 
                {
                    m_nodeName = GetHou()->GetString(m_nodeInfo.nameSH);
                }
                return m_nodeInfo;
            }

            HAPI_GeoInfo GetGeometryInfo() override
            {            
                HAPI_GetDisplayGeoInfo(m_session, m_nodeId, &m_geoInfo);
                return m_geoInfo;
            }
            
            HAPI_AssetInfo GetAssetInfo() override
            {
				HAPI_GetAssetInfo(m_session, m_nodeInfo.id, &m_assetInfo);
                return m_assetInfo;
            }
			
            bool HasEditableGeometryInfo() override
            {
                return !m_editableGeoInfo.empty();
            }

			HAPI_GeoInfo GetEditableGeometryInfo() override
			{
				return m_editableGeoInfo[0];
			}

            bool IsEditableGeometryBuilt() override
            {
                return m_hasEditableGeometryBuilt;
            }

            void SetEditableGeometryBuilt(bool flag) override
            {
                m_hasEditableGeometryBuilt = flag;
            }

            const AZStd::vector<HAPI_PartInfo>& GetGeometryParts() override
            {
                HAPI_GeoInfo geomInfo = GetGeometryInfo();
                m_geomParts.clear();

                for (int i = 0; i < geomInfo.partCount; i++)
                {
                    HAPI_PartInfo partInfo;
                    HAPI_GetPartInfo(m_session, geomInfo.nodeId, i, &partInfo);
                    m_geomParts.push_back(partInfo);
                }

                return m_geomParts;
            }

            AZ::Transform GetObjectTransform() override;

            void SetObjectTransform(const AZ::Transform& transform) override;

            //Getters:
            IHoudini* GetHou() override { return m_hou; }
            HAPI_NodeId GetId() override { return m_nodeId; }
            const AZStd::string& GetOperatorName() override { return m_operatorName; }
            const AZStd::string& GetNodeName() override { GetNodeInfo(); return m_nodeName; }
            const AZStd::string& GetHelpText() override { return m_helpText; }
            const AZStd::string GetNodePath() override;
            const AZStd::vector<HAPI_NodeId>& GetChildren() override { return m_children; }
            const AZStd::vector<HoudiniParameterPtr>& GetParameters() override { return m_parameters; }
            const AZ::EntityId & GetEntityId() const override { return m_entityId; }
            const AZStd::string& GetLastError() override { return m_lastCookError; }

            //Setters:
            void SetEntityId(const AZ::EntityId& entityId) override { m_entityId = entityId; }
            void SetParent(HoudiniNodePtr parent) override { m_parent = parent; }
            
            void SetInputEntity(int index, const AZ::EntityId& entityId) override;
            void SetInputEntity(const AZStd::string& name, const AZ::EntityId& entityId) override;

            AZStd::vector<AZStd::string> GetInputs() override;

            HoudiniParameterPtr GetParameter(AZStd::string name) override;
            
            
            bool IsGeometryCached() override { return m_isGeometryCached; }
            void SetGeometryCached(bool state) override { m_isGeometryCached = state; }
            void SetDirty() override { m_isGeometryCached = false; }
    };
}
