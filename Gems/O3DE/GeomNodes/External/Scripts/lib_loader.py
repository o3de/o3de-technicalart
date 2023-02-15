import ctypes
import os
import numpy as np

GNLibs = None

callback_type = ctypes.CFUNCTYPE(ctypes.c_long, ctypes.c_ulonglong, ctypes.POINTER(ctypes.c_ubyte), ctypes.c_ulonglong)

def init_lib(exePath, id):
    lib_path = exePath + '\\Bridge.dll'
    print('lib path: ' + lib_path)
    print(type(int(id)))

    global GNLibs
    
    GNLibs = ctypes.cdll.LoadLibrary(lib_path)
    GNLibs.Init.argtypes = [ctypes.c_ulonglong, callback_type]
    GNLibs.Init.restype = None

    GNLibs.Uninitialize.argtypes = None
    GNLibs.Uninitialize.restype = None

    GNLibs.SendMsg.argtypes = (ctypes.POINTER(ctypes.c_ubyte), ctypes.c_ulonglong, ctypes.c_ulonglong)
    GNLibs.SendMsg.restype = None

    GNLibs.IsConnected.argtypes = None
    GNLibs.IsConnected.restype = ctypes.c_bool

    GNLibs.CheckForMsg.argtypes = None
    GNLibs.CheckForMsg.restype = ctypes.c_ulonglong

    GNLibs.ReadMsg.argtypes = (ctypes.POINTER(ctypes.c_ubyte), ctypes.c_ulonglong)
    GNLibs.ReadMsg.restype = ctypes.c_ulonglong

    # Map functions
    GNLibs.RequestSHM.argtypes = (ctypes.c_ulonglong,)
    GNLibs.RequestSHM.restype = ctypes.c_ulonglong

    GNLibs.OpenSHM.argtypes = (ctypes.c_ulonglong,)
    GNLibs.OpenSHM.restype = ctypes.c_bool

    GNLibs.ReadSHM.argtypes = (ctypes.c_ulonglong, ctypes.c_void_p, ctypes.POINTER(ctypes.c_ulonglong))
    GNLibs.ReadSHM.restype = ctypes.c_bool

    GNLibs.WriteSHM.argtypes = (ctypes.c_ulonglong, ctypes.POINTER(ctypes.c_ubyte), ctypes.c_ulonglong)
    GNLibs.WriteSHM.restype = None

    GNLibs.ClearSHM.argtypes = (ctypes.c_ulonglong,)
    GNLibs.ClearSHM.restype = None

    cb_func = callback_type() # passing a null ptr as we don't use callback in python yet(if we figure out how to properly use a callback here)
    GNLibs.Init(int(id), cb_func)

class MessageReader:
    buffer = 0
    length = 0
    dtype = None
    
    def __init__(self, dtype=np.byte) -> None:
        length = GNLibs.CheckForMsg()
        if length > 0:
            buffer = (ctypes.c_ubyte * length)()
            if GNLibs.ReadMsg(buffer, length):    
                bytes = buffer
        else:
            bytes = b''

        self.buffer = bytes
        self.length = length
        self.dtype = dtype
    
    def as_array(self, num_components=1):
        array = np.frombuffer(self.buffer, np.dtype(self.dtype))
        if num_components > 1:
            array = array.reshape(array.size//num_components, num_components)
        return array

    def as_list(self, num_components=1):
        return self.as_array(num_components).tolist()

    def as_value(self):
        return self.as_array().item()

    def as_string(self):
        string = str(self.buffer, encoding = 'ascii')
        string = string.replace('\x00', '')
        return string

class MessageWriter:
    dtype = None
    
    def __init__(self, dtype=np.byte) -> None:
        self.dtype = dtype

    def from_array(self, array):
        GNLibs.SendMsg(array.ctypes.data_as(ctypes.POINTER(ctypes.c_ubyte)), array.nbytes, 1)

    def from_value(self, value):
        array = np.asarray(value, self.dtype)
        GNLibs.SendMsg(array.ctypes.data_as(ctypes.POINTER(ctypes.c_ubyte)), array.nbytes, 1)
        
    def from_buffer(self, value):
        array = np.frombuffer(value, self.dtype)
        GNLibs.SendMsg(array.ctypes.data_as(ctypes.POINTER(ctypes.c_ubyte)), array.nbytes, 1)

class MapWriter():
    dtype = None

    def __init__(self, mapId, dtype=np.byte) -> None:
        self.dtype = dtype
        self.mapId = mapId

    def from_array(self, array):
        GNLibs.WriteSHM(self.mapId, array.ctypes.data_as(ctypes.POINTER(ctypes.c_ubyte)), array.nbytes)

    def from_value(self, value):
        array = np.asarray(value, self.dtype)
        GNLibs.WriteSHM(self.mapId, array.ctypes.data_as(ctypes.POINTER(ctypes.c_ubyte)), array.nbytes)
        
    def from_buffer(self, value):
        array = np.frombuffer(value, self.dtype)
        GNLibs.WriteSHM(self.mapId, array.ctypes.data_as(ctypes.POINTER(ctypes.c_ubyte)), array.nbytes)

class MapWriterSize():
    dtype = None

    def __init__(self, dtype=np.byte) -> None:
        x = np.array([1], dtype=np.int32)
        self.chunksize = x.itemsize
        self.dtype = dtype

    def from_array(self, array):
        return self.chunksize + array.nbytes

    def from_value(self, value):
        array = np.asarray(value, self.dtype)
        return self.chunksize + array.nbytes
        
    def from_buffer(self, value):
        array = np.frombuffer(value, self.dtype)
        return self.chunksize + array.nbytes
