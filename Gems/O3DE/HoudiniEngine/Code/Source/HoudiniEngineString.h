#pragma once
#include <AzCore/std/string/string.h>
#include <AzCore/std/containers/vector.h>

namespace HoudiniEngine
{
    class HoudiniEngineString
    {
        public:

            HoudiniEngineString();
            HoudiniEngineString(AZ::s32 InStringId);
            HoudiniEngineString(const HoudiniEngineString & Other);

            HoudiniEngineString & operator=(const HoudiniEngineString & Other);

            bool operator==(const HoudiniEngineString & Other) const;
            bool operator!=(const HoudiniEngineString & Other) const;

            // Conversion functions
            bool ToAZString(AZStd::string & String) const;
            
            // Static converters
            static bool ToAZString(const AZ::s32& InStringId, AZStd::string & String);
            
			// Array converter, uses a map to avoid redundant calls to HAPI
			static bool SHArrayToStringArray(const AZStd::vector<AZ::s32>& InStringIdArray, AZStd::vector<AZStd::string>& OutStringArray);

			// Array converter, uses string batches and a map to reduce HAPI calls
			static bool SHArrayToStringArray_Batch(const AZStd::vector<AZ::s32>& InStringIdArray, AZStd::vector<AZStd::string>& OutStringArray);

			// Array converter, uses a map to reduce HAPI calls
			static bool SHArrayToStringArray_Singles(const AZStd::vector<AZ::s32>& InStringIdArray, AZStd::vector<AZStd::string>& OutStringArray);

            // Return id of this string.
            AZ::s32 GetId() const;

            // Return true if this string has a valid id.
            bool HasValidId() const;

        protected:

            // Id of the underlying Houdini Engine string.
            AZ::s32 StringId;
    };
}