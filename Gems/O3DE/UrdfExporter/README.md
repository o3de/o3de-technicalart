# urdf-exporter-gem
O3DE to URDF Exporter Gem
![InO3DE](https://user-images.githubusercontent.com/87207603/206539263-a9d45d0c-9799-4843-b343-83ffa05cb535.png)
# Tested on: O3DE 22.10.0 Release on Linux Ubuntu-22.04, ROS2 Humble Desktop

https://docs.ros.org/en/humble/Installation/Ubuntu-Install-Debians.html

# Requirements: ROS2 Humble, Linux OS
# sudo apt install python3-colcon-common-extensions
# ROS2 Required Packages: 
- gazebo-ros-pkgs
- joint_state_publisher
- joint_state_publisher_gui
- robot_state_publisher
- xacro


# INSTRUCTIONS:
1. Download the URDF Exporter gem.
2. Enable the GEM in a project. Learn more about O3DE Gems: https://www.o3de.org/docs/user-guide/gems/reference/aws/
3. In O3DE Editor open the URDF Exporter Tool Panel
4. Select the ROS2 Distro to Source
5. Select the ROS2 Workspace to Source and save URDF
6. Export URDF
7. (Optional) View URDF in ROS2 rVIS
8. (Optional) View SDF in Gazebo

# INSTALL GEM
1. Download and install the gem in your project gem settings:

Use this Github Repo: https://github.com/shawstar/urdf-exporter-gem

2. Start a new O3DE Project:
![image](https://user-images.githubusercontent.com/87207603/215607459-15a08735-0893-4810-8c8d-d532d7517307.png)

3. Name your new robotics project:
![image](https://user-images.githubusercontent.com/87207603/215607854-d6b26e1a-303c-45d4-93c8-c37af0e561d4.png)

4. Add the urdf-exporter-gem to your project:

On the far right top of the project gem manager window
there is a hamburger menu button, click Add Existing Gem.
Select the downloaded urdf-exporter-gem folder.
![image](https://user-images.githubusercontent.com/87207603/215608167-cea2c072-e8f9-48d2-8580-78da75995e68.png)

5. Turn on and activate the urdf-exporter-gem Gem:
![image](https://user-images.githubusercontent.com/87207603/215609362-a3292199-4a58-4e43-b1f6-dcd5f5fbcbdb.png)

6. You must also have QT for Python Gem activated:
![image](https://user-images.githubusercontent.com/87207603/215609693-95a17906-1d10-411f-8ef2-575569520a4d.png)

7. Create your project. This menu will just remind you of the ROS2 Distro Requirments.
![image](https://user-images.githubusercontent.com/87207603/215610453-7b805088-78f9-4924-8790-45d29eaf5c9d.png)

8. Build your project:
![image](https://user-images.githubusercontent.com/87207603/215610722-5c3b5b0f-dd82-4efd-b170-68e46da56748.png)

9. Create a new level:
![image](https://user-images.githubusercontent.com/87207603/215819105-bbdfca88-2c34-4e05-8dad-ad989f10c3f1.png)


10. O3DE Menu Tools > Examples > urdf_exporter_gem:
Also the o3de Menu Bar will have a new Robot Icon.
![image](https://user-images.githubusercontent.com/87207603/215824488-3cccd8f3-277a-4ddf-9751-65d8bb9835f0.png)

11. Lets make a very simple urdf test:
Create a new Entity and name it base_link. This will be your frist link. Please leave this empty.
![image](https://user-images.githubusercontent.com/87207603/215825173-8e06191b-e326-450e-89ca-627730154465.png)

12. Lets add a simple mesh and add to the project:
Please note, you must have the meshes in your project to be found by exporter, also meshs need to be in lower case.
Add a mew Entity under base_link and add a mesh, lets add the box mesh.
![image](https://user-images.githubusercontent.com/87207603/215832185-9a2eaffc-08cf-4958-b487-d30be79151b6.png)

13. Next lets add some Components:
Lets add a PhysX Collider. Currently only Shapes are supported. In the future mesh PhysicsAssest will be supported.
Lets make a Box Shape around the cube.
![image](https://user-images.githubusercontent.com/87207603/215832906-a4e27055-a238-4618-8a79-61bf1b7da840.png)

14. Next lets add a PhysX Rigid Body
![image](https://user-images.githubusercontent.com/87207603/215834755-98650b6c-4ec3-4015-8cae-a65b68f79479.png)

15. Now lets add a joint target, we will use this as the lead target for our joint on are next Entity
Place the Pivot location where you would like the Pivot axis to be for your joint.
![image](https://user-images.githubusercontent.com/87207603/215835590-9453c589-8f04-40b7-9aa8-6304c1396531.png)

16. Create a new Entity, lets add a Hinge Joint
Add a new mesh component, PhysX Rigid Body, PhysX Hinge Joint, PhysX Collider. Make the joint Lead Entity pointed to the joint_lead_target.
![image](https://user-images.githubusercontent.com/87207603/215837759-0cff6d2c-82f7-401b-9ffc-2652028c29ac.png)

17. Lets use the URDF Exporter Tools:
Set your ROS2 Location, Set your ROS2 Workspace Source Location, this can also point to your projects assets.
![image](https://user-images.githubusercontent.com/87207603/216105902-2b2f94c0-2826-4dab-a16f-c4a6df6e1ccb.png)

18. Click Export Robot Package
![image](https://user-images.githubusercontent.com/87207603/216106266-ae1ab57d-5eba-4f98-9aa7-e7b5caf18c0a.png)
# You will see some shell windows build your URDF Package. On disk it will look like this:
![image](https://user-images.githubusercontent.com/87207603/216107243-611fb0b9-e3bc-45e2-ab7b-a882132f514c.png)
Also these files will autobuild with colcon build be placed in your workspace install folder. 

19. Preview your URDF in ROS2 RViz
Next, lets click the LAUNCH RVIS Button, this will Launch your ros package in Rvis.
![image](https://user-images.githubusercontent.com/87207603/216108467-48e91969-83ab-46fe-a239-ca801e9ce49f.png)

20. Move your URDF in Joint State Publisher:
We made a joint in our robot, you can move it by sliding the joint exposed.
![image](https://user-images.githubusercontent.com/87207603/216108687-c9c5a489-60d7-47dd-817f-13646029b168.png)

21. Preview your SDF in Gazebo.
We automaticly convert your URDF into an SDF for Gazebo, Click LAUNCH GAZEBO to view.
![image](https://user-images.githubusercontent.com/87207603/216109269-720ac941-e5df-479b-a18c-9b87ee57800b.png)

# Robot Arm Sample
## Lets try a more advanced sample that comes with this gem.
## Frist Step, Move sample Prefab into your project.

1. You must copy the contents of the Prefabs folder itself into your Projects Prefabs folder for o3de to find the meshs properly.
![image](https://user-images.githubusercontent.com/87207603/215591984-756f630d-5020-45a2-86ce-06500c1dc381.png)

2. Move to your projects prefabs:
![image](https://user-images.githubusercontent.com/87207603/215591676-b814f395-8822-48e4-a540-debd92947da5.png)
 ![image](https://user-images.githubusercontent.com/87207603/215592150-ede2d8a9-2157-465f-9d97-5cefdef85693.png)

3. Instantiate Prefab of the robotic_arm model.
![image](https://user-images.githubusercontent.com/87207603/216175892-31c662a3-b762-49bc-bc47-10a06b2f9ffa.png)
![image](https://user-images.githubusercontent.com/87207603/216176222-d1b225aa-50d7-4141-90c8-4c3a53513644.png)

4. Set your ROS2 Location and your Export Workspace Location, this can be in O3DE Assets.
![image](https://user-images.githubusercontent.com/87207603/216176680-9921a0ae-5ab7-4ab8-9412-2435202dfc4d.png)

5. Export your Robot URDF Package and Launch RViz to test your robot.
![image](https://user-images.githubusercontent.com/87207603/216177076-559fd0e4-3064-4a06-9be0-d139471700ef.png)


# Known Limitations:
 
1. You must use Collada .DAE for your meshes:
![image (12)](https://user-images.githubusercontent.com/87207603/206542719-ea50d79c-7e32-409f-bd4a-8674f5b90882.png)
 
2. You must have you robot under a Entity called base_link:
![image (13)](https://user-images.githubusercontent.com/87207603/206543064-54ea9fae-cb4f-436b-bebb-4d0900c6c5be.png)
 The base_link Entity needs to have no components
 
3. You must Add Material Component to meshes:
![image](https://user-images.githubusercontent.com/87207603/216165411-9ecb1955-bff3-433a-bcee-ea620f010120.png)

4. Pivots must be added into your mesh:
![image](https://user-images.githubusercontent.com/87207603/216097547-d9bdb95a-4cd6-46f3-90b2-447a215997c6.png)
When exporting your 3D meshes, you must have your mesh and pivot you wish at worldspace (0.0, 0.0, 0.0)
![image](https://user-images.githubusercontent.com/87207603/216104718-dea5609d-1bc3-4f39-94fa-9fb3bde43a96.png)
Your ground mesh must have the Pivot at the bottom, math transforms will calculate from the pivot.

5. You must name your meshes lowercase
![image](https://user-images.githubusercontent.com/87207603/216098989-749e669c-fb77-4b4a-98df-8ec4b7ac40d2.png)

6. PhysX Collider is supported, but Shape PhysicsAsst Meshes are not yet supported.
![image](https://user-images.githubusercontent.com/87207603/216100506-f9e2dd3a-fc04-4cc2-b659-9498ce688652.png)
O3DE PhysX Shape Collider Component not yet supported.

7. O3DE Materials are not yet supported.
O3DE Materials and Textures are not yet fully supported. The Material component is added when it is on an entity, but colors and or textures are not yet added to the URDF.

8. Gazebo importing, you must have your 3D model correctly assembled with the proper physics and colliders. Currently, the sample arm can be seen when time and gravity are paused. The colliders and settings need more work to work correctly in Gaze
![image](https://user-images.githubusercontent.com/87207603/220388049-556dc95d-0e97-4b10-9e31-05c1f7ddc694.png)

9. Missing Joint Types
![image](https://user-images.githubusercontent.com/87207603/220390455-515b2366-476d-493d-93ba-33200bab55db.png)

10. Mesh Paths must be in the Assets folder of the Project.
When building your 3D model in O3DE, make sure your meshes are located in the project assets folder and the meshes are named in lower case. If you would like to import your exported URDF with the O3DE Ros2 Gem, make sure you export your URDF in the project assets folder to import the mesh paths.


# Backlog:

## euler_yzx_to_axis_angle in utilities:
https://github.com/o3de/o3de-technicalart/pull/5#discussion_r1107913594
Do we have any math utils exposed to do this? If we can offload this to o3de optimized C++ libraries, we should.

## create_directory_folder in utilities:
https://github.com/o3de/o3de-technicalart/pull/5#discussion_r1107919397
You should have access to the LYTestTools in any O3DE project IIRC. If you do, I'd use the LyTestTools file_system

## create_gazebo_launch_file in utilities:
https://github.com/o3de/o3de-technicalart/pull/5#discussion_r1107933647
Have you considered using Jinja2 for all these template files?
The o3de install Python has Jinja2.11.3 installed in its site-packages.
https://jinja.palletsprojects.com/en/3.1.x/

## create_empty_world_file in utilities:
https://github.com/o3de/o3de-technicalart/pull/5#discussion_r1107953028
Since this is a file that doesn't use any updated strings, we could store a physical template file in the gem then just copy that file to the target destination.

## Moving editor_entity_utils to the EditorPythonBindings 
cgalvan mentioned this pull request Feb 16, 2023
Proposed RFC Suggestion: Moving editor_entity_utils to the EditorPythonBindings gem o3de/sig-testing#67 

## Linear Velocity to look at in export.py:
https://github.com/o3de/o3de-technicalart/pull/5#discussion_r1107012545

Linear velocity is not a row of inertia matrix.
The inertia matrix is 9 dimensions equivalent to mass for rotation motion (huge simplification).
In o3de only the diagonal inertia matrix is supported (AFAIK).
Please adjust the code here.

## PhysX in O3DE vs URDF
https://github.com/o3de/o3de-technicalart/pull/5#discussion_r1107011206

Note that PhysX and o3de has completely different direction of joints:
Please refer to my comment in URDF importer on that : https://github.com/o3de/o3de-extras/blob/8069a342837b8bb696d9089b67d3dd5349fe231d/Gems/ROS2/Code/Source/RobotImporter/URDF/JointsMaker.cpp#L28

## Rotation Matrix and how its constructed in o3de:
https://github.com/o3de/o3de-technicalart/pull/5#discussion_r1107011024
