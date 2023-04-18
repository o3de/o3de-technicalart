#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

import bpy
import numpy as np
from pathlib import Path
import re
from mathutils import Matrix

def get_geomnodes_obj():
    for obj in bpy.data.objects:
        for modifier in obj.modifiers:
            if modifier.type == 'NODES':
                return obj

def create_geomnodes_copies(obj_name):
    '''! This function creates copies of objects/meshes produced by the Geometry Nodes modifier. Depending on the current parameters set this could vary.
        Note that since objects are created you have to remove the objects later after using them.
    @param obj_name is the currently selected object name.
    @return copies of objects of meshes
    '''
    geomnodes_obj = bpy.data.objects.get(obj_name)
    if geomnodes_obj == None:
        geomnodes_obj = get_geomnodes_obj()
    
    if geomnodes_obj.hide_get():
        geomnodes_obj.hide_set(False, view_layer=bpy.context.view_layer)

    depsgraph = bpy.context.evaluated_depsgraph_get()        
    eval_geomnodes_data = geomnodes_obj.evaluated_get(depsgraph).data

    scene_scale_len = bpy.context.scene.unit_settings.scale_length

    geomnodes_objects = []
    if geomnodes_obj.type == 'MESH':
        new_obj = bpy.data.objects.new(name=remove_special_chars(eval_geomnodes_data.name), object_data=eval_geomnodes_data.copy())
        new_obj.matrix_world = geomnodes_obj.matrix_world.copy() * scene_scale_len
        new_obj.instance_type = geomnodes_obj.instance_type
        geomnodes_objects.append(new_obj)

    for instance in depsgraph.object_instances:
        if instance.instance_object and instance.parent and instance.parent.original == geomnodes_obj:
            if instance.object.type == 'MESH':
                new_obj = bpy.data.objects.new(name=remove_special_chars(instance.object.name), object_data=instance.object.data.copy())
                new_obj.matrix_world = instance.matrix_world.copy() * scene_scale_len
                new_obj.instance_type = instance.object.instance_type
                geomnodes_objects.append(new_obj)

    return geomnodes_objects

def get_geomnodes_data_arrays(obj_name):
    """! Produce the data arrays needed for sending to the gem. This are based on the objects, meshes, instances produced by the Geometry Nodes modifier depending on the current parameters and object selected.
    @param obj_name is the currently selected object name.
    @return data arrays
    """
    meshes = []
    hashes = []
    instance_meshes = []
    local_matrices = []
    world_matrices = []

    geomnodes_obj = bpy.data.objects.get(obj_name)
    if geomnodes_obj == None:
        geomnodes_obj = get_geomnodes_obj()

    if geomnodes_obj.hide_get():
        geomnodes_obj.hide_set(False, view_layer=bpy.context.view_layer)

    depsgraph = bpy.context.evaluated_depsgraph_get()        
    eval_geomnodes_data = geomnodes_obj.evaluated_get(depsgraph).data
    
    scene_scale_len = bpy.context.scene.unit_settings.scale_length

    # Get number of meshes and instances
    if geomnodes_obj.type == 'MESH':
        hashes = [hash(geomnodes_obj)]
        meshes = [eval_geomnodes_data]
        instance_meshes = [eval_geomnodes_data]
        local_matrices = [Matrix() * scene_scale_len]
        world_matrices = [geomnodes_obj.matrix_world.copy() * scene_scale_len]

    for instance in depsgraph.object_instances:
        if instance.instance_object and instance.parent and instance.parent.original == geomnodes_obj:
            if instance.object.type == 'MESH':
                # add only if the instance is not in the hash array
                if hash(instance.object.data) not in hashes:
                    hashes += [hash(instance.object.data)]
                    meshes += [instance.object.data]
                # save the instance data and transforms
                if hash(instance.object.data) in hashes:
                    instance_meshes += [instance.object.data]
                    local_matrices += [(instance.parent.matrix_world.inverted() @ instance.matrix_world) * scene_scale_len]
                    world_matrices += [instance.matrix_world.copy() * scene_scale_len]
    return meshes, instance_meshes, local_matrices, world_matrices

def get_prop_collection(prop_collection, attr, multipler, data_type):
    array = np.empty(len(prop_collection) * multipler, dtype=data_type)  
    prop_collection.foreach_get(attr, array)

    return array

def add_remove_modifier(modifier_name, add):
    """!
    This function will add or remove a modifier to selected
    @param modifier_name is the name of the modifier you wish to add or remove
    @param add if Bool True will add the modifier, if False will remove 
    """
    context = bpy.context

    for selected_obj in context.selected_objects:
        if selected_obj is not []:
            if selected_obj.type == "MESH":
                if add:
                    # Set the mesh active
                    bpy.context.view_layer.objects.active = selected_obj
                    # Add Modifier
                    bpy.ops.object.modifier_add(type=modifier_name)
                    if modifier_name == "TRIANGULATE":
                        bpy.context.object.modifiers["Triangulate"].keep_custom_normals = True
                else:
                    # Set the mesh active
                    bpy.context.view_layer.objects.active = selected_obj
                    # Remove Modifier
                    bpy.ops.object.modifier_remove(modifier=modifier_name)
    

def remove_special_chars(string: str) -> str:
    return re.sub(r'[^\w\s]+', '_', string)

def select_set_objects(object_list: list, flag: bool):
    """! This function will set the select value of all objects in the list.
    @param object_list is the list of objects.
    @flag value that will be set
    """
    for obj in object_list:
        obj.select_set(flag)