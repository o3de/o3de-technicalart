"""
Copyright (c) Contributors to the Open 3D Engine Project.
For complete copyright and license terms please see the LICENSE at the root of this distribution.

SPDX-License-Identifier: Apache-2.0 OR MIT
"""
# -------------------------------------------------------------------------
import azlmbr
import azlmbr.bus as bus
import azlmbr.entity as entity
import azlmbr.legacy.general as general
import azlmbr.editor as editor
import azlmbr.render as render
import azlmbr.math as math
import azlmbr.asset as asset
import azlmbr.paths as projectPath

import xml.etree.cElementTree as xml_et
from pathlib import Path
import utilities as py_utilities
import editor_tools as editor_tools
import constants as const
import os
import subprocess
import logging


logger = logging.getLogger(__name__)
_PROCESS_OUTPUT_ENCODING = 'utf-8'

def search_tree(root: object, xml_obj: object, urdf_mesh_path: Path, robot_name: str):
    """
    This function will search the tree from the root base_link Entity ID found by search

    :param root: Entity Object ID
    :param xml_obj: XML Object
    :param urdf_mesh_path: The Path to URDF Meshes Folder
    :param robot_name: Robot Description Name
    """
    # base_link Name_link_obj
    name = editor_tools.get_entity_name(root)
    
    # Check if this node name is base_link
    if name == "base_link":
        add_elements(root, xml_obj, urdf_mesh_path, robot_name)
    
    # Go down the tree chain
    model_tree = editor.EditorEntityInfoRequestBus(bus.Event, 'GetChildren', root)
    
    # Loop the tree and find the childern
    for child_id in model_tree:
        add_elements(child_id, xml_obj, urdf_mesh_path, robot_name)
        
        # Get the child ID if this entity has one
        entity_child_id = editor.EditorEntityInfoRequestBus(bus.Event, "GetChildren", child_id)
        
        # Get the name of the entity   
        name = editor_tools.get_entity_name(root)
        
        # Search through children of children.
        for sub_child_id in entity_child_id:
            name = editor_tools.get_entity_name(root)
            add_elements(sub_child_id, xml_obj, urdf_mesh_path, robot_name)
            # Search tree for subchildern
            search_tree(sub_child_id, xml_obj, urdf_mesh_path, robot_name)

