import bpy
import sys
from materials.blender_materials import get_o3de_materials
import logging as _logging
_LOGGER = _logging.getLogger('GeomNodes.External.Scripts.params')

def get_geometry_nodes_params(obj):
    '''Get exposed geometry nodes params, defaults from OBJECT instead of node group, so user gets exactly whats in the blend file'''
    params = []
    for modifier in obj.modifiers:
        if modifier.type == 'NODES':
            if modifier.node_group:
                for input in modifier.node_group.inputs:                
                    
                    # Ignored
                    if input.type == 'MATERIAL' or input.type == 'GEOMETRY':
                        continue

                    new_param = {}
                    new_param['Id'] = input.identifier
                    new_param['Name'] = input.name
                    new_param['Type'] = input.type

                    if input.type == 'INT' or input.type == 'VALUE' or input.type == 'VECTOR':
                        new_param['MinValue'] = input.min_value
                        new_param['MaxValue'] = input.max_value

                    if input.type == "VECTOR":
                        new_param['DefaultValue'] = [modifier[input.identifier][0], modifier[input.identifier][1], modifier[input.identifier][2]]                    
                    elif input.type == "RGBA":
                        new_param['DefaultValue'] = [modifier[input.identifier][0], modifier[input.identifier][1], modifier[input.identifier][2], modifier[input.identifier][3]]
                    
                    
                    if input.type == 'INT' or input.type == 'VALUE' or input.type == 'BOOLEAN' or input.type == 'STRING':
                        new_param['DefaultValue'] = modifier[input.identifier]

                    params += [new_param]
    return params

def get_geometry_nodes_objs():
    geometry_nodes_objs = []
    for obj in bpy.data.objects:
        for modifier in obj.modifiers:
            if modifier.type == 'NODES':
                geometry_nodes_objs += [obj]
    
    if len(geometry_nodes_objs) < 1:
        _LOGGER.error("No geometry nodes object found")

    obj_arr = []
    obj_names = []
    for obj in geometry_nodes_objs:
        obj_names += [obj.name]   
        obj_arr += [{'Object': obj.name, 'Params': get_geometry_nodes_params(obj)}]

    return {'ObjectNames': obj_names, 'Objects': obj_arr, 'Materials' : get_o3de_materials()}