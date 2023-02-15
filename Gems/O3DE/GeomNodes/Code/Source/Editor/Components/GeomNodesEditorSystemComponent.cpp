
#include <AzCore/Serialization/SerializeContext.h>
#include "GeomNodesEditorSystemComponent.h"

namespace GeomNodes
{
    void GeomNodesEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GeomNodesEditorSystemComponent, GeomNodesSystemComponent>()
                ->Version(0);
        }
    }

    GeomNodesEditorSystemComponent::GeomNodesEditorSystemComponent() = default;

    GeomNodesEditorSystemComponent::~GeomNodesEditorSystemComponent() = default;

    void GeomNodesEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("GeomNodesEditorService"));
    }

    void GeomNodesEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("GeomNodesEditorService"));
    }

    void GeomNodesEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void GeomNodesEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void GeomNodesEditorSystemComponent::Activate()
    {
        GeomNodesSystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
        m_system.OnActivate();
    }

    void GeomNodesEditorSystemComponent::Deactivate()
    {
        m_system.OnDeactivate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        GeomNodesSystemComponent::Deactivate();
    }

} // namespace GeomNodes