def add_elements(entity_id: object, xml_obj: object, urdf_mesh_path: Path, robot_name: str):
    """
    This function will add the XML elements from the search loop

    :param entity_id: Entity Object ID
    :param xml_obj: XML Object
    :param urdf_mesh_path: The Path to URDF Meshes Folder
    :param robot_name: Robot Description Name
    """
    joint_type = ""
    lead_name = ""
    lead_entity_object_id = None
    neg_limit_value = 0
    pos_limit_value = 0
    joint_origin_xyz = 0
    joint_origin_rpy = 0
    
    # search through childern of root, find the entity names and ids
    child_name = editor_tools.get_entity_name(entity_id)
    
    # XML- this is the XML SubElement Object
    xml_child_obj = xml_et.SubElement(xml_obj, "link", name=child_name)
    
    # We need to add this for the root base link
    if child_name == "base_link":
        # This function will return Origin Rotation and Transform
        local_translation, local_rotation, local_uniform_scale  = editor_tools.get_transforms_from_id(entity_id)
        xml_et.SubElement(xml_child_obj, "origin", rpy=f"{float(local_rotation.x)} {float(local_rotation.y)} {float(local_rotation.z)}",
                            xyz=f"{float(local_translation.x)} {float(local_translation.y)} {float(local_translation.z)}")
    
    # This function will return the component type names and their ids that a detected
    component_type_name, component_type_id = editor_tools.get_urdf_component_type_id(entity_id)
    
    # If not empty
    if len(component_type_name) > 0:
        joint_origin_xyz, joint_origin_rpy, lead_entity_object_id, joint_type, axis, pos_limit_value, neg_limit_value = look_up_component_type(component_type_name, entity_id, 
                                                                        component_type_id, urdf_mesh_path, xml_child_obj, robot_name)
    else:
        print("No components")
        if not child_name == "base_link":
            # This function will return Origin Rotation and Transform
            local_translation, local_rotation, local_uniform_scale  = editor_tools.get_transforms_from_id(entity_id)
            xml_et.SubElement(xml_child_obj, "origin", rpy=f"{float(local_rotation.x)} {float(local_rotation.y)} {float(local_rotation.z)}",
                                xyz=f"{float(local_translation.x)} {float(local_translation.y)} {float(local_translation.z)}")
            # For Gazebo
            # Create XML for Inertial
            inertial = xml_et.SubElement(xml_child_obj, "inertial")
            xml_et.SubElement(inertial, "mass", value="0.1")
            xml_et.SubElement(inertial, "inertia", ixx="0.0", ixy="0.0",ixz="0.0", iyy="0.0", iyz="0.0", izz="0.0")
           
    # Get the parent ID of this entity ID
    parent_id, parent_name = editor_tools.get_parent(entity_id)
    
    # Get the lead enitity name
    if lead_entity_object_id is None:
        lead_name = editor_tools.get_entity_name(lead_entity_object_id)
    else:
        lead_name = parent_name
    
    # write joint xml child
    if not child_name == "base_link": # Untill figure out
        if joint_type == "":
            local_translation, local_rotation, local_uniform_scale  = editor_tools.get_transforms_from_id(entity_id)
            joint_link_obj = xml_et.SubElement(xml_obj, "joint", name=f"{child_name}_to_{parent_name}", type="fixed")
            xml_et.SubElement(joint_link_obj, "origin", xyz=f"{float(local_translation.x)} {float(local_translation.y)} {float(local_translation.z)}", rpy="0 0 0")
            xml_et.SubElement(joint_link_obj, "parent", link=f"{parent_name}")
            xml_et.SubElement(joint_link_obj, "child", link=f"{child_name}")
            xml_et.SubElement(joint_link_obj, "axis", xyz="0.0 0.0 1.0") # Defaults for now WIP
        else:
            try:
                axis = py_utilities.euler_yzx_to_axis_angle(joint_origin_rpy.x, joint_origin_rpy.y, joint_origin_rpy.z)
            except AttributeError:
                axis = py_utilities.euler_yzx_to_axis_angle(0,0,0)

            joint_link_obj = xml_et.SubElement(xml_obj, "joint", name=f"{child_name}_to_{lead_name}", type=f"{joint_type}")
            try:
                xml_et.SubElement(joint_link_obj, "origin", xyz=f"{float(joint_origin_xyz.x)} {float(joint_origin_xyz.y)} {float(joint_origin_xyz.z)}", rpy="0 0 0")
            except AttributeError:
                xml_et.SubElement(joint_link_obj, "origin", xyz="0 0 0", rpy="0 0 0")

            xml_et.SubElement(joint_link_obj, "parent", link=f"{lead_name}")
            xml_et.SubElement(joint_link_obj, "child", link=f"{child_name}")
            
            # This will be default for now untill we can get an XYZ Euler Rotation conversion to a Axis Rotation Angle
            xml_et.SubElement(joint_link_obj, "axis", xyz=f"{float(axis[0])} {float(axis[1])} {float(axis[2])}")
            
            # Chech for joint types
            if joint_type == "revolute":
                xml_et.SubElement(joint_link_obj, "dynamics", damping="2.0", friction="1.0") # Defaults for now WIP
                xml_et.SubElement(joint_link_obj, "limit", effort="1000.0", lower=f"{neg_limit_value}", upper=f"{pos_limit_value}", velocity="1.0")
            if joint_type == "continuous":
                xml_et.SubElement(joint_link_obj, "dynamics", damping="2.0", friction="1.0") # Defaults for now WIP
                xml_et.SubElement(joint_link_obj, "limit", effort="1000.0", lower=f"{neg_limit_value}", upper=f"{pos_limit_value}", velocity="1.0")

