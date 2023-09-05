#include "HoudiniEngineString.h"
#include <HAPI/HAPI.h>
#include <HoudiniEngine/HoudiniApi.h>
#include <HoudiniEngine/HoudiniEngineBus.h>
namespace HoudiniEngine
{
    HoudiniEngineString::HoudiniEngineString()
        : StringId(-1)
    {}

    HoudiniEngineString::HoudiniEngineString(AZ::s32 InStringId)
        : StringId(InStringId)
    {}

    HoudiniEngineString::HoudiniEngineString(const HoudiniEngineString& Other)
        : StringId(Other.StringId)
    {}

    HoudiniEngineString &
    HoudiniEngineString::operator=(const HoudiniEngineString& Other)
    {
        if (this != &Other)
            StringId = Other.StringId;

        return *this;
    }

    bool
    HoudiniEngineString::operator==(const HoudiniEngineString& Other) const
    {
        return Other.StringId == StringId;
    }

    bool
    HoudiniEngineString::operator!=(const HoudiniEngineString& Other) const
    {
        return Other.StringId != StringId;
    }

    AZ::s32
    HoudiniEngineString::GetId() const
    {
        return StringId;
    }

    bool
    HoudiniEngineString::HasValidId() const
    {
        return StringId > 0;
    }

    bool
    HoudiniEngineString::ToAZString(AZStd::string& String) const
    {
		HoudiniPtr hou;
		HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);

		if (hou == nullptr)
		{
			return false;
		}

		String = "";

		// Null string ID / zero should be considered invalid
		// (or we'd get the "null string, should never see this!" text)
		if (StringId <= 0)
		{
			return false;
		}

		AZ::s32 NameLength = 0;
		if (HAPI_RESULT_SUCCESS != HAPI_GetStringBufLength(&hou->GetSession(), StringId, &NameLength))
		{
			return false;
		}

		if (NameLength <= 0)
			return false;

		std::vector<char> NameBuffer(NameLength, '\0');
		if (HAPI_RESULT_SUCCESS != HAPI_GetString(&hou->GetSession(), StringId, &NameBuffer[0], NameLength))
		{
			return false;
		}

		String = AZStd::string(NameBuffer.begin(), NameBuffer.end());

		return true;
    }

    bool
    HoudiniEngineString::ToAZString(const AZ::s32& InStringId, AZStd::string& OutString)
    {
        HoudiniEngineString HAPIString(InStringId);
        return HAPIString.ToAZString(OutString);
    }

    bool HoudiniEngineString::SHArrayToStringArray(const AZStd::vector<AZ::s32>& InStringIdArray, AZStd::vector<AZStd::string>& OutStringArray)
    {
		if (SHArrayToStringArray_Batch(InStringIdArray, OutStringArray))
			return true;

		return SHArrayToStringArray_Singles(InStringIdArray, OutStringArray);
    }

    bool HoudiniEngineString::SHArrayToStringArray_Batch(const AZStd::vector<AZ::s32>& InStringIdArray, AZStd::vector<AZStd::string>& OutStringArray)
    {
		bool bReturn = true;

		HoudiniPtr hou;
		HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);

		if (hou == nullptr)
		{
			return bReturn;
		}

		OutStringArray.resize(InStringIdArray.size());

		AZStd::set<AZ::s32> UniqueSH;
		for (const auto& CurrentSH : InStringIdArray)
		{
			UniqueSH.emplace(CurrentSH);
		}

		AZStd::vector<AZ::s32> UniqueSHArray(UniqueSH.begin(), UniqueSH.end());

		AZ::s32 BufferSize = 0;
		if (HAPI_RESULT_SUCCESS != HAPI_GetStringBatchSize(&hou->GetSession(), UniqueSHArray.data(), UniqueSHArray.size(), &BufferSize))
			return false;

		if (BufferSize <= 0)
			return false;

		AZStd::string Buffer;
		Buffer.resize(BufferSize);
		if (HAPI_RESULT_SUCCESS != HAPI_GetStringBatch(&hou->GetSession(), &Buffer[0], BufferSize))
			return false;

		// Parse the buffer to a string array
		AZStd::map<int, AZStd::string> StringMap;
		int Index = 0;
		int StringOffset = 0;
		while (StringOffset < BufferSize)
		{
			// Add the current string to our dictionary.
			StringMap.emplace(UniqueSHArray[Index], Buffer);

			// Move on to next indexed string
			Index++;
			while (Buffer[StringOffset] != 0 && StringOffset < BufferSize)
				StringOffset++;

			StringOffset++;
		}

		if (StringMap.size() != UniqueSH.size())
			return false;

		// Fill the output array using the map
		for (AZ::s32 IdxSH = 0; IdxSH < InStringIdArray.size(); IdxSH++)
		{
			OutStringArray[IdxSH] = StringMap[InStringIdArray[IdxSH]];
		}

		return true;
    }
    
    bool HoudiniEngineString::SHArrayToStringArray_Singles(const AZStd::vector<AZ::s32>& InStringIdArray, AZStd::vector<AZStd::string>& OutStringArray)
    {
		bool bReturn = true;
		OutStringArray.resize(InStringIdArray.size());

		// Avoid calling HAPI to resolve the same strings again and again
		AZStd::map<HAPI_StringHandle, AZ::s32> ResolvedStrings;
		for (AZ::s32 IdxSH = 0; IdxSH < InStringIdArray.size(); IdxSH++)
		{
			auto ResolvedString = ResolvedStrings.find(InStringIdArray[IdxSH]);
			if (ResolvedString != ResolvedStrings.end())
			{
				// Already resolved earlier, copy the string instead of calling HAPI.
				OutStringArray[IdxSH] = OutStringArray[ResolvedString->second];
			}
			else
			{
				AZStd::string CurrentString;
				if (!ToAZString(InStringIdArray[IdxSH], CurrentString))
					bReturn = false;

				OutStringArray[IdxSH] = CurrentString;
				ResolvedStrings.emplace(InStringIdArray[IdxSH], IdxSH);
			}
		}

		return bReturn;
    }
}