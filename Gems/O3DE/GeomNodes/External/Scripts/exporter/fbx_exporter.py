import bpy
from pathlib import Path
import utils
from materials import mat_helper
# import logging as _logging
# _LOGGER = _logging.getLogger('GeomNodes.External.Scripts.exporter.fbx_exporter')

def fbx_file_exporter(obj_name, fbx_file_path, mesh_to_triangles):
    """!
    This function will send to selected .FBX to an O3DE Project Path
    @param fbx_file_path this is the o3de project path where the meshe(s)
    will be exported as an .fbx
    @param file_name A custom file name string
    """

    print(fbx_file_path)

    # Lets create a dictionary to store all the source paths to place back after export
    stored_image_source_paths = {}
    source_file_path = Path(fbx_file_path) # Covert string to path
    mat_helper.clone_repath_images(source_file_path, stored_image_source_paths)

    # Deselect all objects
    bpy.ops.object.select_all(action='DESELECT')

    geomnodes_obj = bpy.data.objects.get(obj_name)
    if geomnodes_obj == None:
        geomnodes_obj = utils.get_geomnodes_obj()
    
    if geomnodes_obj.hide_get():
        geomnodes_obj.hide_set(False, view_layer=bpy.context.view_layer)

    depsgraph = bpy.context.evaluated_depsgraph_get()        
    eval_geomnodes_data = geomnodes_obj.evaluated_get(depsgraph).data

    # Create a new collection to hold the duplicated objects
    collection = bpy.data.collections.new(name="Export Collection")
    bpy.context.scene.collection.children.link(collection)

    scene_scale_len = bpy.context.scene.unit_settings.scale_length

    if geomnodes_obj.type == 'MESH':
        new_obj = bpy.data.objects.new(name=utils.remove_special_chars(eval_geomnodes_data.name), object_data=eval_geomnodes_data.copy())
        new_obj.matrix_world = geomnodes_obj.matrix_world.copy() * scene_scale_len
        new_obj.instance_type = geomnodes_obj.instance_type
        collection.objects.link(new_obj)

    for instance in depsgraph.object_instances:
        if instance.instance_object and instance.parent and instance.parent.original == geomnodes_obj:
            if instance.object.type == 'MESH':
                new_obj = bpy.data.objects.new(name=utils.remove_special_chars(instance.object.name), object_data=instance.object.data.copy())
                new_obj.matrix_world = instance.matrix_world.copy() * scene_scale_len
                new_obj.instance_type = instance.object.instance_type
                collection.objects.link(new_obj)
    
    for obj in collection.objects:
        obj.select_set(True)
    
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
    