def look_up_component_type(component_type_name: str, root: object, component_type_id: object,
                           urdf_mesh_path: Path, xml_obj: object,
                           robot_name: str) -> tuple [object, object, str, str, object]:
    """
    This function will look at the component type name and if found run the components function to look up
    property paths of the component and build the xml element with properties. 

    :param component_type_name (String): This is a string describing the component type
    :param root (Object): Entity Object ID
    :param component_type_id: Component Type on Entity Object ID
    :param xml_obj): XML Child Object

    :return: origin_xyz: The joint is Local Position Origin,
                origin_rpy: The joint is Local Rotation Origin,
                lead_entity_value: The Lead Entity, most cases the Parent,
                joint_type: Type of joint name,
                axis: This is the axis of rotation for revolute joints,
                        the axis of translation for prismatic joints,
                        surface normal for planar joints. Fixed and floating
                        joints do not use the axis field. 
    """
    joint_type = ""
    origin_xyz = ""
    origin_rpy = ""
    lead_entity_object_id = None
    axis = ""
    pos_limit_value = ""
    neg_limit_value = ""

    if 'mesh' in component_type_name:
        index = component_type_name.index('mesh')
        if 'material' in component_type_name:
            mat_index = component_type_name.index('mesh')
            component_is_mesh(root, component_type_id[index], component_type_id[mat_index], urdf_mesh_path, xml_obj, robot_name)
        else:
            # Add material if its not added already.
            editor_tools.add_component(root, 'Material')
            mat_index = component_type_name.index('mesh')
            component_is_mesh(root, component_type_id[index], component_type_id[mat_index], urdf_mesh_path, xml_obj, robot_name)
            
    # Check to see if there is a PhysX Collider
    if 'collision' in component_type_name:
        index = component_type_name.index('collision')
        component_is_collision(root, component_type_id[index], xml_obj)
    
    # Check to see if there is a PhysX Collider, this is for Inertia
    if 'rigid_body' in component_type_name:
        index = component_type_name.index('rigid_body')
        component_is_rigid_body(root, component_type_id[index], xml_obj)
    
    # Check to see if there is a PhysX Joints
    if 'continuous' in component_type_name:
        index = component_type_name.index('continuous')
        origin_xyz, origin_rpy, lead_entity_object_id, axis, pos_limit_value, neg_limit_value = get_joint_details(root, component_type_id[index], 'continuous')
        joint_type = 'continuous'
    if 'revolute' in component_type_name:
        index = component_type_name.index('revolute')
        origin_xyz, origin_rpy, lead_entity_object_id, axis, pos_limit_value, neg_limit_value = get_joint_details(root, component_type_id[index], 'revolute')
        joint_type = 'revolute'
    if 'fixed' in component_type_name:
        index = component_type_name.index('fixed')
        origin_xyz, origin_rpy, lead_entity_object_id, axis, pos_limit_value, neg_limit_value = get_joint_details(root, component_type_id[index], 'fixed')
        joint_type = 'fixed'
    if 'prismatic' in component_type_name:
        index = component_type_name.index('prismatic')
        origin_xyz, origin_rpy, lead_entity_object_id, axis, pos_limit_value, neg_limit_value = get_joint_details(root, component_type_id[index], 'prismatic')
        joint_type = 'prismatic'
    return origin_xyz, origin_rpy, lead_entity_object_id, joint_type, axis, pos_limit_value, neg_limit_value

