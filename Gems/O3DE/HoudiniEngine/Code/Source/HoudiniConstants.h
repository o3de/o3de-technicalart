/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

namespace HoudiniEngine
{
    //! Attributes for mesh vertices.
    enum class AttributeType
    {
        Position,
        Normal,
        Tangent,
        Bitangent,
        UV,
        Color
    };

    namespace Handlers
    {
        static const AZ::Crc32 FolderList = AZ_CRC("HEFolderList");
    }
} // namespace HoudiniEngine