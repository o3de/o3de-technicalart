#pragma once

namespace GeomNodes
{
	namespace Field
	{
		static constexpr char Initialized[] = "Initialized";
		static constexpr char Heartbeat[] = "Heartbeat";
		static constexpr char ObjectNames[] = "ObjectNames";
		static constexpr char Objects[] = "Objects";
		static constexpr char Object[] = "Object";
		static constexpr char SHMOpen[] = "SHMOpen";
		static constexpr char SHMClose[] = "SHMClose";
		static constexpr char MapId[] = "MapId";
		static constexpr char Export[] = "Export";
		static constexpr char Error[] = "Error";

		static constexpr char Params[] = "Params";
		static constexpr char Materials[] = "Materials";
		static constexpr char Id[] = "Id";
		static constexpr char Name[] = "Name";
		static constexpr char Type[] = "Type";
		static constexpr char DefaultValue[] = "DefaultValue";
		static constexpr char Value[] = "Value";
		static constexpr char MinValue[] = "MinValue";
		static constexpr char MaxValue[] = "MaxValue";

		static constexpr char FBXPath[] = "FBXPath";
	}

	//! Attributes for mesh vertices.
	enum class AttributeType
	{
		Position,
		Normal,
		Tangent,
		Bitangent,
		UV,
		Color
	};

    static constexpr AZStd::string_view AssetsFolderPath = "assets/geomNodes/";
    static constexpr AZStd::string_view MaterialsFolder = "materials";
    static constexpr AZStd::string_view MaterialExtension = ".material";
    static constexpr AZStd::string_view AzMaterialExtension = ".azmaterial";
    static constexpr AZStd::string_view FbxExtension = ".fbx";
    static constexpr AZStd::string_view AzModelExtension = ".azmodel";

    typedef AZStd::vector<AZStd::string> StringVector;
} // namespace GeomNodes