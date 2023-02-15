import bpy
import bpy_types
import numpy as np
from mathutils import Matrix
from utils import get_prop_collection

import json
import datetime
from utils import get_geomnodes_obj
from lib_loader import MapWriterSize, MapWriter

################ MESH DATA BUILDER ############################
def get_mesh_colors_data(mesh):
    has_vertex_indexed_colors = False

    if len(mesh.vertex_colors) > 0:
        colors = get_prop_collection(mesh.vertex_colors[0].data, 'color', 4, np.float32)
    elif mesh.attributes.get('Col') and len(mesh.attributes.get('Col').data) > 0:
        attribute_colors = mesh.attributes.get('Col')
        colors = get_prop_collection(attribute_colors.data, 'color', 4, np.float32)
        has_vertex_indexed_colors = attribute_colors.domain == 'POINT'
    else:
        colors = np.zeros(len(mesh.loops)*4, dtype=np.float32)

    vertex_indexed_colors = np.asarray(has_vertex_indexed_colors, np.bool8) 
    return colors, vertex_indexed_colors


def get_mesh_uv_data(mesh):
    convert_uvs = False
    has_vertex_indexed_uvs = False

    if mesh.attributes.get('UVMap') and len(mesh.attributes.get('UVMap').data) > 0:
        attribute_uvmap = mesh.attributes.get('UVMap')     
        uv_size = len(attribute_uvmap.data[0].vector)
        convert_uvs = uv_size == 3
        has_vertex_indexed_uvs = attribute_uvmap.domain == 'POINT'
        uvs = get_prop_collection(attribute_uvmap.data, 'vector', uv_size, np.float32)
    elif mesh.attributes.get('uv_map') and len(mesh.attributes.get('uv_map').data) > 0:
        attribute_uvmap = mesh.attributes.get('uv_map')
        uv_size = len(attribute_uvmap.data[0].vector)
        convert_uvs = uv_size == 3
        has_vertex_indexed_uvs = attribute_uvmap.domain == 'POINT'
        uvs = get_prop_collection(attribute_uvmap.data, 'vector', uv_size, np.float32)
    elif len(mesh.uv_layers) > 0:
        uvs = get_prop_collection(mesh.uv_layers[0].data, 'uv', 2, np.float32)
    else:
        uvs = np.zeros(len(mesh.loops)*2, dtype=np.float32)
    
    # make float3 uvs into float2
    if convert_uvs:
        uvs = uvs.reshape((int(len(uvs)/3),3))
        uvs = np.delete(uvs, 2, 1)
        uvs = np.ravel(uvs)

    vertex_indexed_uvs = np.asarray(has_vertex_indexed_uvs, np.bool8)

    return uvs, vertex_indexed_uvs

def get_materials(mesh):
    names = []
    for material in mesh.materials[:]:
        if material is not None:
            names += [material.name_full]
        else:
            names += ['Default']
    return names

def get_materials_data(mesh):
    material_indices = get_prop_collection(mesh.loop_triangles, 'material_index', 1, np.int32)
    material_names = np.frombuffer(bytes(json.dumps({'materials' : get_materials(mesh)}), "UTF-8"), np.byte)

    return material_indices, material_names

def get_mesh_data(mesh):
    mesh.calc_loop_triangles()
    mesh.calc_normals_split()

    vertices = get_prop_collection(mesh.vertices, 'co', 3, np.float32)
    normals = get_prop_collection(mesh.loop_triangles, 'split_normals', 3*3, np.float32)
    indices = get_prop_collection(mesh.loop_triangles, 'vertices', 3, np.int32)
    triangle_loops = get_prop_collection(mesh.loop_triangles, 'loops', 3, np.int32)
    loops = get_prop_collection(mesh.loops, 'vertex_index', 1, np.int32)

    mesh_hash = np.asarray(hash(mesh), np.int64)

    colors, vertex_indexed_color = get_mesh_colors_data(mesh)
    uvs, vertex_indexed_uvs = get_mesh_uv_data(mesh)
    material_indices, material_names = get_materials_data(mesh)
    
    return vertices, normals, indices, triangle_loops, loops, mesh_hash, colors, vertex_indexed_color, uvs, vertex_indexed_uvs, material_indices, material_names

