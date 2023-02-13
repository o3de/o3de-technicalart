
from pathlib import Path
import shutil
import xml.etree.cElementTree as xml_et
import constants as const
import math
import numpy as np


def euler_yzx_to_axis_angle(roll: float, pitch: float, yaw: float) -> list[float,float,float]:
    """
    This function will convert a xyz Euler to an xyz Axis.
    
    :param roll: The roll or x
    :param pitch: The pitch or z
    :param yaw: The yaw or y:
    :return: normlized float3 axis xyz
    """

    # Convert the Euler angles to a 3x3 rotation matrix
    yawMatrix = np.matrix([
    [math.cos(yaw), -math.sin(yaw), 0],
    [math.sin(yaw), math.cos(yaw), 0],
    [0, 0, 1]
    ])

    pitchMatrix = np.matrix([
    [math.cos(pitch), 0, math.sin(pitch)],
    [0, 1, 0],
    [-math.sin(pitch), 0, math.cos(pitch)]
    ])

    rollMatrix = np.matrix([
    [1, 0, 0],
    [0, math.cos(roll), -math.sin(roll)],
    [0, math.sin(roll), math.cos(roll)]
    ])

    # Multiply the three matrices to obtian the total rotation matrix
    R = yawMatrix * pitchMatrix * rollMatrix
    # Extract the rotation axis and angle from rotation matrix
    theta = math.acos(((R[0, 0] + R[1, 1] + R[2, 2]) - 1) / 2)
    try:
        multi = 1 / (2 * math.sin(theta))
    except ZeroDivisionError:
        multi = 0.0

    #multiply Value
    rx = multi * (R[2, 1] - R[1, 2]) * theta
    ry = multi * (R[0, 2] - R[2, 0]) * theta
    rz = multi * (R[1, 0] - R[0, 1]) * theta
    # This is our raw axis
    raw_value = [rx, rz, ry] #swap the z and y
    # Normalized Value
    if not raw_value == [0,0,0]:
        normalized = [(float(i)-min(raw_value))/(max(raw_value)-min(raw_value)) for i in raw_value]
    else:
        normalized = [0.0,0.0,1.0]
    # Lets return the normalized
    return normalized

def create_directory_folder(directory: Path):
    """
    This function will create a directory
    
    :param directory : Directory Path
    """
    Path(directory).mkdir(parents=True, exist_ok=True)

def copy_files_assets(source: Path, destination: Path):
    """
    This function will copy a source file to destination
    
    :param source: Source Directory Path to copy from
    :param destination: Destination Directory Path to copy to
    """
    try:
        shutil.copy(source, destination)
    except (FileNotFoundError, IsADirectoryError) as error:
        print(f'Copy Files Assets Error: {error}')

def check_is_ascii(string: str) -> bool:
    """
    This function will check and validate an string for proper ascii

    :param string: ASCII String to validate
    :return bool:
    """
    try:
        string.encode('ascii')
    except UnicodeEncodeError:
        return False
    else:
        return True

def create_package_file(robot_name: str, package_path: Path):
    """
    Creates a ros robot urdf description xml package file
    
    :param robot_name: Name of the robots package
    :param package_path: This is the path to the Robot Description
    """
    root_xml = xml_et.Element('package', format='3')
    xml_et.SubElement(root_xml, 'name').text = robot_name
    xml_et.SubElement(root_xml, 'version').text ='1.0.0'
    xml_et.SubElement(root_xml, 'description').text = 'URDF from O3DE'
    xml_et.SubElement(root_xml,'author', email='shawstar@amazon.com').text = 'Starr Shaw'
    xml_et.SubElement(root_xml,'maintainer', email='shawstar@amazon.com').text = 'Starr Shaw'
    xml_et.SubElement(root_xml,'license').text = 'MIT'
    xml_et.SubElement(root_xml,'url').text = 'https://www.o3de.org/'
    xml_et.SubElement(root_xml,'buildtool_depend').text = 'ament_cmake'
    xml_et.SubElement(root_xml,'exec_depend').text = 'joint_state_publisher'
    xml_et.SubElement(root_xml,'exec_depend').text = 'joint_state_publisher_gui'
    xml_et.SubElement(root_xml,'exec_depend').text = 'robot_state_publisher'
    xml_et.SubElement(root_xml,'exec_depend').text = 'rviz2'
    xml_et.SubElement(root_xml,'exec_depend').text = 'xacro'
    xml_et.SubElement(root_xml,'test_depend').text = 'ament_lint_auto'
    export = xml_et.SubElement(root_xml, 'export')
    xml_et.SubElement(export,'build_type').text = 'ament_cmake'
    
    # Export the XML
    save_urdf_xml('package', 'xml', root_xml, package_path)

