#pragma once

#include "Editor/UI/FunctorValidator.h"

#include <AzCore/EBus/EBus.h>

namespace GeomNodes
{
    class ValidatorTraits
        : public AZ::EBusTraits
    {
    public:
        using Bus = AZ::EBus<ValidatorTraits>;

        // Bus Configuration
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;

        virtual FunctorValidator* GetValidator(FunctorValidator::FunctorType) = 0;
        virtual void TrackValidator(FunctorValidator*) = 0;
    };

    typedef AZ::EBus<ValidatorTraits> ValidatorBus;
} // namespace GeomNodes
