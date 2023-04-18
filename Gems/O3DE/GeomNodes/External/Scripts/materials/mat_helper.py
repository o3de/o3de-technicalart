#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

import bpy
import shutil
from pathlib import Path
import constants

def get_material_node_attributes(node, target_properties: dict, use_o3de_mat_names=False):
    attribute_dictionary = {}
    for material_input in node.inputs:
        if material_input.name in target_properties.keys():
            try:
                material_input_name = get_material_input_name(material_input.name, target_properties, use_o3de_mat_names)
                attribute_dictionary[material_input_name] = get_dict_safe_value(material_input.default_value)
            except AttributeError:
                pass
    return attribute_dictionary

def get_material_input_name(material_input_name: str, target_properties: dict, use_o3de_mat_names=False):
    return target_properties[material_input_name] if use_o3de_mat_names else material_input_name

def get_dict_safe_value(value):
    if type(value) == bpy.types.bpy_prop_array:
        return list(value)
    else:
        return value
    
def get_selected_materials(selected):
    """!
    This function will check the selected mesh and find material are connected
    then build a material list for selected.
    @param selected This is your current selected mesh(s)
    """
    materials = []
    # Find Connected materials attached to mesh
    for obj in selected:
        try:
            materials = get_all_materials(obj, materials)
        except AttributeError:
            pass
    return materials

def get_all_materials(obj, materials):
    """!
    This function will loop through all assigned materails slots on a mesh and the attach textures.
    then build a material list for selected.
    @param selected This is your current selected mesh(s)
    """
    for mat in obj.material_slots:
        try:
            if mat.name not in materials:
                materials.append(mat.name)
        except AttributeError:
            pass
    return materials

def loop_through_selected_materials(texture_file_path, stored_image_source_paths):
    """!
    This function will loop the the selected materials and copy the files to the o3de project folder.
    @param texture_file_path This is the current o3de projects texture file path selected for texture export.
    @param stored_image_source_paths contains a copy of all store image source paths
    """
    if not Path(texture_file_path).exists():
        Path(texture_file_path).mkdir(exist_ok=True)
    # retrive the list of seleted mesh materials
    selected_materials = get_selected_materials(bpy.context.selected_objects)
    # Loop through Materials
    for mat in selected_materials:
        if mat:
            # Get the material
            material = bpy.data.materials[mat]
            # Loop through material node tree and get all the texture iamges
            for img in material.node_tree.nodes:
                if img.type == 'TEX_IMAGE':
                    # First make sure the image is not packed inside blender
                    if img.image.packed_file:
                        if Path(img.image.name).suffix in constants.IMAGE_EXT:
                            bpy.data.images[img.image.name].filepath = str(Path(texture_file_path).joinpath(img.image.name))
                        else:
                            ext = '.png'
                            build_string = f'{img.image.name}{ext}'
                            bpy.data.images[img.image.name].filepath = str(Path(texture_file_path).joinpath(build_string))
                        
                        bpy.data.images[img.image.name].save()
                    full_path = Path(bpy.path.abspath(img.image.filepath, library=img.image.library))
                    base_name = Path(full_path).name
                    if not full_path.exists(): # if embedded path doesnt exist, check current folder
                        full_path = Path(bpy.data.filepath).parent.joinpath(base_name)
                    o3de_texture_path = Path(texture_file_path).joinpath(base_name)
                    # Add to stored_image_source_paths to replace later
                    if not check_file_paths_duplicate(full_path, o3de_texture_path): # We check first if its not already copied over.
                        stored_image_source_paths[img.image.name] = full_path
                        # Copy the image to Destination
                        try:
                            bpy.data.images[img.image.name].filepath = str(o3de_texture_path)
                            if full_path.exists():
                                copy_texture_file(full_path, o3de_texture_path)
                            else:
                                bpy.data.images[img.image.name].save() 
                                # Save image to location
                        except (FileNotFoundError, RuntimeError):
                            pass
                    img.image.reload()

def copy_texture_file(source_path, destination_path):
    """!
    This function will copy project texture files to a O3DE textures folder
    @param source_path This is the texture source path
    @param destination_path This is the O3DE destination path
    """
    destination_size = -1
    source_size = source_path.stat().st_size
    if destination_path.exists():
        destination_size = destination_path.stat().st_size
    if source_size != destination_size:
        shutil.copyfile(source_path, destination_path)

def clone_repath_images(texture_file_path, stored_image_source_paths):
    """!
    This function will copy project texture files to a
    O3DE textures folder and repath the Blender materials
    then repath them to thier orginal.
    @param texture_file_path This is our o3de projects texture file export path
    @param stored_image_source_paths contains a copy of all store image source paths
    """
    
    dir_path = Path(texture_file_path)
    texture_file_path = Path(dir_path.parent).joinpath('textures')
    if not Path(texture_file_path).exists():
        Path(texture_file_path).mkdir(exist_ok=True)
    loop_through_selected_materials(texture_file_path, stored_image_source_paths)

def check_file_paths_duplicate(source_path, destination_path):
    """!
    This function check to see if Source Path and Dest Path are the same
    @param source_path This is the non o3de source texture path of the texture
    @param destination_path This is the destination o3de project texture path
    """
    try:
        check_file = Path(source_path)
        if check_file.samefile(destination_path):
            return True
        else:
            return False
    except FileNotFoundError:
        pass

def replace_stored_paths(stored_image_source_paths):
    """!
    This Function will replace all the repathed image paths to thier origninal source paths
    @param stored_image_source_paths contains a copy of all store image source paths
    """
    for image in bpy.data.images:
        try:
            # Find image and replace source path
            bpy.data.images[image.name].filepath = str(stored_image_source_paths[image.name])
        except KeyError:
            pass
        image.reload()
    stored_image_source_paths = {}


def duplicate_selected(selected_objects, rename):
    """!
    This function will duplicate selected objects with a new name string
    @param selected_objects This is the current selected object
    @param rename this is the duplicate object name
    @param return duplicated object
    """
    # Duplicate the mesh and add add the UDP name extension
    duplicate_object = bpy.data.objects.new(f'{selected_objects.name}{bpy.types.Scene.udp_type}', bpy.data.objects[selected_objects.name].data)
    # Add copy to current collection in scene
    bpy.context.collection.objects.link(duplicate_object)
    # Rename duplicated object
    duplicate_object.name = rename
    duplicate_object.data.name = rename
    return duplicate_object
