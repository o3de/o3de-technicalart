#include "HoudiniEngineUtils.h"
#include "HoudiniEngineString.h"
#include <HAPI/HAPI.h>
#include <HoudiniEngine/HoudiniApi.h>
#include <HoudiniEngine/HoudiniEngineBus.h>
#include <AzCore/StringFunc/StringFunc.h>
namespace HoudiniEngine
{
	HAPI_CookOptions HoudiniEngineUtils::GetDefaultCookOptions()
	{
		// Default CookOptions
		HAPI_CookOptions CookOptions;
		HAPI_CookOptions_Init(&CookOptions);

		CookOptions.curveRefineLOD = 8.0f;
		CookOptions.clearErrorsAndWarnings = false;
		CookOptions.maxVerticesPerPrimitive = 3;
		CookOptions.splitGeosByGroup = false;
		CookOptions.splitGeosByAttribute = false;
		CookOptions.splitAttrSH = 0;
		CookOptions.refineCurveToLinear = true;
		CookOptions.handleBoxPartTypes = false;
		CookOptions.handleSpherePartTypes = false;
		CookOptions.splitPointsByVertexAttributes = false;
		CookOptions.packedPrimInstancingMode = HAPI_PACKEDPRIM_INSTANCING_MODE_FLAT;
		CookOptions.cookTemplatedGeos = true;

		return CookOptions;
	}

	bool HoudiniEngineUtils::HapiCookNode(const HAPI_NodeId& InNodeId, HAPI_CookOptions* InCookOptions, const bool& bWaitForCompletion)
	{
		// Check for an invalid node id
		if (InNodeId < 0)
			return false;

		HoudiniPtr hou;
		HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);

		if (hou == nullptr)
			return false;

		// No Cook Options were specified, use the default one
		if (InCookOptions == nullptr)
		{
			// Use the default cook options
			HAPI_CookOptions CookOptions = GetDefaultCookOptions();
			HAPI_CookNode(&hou->GetSession(), InNodeId, &CookOptions);
		}
		else
		{
			// Use the provided CookOptions
			HAPI_CookNode(&hou->GetSession(), InNodeId, InCookOptions);
		}

		// If we don't need to wait for completion, return now
		if (!bWaitForCompletion)
			return true;

