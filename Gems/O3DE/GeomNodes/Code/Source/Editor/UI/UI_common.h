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
