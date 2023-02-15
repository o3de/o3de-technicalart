#pragma once

#include <AzCore/std/string/string.h>

#include <QString>
#include <QValidator>

// Derived from ProjectSettingsTool
namespace GeomNodes
{
    void* ConvertFunctorToVoid(AZStd::pair<QValidator::State, const QString>(*func)(const QString&));
    void* ConvertFunctorToVoid(void (*func)(const AZStd::string&));
    AZStd::string GetEngineRoot();
    AZStd::string GetProjectRoot();
    AZStd::string GetProjectName();
    
    // Open file dialogs for each file type and return the result
    // CurrentFile is where the dialog opens
    QString SelectBlendFromFileDialog(const QString& currentFile);
}
