#pragma once

#include <HoudiniEngine/HoudiniApi.h>

namespace HoudiniEngine 
{
    class HoudiniAsset : public IHoudiniAsset
    {
        protected:
            IHoudini* m_hou;
            HAPI_Session* m_session;
            HAPI_AssetLibraryId m_id;
            AZStd::string m_hdaFile;
            AZStd::string m_hdaName;
            
            int m_assetCount;
            AZStd::vector<AZStd::string> m_assets;            
        public:

            HoudiniAsset(IHoudini* hou, HAPI_AssetLibraryId id, AZStd::string hdaFile, AZStd::string hdaName);
            virtual ~HoudiniAsset() = default;

            const AZStd::string& GetHdaName()
            {
                return m_hdaName;
            }
            
            const AZStd::string& GetHdaFile()
            {
                return m_hdaFile;
            }

            const AZStd::vector<AZStd::string>& getAssets()
            {
                return m_assets;
            }
    };
}
