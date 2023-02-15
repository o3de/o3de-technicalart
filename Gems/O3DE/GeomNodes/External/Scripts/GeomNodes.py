import bpy
import sys
import os
import time
import datetime
import json
from messages import poll_for_messages

dir = os.path.dirname(bpy.data.filepath)
if not dir in sys.path:
    sys.path.append(dir)


from lib_loader import init_lib, MessageReader, MessageWriter

# def callback(id, content, length):
#     print('callback')
#     #print("id: %d, content: %s, length: %d)" % (id, content, length))
#     # buffer = bytes(json.dumps({'message' : 'from callback'}), "UTF-8")
#     # array = np.frombuffer(buffer, np.byte)
#     # from lib_loader import GNLibs
#     # GNLibs.SendMsg(array.ctypes.data_as(c.POINTER(c.c_ubyte)), array.nbytes, 0)
#     return 0

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
    
    #print(json.dumps(get_geometry_nodes_objs(), indent=4))

    while idle_time < 60:
        from lib_loader import GNLibs
        idle_time = idle_time + update_rate

        start = datetime.datetime.now()
        if poll_for_messages():
            idle_time = 0

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