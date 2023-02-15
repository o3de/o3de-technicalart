#include "GNRenderModel.h"

namespace GeomNodes
{
    GNRenderModel::GNRenderModel(AZ::EntityId entityId)
        : m_entityId(entityId)
    {
    }

    GNRenderModel::~GNRenderModel()
    {
        ClearMeshes();
        AZ::TickBus::Handler::BusDisconnect();
    }

    void GNRenderModel::ClearMeshes()
    {
        for (auto& mesh : m_meshes)
        {
            delete mesh;
        }

        m_meshes.clear();
    }

    void GNRenderModel::BuildMeshes(const GNModelData& renderData, const AZ::Transform& worldFromLocal)
    {
        ////TODO: need to optimize this at this is the point where it slows down.
        ClearMeshes(); // for now we always clear the meshes

        for (auto& meshData : renderData.GetMeshes())
        {
            auto renderMesh = aznew GNRenderMesh(m_entityId);
            renderMesh->BuildMesh(meshData, worldFromLocal);
            m_meshes.push_back(renderMesh);
        }

        //AZ::TickBus::Handler::BusConnect(); // connect the TickBus for a one time Update of transform.
        UpdateTransform(worldFromLocal);
    }

    void GNRenderModel::UpdateTransform(const AZ::Transform& worldFromLocal)
    {
        for (auto& mesh : m_meshes)
        {
            mesh->UpdateTransform(worldFromLocal);
        }
    }

    bool GNRenderModel::IsVisible() const
    {
        if (m_meshes.empty())
            return false;

        return m_meshes[0]->IsVisible();
    }

    void GNRenderModel::SetVisiblity(bool visibility)
    {
        for (auto& mesh : m_meshes)
        {
            mesh->SetVisiblity(visibility);
        }

    }

    void GNRenderModel::OnTick(float /*deltaTime*/, AZ::ScriptTimePoint /*time*/)
    {
        AZ::Transform worldTransform;
        AZ::TransformBus::EventResult(worldTransform, m_entityId, &AZ::TransformBus::Events::GetWorldTM);

        BuildMeshes(m_modelData, worldTransform);
        
        //UpdateTransform(worldTransform);
        AZ::TickBus::Handler::BusDisconnect();
    }

    void GNRenderModel::QueueBuildMeshes(const GNModelData& renderData)
    {
        m_modelData = renderData;
        AZ::TickBus::Handler::BusConnect();
    }
} // namespace GeomNodes