def component_is_mesh(child_id: object, component_type_id: object,
                      material_type_id: object, urdf_mesh_path: Path,
                      xml_obj: Path, robot_name: str):
    """
    This function will look at your entity child_id and the attached component id, find the property path
    and get the file path value and add this value to the package xml link, copy the mesh files to the package

    :param child_id: Entity Object ID of Child of Parent Entity
    :param component_type_id: Mesh Component on Entity Object ID
    :param material_type_id: Mesh Material Component on Entity Object ID
    :param xml_obj: XML Child Object
    """
    mesh_file_path = ""
    # returns a list of mesh component IDs for a component type
    mesh_multiple_component_outcome = editor.EditorComponentAPIBus(bus.Broadcast, 'GetComponentsOfType', child_id, component_type_id)
    
    # Check if this is a success
    if (mesh_multiple_component_outcome.IsSuccess()):
        first_mesh_component_id = mesh_multiple_component_outcome.GetValue()[0]
    else:
        first_mesh_component_id = None

    if not first_mesh_component_id is None:
        # Property path for Controller|Configuration|Mesh Asset
        value_outcome = editor.EditorComponentAPIBus(bus.Broadcast, 'GetComponentProperty', first_mesh_component_id,
                                                        'Controller|Configuration|Mesh Asset')
        if not material_type_id == 'None':
            # returns a list of material component IDs for a component type
            material_multiple_component_outcome = editor.EditorComponentAPIBus(bus.Broadcast, 'GetComponentsOfType', child_id, material_type_id)
        
        # Get the Value from the valueOutcome
        value = value_outcome.GetValue()
        
        # Get the asset path from the value
        asset_path = asset.AssetCatalogRequestBus(bus.Broadcast, 'GetAssetPathById', value)
        
        # Lets replace the .azmodel with the .dae
        file_path = asset_path.replace(".azmodel", ".dae")
        
        # LINUX ONLY Lets replace the lower case assets with Assets .azmodel with the .dae
        if "assets" in file_path:
            mesh_file_path = file_path.replace("assets", "Assets")
        if "prefabs" in file_path:
            mesh_file_path = file_path.replace("prefabs", "Prefabs")
        
        # Copy mesh to urdf mesh directory
        source = Path(projectPath.projectroot, mesh_file_path)
        
        # Get the file name
        file_name = Path(mesh_file_path).name
        destination = Path(urdf_mesh_path, file_name)
        
        # This function will copy the files from o3de to the used selected path destination
        py_utilities.copy_files_assets(source, destination)

        # Create XML URDF Mesh Path, We man need to have a full path for use back into o3de and gazebo
        portable = False
        if portable:
            to_directory_path = f'package://{robot_name}/{const.URDF_DESCRIPTION_SUB_FOLDER_LIST[1]}/{file_name}'
        else:
            full_path = str(Path(urdf_mesh_path, file_name))
            to_directory_path = f'file://{full_path}'
        
        # This function will return Origin Rotation and Transform
        local_translation, local_rotation, local_uniform_scale  = editor_tools.get_transforms_from_id(child_id)
        
        # Create XML for Visual and Geometry
        visual = xml_et.SubElement(xml_obj, "visual")
        geometry = xml_et.SubElement(visual, "geometry")
        
        # Create XML for Material if we find material component
        if not material_type_id == 'None':
            xml_et.SubElement(visual, "material", name="white")
        
        # Create XML for Mesh and Origin
        xml_et.SubElement(geometry, "mesh", filename=to_directory_path,
                            scale=f"{float(local_uniform_scale)} {float(local_uniform_scale)} {float(local_uniform_scale)}")
        xml_et.SubElement(visual, "origin", rpy=f"{float(local_rotation.x)} {float(local_rotation.y)} {float(local_rotation.z)}",
                            xyz=f"{float(local_translation.x)} {float(local_translation.y)} {float(local_translation.z)}")

def get_joint_details(entity_id: object, component_type_id: object, joint_type: str) -> tuple[list[float,float,float],
                        list[float,float,float], object, list[float,float,float], float, float]:
    """
    This function will look at your entity_id and the PhysX Ball Joint component id, find the
    property paths of the component and build the xml element with properties.

    :param entity_id: Entity Object ID of Child of Parent Entity
    :param component_type_id: Mesh Component on Entity Object ID
    :param xml_obj: XML Child Object
    :param joint_type: What type of joint it is

    :return:
        local_position_value: The joint is Local Position Origin,
        local_rotation_value: The joint is Local Rotation Origin,
        lead_entity_value: The Lead Entity, most cases the Parent,
        axis: This is the axis of rotation for revolute joints,
                        the axis of translation for prismatic joints,
                        surface normal for planar joints. Fixed and floating
                        joints do not use the axis field.
    """
    # returns a list of joints, we will call inertial component IDs for a component type
    joint_multiple_component_outcome = editor.EditorComponentAPIBus(bus.Broadcast, 'GetComponentsOfType', entity_id, component_type_id)
    
    # If we find the Joint Component
    if (joint_multiple_component_outcome.IsSuccess()):
        pos_limit_value = 0.0
        neg_limit_value = 0.0
        axis = 0.0
        local_position_value = 0.0
        local_rotation_value = 0.0
        lead_object_id = ''

        first_joint_component_id = joint_multiple_component_outcome.GetValue()[0]
        
        # Print the property list - This is for working dev of pluing to print out component get_property
        editor_tools.get_property_list(first_joint_component_id)
        
        # origin
        local_position_value = editor_tools.get_property(first_joint_component_id, 'Standard Joint Parameters|Local Position')
        local_rotation_value = editor_tools.get_property(first_joint_component_id, 'Standard Joint Parameters|Local Rotation')
        
        # get the Lead Entity
        lead_value = editor_tools.get_property(first_joint_component_id, 'Standard Joint Parameters|Lead Entity')
        if not lead_value is None:
            lead_object_id = editor_tools.get_value_from_property_path(first_joint_component_id, 'Standard Joint Parameters|Lead Entity')
        else:
            lead_object_id = None
        
        # Joint Specific
        if joint_type == "revolute":
            pos_limit_value = editor_tools.get_property(first_joint_component_id, 'Angular Limit|Positive angular limit')
            neg_limit_value = editor_tools.get_property(first_joint_component_id, 'Angular Limit|Negative angular limit')
        if joint_type == "continuous":
            pos_limit_value = editor_tools.get_property(first_joint_component_id, 'Swing Limit|Y axis angular limit')
            neg_limit_value = editor_tools.get_property(first_joint_component_id, 'Swing Limit|Z axis angular limit')

        return local_position_value, local_rotation_value, lead_object_id, axis, pos_limit_value, neg_limit_value

