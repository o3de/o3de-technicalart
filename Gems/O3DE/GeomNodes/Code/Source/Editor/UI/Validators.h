#pragma once

#include "FunctorValidator.h"

namespace GeomNodes
{
    namespace Validators
    {
        namespace Internal
        {
            // Returns true if file is readable and the correct mime type
            FunctorValidator::ReturnType FileReadableAndCorrectType(const QString& path, const QString& fileType);
        } // namespace Internal

        // Returns true if valid cross platform file or directory name
        FunctorValidator::ReturnType FileName(const QString& name);
        // Returns true if valid cross platform file or directory name or empty
        FunctorValidator::ReturnType FileNameOrEmpty(const QString& name);
        // Returns true if string isn't empty
        FunctorValidator::ReturnType IsNotEmpty(const QString& value);
        // Returns true if path is empty or a valid blend file relative to <build dir>
        FunctorValidator::ReturnType ValidBlenderOrEmpty(const QString& path);
    }
}
