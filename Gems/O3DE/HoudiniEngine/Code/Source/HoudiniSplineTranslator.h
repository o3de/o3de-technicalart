#pragma once

#include <AzCore/std/string/string.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Component/Entity.h>

namespace HoudiniEngine
{
    struct HoudiniSplineTranslator
    {
		// Get the cooked Houdini curve.
		static bool UpdateHoudiniCurve(AZ::EntityId entityId, const AZ::s32& GeoId, bool bIsLegacyInputCurve);

		// Get the cooked Houdini curve.
		static bool UpdateHoudiniCurveLegacy(AZ::EntityId entityId, const AZ::s32& GeoId);

		// Indicates if the node is a valid curve input node
		static bool IsCurveInputNodeValid(const HAPI_NodeId& InNodeId, const bool& bLegacyNode = false);

		// Upload Houdini spline component data to the curve node, and then sync the Houdini Spline Component with the curve node.
		static bool HapiUpdateNodeForHoudiniSplineComponent(AZ::EntityId entityId, HAPI_NodeId& CurveNodeId, bool bInSetRotAndScaleAttributes);

		// Update curve node data using curve::1.0
		// Update the curve node data, or create a new curve node if the CurveNodeId is valid.
		static bool HapiCreateCurveInputNodeForDataLegacy(
			HAPI_NodeId& CurveNodeId,
			const HAPI_NodeId& ParentNodeId,
			const AZStd::string& InputNodeName,
			AZStd::vector<AZ::Vector3>* Positions, AZ::s32 InCurveType,
			AZ::s32 InCurveMethod, bool InClosed, bool InReversed, bool InForceClose);

		// Helper functions.
	    static void ExtractStringPositions(const AZStd::string& Positions, AZStd::vector<AZ::Vector3>& OutPositions);
		static void CreatePositionsString(const AZStd::vector<AZ::Vector3>& InPositions, AZStd::string& OutPositionString);

		// Create a Houdini spline component from a given editable node. (Only called once when first build the editable node.)
		static void CreateHoudiniSplineComponentFromHoudiniEditableNode(AZ::EntityId entityId, const AZ::s32& GeoId);

		static void CreateSplineComponentFromCurvePoints(AZ::EntityId entityId, AZStd::vector<AZ::Vector3>& curvePoints);
		static void RemoveSplineComponent(AZ::EntityId entityId);
		static AZ::Component* GetSplineComponent(AZ::EntityId entityId);
    };
}