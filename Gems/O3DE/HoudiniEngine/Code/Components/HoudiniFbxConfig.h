#pragma once

#include <AzCore/RTTI/ReflectContext.h>
namespace HoudiniEngine
{
    class HoudiniFbxConfig
    {
    public:
        AZ_CLASS_ALLOCATOR(HoudiniFbxConfig, AZ::SystemAllocator);
        AZ_TYPE_INFO(HoudiniFbxConfig, "{75F6B0E3-685B-459C-896E-6164AA567B34}");
        static void Reflect(AZ::ReflectContext* context);

        bool m_removeHdaAfterBake = false;
        bool m_replacePreviousBake = false;
        bool m_cmdSaveFbx = false;
        AZ::u32 m_bakeCounter = 0;
        void Bake();
    };
}