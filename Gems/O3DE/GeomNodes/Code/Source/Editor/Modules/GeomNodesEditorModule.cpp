/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "Editor/Components/EditorGeomNodesComponent.h"
#include "Editor/Components/EditorGeomNodesSystemComponent.h"
#include "Editor/Systems/GeomNodesSystem.h"
#include <Editor/Configuration/GNEditorSettingsRegistryManager.h>
#include <GeomNodes/GeomNodesTypeIds.h>
#include <GeomNodesModuleInterface.h>


namespace GeomNodes
{
    class GeomNodesEditorModule : public GeomNodesModuleInterface
    {
    public:
        AZ_RTTI(GeomNodesEditorModule, GeomNodesEditorModuleTypeId, GeomNodesModuleInterface);
        AZ_CLASS_ALLOCATOR(GeomNodesEditorModule, AZ::SystemAllocator);

        GeomNodesEditorModule()
            : m_gnSystem(AZStd::make_unique<GNEditorSettingsRegistryManager>())
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and
            // EditContext. This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(
                m_descriptors.end(),
                {
                    EditorGeomNodesSystemComponent::CreateDescriptor(),
                    EditorGeomNodesComponent::CreateDescriptor(),
                });
        }

        virtual ~GeomNodesEditorModule()
        {
            m_gnSystem.Shutdown();
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<EditorGeomNodesSystemComponent>(),
                azrtti_typeid<EditorGeomNodesComponent>(),
            };
        }

    private:
        GeomNodesSystem m_gnSystem;
    };
} // namespace GeomNodes

AZ_DECLARE_MODULE_CLASS(Gem_GeomNodes, GeomNodes::GeomNodesEditorModule)
