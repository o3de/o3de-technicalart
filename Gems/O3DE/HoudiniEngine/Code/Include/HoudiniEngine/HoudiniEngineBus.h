
#pragma once

#include <HoudiniEngine/HoudiniCommonForwards.h>
#include <LmbrCentral/Shape/SplineAttribute.h>

namespace HoudiniEngine
{
    #define HOUDINI_CONFIG_GUID                     "{6DF7BB00-AF41-41DC-862E-B29B87D1CECC}"        
    #define HOUDINI_ASSET_COMPONENT_GUID            "{6DF7BB00-AF41-41DC-862E-B29B87D1CECE}"
    #define HOUDINI_SCATTER_COMPONENT_GUID          "{6DF7BB00-AF41-41DC-862E-B29B87D1CECD}"
    #define HOUDINI_CURVE_ATTRIBUTE_COMPONENT_GUID  "{6DF7BB00-AF41-41DC-862E-B29B87D1CECB}"
    #define HOUDINI_SCATTER_ELEMENT_GUID            "{6DF7BB00-AF41-41DC-862E-00000000000A}"
    #define HOUDINI_CACHE_COMPONENT_GUID            "{6DF7BB00-AF41-41DC-862E-00000000000B}"
    #define HOUDINI_CACHE_ITEM_GUID                 "{6DF7BB00-AF41-41DC-862E-00000000000C}"
    #define HOUDINI_MESH_COMPONENT_GUID             "{6DF7BB00-AF41-41DC-862E-00000000000D}"
    #define HOUDINI_MESH_DATA_GUID                  "{6DF7BB00-AF41-41DC-862E-00000000000E}"
    #define HOUDINI_MESH_SETTINGS_GUID              "{6DF7BB00-AF41-41DC-862E-00000000001A}"
    #define HOUDINI_NODE_EXPORT_GUID                "{6DF7BB00-AF41-41DC-862E-00000000000F}"
    #define HOUDINI_TERRAIN_COMPONENT_GUID          "{283643E1-6AFF-4479-976D-C14EC38192E8}"

    enum OperatorMode
    {
        Assets,
        Terrain,
        Scatter,
    };

    /*!
    * Configuration data for Houdini Property Stuff.
    */
    class IHoudiniNodeComponentConfig
    {
    public:
        virtual bool IsInitialized() = 0;
        virtual IHoudiniNode* LoadHda(const AZStd::string&, const AZStd::string& nodeName, AZStd::function<void(IHoudiniNode*)> onLoad = {}) = 0;
        virtual AZStd::vector<AZStd::string> getOperatorNames() = 0;
        virtual AZ::ScriptProperty* GetProperty(const AZStd::string& name) = 0;
        
        virtual IHoudiniNode* GetNode() = 0;
        virtual const AZStd::string& GetNodeName() = 0;
        virtual const AZStd::string& GetOperatorName() = 0;
        virtual bool UpdateNode() = 0;
        virtual void RenameNode(const AZStd::string& newName) = 0;
        virtual void FixEntityPointers() = 0;

        virtual bool SetPropertyValueBool(const AZStd::string& name, bool value) = 0;
        virtual bool SetPropertyValueInt(const AZStd::string& name, int value) = 0;
        virtual bool SetPropertyValueFloat(const AZStd::string& name, float value) = 0;
        virtual bool SetPropertyValueVec2(const AZStd::string& name, const AZ::Vector2& value) = 0;
        virtual bool SetPropertyValueVec3(const AZStd::string& name, const AZ::Vector3& value) = 0;
        virtual bool SetPropertyValueVec4(const AZStd::string& name, const AZ::Vector4& value) = 0;
        virtual bool SetPropertyValueEntityId(const AZStd::string& name, const AZ::EntityId & value) = 0;
        virtual bool SetPropertyValueString(const AZStd::string& name, const AZStd::string & value) = 0;

        virtual bool SetInputEntityIdByName(const AZStd::string& name, const AZ::EntityId & value) = 0;
        virtual bool SetInputEntityId(int index, const AZ::EntityId & value) = 0;

