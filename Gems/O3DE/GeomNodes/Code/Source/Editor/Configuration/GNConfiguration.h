/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Memory/Memory.h>
#include <AzCore/std/string/string.h>
#include <GeomNodes/GeomNodesTypeIds.h>


namespace AZ
{
    class ReflectContext;
}

namespace GeomNodes
{
    //! Configuration object that contains global data for GeomNodes System
    struct GNConfiguration
    {
        AZ_CLASS_ALLOCATOR_DECL;
        AZ_TYPE_INFO(GNConfiguration, GNConfigurationTypeId);
        static void Reflect(AZ::ReflectContext* context);

        static GNConfiguration CreateDefault();

        //! Blender executable path in user's machine.
        AZStd::string m_blenderPath;
        //! Last file path used when selecting a blender file.
        AZStd::string m_lastFilePath;

        bool operator==(const GNConfiguration& other) const;
        bool operator!=(const GNConfiguration& other) const;

    private:
        AZ::u32 OnBlenderPathChanged();
    };
} // namespace GeomNodes