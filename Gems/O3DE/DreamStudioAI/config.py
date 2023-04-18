
#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#
# -------------------------------------------------------------------------
"""! brief: configuration for DreamStudioAI Gem (aka DreamStudioAI)"""

# imports
import sys
import os
from pathlib import Path
import logging as _logging

# let our parent pkg into the house with us
path_root = Path(__file__).parents[1]
sys.path.append(str(path_root))

from DreamStudioAI import _PACKAGENAME
_MODULENAME = f'{_PACKAGENAME}.config'
_LOGGER = _logging.getLogger(_MODULENAME)
_LOGGER.debug('Initializing: {0}.'.format({_MODULENAME}))

from dynaconf import Dynaconf

settings = Dynaconf(
    settings_files=[
        "settings.json",
        "settings.local.json",
        ".secrets.json"
    ],
    environments=True,
    load_dotenv=True,
    envvar_prefix="DYNACONF",
    env_switcher="ENV_FOR_DYNACONF",
    dotenv_path=".env"
)

ENVAR_STABILITY_API_KEY = 'STABILITY_API_KEY'
_stability_api_key = os.getenv(ENVAR_STABILITY_API_KEY, "< NOT SET >")
os.environ[f'DYNACONF_{ENVAR_STABILITY_API_KEY}'] = _stability_api_key
settings.set(ENVAR_STABILITY_API_KEY, _stability_api_key)

ENVAR_SECRETS_FILE = 'SECRETS_FILE'
_secrets_file = Path(Path(__file__).parent, ".secrets.json").resolve()
os.environ[f'DYNACONF_{ENVAR_SECRETS_FILE}'] = str(_secrets_file)
settings.set(ENVAR_SECRETS_FILE, _secrets_file)

ENVAR_DEFAULT_DREAM_PATH = 'DEFAULT_DREAM_PATH'
_default_dream_file = Path(Path(__file__).parent, 'Assets', 'Dreams', 'default_dream_basecolor.png').resolve()
os.environ[f'DYNACONF_{ENVAR_DEFAULT_DREAM_PATH}'] = str(_default_dream_file)
settings.set(ENVAR_DEFAULT_DREAM_PATH, _default_dream_file)

if _secrets_file.exists():
    settings.load_file(path=_secrets_file) # not working, investigate later!?!?

settings.setenv()