        virtual int GetPropertyValueInt(const AZStd::string& name) = 0;
        virtual float GetPropertyValueFloat(const AZStd::string& name) = 0;
        virtual AZ::Vector2 GetPropertyValueVec2(const AZStd::string& name) = 0;
        virtual AZ::Vector3 GetPropertyValueVec3(const AZStd::string& name) = 0;
        virtual AZ::Vector4 GetPropertyValueVec4(const AZStd::string& name) = 0;
        virtual AZStd::string GetPropertyValueString(const AZStd::string& name) = 0;
        virtual AZ::EntityId GetPropertyValueEntityId(const AZStd::string& name) = 0;

        virtual AZ::EntityId GetInputEntityIdByName(const AZStd::string& name) = 0;
        virtual AZ::EntityId GetInputEntityId(int index) = 0;
    };
       
    class HoudiniEngineRequests : public AZ::EBusTraits
    {
        public:
            //////////////////////////////////////////////////////////////////////////
            // EBusTraits overrides
            static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
            static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
            //////////////////////////////////////////////////////////////////////////

            // Put your public methods here
            virtual HoudiniPtr GetHoudiniEngine() = 0;
            virtual bool IsActive() = 0;
            virtual void CancelProcessorThread() = 0;
            virtual void CancelProcessorJob(AZ::EntityId entityToRemove) = 0;
            virtual void JoinProcessorThread() = 0;
            virtual AZStd::string GetHoudiniResultByCode(int code) = 0;  // FL[FD-10714] Houdini integration to 1.21
    };

    class HoudiniMaterialRequests : public AZ::ComponentBus
    {
        public:
            virtual void SetMaterialPath(const AZStd::string& materialName, const AZStd::string& materialPath) = 0;
    };

    class HoudiniAssetRequests : public AZ::ComponentBus
    {
        public:
            // Put your public methods here
            virtual IHoudiniNode* GetNode() = 0;
            virtual IHoudiniNode* LoadHda(const AZStd::string&, AZStd::function<void(IHoudiniNode*)> onLoad = {}) = 0;
            
            virtual void FixEntityPointers() = 0;
            virtual IHoudiniNodeComponentConfig* GetConfig() = 0;
            virtual const AZStd::string& GetNodeName() = 0;
            virtual AZ::EntityId GetEntityIdFromNodeName(const AZStd::string& nodeName) = 0;
            virtual void SaveToFbx() = 0;

            virtual bool IsLiveUpdate() { return false; }
            virtual void SetLiveUpdate(bool /*state*/) { }
    };
  
    //struct HoudiniMeshStatObject
    //{
    //    int MaterialIndex;
    //    //IStatObj* StatObject; //ATOMCONVERT
    //};

    //class HoudiniMeshRequests : public AZ::ComponentBus
    //{
    //    public:
    //        virtual AZStd::vector<HoudiniMeshStatObject> GetStatObjects() = 0;
    //};

    class HoudiniCurveAttributeRequests : public AZ::ComponentBus
    {
        public:
            virtual AZStd::string GetName() = 0;
            virtual void SetName(const AZ::ComponentId& id, const AZStd::string& value) = 0;
            virtual void SetValue(const AZStd::string& name, int index, float value) = 0;
            virtual bool GetValue(const AZStd::string& name, int index, float &value) = 0;
            virtual bool GetValueCount(const AZStd::string& name, int& count) = 0;            
            virtual bool GetValueRange(const AZStd::string& name, float& minValue, float & maxValue) = 0;
            virtual bool GetPaintValue(const AZStd::string& name, float& paintValue) = 0;
            virtual bool GetFloatAttribute(const AZStd::string& name, LmbrCentral::SplineAttribute<float>*& splinePointer) = 0;
            virtual void CommitChanges() = 0;
    };    
    
    //Buses:
    using HoudiniAssetRequestBus = AZ::EBus<HoudiniAssetRequests>;
    using HoudiniCurveAttributeRequestBus = AZ::EBus<HoudiniCurveAttributeRequests>;
    using HoudiniEngineRequestBus = AZ::EBus<HoudiniEngineRequests>;
    //using HoudiniMeshRequestBus = AZ::EBus<HoudiniMeshRequests>;
    using HoudiniMaterialRequestBus = AZ::EBus<HoudiniMaterialRequests>;

} // namespace HoudiniEngine
