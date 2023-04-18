# coding:utf-8
#!/usr/bin/python
#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#
# --------------------------------------------------------------------------
"""! @brief

o3de_dreamstudio/__init__.py

Allow o3de-dream-studio Gem to be the parent python pkg
"""

_INFO = {
    'name': 'O3DE_DREAMSTUDIO_GEM',
    'description': 'A dreamstudio.ai integration as a o3de python Gem',
    'status': 'prototype',
    'version': (0, 0, 1),
    'platforms': ({
        'include': ('win'),   # windows
        'exclude': ('darwin', # mac
                    'linux')  # linux, linux_x64
    }),
    'doc_url': 'readme.md'
}

# -------------------------------------------------------------------------
# standard imports
import os
from pathlib import Path
import logging as _logging

__all__ = ['config', 'Editor']

# -------------------------------------------------------------------------
# global scope
_PACKAGENAME = 'DreamStudioAI'

STR_CROSSBAR = f"{('-' * 74)}"
FRMT_LOG_LONG = "[%(name)s][%(levelname)s] >> %(message)s (%(asctime)s; %(filename)s:%(lineno)d)"

# default loglevel to info unless set
ENVAR_DCCSI_LOGLEVEL = 'DCCSI_LOGLEVEL'
DCCSI_LOGLEVEL = int(os.getenv(ENVAR_DCCSI_LOGLEVEL, _logging.INFO))

# configure basic logger since this is the top-level module
# note: not using a common logger to reduce cyclical imports
_logging.basicConfig(level=DCCSI_LOGLEVEL,
                     format=FRMT_LOG_LONG,
                     datefmt='%m-%d %H:%M')

_LOGGER = _logging.getLogger(_PACKAGENAME)
_LOGGER.debug(STR_CROSSBAR)
_LOGGER.debug(f'Initializing: {_PACKAGENAME}')

# be careful when pulling from this __init__.py module
# avoid cyclical imports, this module should not import from sub-modules

# we need to set up basic access to the DCCsi
_MODULE_PATH = Path(__file__)
_LOGGER.debug(f'_MODULE_PATH: {_MODULE_PATH}')

from DccScriptingInterface import add_site_dir
PATH_GEM_PARENT = _MODULE_PATH.parents[1].resolve()
PATH_GEM_PARENT = add_site_dir(PATH_GEM_PARENT)
# -------------------------------------------------------------------------
