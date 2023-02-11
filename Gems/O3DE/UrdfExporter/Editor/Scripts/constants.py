# coding:utf-8
#!/usr/bin/python
#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#
# -------------------------------------------------------------------------
from pathlib import Path
# -------------------------------------------------------------------------

USER_HOME = Path.home()
WIKI_URL = 'README.md'
PLUGIN_VERSION = '1.0'

ROBOT_BASE_LINK_NODE = 'base_link'
URDF_DESCRIPTION_SUB_FOLDER_LIST = ['urdf', 'meshes', 'launch', 'rviz', 'images', 'worlds']
URDF_COLLISION_SHAPE_LIST = ['sphere', 'box', 'capsule', 'cylinder'] 
URDF_RVIS_DEFUALT = 'rviz/urdf.rviz'
# capsule is not supported in URDF but is in PhysX
URDF_COLLISION_SHAPE_PATH = {0: {'shape': 'sphere', 'radius': 'Shape Configuration|Sphere|Radius'},
                            1: {'shape': 'box', 'size': 'Shape Configuration|Box|Dimensions'},
                            2: {'shape': 'capsule', 'radius': 'Shape Configuration|Capsule|Radius', 'length': 'Shape Configuration|Capsule|Height'},
                            3: {'shape': 'cylinder', 'radius': 'Shape Configuration|Cylinder|Radius', 'length': 'Shape Configuration|Cylinder|Height'}
                            }

