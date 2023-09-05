#pragma once

#include <AzCore/std/containers/vector.h>
#include <HAPI/HAPI.h>
#include <Game/HoudiniMeshData.h>
namespace HoudiniEngine
{
    class HoudiniMaterialTranslator
    {
    public:
        static bool CreateHoudiniMaterials(
            const HAPI_NodeId& InNodeId,
            const AZStd::vector<AZ::s32>& InUniqueMaterialIds,
            const AZStd::vector<HAPI_MaterialInfo>& InUniqueMaterialInfos,
			const AZStd::string& HoudiniAssetName,
			HoudiniMeshData& meshData,
			AZStd::vector<AZStd::string>& materialWaitList,
            const bool& bForceRecookAll,
            bool bInTreatExistingMaterialsAsUpToDate = false);

		// Returns a unique name for a given material, its relative path (to the asset)
		static bool GetMaterialRelativePath(const HAPI_NodeId& InAssetId, const HAPI_MaterialInfo& InMaterialNodeInfo, AZStd::string& OutRelativePath);

		//
		static AZStd::string GetMaterialFilePath(const HAPI_NodeId& InMaterialNodeId, const AZStd::string& InMaterialName, const AZStd::string& HoudiniAssetName, const AZStd::string& materialFolderPath);

		// Returns true if a texture parameter was found
		// Ensures that the texture is not disabled via the "UseTexture" Parm name/tag
		static bool FindTextureParamByNameOrTag(const HAPI_NodeId& InNodeId, const AZStd::string& InTextureParmName, const AZStd::string& InUseTextureParmName, const bool& bFindByTag, HAPI_ParmId& OutParmId, HAPI_ParmInfo& OutParmInfo);
    protected:
		// Create various material components.
		static bool CreateMaterialComponentDiffuse(const HAPI_NodeId& InAssetId, const HAPI_MaterialInfo& InMaterialInfo, AZStd::string& propertyValue);
		static bool CreateMaterialComponentNormal(const HAPI_NodeId& InAssetId, const HAPI_MaterialInfo& InMaterialInfo, AZStd::string& propertyValue);
		static bool CreateMaterialComponentSpecular(const HAPI_NodeId& InAssetId, const HAPI_MaterialInfo& InMaterialInfo, AZStd::string& propertyValue);
		static bool CreateMaterialComponentRoughness(const HAPI_NodeId& InAssetId, const HAPI_MaterialInfo& InMaterialInfo, AZStd::string& propertyValue);
		static bool CreateMaterialComponentMetallic(const HAPI_NodeId& InAssetId, const HAPI_MaterialInfo& InMaterialInfo, AZStd::string& propertyValue);
		static bool CreateMaterialComponentEmissive(const HAPI_NodeId& InAssetId, const HAPI_MaterialInfo& InMaterialInfo, AZStd::string& propertyValue);
		static bool CreateMaterialComponentOpacity(const HAPI_NodeId& InAssetId, const HAPI_MaterialInfo& InMaterialInfo, AZStd::string& propertyValue);
		static bool CreateMaterialComponentOpacityMask(const HAPI_NodeId& InAssetId, const HAPI_MaterialInfo& InMaterialInfo, AZStd::string& propertyValue);
    };
}