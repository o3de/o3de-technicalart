
#pragma once

#include <LmbrCentral/Shape/SplineComponentBus.h>
#include <HAPI/HAPI.h>
#include <HoudiniEngine/HoudiniApi.h>
#include <Atom/RPI.Public/Model/Model.h>
#include <Atom/RPI.Public/Base.h>

#define HOUDINI_ZERO_ID 0
#define HOUDINI_INVALID_ID -1
#define HOUDINI_NOT_READY_ID -10
#define HOUDINI_ROOT_NODE_ID -1

namespace HoudiniEngine
{
    struct HoudiniCurveContext
    {
        bool m_dirty = true;
        HAPI_NodeId m_node = HOUDINI_INVALID_ID;
        bool m_cooking = false;
    };

    class InputNodeManager : public IInputNodeManager
        , public LmbrCentral::SplineComponentNotificationBus::MultiHandler
        // FL[FD-10789] Support Mesh as Input to Houdini Digital Asset
        , public AZ::TransformNotificationBus::MultiHandler
    {
    protected:
        Houdini* m_hou;
        AZStd::map<AZ::EntityId, HoudiniCurveContext > m_inputCache;
        HAPI_NodeId m_terrainCache;
        AZStd::vector<AZ::EntityId> m_splineChangeHandlers;

        AZStd::vector<int> m_faces;
        AZStd::vector<int> m_faceCounts;
        AZStd::vector<float> m_points;

    public:

        InputNodeManager()
            : m_hou(nullptr)
            , m_terrainCache(HOUDINI_INVALID_ID) 
        {}

        InputNodeManager(Houdini* hou) :
            m_hou(hou)
            , m_terrainCache(HOUDINI_INVALID_ID) 
        {}

        ~InputNodeManager() {}
                
        void Reset() override;

        HAPI_NodeId GetNodeIdFromEntity(const AZ::EntityId& id) override;
        HAPI_NodeId CreateInputNodeFromSpline(const AZ::EntityId& id) override;
        HAPI_NodeId CreateInputNodeFromTerrain(const AZ::EntityId& id) override;
        HAPI_NodeId CreateInputNodeFromMesh(const AZ::EntityId& id) override;  // FL[FD-10789] Support Mesh as Input to Houdini Digital Asset

        void OnSplineChanged() override;
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        void AddSplineChangeHandler(const AZ::EntityId& id) override
        {
            LmbrCentral::SplineComponentNotificationBus::MultiHandler::BusConnect(id);
            m_splineChangeHandlers.push_back(id);
        }

        void RemoveSplineChangeHandler(const AZ::EntityId& id) override
        {
            auto it = AZStd::find(m_splineChangeHandlers.begin(), m_splineChangeHandlers.end(), id);
            if (it != m_splineChangeHandlers.end())
            {
                LmbrCentral::SplineComponentNotificationBus::MultiHandler::BusDisconnect(id);
                m_splineChangeHandlers.erase(it);
            }
        }

        // FL[FD-10789] Support Mesh as Input to Houdini Digital Asset       
        virtual void OnTransformChanged(const AZ::Transform& /*local*/, const AZ::Transform& /*world*/) override;
        
        AZ::RPI::ModelLodIndex GetModelLodIndex(const AZ::RPI::ViewPtr view, AZ::Data::Instance<AZ::RPI::Model> model, AZ::EntityId id) const;

        AZStd::unordered_map<AZ::EntityId, HAPI_NodeId> m_meshNodesCache;
    };
}