def to_instance_arrays(mesh, local_matrix, world_matrix):
    mesh_hash = np.asarray(hash(mesh), np.int64)
    local_matrix = np.array(local_matrix, np.float32)
    world_matrix = np.array(world_matrix, np.float32)
    
    return mesh_hash, local_matrix, world_matrix

def build_mesh_data(obj_name):
    geomnodes_obj = bpy.data.objects.get(obj_name)
    if geomnodes_obj == None:
        geomnodes_obj = get_geomnodes_obj()

    if geomnodes_obj.hide_get():
        geomnodes_obj.hide_set(False, view_layer=bpy.context.view_layer)

    print('Started building Mesh Data')
    start = datetime.datetime.now()
    depsgraph = bpy.context.evaluated_depsgraph_get()        
    eval_geomnodes_data = geomnodes_obj.evaluated_get(depsgraph).data

    mesh_arr = []
    hash_arr = []
    instance_arr = []
    instance_matrix_loc_arr = []
    instance_matrix_world_arr = []
    
    scene_scale_len = bpy.context.scene.unit_settings.scale_length

    # Get number of meshes and instances
    if geomnodes_obj.type == 'MESH':
        hash_arr = [hash(geomnodes_obj)]
        mesh_arr = [eval_geomnodes_data]
        instance_arr = [eval_geomnodes_data]
        instance_matrix_loc_arr = [Matrix() * scene_scale_len]
        instance_matrix_world_arr = [geomnodes_obj.matrix_world.copy() * scene_scale_len]

    for instance in depsgraph.object_instances:
        if instance.instance_object and instance.parent and instance.parent.original == geomnodes_obj:
            if instance.object.type == 'MESH':
                # add only if the instance is not in the hash array
                if hash(instance.object.data) not in hash_arr:
                    hash_arr += [hash(instance.object.data)]
                    mesh_arr += [instance.object.data]
                # save the instance data and transforms
                if hash(instance.object.data) in hash_arr:
                    instance_arr += [instance.object.data]
                    local_matrix = instance.parent.matrix_world.inverted() @ instance.matrix_world                        
                    instance_matrix_loc_arr += [local_matrix * scene_scale_len]
                    instance_matrix_world_arr += [instance.matrix_world.copy() * scene_scale_len]
                    
    
    total_bytes = 0
    total_bytes += MapWriterSize(np.int32).from_value(len(mesh_arr))
    total_bytes += MapWriterSize(np.int32).from_value(len(instance_arr))
    
    # Export meshes
    for mesh in mesh_arr:
        for array in get_mesh_data(mesh):
            total_bytes += MapWriterSize().from_array(array)

    # Export instances
    for instance, local_matrix, world_matrix in zip(instance_arr, instance_matrix_loc_arr, instance_matrix_world_arr):
        for array in to_instance_arrays(instance, local_matrix, world_matrix):
            total_bytes += MapWriterSize().from_array(array)

    from lib_loader import GNLibs
    map_id = GNLibs.RequestSHM(total_bytes)
    print('map_id = ' + str(map_id) + ' total_bytes = ' + str(total_bytes), flush=True)
    MapWriter(map_id, np.int32).from_value(len(mesh_arr))
    MapWriter(map_id, np.int32).from_value(len(instance_arr))
    
    # Export meshes
    for mesh in mesh_arr:
        for array in get_mesh_data(mesh):
            MapWriter(map_id).from_array(array)
    
    # Export Instances
    for instance, local_matrix, world_matrix in zip(instance_arr, instance_matrix_loc_arr, instance_matrix_world_arr):
        for array in to_instance_arrays(instance, local_matrix, world_matrix):
            MapWriter(map_id).from_array(array)

    end = datetime.datetime.now()
    print('Built Mesh Data and sent in ' + str((end-start).seconds + (end-start).microseconds/1000000) + 's', flush=True)
    return map_id
###############################################################