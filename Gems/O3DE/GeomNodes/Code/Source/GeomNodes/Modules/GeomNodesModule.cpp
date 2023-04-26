/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <GeomNodes/GeomNodesTypeIds.h>
#include <GeomNodesModuleInterface.h>

namespace GeomNodes
{
    class GeomNodesModule : public GeomNodesModuleInterface
    {
    public:
        AZ_RTTI(GeomNodesModule, GeomNodesModuleTypeId, GeomNodesModuleInterface);
        AZ_CLASS_ALLOCATOR(GeomNodesModule, AZ::SystemAllocator);
    };
} // namespace GeomNodes

AZ_DECLARE_MODULE_CLASS(Gem_GeomNodes, GeomNodes::GeomNodesModule)
