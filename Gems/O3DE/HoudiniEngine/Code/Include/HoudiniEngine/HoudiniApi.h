#pragma once

#include <HAPI/HAPI.h>
#include <LmbrCentral/Shape/SplineAttribute.h>
#include <AzCore/std/smart_ptr/shared_ptr.h>

#include <AzCore/Component/TickBus.h>

namespace HoudiniEngine
{
    class IHoudini;
    class IHoudiniNode;
    class IHoudiniParameter;
    class IHoudiniAsset;
    class IInputNodeManager;
    
    typedef AZStd::shared_ptr<IHoudini> HoudiniPtr;
    typedef AZStd::shared_ptr<IHoudiniAsset> HoudiniAssetPtr;
    typedef AZStd::shared_ptr<IHoudiniNode> HoudiniNodePtr;
    typedef AZStd::shared_ptr<IHoudiniParameter> HoudiniParameterPtr;
    typedef AZStd::shared_ptr<IInputNodeManager> InputNodeManagerPtr;

    class HoudiniEntityContext
    {
    public:
        bool m_isSelected = false;
        AZ::Entity* m_entity = nullptr;
        AZStd::string m_entityName = "";
        AZ::SplinePtr m_spline = nullptr;
        AZ::Transform m_transform = AZ::Transform::CreateIdentity();

        AZStd::vector<AZStd::string> m_attributeNames;
        AZStd::map<AZStd::string, LmbrCentral::SplineAttribute<float>*> m_attributes;
    };

    class IHoudini
    {
        public:
            virtual void ExecuteCommand(AZ::EntityId currentId, AZStd::function<bool()>) = 0;
            virtual void RaiseCommandPriority(AZ::EntityId newId) = 0;
            virtual void GetProgress(AZStd::string& statusText, int & statusPercent, int& assetsInQueue) = 0;
            virtual void SetProgress(const AZStd::string& statusText, int statusPercent) = 0;

            virtual void CancelProcessorThread() = 0;
            virtual void CancelProcessorJob(AZ::EntityId) = 0;
            virtual void JoinProcessorThread() = 0;

            virtual bool IsActive() = 0;
            virtual int GetHoudiniMode() = 0;
            virtual bool CheckForErrors(bool printErrors = true, bool includeCookingErrors = true) = 0;
            virtual void LoadAllAssets() = 0;
            virtual void ReloadAllAssets() = 0;  // FL[FD-10790] Houdini Digital Asset List Hot Reload
            virtual const HAPI_Session& GetSession() = 0;

            virtual HoudiniAssetPtr LoadHoudiniDigitalAsset(const AZStd::string& hdaName) = 0;

            //Thread-safe Lookups:
            virtual void RemoveLookupId(const AZ::EntityId& id) = 0;
            virtual void LookupId(const AZ::EntityId& id) = 0;
            virtual HoudiniEntityContext LookupEntityData(const AZ::EntityId& id) = 0;

            //Pre-defined Look ups:
            virtual bool LookupIsSelected(const AZ::EntityId& id) = 0;
            virtual AZ::Entity* LookupFindEntity(const AZ::EntityId& id) = 0;
            virtual AZStd::string LookupEntityName(const AZ::EntityId& id) = 0;
            virtual AZ::SplinePtr LookupSpline(const AZ::EntityId& id) = 0;
            virtual AZStd::vector<AZStd::string> LookupAttributeNames(const AZ::EntityId& id) = 0;
            virtual LmbrCentral::SplineAttribute<float>* LookupAttributeData(const AZ::EntityId& id, const AZStd::string& attribName) = 0;
            virtual AZ::Transform LookupTransform(const AZ::EntityId& id) = 0;


            virtual void RemoveNode(const AZStd::string& oldNodeName, IHoudiniNode* node) = 0;
            virtual void RenameNode(const AZStd::string& oldNodeName, IHoudiniNode* node) = 0;
            virtual HoudiniNodePtr CreateNode(const AZStd::string& operatorName, const AZStd::string& nodeName, IHoudiniNode* parent = nullptr) = 0;
            virtual HAPI_NodeId FindNode(HAPI_NodeType networkType, const AZStd::string& path) = 0;
            virtual HoudiniNodePtr GetRootNode() = 0;                        
            virtual void CookNode(HAPI_NodeId node, const AZStd::string& entityName) = 0;
            virtual void OnTick(float deltaTime, AZ::ScriptTimePoint time) = 0;

