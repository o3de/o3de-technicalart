#pragma once

#include <AzCore/Component/TickBus.h>
//#include <Editor/Rendering/GNRenderMeshInterface.h>
#include <Editor/Rendering/GNModelData.h>
#include <Editor/Rendering/GNRenderMesh.h>

namespace GeomNodes
{
    class GNRenderModel
        : //public GNRenderMeshInterface
        //, private AZ::TickBus::Handler
          private AZ::TickBus::Handler
    {
    public:
        //AZ_RTTI(GNRenderModel, "{510773BC-C8A9-4250-B412-F8525816A257}", GNRenderMeshInterface);

        explicit GNRenderModel(AZ::EntityId entityId);
        ~GNRenderModel();   

        void ClearMeshes();

        // RenderMeshInterface ...
        void BuildMeshes(const GNModelData& renderData, const AZ::Transform& worldFromLocal);
        void UpdateTransform(const AZ::Transform& worldFromLocal);
        //void UpdateMaterial(const WhiteBoxMaterial& material) override;
        bool IsVisible() const;
        void SetVisiblity(bool visibility);

        // AZ::TickBus overrides ...
        void OnTick(float deltaTime, AZ::ScriptTimePoint time);

        void QueueBuildMeshes(const GNModelData& renderData);
    private:
        AZ::EntityId m_entityId;

        AZStd::vector<GNRenderMesh*> m_meshes;

        GNModelData m_modelData;
    };
}
