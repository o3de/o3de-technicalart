#include "StdAfx.h"

#include <HoudiniCommon.h>

namespace HoudiniEngine
{
    HoudiniAsset::HoudiniAsset(IHoudini* hou, HAPI_AssetLibraryId id, AZStd::string hdaFile, AZStd::string hdaName) :
        m_hou(hou)
        , m_id(id)
        , m_hdaFile(hdaFile)
        , m_hdaName(hdaName)
    {
        m_session = const_cast<HAPI_Session*>(&hou->GetSession());
        m_assetCount = 0;

        if (m_id >= 0)
        {
            HAPI_GetAvailableAssetCount(m_session, m_id, &m_assetCount);

            if (m_assetCount > 0)
            {
                AZStd::vector<HAPI_StringHandle> assetNameHandles(m_assetCount);
                HAPI_GetAvailableAssets(m_session, m_id, &assetNameHandles[0], m_assetCount);

                for (int i = 0; i < m_assetCount; i++)
                {
                    AZStd::string assetName = hou->GetString(assetNameHandles[i]);
                    m_assets.push_back(assetName);
                }
            }
        }
    }
}// Namespace HoudiniEngine