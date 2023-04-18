/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Component/Component.h>
#include <GeomNodes/GeomNodesTypeIds.h>

namespace AZ
{
    class Transform;
}

namespace GeomNodes
{
    //struct GNMaterial;
    struct WhiteBoxRenderData;

    //! A generic interface for the GeomNodes Component to communicate
    //! with regardless of the rendering backend.
    class GNRenderMeshInterface
    {
    public:
        AZ_RTTI(GNRenderMeshInterface, GNRenderMeshInterfaceTypeId);

        virtual ~GNRenderMeshInterface() = 0;

        //! Take GeomNodes render data and populate the render mesh from it.
        virtual void BuildMesh(const GNModelData& renderData, const AZ::Transform& worldFromLocal) = 0;

        //! Update the transform of the render mesh.
        virtual void UpdateTransform(const AZ::Transform& worldFromLocal) = 0;

        //! Update the material of the render mesh.
        //virtual void UpdateMaterial(const GNMaterial& material) = 0;

        // Return if the GeomNodes mesh is visible or not.
        virtual bool IsVisible() const = 0;

        //! Set the GeomNodes mesh visible (true) or invisible (false).
        virtual void SetVisiblity(bool visibility) = 0;
    };
} // namespace GeomNodes