def create_cmake_file(robot_name: str, package_directory_path: Path):
    """
    Create a Cmake file for the Robot Description
    
    :param robot_name: Name of the robots package
    :param package_directory_path: This is the path to the Robot Description
    """
    text = [ 'cmake_minimum_required(VERSION 3.5)',
            f'project({robot_name})', '',
            'find_package(ament_cmake REQUIRED)', '',
            'install(',
            '   DIRECTORY images launch meshes rviz urdf',
            '   DESTINATION share/${PROJECT_NAME}',
            ')', '',
            'if(BUILD_TESTING)',
            '   find_package(ament_lint_auto REQUIRED)',
            '   ament_lint_auto_find_test_dependencies()',
            'endif()', '',
            'ament_package()'
            ]
    # Save File
    file_path = Path(package_directory_path, 'CMakeLists.txt')
    with open (file_path, 'w') as f:
        for item in text:
            f.write(item + '\n')

def create_launch_file(robot_name: str, urdf_launch_path: Path):
    """
    Create a ros launch file for the Robot Description

    :param robot_name: Name of the robots package
    :param urdf_launch_path: This is the path to the Robot Description Launch Folder
    """
    text = [
            'from ament_index_python.packages import get_package_share_path',
            '',
            'from launch import LaunchDescription',
            'from launch.actions import DeclareLaunchArgument',
            'from launch.conditions import IfCondition, UnlessCondition',
            'from launch.substitutions import Command, LaunchConfiguration',
            '',
            'from launch_ros.actions import Node',
            'from launch_ros.parameter_descriptions import ParameterValue',
            '',
            'from pathlib import Path',
            '',
            'def generate_launch_description():',
            f'\tpackage_path = get_package_share_path("{robot_name}")',
            f'\tdefault_model_path = Path(package_path, "urdf/{robot_name}.urdf")',
            f'\tdefault_rviz_config_path = Path(package_path, "{const.URDF_RVIS_DEFUALT}")',
            '',
            '\tgui_arg = DeclareLaunchArgument(name="gui", default_value="true", choices=["true", "false"], description="Flag to enable joint_state_publisher_gui")',
            '\tmodel_arg = DeclareLaunchArgument(name="model", default_value=str(default_model_path), description="Absolute path to robot urdf file")',
            '\trviz_arg = DeclareLaunchArgument(name="rvizconfig", default_value=str(default_rviz_config_path), description="Absolute path to rviz config file")',
            '',
            '\trobot_description = ParameterValue(Command(["xacro ", LaunchConfiguration("model")]), value_type=str)',
            '',
            '\trobot_state_publisher_node = Node(package="robot_state_publisher", executable="robot_state_publisher", parameters=[{"robot_description": robot_description}])',
            '',
            '\tjoint_state_publisher_node = Node(package="joint_state_publisher", executable="joint_state_publisher", condition=UnlessCondition(LaunchConfiguration("gui")))',
            '',
            '\tjoint_state_publisher_gui_node = Node(package="joint_state_publisher_gui", executable="joint_state_publisher_gui", condition=IfCondition(LaunchConfiguration("gui")))',
            '',
            '\trviz_node = Node(package="rviz2", executable="rviz2", name="rviz2", output="screen", arguments=["-d", LaunchConfiguration("rvizconfig")])',
            '',
            '\treturn LaunchDescription([gui_arg, model_arg, rviz_arg, joint_state_publisher_node, joint_state_publisher_gui_node, robot_state_publisher_node, rviz_node])'
    ]
    # Save File
    file_path = Path(urdf_launch_path, 'display.launch.py')
    with open (file_path, 'w') as f:
        for item in text:
            f.write(item + '\n')

def create_gazebo_launch_file(robot_name: str, urdf_launch_path: Path):
    """
    Create a ros Gazebo launch file for the Robot Description

    :param robot_name: Name of the robots package
    :param urdf_launch_path: This is the path to the Robot Description Launch Folder
    """
    slash = '\\\"'
    xml_replace = f"\'\"\', \'{slash}\'"
    text = [
            'from ament_index_python.packages import get_package_share_directory',
            'from launch import LaunchDescription',
            'from launch.actions import ExecuteProcess',
            'from launch.substitutions import LaunchConfiguration',
            'from pathlib import Path',
            '',
            'def generate_launch_description():',
            '\tuse_sim_time = LaunchConfiguration("use_sim_time", default="true")',
            f'\trobot_name = "{robot_name}"',
            '\tworld_file_name = "empty.world"',
            '',
            '\tworld = str(Path(get_package_share_directory(',
            '\t\trobot_name), "worlds", world_file_name))',
            '',
            '\tsdf = str(Path(get_package_share_directory(',
            f'\t\trobot_name), "urdf", "{robot_name}.sdf"))',
            '',
            '\txml = open(sdf, "r").read()',
            '',
            f"\txml = xml.replace({xml_replace})",
            '',
            "\tswpan_args = '{name: \"' + robot_name + '\", xml: \"' + xml + '\" }' ",
            '',
            '\treturn LaunchDescription([',
            '\t\tExecuteProcess(',
            '\t\t\tcmd=["gazebo", "--verbose", world,',
            '\t\t\t\t"-s", "libgazebo_ros_factory.so"],',
            '\t\t\toutput="screen"),',
            '',
            '\t\tExecuteProcess(',
            '\t\t\tcmd=["ros2", "param", "set", "/gazebo",',
            '\t\t\t\t"use_sim_time", use_sim_time],',
            '\t\t\toutput="screen"),',
            '',
            '\t\tExecuteProcess(',
            '\t\t\tcmd=["ros2", "service", "call", "/spawn_entity" ,',
            '\t\t\t\t"gazebo_msgs/SpawnEntity", swpan_args],',
            '\t\t\toutput="screen"),',
            '\t])'
    ]
    # Save File
    file_path = Path(urdf_launch_path, 'gazebo.launch.py')
    with open (file_path, 'w') as f:
        for item in text:
            f.write(item + '\n')

