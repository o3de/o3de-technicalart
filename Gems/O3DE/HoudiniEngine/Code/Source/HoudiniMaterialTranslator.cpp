#include "StdAfx.h"
#include "HoudiniMaterialTranslator.h"
#include <HoudiniEngine/HoudiniApi.h>
#include <HoudiniEngine/HoudiniEngineBus.h>
#include <HoudiniEngineString.h>
#include "HoudiniEngineUtils.h"

#include <AzCore/Utils/Utils.h>
#include <AzCore/Serialization/Json/JsonUtils.h>
namespace HoudiniEngine
{
	bool HoudiniMaterialTranslator::CreateHoudiniMaterials(const HAPI_NodeId& InAssetId, const AZStd::vector<AZ::s32>& InUniqueMaterialIds, const AZStd::vector<HAPI_MaterialInfo>& InUniqueMaterialInfos, const AZStd::string& HoudiniAssetName,
		HoudiniMeshData& meshData, AZStd::vector<AZStd::string>& materialWaitList, const bool& /*bForceRecookAll*/, bool /*bInTreatExistingMaterialsAsUpToDate*/)
    {
		if (InUniqueMaterialIds.size() <= 0)
			return false;

		if (InUniqueMaterialInfos.size() != InUniqueMaterialIds.size())
			return false;

		HoudiniPtr hou;
		HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);
		if (hou == nullptr || hou->IsActive() == false)
		{
			return false;
		}

		//// Empty returned materials.
		//OutMaterials.Empty();

		//// Update context for generated materials (will trigger when object goes out of scope).
		//FMaterialUpdateContext MaterialUpdateContext;

		//// Default Houdini material.
		//UMaterial* DefaultMaterial = FHoudiniEngine::Get().GetHoudiniDefaultMaterial().Get();
		//OutMaterials.Add(HAPI_UNREAL_DEFAULT_MATERIAL_NAME, DefaultMaterial);

		//// Factory to create materials.
		//UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();
		//MaterialFactory->AddToRoot();

		for (AZ::s32 MaterialIdx = 0; MaterialIdx < InUniqueMaterialIds.size(); MaterialIdx++)
		{
			[[maybe_unused]] HAPI_NodeId MaterialId = (HAPI_NodeId)InUniqueMaterialIds[MaterialIdx];

			const HAPI_MaterialInfo& MaterialInfo = InUniqueMaterialInfos[MaterialIdx];
			if (!MaterialInfo.exists)
			{
				// The material does not exist,
				// we will use the default Houdini material in this case.
				continue;
			}

			// Get the material node's node information.
			HAPI_NodeInfo NodeInfo;
			HAPI_NodeInfo_Init(&NodeInfo);
			if (HAPI_RESULT_SUCCESS != HAPI_GetNodeInfo(&hou->GetSession(),MaterialInfo.nodeId, &NodeInfo))
			{
				continue;
			}

			AZStd::string MaterialName;
			if (!HoudiniEngineString::ToAZString(NodeInfo.nameSH, MaterialName))
			{
				// shouldn't happen, give a generic name
				//HOUDINI_LOG_WARNING(TEXT("Failed to retrieve material name!"));
				MaterialName = "Material_" + AZStd::string::format("%i", MaterialInfo.nodeId);
			}

			AZStd::string MaterialPathName;
			if (!HoudiniMaterialTranslator::GetMaterialRelativePath(InAssetId, MaterialInfo, MaterialPathName))
				continue;

//			// Check first in the existing material map
//			UMaterial* Material = nullptr;
//			UMaterialInterface* const* FoundMaterial = InMaterials.Find(MaterialPathName);
//			bool bCanReuseExistingMaterial = false;
//			if (FoundMaterial)
//			{
//				bCanReuseExistingMaterial = (bInTreatExistingMaterialsAsUpToDate || !MaterialInfo.hasChanged) && !bForceRecookAll;
//				Material = Cast<UMaterial>(*FoundMaterial);
//			}
//
//			if (!Material || !bCanReuseExistingMaterial)
//			{
//				// Try to see if another output/part of this HDA has already recreated this material
//				// Since those materials have just been recreated, they are considered up to date and can always be reused.
//				FoundMaterial = InAllOutputMaterials.Find(MaterialPathName);
//				if (FoundMaterial)
//				{
//					Material = Cast<UMaterial>(*FoundMaterial);
//					bCanReuseExistingMaterial = true;
//				}
//			}
//
//			// Check that the existing material is in the expected directory (temp folder could have been changed between
//			// cooks).
//			if (IsValid(Material) && !InPackageParams.HasMatchingPackageDirectories(Material))
//			{
//				bCanReuseExistingMaterial = false;
//				Material = nullptr;
//			}

			// TODO: check if there's an existing material

//			// Get the asset name from the package params
//			FString AssetName = InPackageParams.HoudiniAssetName.IsEmpty() ? TEXT("HoudiniAsset") : InPackageParams.HoudiniAssetName;
//
//			// Get the package and add it to our list
//			UPackage* Package = Material->GetOutermost();
//			OutPackages.AddUnique(Package);
//
//			/*
//			// TODO: This should be handled in the mesh/instance translator
//			// If this is an instancer material, enable the instancing flag.
//			if (UniqueInstancerMaterialIds.Contains(MaterialId))
//				Material->bUsedWithInstancedStaticMeshes = true;
//				*/
//
//				// Reset material expressions.
//#if ENGINE_MINOR_VERSION < 1
//			Material->Expressions.Empty();
//#else
//			Material->GetExpressionCollection().Empty();
//#endif
//
			AZStd::string materialFilePath = GetMaterialFilePath(MaterialInfo.nodeId, MaterialName, HoudiniAssetName, HoudiniEngineUtils::GetRelativeOutputFolder());
			AZStd::string materialString = R"JSON(
												{
													"materialType": "@gemroot:Atom_Feature_Common@/Assets/Materials/Types/EnhancedPBR.materialtype",
													"materialTypeVersion" : 5,
													"propertyValues" : {
														%s
													}
												}
											)JSON";

