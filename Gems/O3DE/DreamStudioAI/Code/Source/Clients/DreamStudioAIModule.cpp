

#include <DreamStudioAIModuleInterface.h>
#include "DreamStudioAISystemComponent.h"

namespace DreamStudioAI
{
    class DreamStudioAIModule
        : public DreamStudioAIModuleInterface
    {
    public:
        AZ_RTTI(DreamStudioAIModule, "{5DC31AAC-489A-4F20-9D54-72EA11B0179F}", DreamStudioAIModuleInterface);
        AZ_CLASS_ALLOCATOR(DreamStudioAIModule, AZ::SystemAllocator, 0);
    };
}// namespace DreamStudioAI

AZ_DECLARE_MODULE_CLASS(Gem_DreamStudioAI, DreamStudioAI::DreamStudioAIModule)
