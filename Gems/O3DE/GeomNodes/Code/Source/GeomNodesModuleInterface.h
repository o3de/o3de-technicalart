/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Memory/Memory_fwd.h>
#include <AzCore/Module/Module.h>
#include <AzCore/RTTI/RTTIMacros.h>
#include <AzCore/RTTI/TypeInfoSimple.h>
#include <GeomNodes/Components/GeomNodesSystemComponent.h>
#include <GeomNodes/GeomNodesTypeIds.h>


namespace GeomNodes
{
    class GeomNodesModuleInterface : public AZ::Module
    {
    public:
        AZ_RTTI(GeomNodesModuleInterface, GeomNodesModuleInterfaceTypeId, AZ::Module);
        AZ_CLASS_ALLOCATOR(GeomNodesModuleInterface, AZ::SystemAllocator);

        GeomNodesModuleInterface()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and
            // EditContext. This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(
                m_descriptors.end(),
                {
                    GeomNodesSystemComponent::CreateDescriptor(),
                });
        }

        AZ::ComponentTypeList GetRequiredSystemComponents() const
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<GeomNodesSystemComponent>(),
            };
        }
    };
} // namespace GeomNodes
