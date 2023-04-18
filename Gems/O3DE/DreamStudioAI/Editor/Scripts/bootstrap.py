"""
Copyright (c) Contributors to the Open 3D Engine Project.
For complete copyright and license terms please see the LICENSE at the root of this distribution.

SPDX-License-Identifier: Apache-2.0 OR MIT
"""
# -------------------------------------------------------------------------
"""o3de_dreamstudio\\editor\\scripts\\boostrap.py
Generated from O3DE PythonToolGem Template"""

import sys
from pathlib import Path
import logging as _logging

# let our parent pkg into the house with us
path_root = Path(__file__).parents[3]
sys.path.append(str(path_root))

from o3de_dreamstudio.Editor.Scripts import _PACKAGENAME
_PACKAGENAME = f'{_PACKAGENAME}.bootstrap'
_LOGGER = _logging.getLogger(_PACKAGENAME)
_LOGGER.debug('Initializing: {0}.'.format({_PACKAGENAME}))

import az_qt_helpers
import azlmbr.editor as editor
from o3de-dreamstudio_dialog import o3de_dreamstudioDialog

if __name__ == "__main__":
    print("o3de_dreamstudio.boostrap, Generated from O3DE PythonToolGem Template")

    # Register our custom widget as a dockable tool with the Editor under an Examples sub-menu
    options = editor.ViewPaneOptions()
    options.showOnToolsToolbar = True
    options.toolbarIcon = ":/o3de-dreamstudio/toolbar_icon.svg"
    az_qt_helpers.register_view_pane('o3de_dreamstudio', o3de_dreamstudioDialog, category="Examples", options=options)
