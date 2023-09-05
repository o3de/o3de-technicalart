#include "StdAfx.h"
#include "HoudiniSplineTranslator.h"
#include "HoudiniEngineUtils.h"
#include "HoudiniEngineString.h"

#include <Math/MathHelper.h>

#include <HAPI/HAPI.h>
#include <HoudiniEngine/HoudiniApi.h>
#include <HoudiniEngine/HoudiniEngineBus.h>
#include <AzCore/StringFunc/StringFunc.h>
#include <AzToolsFramework/Component/EditorComponentAPIBus.h>
namespace HoudiniEngine
{
	bool HoudiniSplineTranslator::UpdateHoudiniCurve(AZ::EntityId entityId, const AZ::s32& GeoId, bool bIsLegacyInputCurve)
	{
		if (bIsLegacyInputCurve)
		{
			return UpdateHoudiniCurveLegacy(entityId, GeoId);
		}

		//TODO: do non-legacy processing here.

		return true;
	}

	bool HoudiniSplineTranslator::UpdateHoudiniCurveLegacy(AZ::EntityId entityId, const AZ::s32& GeoId)
	{
		HoudiniPtr hou;
		HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);
		
		if (hou == nullptr)
			return false;

		// check if spline is created; if not create one, if yes pull the point data, if 
		AZStd::string CurvePointsString;
		if (!HoudiniEngineUtils::HapiGetParameterDataAsString(
			GeoId, "coords", "", CurvePointsString))
		{
			return false;
		}

		AZ::s32 CurveTypeValue = 2;
		if (!HoudiniEngineUtils::HapiGetParameterDataAsInteger(
			GeoId, "type", 0, CurveTypeValue))
		{
			return false;
		}

		AZ::s32 CurveMethodValue = (AZ::s32)0;
		if (!HoudiniEngineUtils::HapiGetParameterDataAsInteger(
			GeoId, "method", 0, CurveMethodValue))
		{
			return false;
		}

		AZ::s32 CurveClosed = 0;
		if (!HoudiniEngineUtils::HapiGetParameterDataAsInteger(
			GeoId, "close", 0, CurveClosed))
		{
			return false;
		}

		AZ::s32 CurveReversed = 0;
		if (!HoudiniEngineUtils::HapiGetParameterDataAsInteger(
			GeoId, "reverse", 0, CurveReversed))
		{
			return false;
		}

		//TODO: set spline component if closed
		//HoudiniSplineComponent->SetClosedCurve(CurveClosed == 1);

		HAPI_NodeInfo NodeInfo;
		HAPI_NodeInfo_Init(&NodeInfo);
		HAPI_GetNodeInfo(&hou->GetSession(), GeoId, &NodeInfo);

		AZStd::vector<float> RefinedCurvePositions;
		HAPI_AttributeInfo AttributeRefinedCurvePositions;
		HAPI_AttributeInfo_Init(&AttributeRefinedCurvePositions);
		if (!HoudiniEngineUtils::HapiGetAttributeDataAsFloat(
			GeoId, 0, HAPI_ATTRIB_POSITION, AttributeRefinedCurvePositions, RefinedCurvePositions))
		{
			return false;
		}

		// Process coords string and extract positions.
		AZStd::vector<AZ::Vector3> CurvePoints;
		HoudiniSplineTranslator::ExtractStringPositions(CurvePointsString, CurvePoints);

		//AZStd::vector<AZ::Vector3> CurveDisplayPoints;
		//FHoudiniEngineUtils::ConvertHoudiniPositionToUnrealVector(RefinedCurvePositions, CurveDisplayPoints);

		// Build curve points for editable curves.
		/*if (HoudiniSplineComponent->CurvePoints.Num() != CurvePoints.Num())
		{
			HoudiniSplineComponent->CurvePoints.SetNum(CurvePoints.Num());
			for (int32 Idx = 0; Idx < CurvePoints.Num(); Idx++)
			{
				FTransform Transform = FTransform::Identity;
				Transform.SetLocation(CurvePoints[Idx]);
				HoudiniSplineComponent->CurvePoints[Idx] = Transform;
			}
		}*/

