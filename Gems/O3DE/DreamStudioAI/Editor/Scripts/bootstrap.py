"""
Copyright (c) Contributors to the Open 3D Engine Project.
For complete copyright and license terms please see the LICENSE at the root of this distribution.

SPDX-License-Identifier: Apache-2.0 OR MIT
"""
# -------------------------------------------------------------------------
"""\DreamStudioAI\\editor\\scripts\\bootstrap.py
Generated from O3DE PythonToolGem Template"""

import sys
from pathlib import Path
import logging as _logging

# let our parent pkg into the house with us
path_root = Path(__file__).parents[3]
sys.path.append(str(path_root))

from DreamStudioAI.Editor.Scripts import _PACKAGENAME
_PACKAGENAME = f'{_PACKAGENAME}.bootstrap'
_LOGGER = _logging.getLogger(_PACKAGENAME)
_LOGGER.debug('Initializing: {0}.'.format({_PACKAGENAME}))

import az_qt_helpers
import azlmbr.editor as editor
from DreamStudioAI.Editor.Scripts.dialog import o3de_dreamstudioDialog

if __name__ == "__main__":
    print(f'{_PACKAGENAME}, Generated from O3DE PythonToolGem Template')

    # Register our custom widget as a dockable tool with the Editor under an Examples sub-menu
    options = editor.ViewPaneOptions()
    options.showOnToolsToolbar = True
    options.toolbarIcon = ":/DreamStudioAI/toolbar_icon.svg"
    az_qt_helpers.register_view_pane('DreamStudioAI',
                                     o3de_dreamstudioDialog,
                                     category="Examples",
                                     options=options)
