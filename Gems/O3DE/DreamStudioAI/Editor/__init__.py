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
import sys
from pathlib import Path
import logging as _logging

__all__ = ['Scripts']

# let our parent pkg into the house with us
path_root = Path(__file__).parents[2]
sys.path.append(str(path_root))

from DreamStudioAI import _PACKAGENAME
_PACKAGENAME = f'{_PACKAGENAME}.Editor'
_LOGGER = _logging.getLogger(_PACKAGENAME)
_LOGGER.debug(f'Initializing: {_PACKAGENAME}.')