			AZStd::string propertyValues;
			// Generate various components for this material.
			bool bMaterialComponentCreated = false;
			
			// Extract diffuse plane.
			bMaterialComponentCreated |= HoudiniMaterialTranslator::CreateMaterialComponentDiffuse(InAssetId, MaterialInfo, propertyValues);

			// Extract metallic plane.
			//bMaterialComponentCreated |= HoudiniMaterialTranslator::CreateMaterialComponentMetallic(InAssetId, MaterialInfo, propertyValues);

			// Extract specular plane.
			// bMaterialComponentCreated |= HoudiniMaterialTranslator::CreateMaterialComponentSpecular(InAssetId, MaterialInfo, propertyValues);

			// Extract roughness plane.
			bMaterialComponentCreated |= HoudiniMaterialTranslator::CreateMaterialComponentRoughness(InAssetId, MaterialInfo, propertyValues);

			// Extract emissive plane.
			bMaterialComponentCreated |= HoudiniMaterialTranslator::CreateMaterialComponentEmissive(InAssetId, MaterialInfo, propertyValues);

			// Extract opacity plane.
			bMaterialComponentCreated |= HoudiniMaterialTranslator::CreateMaterialComponentOpacity(InAssetId, MaterialInfo, propertyValues);

			// Extract opacity mask plane.
			bMaterialComponentCreated |= HoudiniMaterialTranslator::CreateMaterialComponentOpacityMask(InAssetId, MaterialInfo, propertyValues);

			// Extract normal plane.
			bMaterialComponentCreated |= HoudiniMaterialTranslator::CreateMaterialComponentNormal(InAssetId, MaterialInfo, propertyValues);

