/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <GeomNodes/Components/GeomNodesSystemComponent.h>
#include <Editor/Common/GNEvents.h>

namespace GeomNodes
{
    class GeomNodesSystem;

    /// System component for GeomNodes editor
    class EditorGeomNodesSystemComponent
        : public GeomNodesSystemComponent
        , private AzToolsFramework::EditorEvents::Bus::Handler
    {
        using BaseSystemComponent = GeomNodesSystemComponent;
    public:
        AZ_COMPONENT(EditorGeomNodesSystemComponent, GeomNodesEditorSystemComponentTypeId, BaseSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        EditorGeomNodesSystemComponent();
        ~EditorGeomNodesSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        void ActivateGeomNodesSystem();

        // AZ::Component
        void Activate() override;
        void Deactivate() override;

		// AztoolsFramework::EditorEvents overrides...
		void NotifyRegisterViews() override;

        GeomNodesSystem* m_system = nullptr;
		SystemEvents::OnInitializedEvent::Handler m_onSystemInitializedHandler;
		SystemEvents::OnConfigurationChangedEvent::Handler m_onSystemConfigChangedHandler;
    };
} // namespace GeomNodes
