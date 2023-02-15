
#pragma once

#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <GeomNodes/Components/GeomNodesSystemComponent.h>
#include "Editor/Systems/GeomNodesSystem.h"

namespace GeomNodes
{
    /// System component for GeomNodes editor
    class GeomNodesEditorSystemComponent
        : public GeomNodesSystemComponent
        , protected AzToolsFramework::EditorEvents::Bus::Handler
    {
        using BaseSystemComponent = GeomNodesSystemComponent;
    public:
        AZ_COMPONENT(GeomNodesEditorSystemComponent, "{3B2D4C6C-C359-4890-8D78-0DDA7864131C}", BaseSystemComponent);
        static void Reflect(AZ::ReflectContext* context);

        GeomNodesEditorSystemComponent();
        ~GeomNodesEditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate() override;
        void Deactivate() override;

        GeomNodesSystem m_system;
    };
} // namespace GeomNodes
