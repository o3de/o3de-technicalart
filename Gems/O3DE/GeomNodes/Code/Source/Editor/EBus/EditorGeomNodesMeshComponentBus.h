#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <Editor/Rendering/GNMeshData.h>

namespace GeomNodes
{
    
    class EditorGeomNodesMeshComponentEvents : public AZ::ComponentBus
    {
    public:
        virtual void OnMeshDataAssigned(const GNMeshData& /*meshData*/) {};

	protected:
		~EditorGeomNodesMeshComponentEvents() = default;
    };

    using EditorGeomNodesMeshComponentEventBus = AZ::EBus<EditorGeomNodesMeshComponentEvents>;
} // namespace GeomNodes