import bpy
from materials import mat_helper as helper
import logging as _logging
_LOGGER = _logging.getLogger('GeomNodes.External.Scripts.materials.convert_non_node_material')

def get_material_type() -> str:
    return 'Non_Node'

def get_material_info(target_material: str, use_o3de_mat_names=False) -> dict:
    # material_textures = get_material_textures(target_material)
    target_properties = get_transferable_properties()
    material_settings =  get_material_attributes(target_material, target_properties, use_o3de_mat_names)
    # material_info = {
    #     'material_type': 'Non_Node', # just made this up
    #     'dcc_application': 'Blender',
    #     'settings': material_settings,
    #     'textures': material_textures
    # }
    return material_settings

def get_material_textures(target_material: str) -> dict:
    material_textures_dict = {}
    # TODO: implement how to get the textures for non-node materials
    return material_textures_dict

def get_material_attributes(target_material: str, target_properties: dict, use_o3de_mat_names=False):
    attribute_dictionary = {
        helper.get_material_input_name('Diffuse Color', target_properties, use_o3de_mat_names) : list(target_material.diffuse_color),
        helper.get_material_input_name('Metallic', target_properties, use_o3de_mat_names) : target_material.metallic,
        helper.get_material_input_name('Roughness', target_properties, use_o3de_mat_names) : target_material.roughness,
        helper.get_material_input_name('Specular Intensity', target_properties, use_o3de_mat_names) : target_material.specular_intensity
    }
    return attribute_dictionary

def get_transferable_properties():
    """! This is a listing of properties on a non-node material that can be directly transferred to an
    O3DE material.
    """
    target_properties = {
        'Diffuse Color': 'baseColor.color',             # Base Color
        'Metallic': 'metallic.factor',                  # Metallic
        'Roughness': 'roughness.factor',                # Roughness
        'Specular Intensity': 'specularF0.factor',      # Specular Intensity
    }
    return target_properties