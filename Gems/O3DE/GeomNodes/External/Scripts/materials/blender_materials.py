#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

import bpy
import json
from materials import blender_materials_conversion
import logging as _logging
_LOGGER = _logging.getLogger('GeomNodes.External.Scripts.materials.blender_materials')

def get_material_names():
    """! Get all material names present in the scene
    """
    names = []
    for material in bpy.data.materials[:]:
        names += [material.name]
    return names


def get_o3de_materials():
    """! Get all materials in O3DE material format
    """
    materials = []
    for material in bpy.data.materials[:]:
        materials.append({material.name : json.dumps(convert_material_to_o3de(material), indent=4)})
    return materials

bpy.ops.file.make_paths_absolute()

def convert_material_to_o3de(target_material) -> dict:
    """! Converts a target material's settings to O3DE material format (*.material)
    """
    material_info = blender_materials_conversion.get_o3de_property_values(target_material)
    o3de_material = {
        "materialType": "@gemroot:Atom_Feature_Common@/Assets/Materials/Types/EnhancedPBR.materialtype",
        "materialTypeVersion": 5,
        "propertyValues": material_info
    }

    return o3de_material


def print_materials_in_scene():
    """! This is for debugging purposes. It prints out all materials in the scene and list out the
    material info.
    """
    # Print the materials
    for material in bpy.data.materials[:]:
        _LOGGER.debug("Name: " + material.name)
        if material.use_nodes:
            print_material(material)
            _LOGGER.debug("----------------------------")        
            
        #_LOGGER.debug("Use Nodes: ", material.use_nodes)
        _LOGGER.debug(json.dumps(blender_materials_conversion.get_material_info(material), indent=4))
        _LOGGER.debug("----------------------------")        
        _LOGGER.debug(material.name + ".material")
        _LOGGER.debug(json.dumps(convert_material_to_o3de(material), indent=4))
        
        _LOGGER.debug("+++++++++++++++++++++++")

def print_material(target_material):
    # _LOGGER.debug("Nodes: ", material.node_tree.nodes.keys())
    for node in target_material.node_tree.nodes:

        _LOGGER.debug(" " + node.name + " " + node.type)
        out_str = ""
        if node.type == "TEX_IMAGE":
            _LOGGER.debug("      tex image: " + node.image.name)
        for material_input in node.inputs:
            try:
                link = material_input.links[0]
                file_location = link.from_node.image.filepath
                _LOGGER.debug("      file_location: " + file_location)
            except AttributeError:
                pass
            except IndexError:
                pass

            try:
                out_str = "     " + material_input.name + ' : '
                if type(material_input.default_value) == bpy.types.bpy_prop_array:
                    out_str+=str(list(material_input.default_value))
                else:
                    out_str += str(material_input.default_value)
            except AttributeError:
                pass
            _LOGGER.debug(out_str)