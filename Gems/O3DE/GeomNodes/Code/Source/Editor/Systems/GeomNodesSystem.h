#pragma once

#include <AZCore/std/containers/vector.h>
#include <AZCore/std/smart_ptr/unique_ptr.h>
#include <AzToolsFramework/UI/PropertyEditor/PropertyEditorAPI.h>
#include "Editor/EBus/ValidatorBus.h"

namespace GeomNodes
{
    class Validator;
    class ValidationHandler;

    class GeomNodesSystem
        : public ValidatorBus::Handler
    {
    public:
        GeomNodesSystem();
        virtual ~GeomNodesSystem();

        void OnActivate();
        void OnDeactivate();

        FunctorValidator* GetValidator(FunctorValidator::FunctorType) override;
        void TrackValidator(FunctorValidator*) override;

    private:
        // Un/Registers and dis/connect handlers and buses
        void RegisterHandlersAndBuses();
        void UnregisterHandlersAndBuses();

        // Allows lookup and contains all allocated QValidators
        AZStd::unique_ptr<Validator> m_validator;

        AZStd::vector<AzToolsFramework::PropertyHandlerBase*> m_propertyHandlers;
        AZStd::unique_ptr<ValidationHandler> m_validationHandler;
    };
}
