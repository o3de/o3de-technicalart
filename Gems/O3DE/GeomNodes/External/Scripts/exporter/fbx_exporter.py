import bpy
from pathlib import Path
import utils
from materials import mat_helper
# import logging as _logging
# _LOGGER = _logging.getLogger('GeomNodes.External.Scripts.exporter.fbx_exporter')

def group_objects_by_name(objects):
    groups = {}
    for obj in objects:
        name_parts = obj.name.split('.')
        if len(name_parts) > 1:
            name_prefix = '.'.join(name_parts[:-1])
            if name_prefix not in groups:
                groups[name_prefix] = []
            groups[name_prefix].append(obj)
    return groups

def fbx_file_exporter(obj_name, fbx_file_path, mesh_to_triangles):
    """!
    This function will send to selected .FBX to an O3DE Project Path
    @param fbx_file_path this is the o3de project path where the meshe(s)
    will be exported as an .fbx
    @param file_name A custom file name string
    """

    # Lets create a dictionary to store all the source paths to place back after export
    stored_image_source_paths = {}
    source_file_path = Path(fbx_file_path) # Covert string to path
    mat_helper.clone_repath_images(source_file_path, stored_image_source_paths)

    # Deselect all objects
    bpy.ops.object.select_all(action='DESELECT')

    # Create a new collection to hold the duplicated objects
    collection = bpy.data.collections.new(name="Export Collection")
    bpy.context.scene.collection.children.link(collection)

    # create copies of objects/meshes produces by geometry nodes modifier
    geomnodes_objects = utils.create_geomnodes_copies(obj_name)

    # add geomnode objects to the collection. Need this since we will be joining them later and the objects needs to be in the scene
    for obj in geomnodes_objects:
        collection.objects.link(obj)

    groups = group_objects_by_name(geomnodes_objects)
    for _, objects  in groups.items():
        if len(objects) > 2: # join 2 or more objects, if the group only has one it will be included in the collection and in turn will be exported
            # Select all the objects that use this material
            bpy.ops.object.select_all(action='DESELECT')
            bpy.context.view_layer.objects.active = objects[0]
            utils.select_set_objects(objects, True)
            
            # Join the objects
            bpy.ops.object.join()
    
    utils.select_set_objects(collection.objects, True)
    
    if mesh_to_triangles:
        utils.add_remove_modifier("TRIANGULATE", True)
    # Main Blender FBX API exporter
    bpy.ops.export_scene.fbx( 
        filepath=str(fbx_file_path),
        check_existing=False,
        filter_glob='*.fbx',
        use_selection=True,
        use_active_collection=False,
        global_scale=1.0,
        apply_unit_scale=True,
        apply_scale_options='FBX_SCALE_UNITS',
        use_space_transform=True,
        bake_space_transform=False,
        object_types={'ARMATURE', 'CAMERA', 'EMPTY', 'LIGHT', 'MESH', 'OTHER'},
        use_mesh_modifiers=True,
        use_mesh_modifiers_render=True,
        mesh_smooth_type='OFF',
        use_subsurf=False,
        use_mesh_edges=False,
        use_tspace=False,
        use_custom_props=False,
        add_leaf_bones=False,
        primary_bone_axis='Y',
        secondary_bone_axis='X',
        use_armature_deform_only=False,
        armature_nodetype='NULL',
        bake_anim=False,
        bake_anim_use_all_bones=False,
        bake_anim_use_nla_strips=False,
        bake_anim_use_all_actions=False,
        bake_anim_force_startend_keying=False,
        bake_anim_step=1.0,
        bake_anim_simplify_factor=1.0,
        path_mode='AUTO',
        embed_textures=True,
        batch_mode='OFF',
        use_batch_own_dir=False,
        use_metadata=True,
        axis_forward='-Z',
        axis_up='Y')
    
    if mesh_to_triangles:
        utils.add_remove_modifier("TRIANGULATE", False)
    
    mat_helper.replace_stored_paths(stored_image_source_paths)

    # Delete the duplicated objects and the collection
    for obj in collection.objects:
        bpy.data.objects.remove(obj, do_unlink=True)
    bpy.data.collections.remove(collection)
    