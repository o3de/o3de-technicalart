
#pragma once

#include <HoudiniEngine/HoudiniEngineBus.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>

namespace HoudiniEngine
{
	class HoudiniStatusPanel;

	class HoudiniEngineSystemComponent
		: public AZ::Component
		, public AZ::TickBus::Handler
	{
	public:
		AZ_COMPONENT(HoudiniEngineSystemComponent, "{E243F213-21DA-401B-8502-8385169FE879}");
		HoudiniEngineSystemComponent();
		virtual ~HoudiniEngineSystemComponent();

		static void Reflect(AZ::ReflectContext* context);

		static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
		static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
		static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
		static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

	protected:
		////////////////////////////////////////////////////////////////////////
		// AZ::Component interface implementation
		void Init() override;
		void Activate() override;
		void Deactivate() override;
		////////////////////////////////////////////////////////////////////////

		void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

	};
}