def component_is_rigid_body(entity_id: object, component_type_id: object, xml_obj: object):
    """
    This function will look at your entity_id and the PhysX Rigid Body component id, find the
    property paths of the component and build the xml element with properties.

    :param entity_id: Entity Object ID of Child of Parent Entity
    :param component_type_id: Mesh Component on Entity Object ID
    :param xml_obj: XML Child Object
    """
    # returns a list of rigid_body, we will call inertial component IDs for a component type
    inertial_multiple_component_outcome = editor.EditorComponentAPIBus(bus.Broadcast, 'GetComponentsOfType', entity_id, component_type_id)
    
    # If we find the component
    if (inertial_multiple_component_outcome.IsSuccess()):
        first_inertial_component_id = inertial_multiple_component_outcome.GetValue()[0]
        
        # Print the property list
        editor_tools.get_property_list(first_inertial_component_id)
        
        # Get the Mass property
        mass_value = editor_tools.get_property(first_inertial_component_id, 'Configuration|Mass')
        
        # Get the Inertia property
        inertia_linear_value = editor_tools.get_property(first_inertial_component_id, 'Configuration|Initial linear velocity')
        inertia_angular_value = editor_tools.get_property(first_inertial_component_id, 'Configuration|Initial angular velocity')
        
        # Create XML for Inertial
        inertial = xml_et.SubElement(xml_obj, "inertial")
        
        # Create XML for Mass and Inertia
        try:
            xml_et.SubElement(inertial, "mass", value=f"{float(mass_value)}")
        except TypeError:
            xml_et.SubElement(inertial, "mass", value="1.0")

        try:
            xml_et.SubElement(inertial, "inertia", ixx=f"{float(inertia_linear_value.x)}", ixy=f"{float(inertia_linear_value.y)}",
                            ixz=f"{float(inertia_linear_value.z)}", iyy=f"{float(inertia_angular_value.x)}",
                            iyz=f"{float(inertia_angular_value.y)}", izz=f"{float(inertia_angular_value.z)}")
        except AttributeError:
            xml_et.SubElement(inertial, "inertia", ixx="0.0", ixy="0.0",ixz="0.0", iyy="0.0", iyz="0.0", izz="0.0")