		// Wait for the cook to finish
		while (true)
		{
			// Get the current cook status
			int Status = HAPI_STATE_STARTING_COOK;
			HAPI_GetStatus(&hou->GetSession(), HAPI_STATUS_COOK_STATE, &Status);

			if (Status == HAPI_STATE_READY)
			{
				// The cook has been successful.
				return true;
			}
			else if (Status == HAPI_STATE_READY_WITH_FATAL_ERRORS || Status == HAPI_STATE_READY_WITH_COOK_ERRORS)
			{
				// There was an error while cooking the node.
				//FString CookResultString = FHoudiniEngineUtils::GetCookResult();
				//HOUDINI_LOG_ERROR();
				return false;
			}

			// We want to yield a bit.
			AZStd::this_thread::sleep_for(AZStd::chrono::milliseconds(100));
		}
	}

	bool HoudiniEngineUtils::HapiGetParameterDataAsString(const HAPI_NodeId& NodeId, const AZStd::string& ParmName, const AZStd::string& DefaultValue, AZStd::string& OutValue)
    {
        HoudiniPtr hou;
        HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);

        OutValue = DefaultValue;

        if (hou != nullptr)
        {
			// Try to find the parameter by name
			HAPI_ParmId ParmId = -1;
			HAPI_GetParmIdFromName(&hou->GetSession(), NodeId, ParmName.c_str(), &ParmId);

			if (ParmId < 0)
				return false;

			// Get the param info...
			HAPI_ParmInfo FoundParamInfo;
			HAPI_ParmInfo_Init(&FoundParamInfo);
			HAPI_GetParmInfo(&hou->GetSession(), NodeId, ParmId, &FoundParamInfo);

			// .. and value
			HAPI_StringHandle StringHandle;
			HAPI_GetParmStringValues(&hou->GetSession(), NodeId, false, &StringHandle, FoundParamInfo.stringValuesIndex, 1);

			// Convert the string handle to FString
			return HoudiniEngineString::ToAZString(StringHandle, OutValue);
        }
        
		return false;
    }

	bool HoudiniEngineUtils::HapiGetAttributeDataAsStringFromInfo(const HAPI_NodeId& InGeoId, const HAPI_PartId& InPartId, const char* InAttribName, HAPI_AttributeInfo& InAttributeInfo, AZStd::vector<AZStd::string>& OutData, const AZ::s32& InStartIndex, const AZ::s32& InCount)
	{
		if (!InAttributeInfo.exists)
			return false;

		HoudiniPtr hou;
		HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);

		if (hou != nullptr)
		{
			// Handle partial reading of attributes
			AZ::s32 Start = 0;
			if (InStartIndex > 0 && InStartIndex < InAttributeInfo.count)
				Start = InStartIndex;

			AZ::s32 Count = InAttributeInfo.count;
			if (InCount > 0)
			{
				if ((Start + InCount) <= InAttributeInfo.count)
					Count = InCount;
				else
					Count = InAttributeInfo.count - Start;
			}

			// Extract the StringHandles
			AZStd::vector<HAPI_StringHandle> StringHandles;
			StringHandles.resize_no_construct(Count * InAttributeInfo.tupleSize);
			for (AZ::s32 n = 0; n < StringHandles.size(); n++)
				StringHandles[n] = -1;

			HAPI_GetAttributeStringData(&hou->GetSession(), InGeoId, InPartId, InAttribName, &InAttributeInfo, &StringHandles[0], Start, Count);

			// Set the output data size
			OutData.resize(StringHandles.size());

			// Convert the StringHandles to FString.
			// using a map to minimize the number of HAPI calls
			HoudiniEngineString::SHArrayToStringArray(StringHandles, OutData);

			return true;
		}

		return false;
	}

	bool HoudiniEngineUtils::HapiGetAttributeDataAsFloat(const HAPI_NodeId& InGeoId, const HAPI_PartId& InPartId, const char* InAttribName, HAPI_AttributeInfo& OutAttributeInfo, AZStd::vector<float>& OutData, AZ::s32 InTupleSize, HAPI_AttributeOwner InOwner, const AZ::s32& InStartIndex, const AZ::s32& InCount)
	{
		OutAttributeInfo.exists = false;

		// Reset container size.
		OutData.clear();
		
		HoudiniPtr hou;
		HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);

		if (hou != nullptr)
		{
			AZ::s32 OriginalTupleSize = InTupleSize;

			HAPI_AttributeInfo AttributeInfo;
			HAPI_AttributeInfo_Init(&AttributeInfo);
			//HAPI_AttributeInfo AttributeInfo = HAPI_AttributeInfo_Create();
			if (InOwner == HAPI_ATTROWNER_INVALID)
			{
				for (AZ::s32 AttrIdx = 0; AttrIdx < HAPI_ATTROWNER_MAX; ++AttrIdx)
				{
					HAPI_GetAttributeInfo(&hou->GetSession(), InGeoId, InPartId, InAttribName, (HAPI_AttributeOwner)AttrIdx, &AttributeInfo);

					if (AttributeInfo.exists)
						break;
				}
			}
			else
			{
				HAPI_GetAttributeInfo(&hou->GetSession(), InGeoId, InPartId, InAttribName, InOwner, &AttributeInfo);
			}

			if (!AttributeInfo.exists)
				return false;

			if (OriginalTupleSize > 0)
				AttributeInfo.tupleSize = OriginalTupleSize;

			// Store the retrieved attribute information.
			OutAttributeInfo = AttributeInfo;

			// Handle partial reading of attributes
			AZ::s32 Start = 0;
			if (InStartIndex > 0 && InStartIndex < AttributeInfo.count)
				Start = InStartIndex;

			AZ::s32 Count = AttributeInfo.count;
			if (InCount > 0)
			{
				if ((Start + InCount) <= AttributeInfo.count)
					Count = InCount;
				else
					Count = AttributeInfo.count - Start;
			}

			if (AttributeInfo.storage == HAPI_STORAGETYPE_FLOAT)
			{
				// Allocate sufficient buffer for data.
				OutData.resize(Count * AttributeInfo.tupleSize);

				// Fetch the values
				HAPI_GetAttributeFloatData(&hou->GetSession(), InGeoId, InPartId, InAttribName, &AttributeInfo, -1, &OutData[0], Start, Count);

				return true;
			}
			else if (AttributeInfo.storage == HAPI_STORAGETYPE_INT)
			{
				// Expected Float, found an int, try to convert the attribute

				// Allocate sufficient buffer for data.
				AZStd::vector<AZ::s32> IntData;
				IntData.resize(Count * AttributeInfo.tupleSize);

				// Fetch the values
				if (HAPI_RESULT_SUCCESS == HAPI_GetAttributeIntData(&hou->GetSession(), InGeoId, InPartId, InAttribName, &AttributeInfo, -1, &IntData[0], Start, Count))
				{
					OutData.resize(IntData.size());
					for (AZ::s32 Idx = 0; Idx < IntData.size(); Idx++)
					{
						OutData[Idx] = (float)IntData[Idx];
					}

					//HOUDINI_LOG_MESSAGE(TEXT("Attribute %s was expected to be a float attribute, its value had to be converted from integer."), *FString(InAttribName));
					return true;
				}
			}
			else if (AttributeInfo.storage == HAPI_STORAGETYPE_STRING)
			{
				// Expected Float, found a string, try to convert the attribute
				AZStd::vector<AZStd::string> StringData;
				if (HoudiniEngineUtils::HapiGetAttributeDataAsStringFromInfo(
					InGeoId, InPartId, InAttribName,
					AttributeInfo, StringData,
					Start, Count))
				{
					bool bConversionError = false;
					OutData.resize(StringData.size());
					for (AZ::s32 Idx = 0; Idx < StringData.size(); Idx++)
					{
						if (!AZ::StringFunc::LooksLikeFloat(StringData[Idx].data(), &OutData[Idx]))
							bConversionError = true;
					}

					if (!bConversionError)
					{
						//HOUDINI_LOG_MESSAGE(TEXT("Attribute %s was expected to be a float attribute, its value had to be converted from string."), *FString(InAttribName));
						return true;
					}
				}
			}
		}

		//HOUDINI_LOG_WARNING(TEXT("Found attribute %s, but it was expected to be a float attribute and is of an invalid type."), *FString(InAttribName));
		return false;
	}

	bool HoudiniEngineUtils::HapiGetParameterDataAsInteger(const HAPI_NodeId& NodeId, const AZStd::string& ParmName, const AZ::s32& DefaultValue, AZ::s32& OutValue)
	{
		HoudiniPtr hou;
		HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);

		OutValue = DefaultValue;

		if (hou != nullptr)
		{
			// Try to find the parameter by its name
			HAPI_ParmId ParmId = -1;
			HAPI_GetParmIdFromName(&hou->GetSession(), NodeId, ParmName.c_str(), &ParmId);

			if (ParmId < 0)
				return false;

			// Get the param info...
			HAPI_ParmInfo FoundParmInfo;
			HAPI_ParmInfo_Init(&FoundParmInfo);
			HAPI_GetParmInfo(&hou->GetSession(), NodeId, ParmId, &FoundParmInfo);

			// .. and value
			AZ::s32 Value = DefaultValue;
			HAPI_GetParmIntValues(&hou->GetSession(), NodeId, &Value, FoundParmInfo.intValuesIndex, 1);

			OutValue = Value;

			return true;
		}

		return false;
	}

	HAPI_ParmId HoudiniEngineUtils::HapiFindParameterByName(const HAPI_NodeId& InNodeId, const AZStd::string& InParmName, HAPI_ParmInfo& OutFoundParmInfo)
	{
		HoudiniPtr hou;
		HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);
		if (hou == nullptr)
			return -1;

		// Try to find the parameter by its name
		HAPI_ParmId ParmId = -1;
		HAPI_GetParmIdFromName(&hou->GetSession(), InNodeId, InParmName.c_str(), &ParmId);

		if (ParmId < 0)
			return -1;

		HAPI_ParmInfo_Init(&OutFoundParmInfo);
		HAPI_GetParmInfo(&hou->GetSession(), InNodeId, ParmId, &OutFoundParmInfo);

		return ParmId;
	}

	HAPI_ParmId HoudiniEngineUtils::HapiFindParameterByTag(const HAPI_NodeId& InNodeId, const AZStd::string& InParmTag, HAPI_ParmInfo& OutFoundParmInfo)
	{
		HoudiniPtr hou;
		HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);
		if (hou == nullptr)
			return -1;

		// Try to find the parameter by its tag
		HAPI_ParmId ParmId = -1;
		HAPI_GetParmWithTag(&hou->GetSession(), InNodeId, InParmTag.c_str(), &ParmId);

		if (ParmId < 0)
			return -1;

		HAPI_ParmInfo_Init(&OutFoundParmInfo);
		HAPI_GetParmInfo(&hou->GetSession(), InNodeId, ParmId, &OutFoundParmInfo);

		return ParmId;
	}
}