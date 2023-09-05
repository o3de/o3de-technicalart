#pragma once

#include "HAPI/HAPI_Common.h"

namespace HoudiniEngine
{
    struct HoudiniEngineUtils
    {
    public:
		// Default cook options
		// NOTE: this is originally from HoudiniEngine class
		static HAPI_CookOptions GetDefaultCookOptions();

		// Cook the specified node id
		// if the cook options are null, the default one will be used
		// if bWaitForCompletion is true, this call will be blocking until the cook is finished
        static bool HapiCookNode(const HAPI_NodeId& InNodeId, HAPI_CookOptions* InCookOptions = nullptr, const bool& bWaitForCompletion = false);

        static bool HapiGetParameterDataAsString(const HAPI_NodeId& NodeId, const AZStd::string& ParmName, const AZStd::string& DefaultValue, AZStd::string& OutValue);
		static bool HapiGetAttributeDataAsStringFromInfo(const HAPI_NodeId& InGeoId, const HAPI_PartId& InPartId, const char* InAttribName, HAPI_AttributeInfo& InAttributeInfo, AZStd::vector<AZStd::string>& OutData, const AZ::s32& InStartIndex = 0, const AZ::s32& InCount = -1);
		static bool HapiGetAttributeDataAsFloat(const HAPI_NodeId& InGeoId, const HAPI_PartId& InPartId, const char* InAttribName, HAPI_AttributeInfo& OutAttributeInfo, AZStd::vector<float>& OutData, 
            AZ::s32 InTupleSize = 0, HAPI_AttributeOwner InOwner = HAPI_ATTROWNER_INVALID, const AZ::s32& InStartIndex = 0, const AZ::s32& InCount = -1);
		static bool HapiGetParameterDataAsInteger(const HAPI_NodeId& NodeId, const AZStd::string& ParmName, const AZ::s32& DefaultValue, AZ::s32& OutValue);

		// HAPI : Look for a parameter by name and returns its index. Returns -1 if not found.
		static HAPI_ParmId HapiFindParameterByName(const HAPI_NodeId& InNodeId, const AZStd::string& InParmName, HAPI_ParmInfo& OutFoundParmInfo);

		// HAPI : Look for a parameter by tag and returns its index. Returns -1 if not found.
		static HAPI_ParmId HapiFindParameterByTag(const HAPI_NodeId& InNodeId, const AZStd::string& InParmTag, HAPI_ParmInfo& OutFoundParmInfo);

		static AZStd::string GetLevelPath()
		{

			AZStd::string levelName;
			AzToolsFramework::EditorRequestBus::BroadcastResult(levelName, &AzToolsFramework::EditorRequestBus::Events::GetLevelName);

			return "levels/" + levelName;
		}

		static AZStd::string GetProjectRoot()
		{
			AZStd::string folder = AZStd::string("@projectroot@/");
			char filePath[AZ_MAX_PATH_LEN] = { 0 };
			AZ::IO::FileIOBase::GetInstance()->ResolvePath(folder.c_str(), filePath, AZ_MAX_PATH_LEN);
			folder = AZStd::string(filePath);
			return folder;
		}

		static AZStd::string GetOutputExportFolder()
		{
			const AZStd::string& levelName = GetLevelPath();
			AZStd::string folder = AZStd::string("@projectroot@/") + levelName + AZStd::string("/ExportedGeometry/");
			char filePath[AZ_MAX_PATH_LEN] = { 0 };
			AZ::IO::FileIOBase::GetInstance()->ResolvePath(folder.c_str(), filePath, AZ_MAX_PATH_LEN);
			folder = AZStd::string(filePath);
			return folder;
		}

		static AZStd::string GetOutputFolder()
		{
			const AZStd::string& levelName = GetLevelPath();
			AZStd::string folder = AZStd::string("@projectroot@/") + levelName + AZStd::string("/HoudiniCache/");
			char filePath[AZ_MAX_PATH_LEN] = { 0 };
			AZ::IO::FileIOBase::GetInstance()->ResolvePath(folder.c_str(), filePath, AZ_MAX_PATH_LEN);
			folder = AZStd::string(filePath);
			return folder;
		}

		static AZStd::string GetRelativeOutputFolder()
		{
			const AZStd::string& levelName = GetLevelPath();
			const AZStd::string assetPath = levelName + AZStd::string("/HoudiniCache/");
			return assetPath;
		}
    };
    
}