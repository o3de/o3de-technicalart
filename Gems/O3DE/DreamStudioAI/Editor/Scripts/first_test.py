"""
Copyright (c) Contributors to the Open 3D Engine Project.
For complete copyright and license terms please see the LICENSE at the root of this distribution.

SPDX-License-Identifier: Apache-2.0 OR MIT
"""
# -------------------------------------------------------------------------
"""o3de_dreamstudio\\editor\\scripts\\first_test.py
This will run a generation from the static prompt below"""

import sys
from pathlib import Path
import logging as _logging

import io
import os
import warnings
from PIL import Image
from stability_sdk import client
import stability_sdk.interfaces.gooseai.generation.generation_pb2 as generation
# -------------------------------------------------------------------------


# -------------------------------------------------------------------------
# let our parent pkg into the house with us
path_root = Path(__file__).parents[3]
sys.path.append(str(path_root))

from o3de_dreamstudio.Editor.Scripts import _PACKAGENAME
_PACKAGENAME = f'{_PACKAGENAME}.first_test'
_LOGGER = _logging.getLogger(_PACKAGENAME)
_LOGGER.debug('Initializing: {0}.'.format({_PACKAGENAME}))

from o3de_dreamstudio.config import settings
from o3de_dreamstudio.config import SECRETS_FILE
# -------------------------------------------------------------------------


# -------------------------------------------------------------------------
from box import Box

_settings_box = Box.from_json(filename=str(SECRETS_FILE),
                              encoding="utf-8",
                              errors="strict")

ENVAR_STABILITY_API_KEY = 'STABILITY_API_KEY'

# https://github.com/Stability-AI/stability-sdk/blob/main/src/stability_sdk/client.py

stability_api = client.StabilityInference(
    key=_settings_box[ENVAR_STABILITY_API_KEY],
    verbose=True,
)

answers = stability_api.generate(
    prompt="houston, we are a 'go' for launch!",
    height = 768,
    width = 1024
)

for resp in answers:
    for artifact in resp.artifacts:
        if artifact.finish_reason == generation.FILTER:
            warnings.warn(
                "Your request activated the API's safety filters and could not be processed."
                "Please modify the prompt and try again.")
        if artifact.type == generation.ARTIFACT_IMAGE:
            img = Image.open(io.BytesIO(artifact.binary))
            img.show(img)
# -------------------------------------------------------------------------
