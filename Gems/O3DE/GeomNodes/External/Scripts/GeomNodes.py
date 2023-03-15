import bpy
import sys
import os
import time
import datetime
import json
from messages import poll_for_messages, PollReturn
#from materials.blender_materials import print_materials_in_scene, get_o3de_materials

dir = os.path.dirname(bpy.data.filepath)
if not dir in sys.path:
    sys.path.append(dir)

from lib_loader import init_lib, MessageWriter

def init(exePath, id):
    init_lib(exePath, id)

def run():    
    update_rate = 0.005
    idle_time = 0

    # make object visible
    for layer_collection in bpy.context.view_layer.layer_collection.children:
        layer_collection.exclude = False

    # send message to the gem that script is initialized
    MessageWriter().from_buffer(bytes(json.dumps({'Initialized' : True}), "UTF-8"))
    
    # TEST
    #print(json.dumps(get_o3de_materials(), indent=4))
    #print_materials_in_scene()
    
    heartbeat_sent = False
    heartbeat_wait_time = 0
    heartbeat_time = 0
    while idle_time < 60:
        from lib_loader import GNLibs
        idle_time = idle_time + update_rate
        heartbeat_time = heartbeat_time + update_rate

        start = datetime.datetime.now()
        poll_return = poll_for_messages()
        if poll_return == PollReturn.MESSAGE:
            idle_time = 0
        elif poll_return == PollReturn.HEARTBEAT:
            heartbeat_time = 0
            heartbeat_wait_time = 0
            heartbeat_sent = False
        
        if heartbeat_sent == True:
            heartbeat_wait_time = heartbeat_wait_time + update_rate
        elif heartbeat_time >= 1: # send heartbeat every 2 secs
            MessageWriter().from_buffer(bytes(json.dumps({'Heartbeat' : True }), "UTF-8")) # send a heartbeat message to the server
            heartbeat_sent = True
        
        if heartbeat_sent and heartbeat_wait_time >= 1.5: # if we haven't received a reply back after some time(~5 secs) we bail out
            break

        end = datetime.datetime.now()
        # print('Export time: ' + str((end-start).seconds + (end-start).microseconds/1000000) + 's', flush=True)
        if bpy.app.background:
            time.sleep(update_rate)
        else:
            break

    if bpy.app.background:
        GNLibs.Uninitialize()
    else:
        return update_rate