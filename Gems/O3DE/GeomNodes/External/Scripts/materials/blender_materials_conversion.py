#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

import bpy
import json
from materials import convert_bsdf_material as bsdf
from materials import convert_subsurface_scattering_material as subsurface_scatter
from materials import convert_non_node_material as non_node
from materials import convert_diffuse_bsdf_material as diffuse_bsdf

import logging as _logging
_LOGGER = _logging.getLogger('GeomNodes.External.Scripts.materials.blender_materials_conversion')

supported_material_types = [
    'Principled BSDF',
    'Subsurface Scattering',
    'Diffuse BSDF', # -> not sure if we have a direct conversion to O3DE but for now we just use basic conversions
]

def get_material_info(target_material, use_o3de_mat_names=False):
    material_info = {
        'dcc_application': 'Blender',
        'settings': {},
        'textures': {}
    }

    if target_material.use_nodes:
        for node in target_material.node_tree.nodes:
            if node.name in supported_material_types:
                material_info['settings'].update(get_material_info_by_type(node, use_o3de_mat_names))                
    else:
        material_info['settings'].update(non_node.get_material_info(target_material, use_o3de_mat_names))

    return material_info

def get_material_info_by_type(node, use_o3de_mat_names=False):
    if node.name == 'Principled BSDF':
        return bsdf.get_material_info(node, use_o3de_mat_names)
    elif node.name == 'Subsurface Scattering':
        return subsurface_scatter.get_material_info(node, use_o3de_mat_names)
    elif node.name == 'Diffuse BSDF':
        return diffuse_bsdf.get_material_info(node, use_o3de_mat_names)

def get_o3de_property_values(target_material):
    """! returns the material's settings or property values in o3de material format.
    """
    material_info = get_material_info(target_material, True)
    return material_info['settings']