def create_rviz_file(robot_name: str, urdf_rviz_path: Path):
    """
    Create a rviz file

    :param robot_name: Name of the robots package (Not used currently, but this might change)
    :param urdf_rviz_path (Path): This is the path to the Robot Description RVIZ Folder
    """
    text = [ 'Panels:',
            '  - Class: rviz_common/Displays',
            '    Name: Displays',
            '  - Class: rviz_common/Views',
            '    Name: Views',
            'Visualization Manager:',
            '  Class: ""',
            '  Displays:',
            '    - Class: rviz_default_plugins/Grid',
            '      Name: Grid',
            '      Value: true',
            '    - Alpha: 0.8',
            '      Class: rviz_default_plugins/RobotModel',
            '      Description Source: Topic',
            '      Description Topic:',
            '        Value: /robot_description',
            '      Enabled: true',
            '      Name: RobotModel',
            '      Value: true',
            '    - Class: rviz_default_plugins/TF',
            '      Name: TF',
            '      Value: true',
            '  Global Options:',
            '    Fixed Frame: base_link',
            '    Frame Rate: 30',
            '  Name: root',
            '  Tools:',
            '    - Class: rviz_default_plugins/MoveCamera',
            '  Value: true',
            '  Views:',
            '    Current:',
            '      Class: rviz_default_plugins/Orbit',
            '      Distance: 1.7',
            '      Name: Current View',
            '      Pitch: 0.33',
            '      Value: Orbit (rviz)',
            '      Yaw: 5.5',
            'Window Geometry:',
            '  Height: 800',
            '  Width: 1200'
            ]
    # Save File
    file_path = Path(urdf_rviz_path, 'urdf.rviz')
    with open (file_path, 'w', encoding = 'utf-8') as file_rvis:
        for item in text:
            file_rvis.write(item + '\n')      
        file_rvis.close()

def create_empty_world_file(gazebo_world_path: Path):
    """
    Create a Gazebo World File

    :param gazebo_world_path(Path): This is the path to the Gazebo World Folder
    """
    text = [
            '<sdf version="1.7">',
            '  <world name="default">',
            '    <include>',
            '      <uri>model://ground_plane</uri>',
            '    </include>',
            '    <include>',
            '      <uri>model://sun</uri>',
            '    </include>',
            '  </world>',
            '</sdf>' 
    ]
    # Save File
    file_path = Path(gazebo_world_path, 'empty.world')
    with open (file_path, 'w', encoding = 'utf-8') as file_world:
        for item in text:
            file_world.write(item + '\n')      
        file_world.close()

def create_urdf_description_folder(robot_description_folder_path: Path):
    """
    This function will create the URDF description folder with subfolders

    :param robot_description_folder_path: root of the Robot Description
    """
    create_directory_folder(robot_description_folder_path)
    # Create sub directories
    for directory in const.URDF_DESCRIPTION_SUB_FOLDER_LIST:
        sub_directory = Path(robot_description_folder_path, directory)
        create_directory_folder(sub_directory)

def save_urdf_xml(file_name: str, file_ext: str, root_xml: object, xml_directory_path: Path):
    """
    This function will export the URDF xml to the robot description folder

    :param file_name: File Name String
    :param file_ext: File Ext String
    :param root_xml: Root XML Object
    :param xml_directory_path: File path for file directory
    """
    # Build Path
    xml_path = Path(xml_directory_path, f'{file_name}.{file_ext}')
    # XML Tree
    tree = xml_et.ElementTree(root_xml)
    xml_et.indent(tree, space="\t", level=0)
    # Write XML
    tree.write(xml_path, encoding = "UTF-8", xml_declaration = True)

    dump = xml_et.dump(root_xml)
    print(dump)
