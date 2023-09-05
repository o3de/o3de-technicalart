
#include "StdAfx.h"

#include <HoudiniEngineModuleInterface.h>
#include "HoudiniEngineEditorSystemComponent.h"
#include <Components/HoudiniAssetComponent.h>
#include <Components/HoudiniScatterComponent.h>
#include <Components/HoudiniTerrainComponent.h>
#include <Components/HoudiniCurveAttributeComponent.h>

namespace HoudiniEngine
{
    class HoudiniEngineEditorModule
        : public HoudiniEngineModuleInterface
    {
    public:
        AZ_RTTI(HoudiniEngineEditorModule, "{11F7E041-CA6B-4BF5-9E84-79E77947BD2E}", HoudiniEngineModuleInterface);
        AZ_CLASS_ALLOCATOR(HoudiniEngineEditorModule, AZ::SystemAllocator, 0);

        HoudiniEngineEditorModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                HoudiniEngineEditorSystemComponent::CreateDescriptor(),
                HoudiniAssetComponent::CreateDescriptor(),
                HoudiniScatterComponent::CreateDescriptor(),
                HoudiniTerrainComponent::CreateDescriptor(),
                HoudiniCurveAttributeComponent::CreateDescriptor(),
                //HoudiniCacheComponent::CreateDescriptor()
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<HoudiniEngineEditorSystemComponent>(),
            };
        }
    };
}// namespace HoudiniEngine

AZ_DECLARE_MODULE_CLASS(Gem_HoudiniEngine, HoudiniEngine::HoudiniEngineEditorModule)
