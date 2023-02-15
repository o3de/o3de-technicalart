#include "GeomNodesValidator.h"

#include "Validators.h"

namespace GeomNodes
{
    Validator::Validator()
    {}

    Validator::~Validator()
    {
        for (AZStd::pair<FunctorValidator::FunctorType, FunctorValidator*>& pair : m_validatorToQValidator)
        {
            delete pair.second;
        }
        for (FunctorValidator* qValidator : m_otherValidators)
        {
            delete qValidator;
        }
    }

    FunctorValidator* Validator::GetQValidator(FunctorValidator::FunctorType validator)
    {
        if (validator != nullptr)
        {
            auto iter = m_validatorToQValidator.find(validator);

            if (iter != m_validatorToQValidator.end())
            {
                return iter->second;
            }
            else
            {
                FunctorValidator* qValidator = new FunctorValidator(validator);
                m_validatorToQValidator.insert(
                    AZStd::pair<FunctorValidator::FunctorType, FunctorValidator*>(validator, qValidator));

                return qValidator;
            }
        }
        else
        {
            return nullptr;
        }
    }

    void Validator::TrackThisValidator(FunctorValidator* validator)
    {
        m_otherValidators.push_back(validator);
    }
} // namespace GeomNodes