def component_is_collision(entity_id: object, component_type_id: object, xml_obj: object):
    """
    This function will look at your entity_id and the PhysX Collider component id, find the
    property paths of the component and build the xml element with properties./

    Note: 
    /// Used to identify shape configuration type from base class.
    enum class ShapeType : AZ::u8
    {
        Sphere,
        Box,
        Capsule,
        Cylinder,
        ConvexHull, ///< Not Supported in physx
        TriangleMesh, ///< Not Supported in physx
        Native, ///< Native shape configuration if user wishes to bypass generic shape configurations.
        PhysicsAsset, ///< Shapes configured in the asset.
        CookedMesh, ///< Stores a blob of mesh data cooked for the specific engine.
        Heightfield ///< Interacts with the physics system heightfield
    };
    Sphere is 0, Box is 1, Capsule is 2, Cylinder is 3, PhysicAsset is None

    URDF_COLLISION_SHAPE_LIST = ['sphere', 'box', 'capsule', 'cylinder']
    /// capsule is not supported in URDF but is in PhysX

    :param entity_id: Entity Object ID of Child of Parent Entity
    :param component_type_id: Mesh Component on Entity Object ID
    :param material_type_id: Mesh Material Component on Entity Object ID
    :param xml_child_obj: XML Child Object
    :param robot_name: Robot Description Name
    """
    # returns a list of collision component IDs for a component type
    collision_multiple_component_outcome = editor.EditorComponentAPIBus(bus.Broadcast, 'GetComponentsOfType', entity_id, component_type_id)
    
    if (collision_multiple_component_outcome.IsSuccess()):
        first_collision_component_id = collision_multiple_component_outcome.GetValue()[0]
        
        # Print the property list
        editor_tools.get_property_list(first_collision_component_id)
        
        # Get Shape Configuration|Shape LIST OF VALUE CODE: Sphere is 0, Box is 1, Capsule is 2, Cylinder is 3, PhysicAsset is None
        shape_value = editor_tools.get_property(first_collision_component_id, 'Shape Configuration|Shape')
        
        # Check to see if we see a shape, if None its a Physic Mesh
        if shape_value is None: 
            print('---------> This is a PhysX Mesh, this is currently not supported. Work in progress.')
            physx_mesh_property_path = 'Shape Configuration|Asset|PhysX Mesh'
        elif shape_value == '':
            print('Key Error, something is wrong, no Key found for shape.')
        elif shape_value in range(4):
            shape_name = const.URDF_COLLISION_SHAPE_PATH[shape_value]['shape']
            
            # Create XML for Collision and Geometry
            collision = xml_et.SubElement(xml_obj, "collision")
            
            # Get Shape Origin and Radius from Property Path
            offset_origin = editor_tools.get_property(first_collision_component_id, 'Collider Configuration|Offset')
            rotation_origin = editor_tools.get_property(first_collision_component_id, 'Collider Configuration|Rotation')
            xml_et.SubElement(collision, "origin", rpy=f"{float(rotation_origin.x)} {float(rotation_origin.y)} {float(rotation_origin.z)}",
                                xyz=f"{float(offset_origin.x)} {float(offset_origin.y)} {float(offset_origin.z)}")
            geometry = xml_et.SubElement(collision, "geometry")
            if shape_name == 'sphere':
                shape_radius_path = const.URDF_COLLISION_SHAPE_PATH[shape_value]['radius']
                
                # Get Shape Radius from Property Path
                shape_radius = editor_tools.get_property(first_collision_component_id, shape_radius_path)
                
                # Create XML for Collision Shape and Size
                xml_et.SubElement(geometry, shape_name, radius=f"{float(shape_radius)}")
            elif shape_name == 'box':
                shape_size_path = const.URDF_COLLISION_SHAPE_PATH[shape_value]['size']
                
                # Get Shape Size from Property Path
                shape_size = editor_tools.get_property(first_collision_component_id, shape_size_path)
                xml_et.SubElement(geometry, shape_name, size=f"{float(shape_size.x)} {float(shape_size.y)} {float(shape_size.z)}")
            elif shape_name == 'cylinder':
                shape_radius_path = const.URDF_COLLISION_SHAPE_PATH[shape_value]['radius']
                shape_length_path = const.URDF_COLLISION_SHAPE_PATH[shape_value]['length']
                
                # Get Shape Radius and Length from Property Paths
                shape_radius = editor_tools.get_property(first_collision_component_id, shape_radius_path)
                shape_length = editor_tools.get_property(first_collision_component_id, shape_length_path)
                xml_et.SubElement(geometry, shape_name, radius=f"{float(shape_radius)}", length=f"{float(shape_length)}")

