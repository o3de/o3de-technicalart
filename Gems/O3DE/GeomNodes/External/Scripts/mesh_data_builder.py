import bpy
import bpy_types
import numpy as np
from mathutils import Matrix
import utils

import json
import datetime
from lib_loader import MapWriterSize, MapWriter
import logging as _logging
_LOGGER = _logging.getLogger('GeomNodes.External.Scripts.mesh_data_builder')

################ MESH DATA BUILDER ############################
def get_mesh_colors_data(mesh):
    has_vertex_indexed_colors = False

    if len(mesh.vertex_colors) > 0:
        colors = utils.get_prop_collection(mesh.vertex_colors[0].data, 'color', 4, np.float32)
    elif mesh.attributes.get('Col') and len(mesh.attributes.get('Col').data) > 0:
        attribute_colors = mesh.attributes.get('Col')
        colors = utils.get_prop_collection(attribute_colors.data, 'color', 4, np.float32)
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
        uvs = utils.get_prop_collection(attribute_uvmap.data, 'vector', uv_size, np.float32)
    elif mesh.attributes.get('uv_map') and len(mesh.attributes.get('uv_map').data) > 0:
        attribute_uvmap = mesh.attributes.get('uv_map')
        uv_size = len(attribute_uvmap.data[0].vector)
        convert_uvs = uv_size == 3
        has_vertex_indexed_uvs = attribute_uvmap.domain == 'POINT'
        uvs = utils.get_prop_collection(attribute_uvmap.data, 'vector', uv_size, np.float32)
    elif len(mesh.uv_layers) > 0:
        uvs = utils.get_prop_collection(mesh.uv_layers[0].data, 'uv', 2, np.float32)
    else:
        uvs = np.zeros(len(mesh.loops)*2, dtype=np.float32)
    
    # make float3 uvs into float2
    if convert_uvs:
        uvs = uvs.reshape((int(len(uvs)/3),3))
        uvs = np.delete(uvs, 2, 1)
        uvs = np.ravel(uvs)

    vertex_indexed_uvs = np.asarray(has_vertex_indexed_uvs, np.bool8)

    return uvs, vertex_indexed_uvs

def get_mesh_materials(mesh):
    names = []
    for material in mesh.materials[:]:
        if material is not None:
            names += [material.name_full]
        else:
            names += ['Default']
    return names

def get_materials_data(mesh):
    material_indices = utils.get_prop_collection(mesh.loop_triangles, 'material_index', 1, np.int32)
    material_names = np.frombuffer(bytes(json.dumps({'Materials' : get_mesh_materials(mesh)}), "UTF-8"), np.byte)

    return material_indices, material_names

def get_mesh_data(mesh):
    mesh.calc_loop_triangles()
    mesh.calc_normals_split()

    vertices = utils.get_prop_collection(mesh.vertices, 'co', 3, np.float32)
    normals = utils.get_prop_collection(mesh.loop_triangles, 'split_normals', 3*3, np.float32)
    indices = utils.get_prop_collection(mesh.loop_triangles, 'vertices', 3, np.int32)
    triangle_loops = utils.get_prop_collection(mesh.loop_triangles, 'loops', 3, np.int32)
    loops = utils.get_prop_collection(mesh.loops, 'vertex_index', 1, np.int32)

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
    #_LOGGER.debug('Started building Mesh Data')
    start = datetime.datetime.now()
    
    meshes, instances, local_matrices, world_matrices = utils.get_geomnodes_data_arrays(obj_name)
    
    total_bytes = 0
    total_bytes += MapWriterSize(np.int32).from_value(len(meshes))
    total_bytes += MapWriterSize(np.int32).from_value(len(instances))
    
    # Export meshes
    for mesh in meshes:
        for array in get_mesh_data(mesh):
            total_bytes += MapWriterSize().from_array(array)

    # Export instances
    for instance, local_matrix, world_matrix in zip(instances, local_matrices, world_matrices):
        for array in to_instance_arrays(instance, local_matrix, world_matrix):
            total_bytes += MapWriterSize().from_array(array)

    from lib_loader import GNLibs
    map_id = GNLibs.RequestSHM(total_bytes)
    #_LOGGER.debug('map_id = ' + str(map_id) + ' total_bytes = ' + str(total_bytes))
    MapWriter(map_id, np.int32).from_value(len(meshes))
    MapWriter(map_id, np.int32).from_value(len(instances))
    
    # Export meshes
    for mesh in meshes:
        for array in get_mesh_data(mesh):
            MapWriter(map_id).from_array(array)
    
    # Export Instances
    for instance, local_matrix, world_matrix in zip(instances, local_matrices, world_matrices):
        for array in to_instance_arrays(instance, local_matrix, world_matrix):
            MapWriter(map_id).from_array(array)

    end = datetime.datetime.now()
    #_LOGGER.debug('Built Mesh Data and sent in ' + str((end-start).seconds + (end-start).microseconds/1000000) + 's')
    return map_id
###############################################################