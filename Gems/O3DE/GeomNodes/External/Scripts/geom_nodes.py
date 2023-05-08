#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

import bpy
import sys
import os
import time
import datetime
import json
from messages import poll_for_messages, PollReturn, update_gn_in_blender
#from materials.blender_materials import print_materials_in_scene, get_o3de_materials
from exporter import fbx_exporter

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
    
    #run_tests()

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
    

def run_tests():
    msg_dict = {
                    "Params": [
                    {
                        "Id": "Input_5",
                        "Value": True,
                        "Type": "BOOLEAN"
                    }
                ,
                    {
                        "Id": "Input_8",
                        "Value": True,
                        "Type": "BOOLEAN"
                    }
                ,
                    {
                        "Id": "Input_6",
                        "Value": 3,
                        "Type": "INT"
                    }
                ,
                    {
                        "Id": "Input_7",
                        "Value": 12,
                        "Type": "INT"
                    }
                ,
                    {
                        "Id": "Input_3",
                        "Value": 3.00000000000000000,
                        "Type": "VALUE"
                    }
                ,
                    {
                        "Id": "Input_9",
                        "Value": 3.00000000000000000,
                        "Type": "VALUE"
                    }
                ],
                    "Object": "building_base",
                    "Frame": 0,
                    "ParamUpdate": True
                }
    obj_name = update_gn_in_blender(msg_dict)
    fbx_exporter.fbx_file_exporter(obj_name, "F:/o3de_proj/TestProject/assets/geomNodes/buildify_1_0/buildify_1_0.fbx", False)
    
    # TEST
    #print(json.dumps(get_o3de_materials(), indent=4))
    #print_materials_in_scene()