def search_for_base_link(robot_name: str, ros2_path: Path, ros2_ws_src_path: Path) -> tuple[bool, str, str]:
    """
    This function will search for base_link entity in the current scene.

    :param robot_name: Robot Description Name
    :param ros2_path: Path to main Ros root folder
    :param ros2_ws_src_path: Path to Users Ros Workspace
        
    :return: tuple of the [Found 1 base_link True or False,
            Message to QT UI if the search found a base_link, Launch Command for rvis subproccess]
    """
    search_filter = entity.SearchFilter()
    search_filter.names = [const.ROBOT_BASE_LINK_NODE] # List of names (matches if any match); can contain wildcards in the name
    search_filter.names_case_sensitive = True # Determines if the name matching should be case-sensitive
    search_filter.components_match_all = False # Determines if the filter should match all component type IDs
    search_filter.names_are_root_based = False # Determines if the names are relative to the root or should be searched in children too

    # The SearchBus interface
    bus_type = bus.Broadcast

    # Iterates through all entities in the current level, and returns a list of the ones that match the conditions
    entity_id_list = entity.SearchBus(bus_type, 'SearchEntities', search_filter)

    if len(entity_id_list) > 1: # make sure there is just 1 base_link in the scene
        message = "Only 1 URDF base_link at a time."
        return False, message
    elif len(entity_id_list) < 1:
        message = "No base_link at a time."
        return False, message
    else:
        message = "base_link found!"

        # Start XML root and name robot from arg.
        if not robot_name is None:
            root_xml = xml_et.Element("robot", name=robot_name)
        else:
            root_xml = xml_et.Element("robot", name="Test_Robot")
        
        # Add a defualt material White
        defualt_material = xml_et.SubElement(root_xml, "material", name="white")
        xml_et.SubElement(defualt_material, "color", rgba="1 1 1 1")
        
        # Create the URDF description folder
        robot_description_folder_path = Path(ros2_ws_src_path, robot_name)
        py_utilities.create_urdf_description_folder(robot_description_folder_path)
        
        # This is the new URDF directory
        urdf_directory_path = Path(robot_description_folder_path, const.URDF_DESCRIPTION_SUB_FOLDER_LIST[0])
        
        # This is the new MESH directory
        urdf_mesh_path = Path(robot_description_folder_path, const.URDF_DESCRIPTION_SUB_FOLDER_LIST[1])
        
        # This is the new LAUNCH directory path
        urdf_launch_path = Path(robot_description_folder_path, const.URDF_DESCRIPTION_SUB_FOLDER_LIST[2])
        
        # This is the new rviz directory path
        urdf_rviz_path = Path(robot_description_folder_path, const.URDF_DESCRIPTION_SUB_FOLDER_LIST[3])

        # This is the new gazebo world directory path
        gazebo_world_path = Path(robot_description_folder_path, const.URDF_DESCRIPTION_SUB_FOLDER_LIST[5])
        
        # Create a Package XML
        py_utilities.create_package_file(robot_name, robot_description_folder_path)
        
        # Create the display.launch xml file
        py_utilities.create_launch_file(robot_name, urdf_launch_path)
        
        # Create the CMake List
        py_utilities.create_cmake_file(robot_name, robot_description_folder_path)
        
        # Create the rviz file
        py_utilities.create_rviz_file(robot_name, urdf_rviz_path)

        # Create the gazebo world file
        py_utilities.create_empty_world_file(gazebo_world_path)

        # Create the gazebo launch_file
        py_utilities.create_gazebo_launch_file(robot_name, urdf_launch_path)
        
        # START SEARCH TREE ------------------
        root = entity_id_list[0]
        search_tree(root, root_xml, urdf_mesh_path, robot_name)

        # Add Gazebo Tags
        gazebo_obj = xml_et.SubElement(root_xml, 'gazebo')
        xml_et.SubElement(gazebo_obj, "plugin", name="gazebo_ros_control", filename="libgazebo_ros_control.so")
        xml_et.SubElement(gazebo_obj, 'robotNamespace').text = '/'
        
        # Export the URDF XML in the robot description folder
        py_utilities.save_urdf_xml(robot_name, 'urdf', root_xml, urdf_directory_path)
        
        # Send CLI Commands
        ros_source = str(Path(ros2_path, 'setup.bash'))
        ws_source = str(Path(ros2_ws_src_path, 'install', 'setup.bash'))
        
        # Send Build Command
        send_ros_build_commands(ros_source, ros2_ws_src_path, ws_source)

        # Send Gazebo SDF Convert Commands (Gazebo must be installed)
        send_gazebo_sdf_build_commands(urdf_directory_path, robot_name)
        
        # Build Launch Command for rvis, this will be used as a subprocess command if user click Open in RVIS button on gui
        rviz_launch_message = f'source {ros_source}; cd {ros2_ws_src_path}; source {ws_source}; ros2 launch {robot_name} display.launch.py'

        # Build Launch Command for gazebo, this will be used as a subprocess command if user click Open in GAZEBO button on gui
        gazebo_launch_message = f'source {ros_source}; cd {ros2_ws_src_path}; source {ws_source}; ros2 launch {robot_name} gazebo.launch.py'
        
    return True, message, rviz_launch_message, gazebo_launch_message

