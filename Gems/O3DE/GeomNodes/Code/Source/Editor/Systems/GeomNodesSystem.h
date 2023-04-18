#pragma once

#include <AzCore/Interface/Interface.h>
#include <Editor/Systems/GNSystemInterface.h>
#include <Editor/Configuration/GNConfiguration.h>
#include <Editor/Configuration/GNSettingsRegistryManager.h>
#include <AZCore/std/containers/vector.h>
#include <AZCore/std/smart_ptr/unique_ptr.h>
#include <AzToolsFramework/UI/PropertyEditor/PropertyEditorAPI.h>
#include "Editor/EBus/ValidatorBus.h"
#include <GeomNodes/GeomNodesTypeIds.h>

namespace GeomNodes
{
    class Validator;
    class ValidationHandler;

    class GeomNodesSystem
        : public AZ::Interface<GNSystemInterface>::Registrar
        , public ValidatorBus::Handler
    {
    public:
        AZ_RTTI(GeomNodesSystem, GeomNodesSystemTypeId, GeomNodes::GNSystemInterface);

        GeomNodesSystem(AZStd::unique_ptr<GNSettingsRegistryManager> registryManager);
        virtual ~GeomNodesSystem();

        // GNSystemInterface interface ...
        void Initialize(const GNConfiguration* config) override;
        void Shutdown() override;
        AZStd::string_view GetBlenderPath() override;
		const GNConfiguration* GetConfiguration() const override;
        void UpdateConfiguration(const GNConfiguration* newConfig) override;

        //! Accessor to get the Settings Registry Manager.
        const GNSettingsRegistryManager& GetSettingsRegistryManager() const;

        FunctorValidator* GetValidator(FunctorValidator::FunctorType) override;
        void TrackValidator(FunctorValidator*) override;

        const GNConfiguration& GetSystemConfiguration() const;

        void SetLastPath(const AZStd::string& lastPath);
        AZStd::string GetLastPath();

    private:
        // Un/Registers and dis/connect handlers and buses
        void RegisterHandlersAndBuses();
        void UnregisterHandlersAndBuses();

        AZStd::unique_ptr<GNSettingsRegistryManager> m_registryManager; //! Handles all settings registry interactions.

        // Allows lookup and contains all allocated QValidators
        AZStd::unique_ptr<Validator> m_validator;

        AZStd::vector<AzToolsFramework::PropertyHandlerBase*> m_propertyHandlers;
        AZStd::unique_ptr<ValidationHandler> m_validationHandler;

        GNConfiguration m_systemConfig;

		enum class State : AZ::u8
		{
			Uninitialized = 0,
			Initialized,
			Shutdown
		};
		State m_state = State::Uninitialized;
    };

	//! Helper function for getting the GeomNOdes System interface from inside the GeomNodes gem.
	GeomNodesSystem* GetGNSystem();
}