		CreateSplineComponentFromCurvePoints(entityId, CurvePoints);
		return true;
	}

	bool HoudiniSplineTranslator::IsCurveInputNodeValid(const HAPI_NodeId& InNodeId, const bool& bLegacyNode)
	{
		// Check if connected asset id is valid, if it is not, we need to create an input asset.
		if (InNodeId < 0)
			return false;

		HoudiniPtr hou;
		HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);

		if (hou == nullptr)
			return false;

		if (bLegacyNode)
		{
			// Legacy Curve::1.0 node
			// Check if the input node has a curve "coords" params.
			// If it doesn't, then the node is not a valid legacy curve1.0 node
			HAPI_ParmId ParmId = -1;
			if (HAPI_GetParmIdFromName(&hou->GetSession(), InNodeId, "coords", &ParmId) != HAPI_RESULT_SUCCESS
				|| ParmId < 0)
			{
				return false;
			}
		}
		else
		{
			// New input curves
			// First, check that the geo type is a curve
			HAPI_GeoInfo InputGeoInfo;
			HAPI_GeoInfo_Init(&InputGeoInfo);
			if (HAPI_GetGeoInfo(&hou->GetSession(), InNodeId, &InputGeoInfo) != HAPI_RESULT_SUCCESS)
				return false;

			if (InputGeoInfo.type != HAPI_GEOTYPE_CURVE)
				return false;

			// Check that the part type is a curve
			HAPI_PartInfo InputPartInfo;
			HAPI_PartInfo_Init(&InputPartInfo);
			if (HAPI_GetPartInfo(&hou->GetSession(), InNodeId, 0, &InputPartInfo) != HAPI_RESULT_SUCCESS)
				return false;

			if (InputPartInfo.type != HAPI_PARTTYPE_CURVE)
				return false;

			// Check if the input node has a curve "coords" params.
			// If it does, then the node a legacy curve1.0 node, so we need to recreate the input node!
			HAPI_ParmId ParmId = -1;
			if (HAPI_GetParmIdFromName(&hou->GetSession(), InNodeId, "coords", &ParmId) == HAPI_RESULT_SUCCESS
				&& ParmId >= 0)
				return false;

			// Previous method. 
			// This caused crashes in HAPI/HARS and forced the user to restart the session manually!
			// We already have an input node, make sure that it's a curve input	
			// if (FHoudiniApi::GetInputCurveInfo(FHoudiniEngine::Get().GetSession(), CurveNodeId, 0, &InputCurveInfo) != HAPI_RESULT_SUCCESS)
			// return false;
		}

		// We can reuse the node!
		return true;
	}

	bool HoudiniSplineTranslator::HapiUpdateNodeForHoudiniSplineComponent(AZ::EntityId entityId, HAPI_NodeId& CurveNodeId, bool /*bInSetRotAndScaleAttributes*/)
	{
		// TODO: InIsLegacyCurve should be coming from a saved object

		AZStd::vector<AZ::Vector3> curvePoints;
		AZ::SplinePtr spline;
		LmbrCentral::SplineComponentRequestBus::EventResult(spline, entityId, &LmbrCentral::SplineComponentRequests::GetSpline);
		if (spline != nullptr)
		{
			curvePoints = spline->GetVertices();
		}


		AZStd::vector<AZ::Vector3> PositionArray;
		for (auto& CurrentTransform : curvePoints)
		{
			PositionArray.push_back(CurrentTransform);
		}

		//TODO: these are saved in a spline component class. Not that we need to pass the component but we could do a class for storing these info.
		AZ::s32 InCurveType = 0;
		AZ::s32 InCurveMethod = 0;
		bool InClosed = false;
		bool InReversed = false;
		bool InForceClose = false;

		bool InIsLegacyCurve = true;
		if (InIsLegacyCurve)
		{
			return HapiCreateCurveInputNodeForDataLegacy(CurveNodeId, -1, "", &PositionArray, InCurveType, InCurveMethod, InClosed, InReversed, InForceClose);
		}

		//TODO: implement non-legacy curve
		return true;
	}

	bool HoudiniSplineTranslator::HapiCreateCurveInputNodeForDataLegacy(HAPI_NodeId& CurveNodeId, const HAPI_NodeId& /*ParentNodeId*/, const AZStd::string& /*InputNodeName*/, AZStd::vector<AZ::Vector3>* Positions, AZ::s32 InCurveType,
		AZ::s32 InCurveMethod, bool InClosed, bool InReversed, bool InForceClose)
	{
		// We also need a valid host asset and 2 points to make a curve
		int32 NumberOfCVs = Positions->size();
		if (NumberOfCVs < 2)
			return false;

		HoudiniPtr hou;
		HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);

		if (hou == nullptr)
			return false;

		// Check if connected asset id is valid, if it is not, we need to create an input asset.
		if (!IsCurveInputNodeValid(CurveNodeId, true))
		{
			//TODO:
			//HAPI_NodeId NodeId = -1;
			//// Create the curve SOP Node
			//if (!FHoudiniSplineTranslator::HapiCreateCurveInputNode(NodeId, ParentNodeId, InputNodeName, true))
			//	return false;

			//// Check if we have a valid id for this new input asset.
			//if (!FHoudiniEngineUtils::IsHoudiniNodeValid(NodeId))
			//	return false;

			//// We now have a valid id.
			//HAPI_NodeId PreviousInputNodeId = CurveNodeId;
			//CurveNodeId = NodeId;

			//// We have now created a valid new input node, delete the previous one
			//if (PreviousInputNodeId >= 0)
			//{
			//	// Get the parent OBJ node ID before deleting!
			//	HAPI_NodeId PreviousInputOBJNode = FHoudiniEngineUtils::HapiGetParentNodeId(PreviousInputNodeId);

			//	if (HAPI_RESULT_SUCCESS != FHoudiniApi::DeleteNode(
			//		FHoudiniEngine::Get().GetSession(), PreviousInputNodeId))
			//	{
			//		HOUDINI_LOG_WARNING(TEXT("Failed to cleanup the previous input curve node"));
			//	}

			//	if (HAPI_RESULT_SUCCESS != FHoudiniApi::DeleteNode(
			//		FHoudiniEngine::Get().GetSession(), PreviousInputOBJNode))
			//	{
			//		HOUDINI_LOG_WARNING(TEXT("Failed to cleanup the previous input curve OBJ node."));
			//	}
			//}
		}
		else
		{
			// We have to revert the Geo to its original state so we can use the Curve SOP:
			// adding parameters to the Curve SOP locked it, preventing its parameters (type, method, isClosed) from working
			HAPI_RevertGeo(&hou->GetSession(), CurveNodeId);
		}

		//
		// In order to be able to add rotations and scale attributes to the curve SOP, we need to cook it twice:
		// 
		// - First, we send the positions string to it, and cook it without refinement.
		//   this will allow us to get the proper curve CVs, part attributes and curve info to create the desired curve.
		//
		// - We then need to send back all the info extracted from the curve SOP to it, and add the rotation 
		//   and scale attributes to it. This will lock the curve SOP, and prevent the curve type and method 
		//   parameters from functioning properly (hence why we needed the first cook to set that up)
		//

		// Set the curve type and curve method parameters for the curve node
		AZ::s32 CurveTypeValue = (AZ::s32)InCurveType;
		HAPI_SetParmIntValue(&hou->GetSession(), CurveNodeId, "type", 0, CurveTypeValue);

		AZ::s32 CurveMethodValue = (AZ::s32)InCurveMethod;
		HAPI_SetParmIntValue(&hou->GetSession(), CurveNodeId, "method", 0, CurveMethodValue);

		AZ::s32 CurveClosed = InClosed ? 1 : 0;
		HAPI_SetParmIntValue(&hou->GetSession(), CurveNodeId, "close", 0, CurveClosed);

		AZ::s32 CurveReversed = InReversed ? 1 : 0;
		HAPI_SetParmIntValue(&hou->GetSession(), CurveNodeId, "reverse", 0, CurveReversed);

		// Reading the curve parameters
		HoudiniEngineUtils::HapiGetParameterDataAsInteger(CurveNodeId, "type", 0, CurveTypeValue);
		HoudiniEngineUtils::HapiGetParameterDataAsInteger(CurveNodeId, "method", 0, CurveMethodValue);
		HoudiniEngineUtils::HapiGetParameterDataAsInteger(CurveNodeId, "close", 0, CurveClosed);
		HoudiniEngineUtils::HapiGetParameterDataAsInteger(CurveNodeId, "reverse", 0, CurveReversed);

		if (InForceClose)
		{
			// We need to update the closed parameter
			HAPI_SetParmIntValue(&hou->GetSession(), CurveNodeId, "close", 0, 1);

			CurveClosed = 1;
		}

		// For closed NURBS (CVs and Breakpoints), we have to close the curve manually, by duplicating its last point
		// in order to be able to set the rotations and scales attributes properly.
		bool bCloseCurveManually = false;
		if (CurveClosed && (CurveTypeValue == HAPI_CURVETYPE_NURBS) && (CurveMethodValue != 2))
		{
			// The curve is not closed anymore
			if (HAPI_RESULT_SUCCESS == HAPI_SetParmIntValue(&hou->GetSession(), CurveNodeId, "close", 0, 0))
			{
				bCloseCurveManually = true;

				// Duplicating the first point to the end of the curve
				// This needs to be done before sending the position string
				AZ::Vector3 pos = (*Positions)[0];
				Positions->push_back(pos);

				CurveClosed = false;
			}
		}

		// Creating the position string
		AZStd::string PositionString = "";
		HoudiniSplineTranslator::CreatePositionsString(*Positions, PositionString);

		// Get param id for the PositionString and modify it
		HAPI_ParmId ParmId = -1;
		if (HAPI_GetParmIdFromName(&hou->GetSession(), CurveNodeId, "coords", &ParmId) != HAPI_RESULT_SUCCESS)
		{
			return false;
		}

		HAPI_SetParmStringValue(&hou->GetSession(), CurveNodeId, PositionString.c_str(), ParmId, 0);

		// If we don't want to add rotations or scale attributes to the curve, 
		// we can just cook the node normally and stop here.
		bool bAddRotations = false/*(Rotations != nullptr)*/;
		bool bAddScales3d = false/*(Scales3d != nullptr)*/;
		if (!bAddRotations && !bAddScales3d)
		{
			// Cook the node, no need to wait for completion
			return HoudiniEngineUtils::HapiCookNode(CurveNodeId, nullptr, false);
		}
		return false;
	}

	void HoudiniSplineTranslator::ExtractStringPositions(const AZStd::string& Positions, AZStd::vector<AZ::Vector3>& OutPositions)
	{
		AZStd::vector<AZStd::string> PointStrings;
		AZ::StringFunc::Tokenize(Positions, PointStrings, " ");

		// need to convert positions to O3DE
		//TODO: put this is a utility/common function for re-use.
		AZ::Matrix3x3 rotateX = AZ::Matrix3x3::CreateRotationX(AZ::DegToRad(90.0f));

		auto rotate4x4 = AZ::Matrix4x4::CreateFromMatrix3x4(AZ::Matrix3x4::CreateFromMatrix3x3(rotateX));

		for (auto point : PointStrings)
		{
			if (point.size() < 6) continue; // shortest string would be 6 in size
			
			AZStd::vector<AZStd::string> vectorString;
			AZ::StringFunc::Tokenize(point, vectorString, ",");

			auto vec3 = rotate4x4 * AZ::Vector3(AZ::StringFunc::ToDouble(vectorString[0].c_str()), AZ::StringFunc::ToDouble(vectorString[1].c_str()), AZ::StringFunc::ToDouble(vectorString[2].c_str()));
			OutPositions.push_back(vec3);
		}
		

		/*
		static const char * PositionSeparators[] =
		{
			" ",
			",",
		};

		AZ::s32 NumCoords = Positions.ParseIntoArray(PointStrings, PositionSeparators, 2);
		OutPositions.SetNum(NumCoords / 3);
		for (AZ::s32 OutIndex = 0; OutIndex < OutPositions.Num(); OutIndex++)
		{
			const AZ::s32& CoordIndex = OutIndex * 3;
			OutPositions[OutIndex].X = FCString::Atod(*(PointStrings[CoordIndex + 0])) * HAPI_UNREAL_SCALE_FACTOR_POSITION;
			OutPositions[OutIndex].Y = FCString::Atod(*(PointStrings[CoordIndex + 2])) * HAPI_UNREAL_SCALE_FACTOR_POSITION;
			OutPositions[OutIndex].Z = FCString::Atod(*(PointStrings[CoordIndex + 1])) * HAPI_UNREAL_SCALE_FACTOR_POSITION;
		}*/
	}

	void HoudiniSplineTranslator::CreatePositionsString(const AZStd::vector<AZ::Vector3>& InPositions, AZStd::string& OutPositionString)
	{
		OutPositionString = "";
		
		AZ::Matrix3x3 rotateX = AZ::Matrix3x3::CreateRotationX(AZ::DegToRad(-90.0f));
		auto rotate4x4 = AZ::Matrix4x4::CreateFromMatrix3x4(AZ::Matrix3x4::CreateFromMatrix3x3(rotateX));

		for (AZ::s32 Idx = 0; Idx < InPositions.size(); ++Idx)
		{
			AZ::Vector3 Position = rotate4x4 * InPositions[Idx];
			OutPositionString += AZStd::string::format("%f, %f, %f ", Position.GetX(), Position.GetY(), Position.GetZ());
		}
	}

	void HoudiniSplineTranslator::CreateHoudiniSplineComponentFromHoudiniEditableNode(AZ::EntityId entityId, const AZ::s32& GeoId)
	{
		if (GeoId < 0)
			return;

		HoudiniPtr hou;
		HoudiniEngineRequestBus::BroadcastResult(hou, &HoudiniEngineRequestBus::Events::GetHoudiniEngine);

		if (hou == nullptr)
			return;

		bool IsLegacyCurve = false;
		AZ::s32 ParmId = -1;
		if (HAPI_GetParmIdFromName(&hou->GetSession(), GeoId, "coords", &ParmId) == HAPI_RESULT_SUCCESS && ParmId > 0)
		{
			IsLegacyCurve = true;
		}

		UpdateHoudiniCurve(entityId, GeoId, IsLegacyCurve);
	}

	void HoudiniSplineTranslator::CreateSplineComponentFromCurvePoints(AZ::EntityId entityId, AZStd::vector<AZ::Vector3>& curvePoints)
	{
		if (curvePoints.size() > 0)
		{
			AzToolsFramework::EntityIdList entityIdList = { entityId };

			AzToolsFramework::EntityCompositionRequests::AddComponentsOutcome addedComponentsResult =
				AZ::Failure(AZStd::string("Failed to call AddComponentsToEntities on EntityCompositionRequestBus"));
			AzToolsFramework::EntityCompositionRequestBus::BroadcastResult(
				addedComponentsResult,
				&AzToolsFramework::EntityCompositionRequests::AddComponentsToEntities,
				entityIdList,
				AZ::ComponentTypeList{ AZ::Uuid("{5B29D788-4885-4D56-BD9B-C0C45BE08EC1}") });

			LmbrCentral::SplineComponentRequestBus::Event(entityId, &LmbrCentral::SplineComponentRequests::SetVertices, curvePoints);
		}
	}
	
	void HoudiniSplineTranslator::RemoveSplineComponent(AZ::EntityId entityId)
	{
		AZ::Component* component = GetSplineComponent(entityId);
		if (component != nullptr)
		{
			LmbrCentral::SplineComponentRequestBus::Event(entityId, &LmbrCentral::SplineComponentRequests::ClearVertices);
			AZStd::vector<AZ::Component*> componentsToRemove = { component };
			AzToolsFramework::EntityCompositionRequests::RemoveComponentsOutcome outcome =
				AZ::Failure(AZStd::string("Failed to remove old shape component."));
			AzToolsFramework::EntityCompositionRequestBus::BroadcastResult(outcome, &AzToolsFramework::EntityCompositionRequests::RemoveComponents, componentsToRemove);
		}
	}

	AZ::Component* HoudiniSplineTranslator::GetSplineComponent(AZ::EntityId entityId)
	{
		AZ::Entity* entity = AZ::Interface<AZ::ComponentApplicationRequests>::Get()->FindEntity(entityId);

		if (entity != nullptr)
		{
			return entity->FindComponent(AZ::Uuid("{5B29D788-4885-4D56-BD9B-C0C45BE08EC1}"));
		}

		return nullptr;
	}
}