import bpy
import sys
import os
import time
import datetime
import json
from messages import poll_for_messages
from materials.blender_materials import print_materials_in_scene, get_o3de_materials

dir = os.path.dirname(bpy.data.filepath)
if not dir in sys.path:
    sys.path.append(dir)


from lib_loader import init_lib, MessageReader, MessageWriter

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
    while idle_time < 60:
        from lib_loader import GNLibs
        idle_time = idle_time + update_rate

        start = datetime.datetime.now()
        reset, heartbeat_sent = poll_for_messages(idle_time, heartbeat_sent)
        if reset:
            idle_time = 0
        if heartbeat_sent and idle_time >= 5: # if we haven't received a reply back after some time(~5 secs) we bail out
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