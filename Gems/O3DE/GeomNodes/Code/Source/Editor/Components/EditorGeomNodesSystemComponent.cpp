
#include <AzCore/Serialization/SerializeContext.h>
#include "EditorGeomNodesSystemComponent.h"

AZ_DEFINE_BUDGET(GeomNodes);

namespace GeomNodes
{
    void EditorGeomNodesSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<EditorGeomNodesSystemComponent, GeomNodesSystemComponent>()
                ->Version(0);
        }
    }

    EditorGeomNodesSystemComponent::EditorGeomNodesSystemComponent() = default;

    EditorGeomNodesSystemComponent::~EditorGeomNodesSystemComponent() = default;

    void EditorGeomNodesSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("EditorGeomNodesSystemService"));
    }

    void EditorGeomNodesSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("EditorGeomNodesSystemService"));
    }

    void EditorGeomNodesSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void EditorGeomNodesSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void EditorGeomNodesSystemComponent::Activate()
    {
        GeomNodesSystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
        m_system.OnActivate();
    }

    void EditorGeomNodesSystemComponent::Deactivate()
    {
        m_system.OnDeactivate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        GeomNodesSystemComponent::Deactivate();
    }

} // namespace GeomNodes
