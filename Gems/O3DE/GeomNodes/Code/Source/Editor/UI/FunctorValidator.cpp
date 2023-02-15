#include "FunctorValidator.h"

namespace GeomNodes
{
    FunctorValidator::FunctorValidator(FunctorType functor)
        : QValidator()
        , m_functor(functor)
    {
    }

    FunctorValidator::FunctorValidator()
        : QValidator()
        , m_functor(nullptr)
    {
    }

    QValidator::State FunctorValidator::validate(QString& input, [[maybe_unused]] int& pos) const
    {
        return m_functor(input).first;
    }

    FunctorValidator::ReturnType FunctorValidator::ValidateWithErrors(const QString& input) const
    {
        return m_functor(input);
    }

    FunctorValidator::FunctorType FunctorValidator::Functor() const
    {
        return m_functor;
    }
} // namespace GeomNodes

#include <moc_FunctorValidator.cpp>
