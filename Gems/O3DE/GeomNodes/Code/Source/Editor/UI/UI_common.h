/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include "Editor/Commons.h"
#include "Utils.h"

namespace GeomNodes
{
    namespace Attributes
    {
        static const AZ::Crc32 FuncValidator = AZ_CRC("GNFuncValidator");
        static const AZ::Crc32 SelectFunction = AZ_CRC("GNSelectFunction");
        static const AZ::Crc32 ObfuscatedText = AZ_CRC("GNObfuscatedText");
        static const AZ::Crc32 ClearButton = AZ_CRC("GNClearButton");
        static const AZ::Crc32 RemovableReadOnly = AZ_CRC("GNRemovableReadOnly");
        static const AZ::Crc32 ValidationChange = AZ_CRC("GNValidationChange");
    } // namespace Attributes

    namespace Handlers
    {
        static const AZ::Crc32 FileSelect = AZ_CRC("GNFileSelect");
        static const AZ::Crc32 QValidatedBrowseEdit = AZ_CRC("GNQValBrowseEdit");
    } // namespace Handlers
} // namespace GeomNodes
