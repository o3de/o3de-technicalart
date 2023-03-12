#include "Utils.h"

#include <AzCore/IO/SystemFile.h>
#include <AzCore/std/algorithm.h>
#include <AzCore/Utils/Utils.h>

#include <QFileDialog>

namespace
{
    template<typename StringType>
    void ToUnixPath(StringType& path)
    {
        AZStd::replace(path.begin(), path.end(), '\\', '/');
    }

    template<typename StringType>
    StringType GetAbsoluteEngineRoot()
    {
        AZ::IO::FixedMaxPath engineRoot = AZ::Utils::GetEnginePath();

        if (engineRoot.empty())
        {
            return "";
        }

        StringType engineRootString(engineRoot.c_str());
        ToUnixPath(engineRootString);
        return engineRootString;
    }

    template<typename StringType>
    StringType GetAbsoluteProjectRoot()
    {
        AZ::IO::FixedMaxPath projectRoot = AZ::Utils::GetProjectPath();

        if (projectRoot.empty())
        {
            return "";
        }

        StringType projectRootString(projectRoot.c_str());
        ToUnixPath(projectRootString);
        return projectRootString;
    }

    template<typename StringType>
    StringType GetProjectName();

    template<>
    AZStd::string GetProjectName()
    {
        auto projectName = AZ::Utils::GetProjectName();
        return AZStd::string{projectName.c_str()};
    }

    template<>
    QString GetProjectName()
    {
        auto projectName = AZ::Utils::GetProjectName();
        return QString::fromUtf8(projectName.c_str(), aznumeric_cast<int>(projectName.size()));
    }
}

namespace GeomNodes
{
    void* ConvertFunctorToVoid(AZStd::pair<QValidator::State, const QString> (*func)(const QString&))
    {
        return reinterpret_cast<void*>(func);
    }

    void* ConvertFunctorToVoid(void (*func)(const AZStd::string&))
    {
        return reinterpret_cast<void*>(func);
    }

	AZStd::string GetEngineRoot()
	{
		return GetAbsoluteEngineRoot<AZStd::string>();
	}
	AZStd::string GetProjectRoot()
	{
		return GetAbsoluteProjectRoot<AZStd::string>();
	}

	AZStd::string GetProjectName()
	{
		return ::GetProjectName<AZStd::string>();
	}

    QString SelectBlendFromFileDialog(const QString& currentFile)
    {
        // The selected file must be relative to this path
        QString defaultPath = GetAbsoluteEngineRoot<QString>();
        QString startPath;

        // Choose the starting path for file dialog
        if (currentFile != "")
        {
            if (currentFile.contains(defaultPath))
            {
                startPath = currentFile;
            }
            else
            {
                startPath = defaultPath + currentFile;
            }
        }
        else
        {
            startPath = defaultPath;
        }

        QString pickedPath = QFileDialog::getOpenFileName(nullptr, QObject::tr("Select Blend file"),
            startPath, QObject::tr("Blender File (*.blend)"));
        ToUnixPath(pickedPath);

        // Remove the default relative path
        if (pickedPath.contains(defaultPath))
        {
            pickedPath = pickedPath.mid(defaultPath.length());
        }

        return pickedPath;
    }
}