            virtual AZStd::string GetString(HAPI_StringHandle string_handle) = 0;
            virtual AZStd::string GetLastHoudiniError() = 0;
            virtual AZStd::string GetLastHoudiniCookError() = 0;

            virtual HoudiniAssetPtr GetAsset(const AZStd::string& assetName) = 0;
            virtual AZStd::vector<AZStd::string> GetAvailableAssetNames() = 0;
            virtual AZStd::vector<HoudiniAssetPtr> GetAvailableAssets() = 0;            
            
            virtual void Log(const AZStd::string& msg) = 0;
            virtual void SaveDebugFile() = 0;

            virtual InputNodeManagerPtr GetInputNodeManager() = 0;
            virtual void Shutdown() = 0;
            
            virtual void CreateNewSession() = 0;
            virtual void ResetSession() = 0;
    };

    template <typename T>
    IHoudini& operator<<(IHoudini& os, T dt)
    {
        os.Log(AZStd::to_string(dt));
        return os;
    }


    template<>
    IHoudini& operator<< (IHoudini& os, const char* dt);

    template<>
    IHoudini& operator<< (IHoudini& os, char* dt);

    template<>
    IHoudini& operator<< (IHoudini& os, const AZStd::string& dt);

    template<>
    IHoudini& operator<< (IHoudini& os, AZStd::string dt);

    class IHoudiniNode
    {
        public:            
            //Houdini API Stuff:
            virtual void Cook() = 0;
            virtual bool HasCookingError() = 0;
            virtual HoudiniNodePtr CreateNode(const AZStd::string& operatorName, const AZStd::string& nodeName) = 0;
            virtual HoudiniNodePtr CreateCurve(const AZStd::string& nodeName) = 0;

            virtual HoudiniParameterPtr GetParameter(AZStd::string name) = 0;
            
            virtual AZStd::vector<int> GetIntPoints(const AZStd::string& attributeName) = 0;
            virtual AZStd::vector<float> GetFloatPoints(const AZStd::string& attributeName) = 0;
            virtual AZStd::vector<AZ::Matrix3x3> GetMatrix3Points(const AZStd::string& attributeName) = 0;
            virtual AZStd::vector<AZ::Vector3> GetGeometryPoints() = 0;
            virtual AZStd::vector<AZ::Vector3> GetGeometryPointGroup(const AZStd::vector<AZStd::string>& groupNames) = 0;

            //Builds the data
            virtual bool IsGeometryCached() = 0;
            virtual void SetGeometryCached(bool state) = 0;

            virtual void SetDirty() = 0;
            virtual bool UpdateData() = 0;
            virtual void UpdateParamInfoFromEngine() = 0;  // FL[FD-11761] Add support for multiparm blocks (lists) to lumberyard houdini engine
            virtual void UpdateEditableNodeFromEngine() = 0;
            virtual void DeleteNode() = 0;

            virtual const AZStd::vector<HAPI_PartInfo>& GetGeometryParts() = 0;

            //Getters:
            virtual IHoudini* GetHou() = 0;
            virtual HAPI_NodeId GetId() = 0;
            virtual const AZStd::string& GetOperatorName() = 0;
            virtual const AZStd::string& GetNodeName() = 0;
            virtual const AZStd::string& GetHelpText() = 0;
            virtual const AZStd::string GetNodePath() = 0;
            virtual AZ::Transform GetObjectTransform() = 0;
            virtual const AZStd::vector<HAPI_NodeId>& GetChildren() = 0;
            virtual const AZStd::vector<HoudiniParameterPtr>& GetParameters() = 0;
            virtual const AZ::EntityId & GetEntityId() const = 0;
            virtual HAPI_NodeInfo GetNodeInfo() = 0;
            virtual HAPI_GeoInfo GetGeometryInfo() = 0;
            virtual HAPI_AssetInfo GetAssetInfo() = 0;
            virtual bool HasEditableGeometryInfo() = 0;
            virtual HAPI_GeoInfo GetEditableGeometryInfo() = 0;
            virtual bool IsEditableGeometryBuilt() = 0;
            virtual void SetEditableGeometryBuilt(bool flag) = 0;
            virtual AZStd::vector<AZStd::string> GetInputs() = 0;

            virtual const AZStd::string& GetLastError() = 0;

            //Setters:
            virtual void SetEntityId(const AZ::EntityId& entityId) = 0;
            virtual void SetParent(HoudiniNodePtr parent) = 0;
            
