
#pragma once

#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <Clients/DreamStudioAISystemComponent.h>

namespace DreamStudioAI
{
    /// System component for DreamStudioAI editor
    class DreamStudioAIEditorSystemComponent
        : public DreamStudioAISystemComponent
        , protected AzToolsFramework::EditorEvents::Bus::Handler
    {
        using BaseSystemComponent = DreamStudioAISystemComponent;
    public:
        AZ_COMPONENT(DreamStudioAIEditorSystemComponent, "{C8AFD8FF-424C-4336-A7BC-C97845537E6D}", BaseSystemComponent);
        static void Reflect(AZ::ReflectContext* context);

        DreamStudioAIEditorSystemComponent();
        ~DreamStudioAIEditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate() override;
        void Deactivate() override;
    };
} // namespace DreamStudioAI
