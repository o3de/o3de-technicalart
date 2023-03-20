#pragma once

#include <AzCore/std/string/string.h>
#include <AzCore/Memory/Memory.h>

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
        AZ_TYPE_INFO(GNConfiguration, "{828F21E8-D1C4-480E-A29B-33B618695874}");
        static void Reflect(AZ::ReflectContext* context);

        static GNConfiguration CreateDefault();

        AZStd::string m_blenderPath = "C:/Program Files/Blender Foundation/Blender 3.4/blender.exe"; //!< Currently set blender path in user's machine.

        AZStd::string m_lastFilePath; //!< Last file path used when selecting a blender file.

        bool operator==(const GNConfiguration& other) const;
        bool operator!=(const GNConfiguration& other) const;
    
    private:
        AZ::u32 OnBlenderPathChanged();
    };
} // namespace GeomNodes