            virtual void SetInputEntity(int index, const AZ::EntityId& entityId) = 0;
            virtual void SetInputEntity(const AZStd::string& name, const AZ::EntityId& entityId) = 0;

            virtual void SetObjectTransform(const AZ::Transform& transform) = 0;
    };

    class IHoudiniAsset
    {
        public:
            virtual const AZStd::string& GetHdaName() = 0;
            virtual const AZStd::string& GetHdaFile() = 0;
            virtual const AZStd::vector<AZStd::string>& getAssets() = 0;
    };

    class IHoudiniParameter
    {
        public:
            virtual IHoudini* GetHou() = 0;
            virtual HAPI_ParmId GetId() = 0;
            virtual HAPI_ParmId GetParentId() = 0;
            virtual AZStd::string GetLabel() = 0;
            virtual AZStd::string GetName() = 0;
            virtual const AZStd::string& GetHelp() = 0;
            virtual IHoudiniNode* GetNode() = 0;
            virtual AZStd::string GetTypeName() = 0;
            virtual HAPI_ParmType GetType() = 0;
            virtual int GetSize() = 0;
            virtual const AZStd::string& GetTypeInfo() = 0;
            virtual const HAPI_ParmInfo& GetInfo() = 0;                        
            
            virtual void SetName(const AZStd::string & value) = 0;
            virtual void SetTypeInfo(const AZStd::string & value) = 0;
            virtual void SetValueBool(bool value) = 0;
            virtual void SetValueInt(int value) = 0;
            virtual void SetValueFloat(float value) = 0;
            virtual void SetValueVec2(const AZ::Vector2& value) = 0;
            virtual void SetValueVec3(const AZ::Vector3& value) = 0;
            virtual void SetValueVec4(const AZ::Vector4& value) = 0;
            virtual bool SetValueEntity(const AZ::EntityId & value) = 0;
            virtual void SetValueString(const AZStd::string & value) = 0;
            virtual void SetValueColor(const AZ::Color& value) = 0;  // FL[FD-11758] Add support for color parameter to lumberyard houdini engine
            virtual void SetValueMultiparm(int value) = 0; // FL[FD-11761] Add support for multiparm blocks (lists) to lumberyard houdini engine

            virtual bool GetValueBoolean() = 0;
            virtual const int GetValueInt() = 0;
            virtual AZStd::vector<int> GetValuesInt(int count) = 0;
            virtual const float GetValueFloat() = 0;
            virtual const AZ::Vector2& GetValueVec2() = 0;
            virtual const AZ::Vector3& GetValueVec3() = 0;
            virtual const AZ::Vector4& GetValueVec4() = 0;
            virtual const AZ::EntityId& GetValueEntity() = 0;
            virtual const AZStd::string& GetValueString() = 0;
            virtual const AZ::Color& GetValueColor() = 0;  // FL[FD-11758] Add support for color parameter to lumberyard houdini engine
            virtual AZStd::vector<AZStd::string> GetValueChoices() = 0;
            // FL[FD-11761] Add support for multiparm blocks (lists) to lumberyard houdini engine
            virtual void SetParmInfo(HAPI_ParmInfo info) = 0;
            virtual const int GetValueMultiparm() = 0;
            virtual HoudiniParameterPtr GetParent() = 0;
            virtual AZStd::vector<HoudiniParameterPtr> GetChildren(int instanceNum) = 0;
            virtual void InsertInstance(const int position) = 0;
            virtual void RemoveInstance(const int position) = 0;

            virtual void SetProcessed(bool flag) = 0;
            virtual bool IsProcessed() = 0;
    };

    class IInputNodeManager
    {
        public:
            virtual void OnTick(float deltaTime, AZ::ScriptTimePoint time) = 0;
            virtual void Reset() = 0;
            
            virtual void AddSplineChangeHandler(const AZ::EntityId& id) = 0;
            virtual void RemoveSplineChangeHandler(const AZ::EntityId& id) = 0;

            virtual HAPI_NodeId GetNodeIdFromEntity(const AZ::EntityId& id) = 0;
            virtual HAPI_NodeId CreateInputNodeFromSpline(const AZ::EntityId& id) = 0;
            virtual HAPI_NodeId CreateInputNodeFromTerrain(const AZ::EntityId& id) = 0;
            virtual HAPI_NodeId CreateInputNodeFromMesh(const AZ::EntityId& id) = 0;  // FL[FD-10789] Support Mesh as Input to Houdini Digital Asset
    };
}
