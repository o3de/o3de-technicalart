import bpy
import sys
import os
import datetime
import json
from lib_loader import MessageReader, MessageWriter
from params import get_geometry_nodes_objs
from mesh_data_builder import build_mesh_data
from utils import get_geomnodes_obj
import logging as _logging
_LOGGER = _logging.getLogger('GeomNodes.External.Scripts.messages')

def import_texture(Id, x, y):
    #_LOGGER.debug(str(x) + ' ' + str(y))
    image = bpy.data.images.new(Id + "_Image", width=x, height=y)
    return image

def update_geometry_node_params(geometry_nodes_obj, new_params: dict):    
    bpy.context.view_layer.objects.active =geometry_nodes_obj

    GN = next(modifier for modifier in bpy.context.object.modifiers if modifier.type == 'NODES')

    for params in new_params['Params']:
        if params['Type'] == 'VECTOR':
            vect = params['Value'][0]
            GN[params['Id']][:] = [vect['X'], vect['Y'], vect['Z']]
        elif params['Type'] == 'INT':
            GN[params['Id']] = int(params['Value'])
        elif params['Type'] == 'VALUE':
            GN[params['Id']] = float(params['Value'])
        elif params['Type'] == 'BOOLEAN':
            GN[params['Id']] = bool(params['Value'])
        # elif params['Type'] == 'MESH':
        #     GN[params['Id']] = params['Object']
        # elif params['Type'] == 'COLLECTION':
        #     GN[params['Id']] = params['Collection']            
        elif params['Type'] == 'STRING':
            GN[params['Id']] = str(params['Value'])
        # elif params['Type'] == 'IMAGE' or params['Type'] == 'TEXTURE':
        #     GN[params['Id']] = params['Tex']
        # elif params['Type'] == 'RGBA':
        #     vect = params['Value'][0]
        #     GN[params['Id']][:] = [vect['R'], vect['G'], vect['B'], vect['A']]

    for obj in bpy.data.objects:
        if obj.data and obj.type == "MESH":
            obj.data.update()

def update_gn_in_blender(msg_dict):
    object_name = msg_dict['Object']
    geometry_nodes_obj = bpy.data.objects.get(object_name)
    if geometry_nodes_obj == None:
        geometry_nodes_obj = get_geomnodes_obj()

    #_LOGGER.debug('Updating Geometry Node')
    
    start = datetime.datetime.now()
    update_geometry_node_params(geometry_nodes_obj, msg_dict)
    end = datetime.datetime.now()
    
    #_LOGGER.debug('Updated in ' + str((end-start).seconds + (end-start).microseconds/1000000) + 's')

    return object_name




def poll_for_messages():
    # poll if there are messages until we exhaust them.
    msg_str = MessageReader().as_string()
    if len(msg_str) > 0:
        try:
            #_LOGGER.debug(msg_str)
            msg_dict = json.loads(msg_str)
        except json.decoder.JSONDecodeError:
            if msg_str != "":
                _LOGGER.debug('couldnt parse json: ' + msg_str)
        else:
            if msg_dict is not None:
                if 'FetchObjectParams' in msg_dict:
                    MessageWriter().from_buffer(bytes(json.dumps(get_geometry_nodes_objs()), "UTF-8"))
                elif 'ParamUpdate' in msg_dict:
                    # based on the parameter updates we pass it to blender to update the objects
                    object_name = update_gn_in_blender(msg_dict)
                    # after the updates, build the mesh data to be sent to the gem
                    map_id = build_mesh_data(object_name)
                    # inform the gem that the 3D data is available on the SHM.
                    MessageWriter().from_buffer(bytes(json.dumps({'SHMOpen' : True, 'MapId' : map_id }), "UTF-8"))
                elif 'SHMClose' in msg_dict:
                    # after the gem read the data it will send a message to the client so that it could close the SHM instance and free it in the map table.
                    map_id = msg_dict['MapId']
                    from lib_loader import GNLibs
                    GNLibs.ClearSHM(map_id)

        return True
    return False