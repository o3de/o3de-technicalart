import bpy
import logging as _logging
from materials import mat_helper as helper

_LOGGER = _logging.getLogger('GeomNodes.External.Scripts.materials.convert_diffuse_bsdf_material')

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
    return 'Diffuse BSDF'

def get_material_info(node, use_o3de_mat_names=False) -> dict:
    target_properties = get_transferable_properties()
    material_settings =  helper.get_material_node_attributes(node, target_properties, use_o3de_mat_names)
    return material_settings

def get_transferable_properties():
    """! This is a listing of properties on a Diffuse BSDF material that can be directly transferred to an
    O3DE material. For now we only do basic conversion but if O3DE add support to a Diffuse BSDF update this accordingly
    """
    target_properties = {
        'Color': 'baseColor.color',                     # Base Color
        'Roughness': 'roughness.factor',                # Roughness
    }
    return target_properties