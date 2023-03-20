import bpy
import numpy as np
from pathlib import Path
import re

def get_geomnodes_obj():
    for obj in bpy.data.objects:
        for modifier in obj.modifiers:
            if modifier.type == 'NODES':
                return obj

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