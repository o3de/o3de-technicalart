#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <Editor/Rendering/GNMeshData.h>

namespace GeomNodes
{
    
    class EditorGeomNodesComponentRequests : public AZ::ComponentBus
    {
    public:
        virtual GNMeshData GetMeshData(AZ::u64 entityId) = 0;
        virtual void SetWorkInProgress(bool flag) = 0;
        virtual bool GetWorkInProgress() = 0;

	protected:
		~EditorGeomNodesComponentRequests() = default;
    };

    using EditorGeomNodesComponentRequestBus = AZ::EBus<EditorGeomNodesComponentRequests>;
} // namespace GeomNodes