			//TODO: write the material here to file
			if (bMaterialComponentCreated)
			{
				AZStd::string materialJsonString = AZStd::string::format(materialString.c_str(), propertyValues.c_str());
				
				auto readResult = AZ::JsonSerializationUtils::ReadJsonString(materialJsonString);
				if (!readResult.IsSuccess())
				{
					AZ_Error(
						"HE", false,
						"Failed to load material."
						"Error message: '%s'", readResult.GetError().c_str());
					return false;
				}

				AZStd::string fullFilePath = HoudiniEngineUtils::GetProjectRoot() + AZStd::string::format("%s.%s", materialFilePath.c_str(), "material");
				auto saveResult = AZ::JsonSerializationUtils::WriteJsonFile(readResult.TakeValue(), fullFilePath);
				if (!saveResult.IsSuccess())
				{
					AZ_Error(
						"HE", false,
						"Failed to save material file."
						"Error message: '%s'", readResult.GetError().c_str());
					return false;
				}

				AZStd::string azMaterialPath = AZStd::string::format("%s.%s", materialFilePath.c_str(), "azmaterial");
				AZStd::to_lower(azMaterialPath.begin(), azMaterialPath.end());
				if (AZ::IO::FileIOBase::GetInstance()->Exists(azMaterialPath.c_str()))
				{
					AZ::Data::AssetId materialAssetId;
					EBUS_EVENT_RESULT(
						materialAssetId,
						AZ::Data::AssetCatalogRequestBus,
						GetAssetIdByPath,
						azMaterialPath.c_str(),
						AZ::Data::s_invalidAssetType,
						false);

					// If found, notify mesh that the mesh data is assigned and material is ready.
					if (!materialAssetId.IsValid())
					{
						materialWaitList.push_back(azMaterialPath);
					}
				}
				else
				{
					materialWaitList.push_back(azMaterialPath);
				}

				meshData.m_materialNames.push_back(azMaterialPath);
				meshData.m_materialMap[azMaterialPath] = MaterialInfo.nodeId;
			}
		}

		return true;
    }

	bool HoudiniMaterialTranslator::GetMaterialRelativePath(const HAPI_NodeId& InAssetId, const HAPI_MaterialInfo& InMaterialInfo, AZStd::string& OutRelativePath)
	{
		if (InAssetId < 0 || !InMaterialInfo.exists)
			return false;

		HoudiniPtr hou;
		HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);
		if (hou == nullptr || hou->IsActive() == false)
		{
			return false;
		}

		// We want to get the asset node path so we can remove it from the material name
		AZStd::string AssetNodeName;
		{
			HAPI_AssetInfo AssetInfo;
			HAPI_AssetInfo_Init(&AssetInfo);
			HAPI_GetAssetInfo(&hou->GetSession(), InAssetId, &AssetInfo);

			HAPI_NodeInfo AssetNodeInfo;
			HAPI_NodeInfo_Init(&AssetNodeInfo);
			HAPI_GetNodeInfo(&hou->GetSession(), AssetInfo.nodeId, &AssetNodeInfo);

			HoudiniEngineString::ToAZString(AssetNodeInfo.internalNodePathSH, AssetNodeName);
		}

		// Get the material name from the info
		AZStd::string MaterialNodeName;
		{
			HAPI_NodeInfo MaterialNodeInfo;
			HAPI_NodeInfo_Init(&MaterialNodeInfo);
			HAPI_GetNodeInfo(&hou->GetSession(), InMaterialInfo.nodeId, &MaterialNodeInfo);

			HoudiniEngineString::ToAZString(MaterialNodeInfo.internalNodePathSH, MaterialNodeName);
		}

		if (AssetNodeName.size() > 0 && MaterialNodeName.size() > 0)
		{
			// Remove AssetNodeName part from MaterialNodeName. Extra position is for separator.
			OutRelativePath = MaterialNodeName.substr(AssetNodeName.size());
			return true;
		}

		return false;
	}

	AZStd::string HoudiniMaterialTranslator::GetMaterialFilePath(const HAPI_NodeId& InMaterialNodeId, const AZStd::string& InMaterialName, const AZStd::string& HoudiniAssetName, const AZStd::string& materialFolderPath)
	{
		AZStd::string MaterialDescriptor = AZStd::string::format("_material_%i_%s", InMaterialNodeId, InMaterialName.c_str());
		AZStd::string materialFilePath = materialFolderPath + HoudiniAssetName + "/";

		if (!HoudiniAssetName.empty())
		{
			materialFilePath += HoudiniAssetName + MaterialDescriptor;
		}
		else
		{
			materialFilePath += MaterialDescriptor;
		}

		return materialFilePath;
	}

	bool HoudiniMaterialTranslator::FindTextureParamByNameOrTag(const HAPI_NodeId& InNodeId, const AZStd::string& InTextureParmName, const AZStd::string& InUseTextureParmName, const bool& bFindByTag, HAPI_ParmId& OutParmId, HAPI_ParmInfo& OutParmInfo)
	{
		HoudiniPtr hou;
		HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);
		if (hou == nullptr || hou->IsActive() == false)
		{
			return false;
		}

		OutParmId = -1;

		if (bFindByTag)
			OutParmId = HoudiniEngineUtils::HapiFindParameterByTag(InNodeId, InTextureParmName, OutParmInfo);
		else
			OutParmId = HoudiniEngineUtils::HapiFindParameterByName(InNodeId, InTextureParmName, OutParmInfo);

		if (OutParmId < 0)
		{
			// Failed to find the texture
			return false;
		}

		// We found a valid parameter, check if the matching "use" parameter exists
		HAPI_ParmInfo FoundUseParmInfo;
		HAPI_ParmId FoundUseParmId = -1;
		if (bFindByTag)
			FoundUseParmId = HoudiniEngineUtils::HapiFindParameterByTag(InNodeId, InUseTextureParmName, FoundUseParmInfo);
		else
			FoundUseParmId = HoudiniEngineUtils::HapiFindParameterByName(InNodeId, InUseTextureParmName, FoundUseParmInfo);

		if (FoundUseParmId >= 0)
		{
			// We found a valid "use" parameter, check if it is disabled
			// Get the param value
			int32 UseValue = 0;
			if (HAPI_RESULT_SUCCESS == HAPI_GetParmIntValues(&hou->GetSession(), InNodeId, &UseValue, FoundUseParmInfo.intValuesIndex, 1))
			{
				if (UseValue == 0)
				{
					// We found the texture parm, but the "use" param/tag is disabled, so don't use it!
					// We still return true as we found the parameter, this will prevent looking for other parms
					OutParmId = -1;
					return true;
				}
			}
		}

		// Finally, make sure that the found texture Parm is not empty!		
		AZStd::string ParmValue;
		HAPI_StringHandle StringHandle;
		if (HAPI_RESULT_SUCCESS == HAPI_GetParmStringValues(&hou->GetSession(), InNodeId, false, &StringHandle, OutParmInfo.stringValuesIndex, 1))
		{
			// Convert the string handle to FString
			HoudiniEngineString::ToAZString(StringHandle, ParmValue);
		}

		if (ParmValue.empty())
		{
			// We found the parm, but it's empty, don't use it!
			// We still return true as we found the parameter, this will prevent looking for other parms
			OutParmId = -1;
			return true;
		}

		return true;
	}

	bool HoudiniMaterialTranslator::CreateMaterialComponentDiffuse([[maybe_unused]] const HAPI_NodeId& InAssetId, const HAPI_MaterialInfo& InMaterialInfo, AZStd::string& propertyValue)
	{
		HoudiniPtr hou;
		HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);
		if (hou == nullptr || hou->IsActive() == false)
		{
			return false;
		}

		// See if a diffuse texture is available.
		HAPI_ParmInfo ParmDiffuseTextureInfo;
		HAPI_ParmId ParmDiffuseTextureId = -1;
		if (!HoudiniMaterialTranslator::FindTextureParamByNameOrTag(InMaterialInfo.nodeId, "ogl_tex1", "ogl_use_tex1", true, ParmDiffuseTextureId, ParmDiffuseTextureInfo))
		{
			HoudiniMaterialTranslator::FindTextureParamByNameOrTag(InMaterialInfo.nodeId, "basecolor_texture", "basecolor_useTexture", false, ParmDiffuseTextureId, ParmDiffuseTextureInfo);
		}
		
		// If we have diffuse texture parameter.
		bool HasDiffuseTexture = false;
		if (ParmDiffuseTextureId >= 0)
		{
			// TODO: add support for textures
			// when diffuse texture exist 
		}

		// See if uniform color is available.
		HAPI_ParmInfo ParmDiffuseColorInfo;
		HAPI_ParmId ParmDiffuseColorId = HoudiniEngineUtils::HapiFindParameterByTag(InMaterialInfo.nodeId, "ogl_diff", ParmDiffuseColorInfo);

		if (ParmDiffuseColorId < 0)
		{
			ParmDiffuseColorId = HoudiniEngineUtils::HapiFindParameterByName(InMaterialInfo.nodeId, "basecolor", ParmDiffuseColorInfo);
		}

		// If we have uniform color parameter.
		if (ParmDiffuseColorId >= 0)
		{
			AZ::Color Color = AZ::Colors::White;
			float colorValues[4];

			if (HAPI_GetParmFloatValues(&hou->GetSession(), InMaterialInfo.nodeId, colorValues, ParmDiffuseColorInfo.floatValuesIndex, ParmDiffuseColorInfo.size) == HAPI_RESULT_SUCCESS)
			{
				Color.Set(colorValues);
				if (ParmDiffuseColorInfo.size == 3)
					Color.SetA(1.0f);

				propertyValue += AZStd::string::format("%s\"baseColor.color\": [%.17f, %.17f, %.17f, %.17f]", HasDiffuseTexture ? ", " : "", Color.GetR(), Color.GetG(), Color.GetB(), Color.GetA());
			}
		}

		return true;
	}

	bool HoudiniMaterialTranslator::CreateMaterialComponentNormal(const HAPI_NodeId& /*InAssetId*/, const HAPI_MaterialInfo& /*InMaterialInfo*/, AZStd::string& /*propertyValue*/)
	{
		//TODO: only supports normal maps
		return false;
	}

	bool HoudiniMaterialTranslator::CreateMaterialComponentSpecular(const HAPI_NodeId& /*InAssetId*/, const HAPI_MaterialInfo& InMaterialInfo, AZStd::string& propertyValue)
	{
		HoudiniPtr hou;
		HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);
		if (hou == nullptr || hou->IsActive() == false)
		{
			return false;
		}

		bool bExpressionCreated = false;
		// See if specular texture is available.
		HAPI_ParmInfo ParmSpecularTextureInfo;
		HAPI_ParmId ParmSpecularTextureId = -1;
		if (!HoudiniMaterialTranslator::FindTextureParamByNameOrTag(InMaterialInfo.nodeId, "ogl_specmap", "ogl_use_specmap", true, ParmSpecularTextureId, ParmSpecularTextureInfo))
		{
			HoudiniMaterialTranslator::FindTextureParamByNameOrTag(InMaterialInfo.nodeId, "reflect_texture", "reflect_useTexture", false, ParmSpecularTextureId, ParmSpecularTextureInfo);
		}

		if (ParmSpecularTextureId >= 0)
		{
			// TODO: implement specular texture
		}

		// See if we have a specular color
		HAPI_ParmInfo ParmSpecularColorInfo;
		HAPI_ParmId ParmSpecularColorId = HoudiniEngineUtils::HapiFindParameterByTag(InMaterialInfo.nodeId, "ogl_spec", ParmSpecularColorInfo);

		if (ParmSpecularColorId < 0)
		{
			ParmSpecularColorId = HoudiniEngineUtils::HapiFindParameterByName(InMaterialInfo.nodeId, "reflect", ParmSpecularColorInfo);
		}

		if (ParmSpecularColorId >= 0)
		{
			AZ::Color Color = AZ::Colors::White;
			float colorValues[4];

			//TODO: Unreal uses a specular color but O3DE only has a specular.factor
			if (HAPI_GetParmFloatValues(&hou->GetSession(), InMaterialInfo.nodeId, colorValues, ParmSpecularColorInfo.floatValuesIndex, ParmSpecularColorInfo.size) == HAPI_RESULT_SUCCESS)
			{
				Color.Set(colorValues);
				if (ParmSpecularColorInfo.size == 3)
					Color.SetA(1.0f);

				propertyValue += AZStd::string::format(", \"specularF0.factor\": [%.17f, %.17f, %.17f, %.17f]", Color.GetR(), Color.GetG(), Color.GetB(), Color.GetA());
			}
		}

		return bExpressionCreated;
	}

	bool HoudiniMaterialTranslator::CreateMaterialComponentRoughness(const HAPI_NodeId& /*InAssetId*/, const HAPI_MaterialInfo& InMaterialInfo, AZStd::string& propertyValue)
	{
		HoudiniPtr hou;
		HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);
		if (hou == nullptr || hou->IsActive() == false)
		{
			return false;
		}

		bool bExpressionCreated = false;
		// See if roughness texture is available.
		HAPI_ParmInfo ParmRoughnessTextureInfo;
		HAPI_ParmId ParmRoughnessTextureId = -1;
		if (!HoudiniMaterialTranslator::FindTextureParamByNameOrTag(InMaterialInfo.nodeId, "ogl_roughmap", "ogl_use_roughmap", true, ParmRoughnessTextureId, ParmRoughnessTextureInfo))
		{
			HoudiniMaterialTranslator::FindTextureParamByNameOrTag(InMaterialInfo.nodeId, "rough_texture", "rough_useTexture", false, ParmRoughnessTextureId, ParmRoughnessTextureInfo);
		}

		if (ParmRoughnessTextureId >= 0)
		{
			// TODO: implement roughness texture
		}

		// See if we have a roughness value
		HAPI_ParmInfo ParmRoughnessValueInfo;
		HAPI_ParmId ParmRoughnessValueId = HoudiniEngineUtils::HapiFindParameterByTag(InMaterialInfo.nodeId, "ogl_rough", ParmRoughnessValueInfo);

		if (ParmRoughnessValueId < 0)
		{
			ParmRoughnessValueId = HoudiniEngineUtils::HapiFindParameterByName(InMaterialInfo.nodeId, "rough", ParmRoughnessValueInfo);
		}

		if (ParmRoughnessValueId >= 0)
		{
			// Metallic value is available.
			float RoughnessValue = 0.0f;

			if (HAPI_GetParmFloatValues(&hou->GetSession(), InMaterialInfo.nodeId, (float*)&RoughnessValue, ParmRoughnessValueInfo.floatValuesIndex, 1) == HAPI_RESULT_SUCCESS)
			{
				// Clamp retrieved value.
				RoughnessValue = AZStd::clamp(RoughnessValue, 0.0f, 1.0f);
				propertyValue += AZStd::string::format(", \"roughness.factor\": %.17f", RoughnessValue);

				bExpressionCreated = true;
			}
		}

		return bExpressionCreated;
	}

	bool HoudiniMaterialTranslator::CreateMaterialComponentMetallic(const HAPI_NodeId& /*InAssetId*/, const HAPI_MaterialInfo& InMaterialInfo, AZStd::string& propertyValue)
	{
		HoudiniPtr hou;
		HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);
		if (hou == nullptr || hou->IsActive() == false)
		{
			return false;
		}
		
		bool bExpressionCreated = false;
		// See if metallic texture is available.
		HAPI_ParmInfo ParmMetallicTextureInfo;
		HAPI_ParmId ParmMetallicTextureId = -1;
		if (!HoudiniMaterialTranslator::FindTextureParamByNameOrTag(InMaterialInfo.nodeId, "ogl_metallicmap", "ogl_use_metallicmap", true, ParmMetallicTextureId, ParmMetallicTextureInfo))
		{
			HoudiniMaterialTranslator::FindTextureParamByNameOrTag(InMaterialInfo.nodeId, "metallic_texture", "metallic_useTexture", false, ParmMetallicTextureId, ParmMetallicTextureInfo);
		}
		
		if (ParmMetallicTextureId >= 0)
		{
			// TODO: implement metallic texture
		}

		// See if uniform color is available.
		HAPI_ParmInfo ParmMetallicValueInfo;
		HAPI_ParmId ParmMetallicValueId = HoudiniEngineUtils::HapiFindParameterByTag(InMaterialInfo.nodeId, "ogl_metallic", ParmMetallicValueInfo);

		if (ParmMetallicValueId < 0)
		{
			ParmMetallicValueId = HoudiniEngineUtils::HapiFindParameterByName(InMaterialInfo.nodeId, "metallic", ParmMetallicValueInfo);
		}

		if (ParmMetallicValueId >= 0)
		{
			// Metallic value is available.
			float MetallicValue = 0.0f;

			if (HAPI_GetParmFloatValues(&hou->GetSession(), InMaterialInfo.nodeId, (float*)&MetallicValue, ParmMetallicValueInfo.floatValuesIndex, 1) == HAPI_RESULT_SUCCESS)
			{
				// Clamp retrieved value.
				MetallicValue = AZStd::clamp(MetallicValue, 0.0f, 1.0f);
				propertyValue += AZStd::string::format(", \"metallic.factor\": %.17f", MetallicValue);

				bExpressionCreated = true;
			}
		}

		return bExpressionCreated;
	}

	bool HoudiniMaterialTranslator::CreateMaterialComponentEmissive(const HAPI_NodeId& /*InAssetId*/, const HAPI_MaterialInfo& InMaterialInfo, AZStd::string& propertyValue)
	{
		HoudiniPtr hou;
		HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);
		if (hou == nullptr || hou->IsActive() == false)
		{
			return false;
		}

		bool bExpressionCreated = false;
		// See if an emissive texture is available.
		HAPI_ParmInfo ParmEmissiveTextureInfo;
		HAPI_ParmId ParmEmissiveTextureId = -1;
		if (!HoudiniMaterialTranslator::FindTextureParamByNameOrTag(InMaterialInfo.nodeId, "ogl_emissionmap", "ogl_use_emissionmap", true, ParmEmissiveTextureId, ParmEmissiveTextureInfo))
		{
			HoudiniMaterialTranslator::FindTextureParamByNameOrTag(InMaterialInfo.nodeId, "emitcolor_texture", "emitcolor_useTexture", false, ParmEmissiveTextureId, ParmEmissiveTextureInfo);
		}

		if (ParmEmissiveTextureId >= 0)
		{
			// TODO: implement metallic texture
		}

		// See if uniform color is available.
		HAPI_ParmInfo ParmEmissiveIntensityInfo;
		HAPI_ParmId ParmEmissiveIntensityId = HoudiniEngineUtils::HapiFindParameterByTag(InMaterialInfo.nodeId, "ogl_emit_intensity", ParmEmissiveIntensityInfo);

		if (ParmEmissiveIntensityId < 0)
		{
			ParmEmissiveIntensityId = HoudiniEngineUtils::HapiFindParameterByName(InMaterialInfo.nodeId, "emitint", ParmEmissiveIntensityInfo);
		}

		float EmmissiveIntensity = 0.0f;
		if (ParmEmissiveIntensityId >= 0)
		{
			if (HAPI_GetParmFloatValues(&hou->GetSession(), InMaterialInfo.nodeId, &EmmissiveIntensity, ParmEmissiveIntensityInfo.floatValuesIndex, 1) == HAPI_RESULT_SUCCESS)
			{
				propertyValue += AZStd::string::format(", \"emissive.intensity\": %.17f", EmmissiveIntensity);
				bExpressionCreated = true;
			}
		}

		return bExpressionCreated;
	}

	bool HoudiniMaterialTranslator::CreateMaterialComponentOpacity(const HAPI_NodeId& /*InAssetId*/, const HAPI_MaterialInfo& InMaterialInfo, AZStd::string& propertyValue)
	{
		HoudiniPtr hou;
		HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);
		if (hou == nullptr || hou->IsActive() == false)
		{
			return false;
		}

		bool bExpressionCreated = false;
		// See if uniform color is available.
		HAPI_ParmInfo ParmOpacityValueInfo;
		HAPI_ParmId ParmOpacityValueId = HoudiniEngineUtils::HapiFindParameterByTag(InMaterialInfo.nodeId, "ogl_alpha", ParmOpacityValueInfo);

		if (ParmOpacityValueId < 0)
		{
			ParmOpacityValueId = HoudiniEngineUtils::HapiFindParameterByName(InMaterialInfo.nodeId, "opac", ParmOpacityValueInfo);
		}

		float OpacityValueRetrieved = 1.0f;
		if (ParmOpacityValueId >= 0)
		{
			if (ParmOpacityValueInfo.size > 0 && ParmOpacityValueInfo.floatValuesIndex >= 0)
			{
				if (HAPI_GetParmFloatValues(&hou->GetSession(), InMaterialInfo.nodeId, &OpacityValueRetrieved, ParmOpacityValueInfo.floatValuesIndex, 1) == HAPI_RESULT_SUCCESS)
				{
					// Clamp retrieved value.
					OpacityValueRetrieved = AZStd::clamp(OpacityValueRetrieved, 0.0f, 1.0f);
					
					propertyValue += AZStd::string::format(", \"opacity.factor\": %.17f", OpacityValueRetrieved);
					bExpressionCreated = true;
				}
			}
		}

		return bExpressionCreated;
	}

	bool HoudiniMaterialTranslator::CreateMaterialComponentOpacityMask(const HAPI_NodeId& /*InAssetId*/, const HAPI_MaterialInfo& /*InMaterialInfo*/, AZStd::string& /*propertyValue*/)
	{
		//TODO: opacity texture maps support
		return false;
	}

}