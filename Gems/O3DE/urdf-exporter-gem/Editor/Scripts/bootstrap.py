"""
Copyright (c) Contributors to the Open 3D Engine Project.
For complete copyright and license terms please see the LICENSE at the root of this distribution.

SPDX-License-Identifier: Apache-2.0 OR MIT
"""
# -------------------------------------------------------------------------
"""urdf_exporter_gem\\editor\\scripts\\boostrap.py
Generated from O3DE PythonToolGem Template"""

import az_qt_helpers
import azlmbr.editor as editor
from urdf_exporter_gem_dialog import urdf_exporter_gemDialog

if __name__ == "__main__":
    print("urdf_exporter_gem.boostrap, Generated from O3DE PythonToolGem Template")

    # Register our custom widget as a dockable tool with the Editor under an Examples sub-menu
    options = editor.ViewPaneOptions()
    options.showOnToolsToolbar = True
    options.toolbarIcon = ":/urdf-exporter-gem/toolbar_icon.svg"
    
    az_qt_helpers.register_view_pane('urdf_exporter_gem', urdf_exporter_gemDialog, category="Examples", options=options)
