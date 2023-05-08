/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "Validators.h"

#include <QDir>
#include <QMimeDatabase>
#include <QRegularExpression>

#define STANDARD_SUCCESS GeomNodes::FunctorValidator::ReturnType(QValidator::Acceptable, "");

namespace
{
    using RetType = GeomNodes::FunctorValidator::ReturnType;

    static const int noMaxLength = -1;
    static const char* blenderMimeType = "application/x-blender";
    static const char* stringEmpty = "String is empty";

    // Returns true if string is valid android package and apple bundle identifier
    RetType RegularExpressionValidator(const QString& pattern, const QString& name, int maxLength = noMaxLength)
    {
        if (maxLength != noMaxLength && name.length() > maxLength)
        {
            return RetType(QValidator::Invalid, QObject::tr("Cannot be longer than %1 characters.").arg(QString::number(maxLength)));
        }
        else if (name.isEmpty())
        {
            return RetType(QValidator::Intermediate, QObject::tr(stringEmpty));
        }

        QRegularExpression regex(pattern);

        QRegularExpressionMatch match = regex.match(name, 0, QRegularExpression::PartialPreferCompleteMatch);

        if (match.hasMatch())
        {
            if (match.capturedLength(0) == name.length())
            {
                return STANDARD_SUCCESS;
            }
            else
            {
                return RetType(QValidator::Intermediate, "Input incorrect.");
            }
        }
        if (match.hasPartialMatch())
        {
            return RetType(QValidator::Intermediate, QObject::tr("Partially matches requirements."));
        }
        else
        {
            return RetType(QValidator::Invalid, QObject::tr("Fails to match requirements at all."));
        }
    }
} // namespace

namespace GeomNodes
{
    namespace Validators
    {
        namespace Internal
        {
            // Returns true if file is readable and the correct mime type
            RetType FileReadableAndCorrectType(const QString& path, const QString& fileType)
            {
                QDir dirPath(path);

                if (dirPath.isReadable())
                {
                    QMimeDatabase mimeDB;
                    QMimeType mimeType = mimeDB.mimeTypeForFile(path);

                    std::string mimeName = mimeType.name().toStdString();
                    if (mimeType.name() == fileType)
                    {
                        return STANDARD_SUCCESS;
                    }
                    else
                    {
                        return RetType(
                            QValidator::Intermediate, QObject::tr("File type should be %1, but is %2.").arg(fileType).arg(mimeType.name()));
                    }
                }
                else
                {
                    return RetType(QValidator::Intermediate, QObject::tr("File is not readable."));
                }
            }
        } // namespace Internal

        // Returns true if valid cross platform file or directory name
        RetType FileName(const QString& name)
        {
            // There was a known issue on android with '.' used in directory names
            // causing problems so it has been omitted from use
            return RegularExpressionValidator("[\\w,-]+", name);
        }

        RetType FileNameOrEmpty(const QString& name)
        {
            if (IsNotEmpty(name).first == QValidator::Acceptable)
            {
                return FileName(name);
            }
            else
            {
                return STANDARD_SUCCESS;
            }
        }

        // Returns true if string isn't empty
        RetType IsNotEmpty(const QString& value)
        {
            if (!value.isEmpty())
            {
                return STANDARD_SUCCESS;
            }
            else
            {
                return RetType(QValidator::Intermediate, QObject::tr(stringEmpty));
            }
        }

        // Returns true if path is empty or a valid blend file relative to <build dir>
        RetType ValidBlenderOrEmpty(const QString& path)
        {
            if (IsNotEmpty(path).first == QValidator::Acceptable)
            {
                return Internal::FileReadableAndCorrectType(path, blenderMimeType);
            }
            else
            {
                return STANDARD_SUCCESS;
            }
        }
    } // namespace Validators
} // namespace GeomNodes
