#pragma once

namespace GeomNodes
{
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