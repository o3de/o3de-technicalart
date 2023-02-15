import bpy
import numpy as np

def get_geomnodes_obj():
    for obj in bpy.data.objects:
            for modifier in obj.modifiers:
                if modifier.type == 'NODES':
                    return obj

def get_prop_collection(prop_collection, attr, multipler, data_type):
    array = np.empty(len(prop_collection) * multipler, dtype=data_type)  
    prop_collection.foreach_get(attr, array)

    return array