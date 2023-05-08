#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

import bpy
import logging as _logging
from materials import mat_helper as helper

_LOGGER = _logging.getLogger('GeomNodes.External.Scripts.materials.convert_bsdf_material')

bsdf_file_connections = [
    'Base Color',
    'Metallic',
    'Specular',
    'Roughness',
    'Emission',
    'Normal',
    'Subsurface',
    'Alpha',
    'Displacement'
]


# Add command to avoid default Blender relative paths
bpy.ops.file.make_paths_absolute()

def get_material_type() -> str:
    return 'BSDF'

def get_material_info(node, use_o3de_mat_names=False) -> dict:
    # material_textures = get_material_textures(target_material)
    target_properties = get_transferable_properties()
    material_settings =  helper.get_material_node_attributes(node, target_properties, use_o3de_mat_names)
    # material_info = {
    #     'material_type': 'BSDF',
    #     'dcc_application': 'Blender',
    #     'settings': material_settings,
    #     'textures': material_textures
    # }
    return material_settings


# def get_material_textures(target_material: str) -> dict:
#     material_textures_dict = {}
#     for node in target_material.node_tree.nodes:
#         for material_input in node.inputs:
#             if material_input.name in bsdf_file_connections:
#                 try:
#                     link = material_input.links[0]
#                     file_location = link.from_node.image.filepath
#                     material_textures_dict[material_input.name] = file_location
#                 except IndexError:
#                     pass
#                 except AttributeError:
#                     pass
#     return material_textures_dict

def get_transferable_properties():
    """! This is a listing of properties on an BSDF material that can be directly transferred to an
    O3DE material.
    """
    target_properties = {

        'Base Color': 'baseColor.color',                # Base Color
        'Metallic': 'metallic.factor',                  # Metallic
        'Roughness': 'roughness.factor',                # Roughness
        'Specular': 'specularF0.factor',                # Specular
        'Clearcoat': 'clearCoat.factor',                # Clearcoat
        'Clearcoat Roughness': 'clearCoat.roughness',   # Clearcoat Roughness
        'Emmision': 'emissive.color',                   # Emissive
        'Emission Strength': 'emissive.intensity'       # Emissive Intensity
    }
    return target_properties