def send_ros_build_commands(ros_source: Path, ros2_ws_src_path: Path, ws_source: Path):
    """
    This function will send ros commands to build the urdf package 

    :param ros_source: Path to main Ros root folder
    :param ros2_ws_src_path: Path to Users Ros Workspace
    :param ws_source: Path to main Ros root folder
    """
    # Linux Bash Commands
    linux_terminal = 'gnome-terminal -e '
    bash = ' "bash -c '
    bash_end = ' "'
    
    # Build Command
    build_cmd_list = f'source {ros_source}; cd {ros2_ws_src_path}; colcon build --merge-install; source {ws_source}'
    
    # Build Subprocess
    build = subprocess.Popen(f"{linux_terminal}{bash}'{build_cmd_list}'{bash_end}", stdout=subprocess.PIPE, shell=True)
    output = build.communicate()[0]
    print(f'Build: {output}')

def send_gazebo_sdf_build_commands(urdf_directory_path: Path, robot_name: str):
    """
    This function will send ros commands to build the urdf package 

    :param ros_source: Path to main Ros root folder
    :param ros2_ws_src_path: Path to Users Ros Workspace
    :param ws_source: Path to main Ros root folder
    """
    # Linux Bash Commands
    linux_terminal = 'gnome-terminal -e '
    bash = ' "bash -c '
    bash_end = ' "'
    
    # Build URDF and SDF Paths
    urdf = Path(urdf_directory_path, f"{robot_name}.urdf")
    sdf = Path(urdf_directory_path, f"{robot_name}.sdf")
    
    # Build Command  gz sdf -p /my_urdf.urdf > /my_sdf.sdf
    build_cmd_list = f"gz sdf -p {urdf} > {sdf}"
    
    # Build Subprocess
    build = subprocess.Popen(f"{linux_terminal}{bash}'{build_cmd_list}'{bash_end}", stdout=subprocess.PIPE, shell=True)
    output = build.communicate()[0]
    print(f'SDF Build: {output}')

def launch_gazebo(message: str):
    """
    This function will launch the ROS2 Gazebo application with robot launch description

    :param message: Launch Command for gazebo subprocess.
    """
    # we are going to pass the system environ
    gazebo_env = os.environ.copy()
    
    # prunes non-string key:value envars
    gazebo_env = {key: value for key, value in gazebo_env.items() if py_utilities.check_is_ascii(key) and py_utilities.check_is_ascii(value)}
    
    # will prune QT_ envars, to be used with QT bases apps like Maya or Wing
    gazebo_env = {key: value for key, value in gazebo_env.items() if not key.startswith("QT_")}
    
    # if o3de env passes PYTHONHOME it will cause systemic boot failure for maya
    gazebo_env = {key: value for key, value in gazebo_env.items() if not 'PYTHONHOM' in key}
    
    # Linux Bash Command Vars
    linux_terminal = 'gnome-terminal -e '
    bash = ' "bash -c '
    bash_end = ' "'
    
    # Build env var
    env: dict = gazebo_env
    
    # Build final command
    command = f"{linux_terminal}{bash}'{message}'{bash_end}"
    
    # Send command to subprocess
    gazebo = subprocess.Popen(args = command,
                                env = env,
                                shell=True,
                                stdout = subprocess.PIPE,
                                stderr = subprocess.PIPE,
                                close_fds=True)
    output = gazebo.communicate()[0]
    print(f'gazebo: {output}')

def launch_rviz(message: str):
    """
    This function will launch the ROS2 rVIS application with robot launch description

    :param message: Launch Command for rvis subprocess.
    """
    # we are going to pass the system environ
    rviz_env = os.environ.copy()
    
    # prunes non-string key:value envars
    rviz_env = {key: value for key, value in rviz_env.items() if py_utilities.check_is_ascii(key) and py_utilities.check_is_ascii(value)}
    
    # will prune QT_ envars, to be used with QT bases apps like Maya or Wing
    rviz_env = {key: value for key, value in rviz_env.items() if not key.startswith("QT_")}
    
    # if o3de env passes PYTHONHOME it will cause systemic boot failure for maya
    rviz_env = {key: value for key, value in rviz_env.items() if not 'PYTHONHOM' in key}
    
    # Linux Bash Command Vars
    linux_terminal = 'gnome-terminal -e '
    bash = ' "bash -c '
    bash_end = ' "'
    
    # Build env var
    env: dict = rviz_env
    
    # Build final command
    command = f"{linux_terminal}{bash}'{message}'{bash_end}"
    
    # Send command to subprocess
    rvis = subprocess.Popen(args = command,
                                env = env,
                                shell=True,
                                stdout = subprocess.PIPE,
                                stderr = subprocess.PIPE,
                                close_fds=True)
    output = rvis.communicate()[0]
    print(f'rvis: {output}')

