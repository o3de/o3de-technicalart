import bpy
from materials import mat_helper as helper
import logging as _logging
_LOGGER = _logging.getLogger('GeomNodes.External.Scripts.materials.convert_subsurface_scattering_material')

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
    return 'Subsurface Scattering'

def get_material_info(node, use_o3de_mat_names=False) -> dict:
    # material_textures = get_material_textures(target_material)
    target_properties = get_transferable_properties()
    material_settings =  helper.get_material_node_attributes(node, target_properties, use_o3de_mat_names)
    material_settings[helper.get_material_input_name('Enable', target_properties, use_o3de_mat_names)] = True
    # material_info = {
    #     'material_type': 'Subsurface Scattering',
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
        'Enable': 'subsurfaceScattering.enableSubsurfaceScattering',      # Enable  
        'Color': 'subsurfaceScattering.scatterColor',                     # Color
        'Scale': 'subsurfaceScattering.transmissionScale',                # Scale
    }
    return target_properties