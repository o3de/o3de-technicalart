"""
Copyright (c) Contributors to the Open 3D Engine Project.
For complete copyright and license terms please see the LICENSE at the root of this distribution.

SPDX-License-Identifier: Apache-2.0 OR MIT
"""
# -------------------------------------------------------------------------
"""o3de_dreamstudio\\editor\\scripts\\o3de_dreamstudio_dialog.py
Generated from O3DE PythonToolGem Template"""

import sys
from pathlib import Path
import logging as _logging

import io
import os
import warnings
from PIL import Image
from stability_sdk import client
import stability_sdk.interfaces.gooseai.generation.generation_pb2 as generation

from PySide2.QtCore import Qt
from PySide2.QtGui import QDoubleValidator
from PySide2.QtWidgets import QCheckBox, QComboBox, QDialog, QFormLayout, QGridLayout, QGroupBox, QLabel, QLineEdit, QPushButton, QVBoxLayout, QWidget
# -------------------------------------------------------------------------


# -------------------------------------------------------------------------
# let our parent pkg into the house with us
path_root = Path(__file__).parents[3]
sys.path.append(str(path_root))

from o3de_dreamstudio.Editor.Scripts import _PACKAGENAME
_PACKAGENAME = f'{_PACKAGENAME}.o3de-dreamstudio_dialog'
_LOGGER = _logging.getLogger(_PACKAGENAME)
_LOGGER.debug('Initializing: {0}.'.format({_PACKAGENAME}))

from o3de_dreamstudio.config import settings
from o3de_dreamstudio.config import _secrets_file
from o3de_dreamstudio.config import ENVAR_STABILITY_API_KEY
from o3de_dreamstudio.config import _stability_api_key
from o3de_dreamstudio.config import _default_dream_file
# -------------------------------------------------------------------------


# -------------------------------------------------------------------------
from box import Box

if _stability_api_key == "< NOT SET >":
    # if the secrets file exists, grab our API key
    if _secrets_file.exists():
        _settings_box = Box.from_json(filename=str(_secrets_file),
                                      encoding="utf-8",
                                      errors="strict")

        _stability_api_key = _settings_box[ENVAR_STABILITY_API_KEY]

        settings.set(ENVAR_STABILITY_API_KEY, _stability_api_key)
# -------------------------------------------------------------------------


# -------------------------------------------------------------------------
class o3de_dreamstudioDialog(QDialog):
    """! Creates DreamStudio Dialog"""

    # defaults in class variables
    api_key_store = settings.STABILITY_API_KEY
    height = 768
    width = 1024
    prompt = "The most beautiful flower garden in the world!"

    def on_api_input_text_changed(self, text):

        self.api_key_store = text

        _settings_box[ENVAR_STABILITY_API_KEY] = self.api_key_store

        _secrets_file.touch(exist_ok=True)

        _settings_box.to_json(filename=_secrets_file.as_posix(),
                              sort_keys=False,
                              indent=4)

        settings.set(ENVAR_STABILITY_API_KEY, self.api_key_store)

    def on_height_input_text_changed(self, text):
        self.height = text

    def on_width_input_text_changed(self, text):
        self.width = text

    def on_prompt_text_changed(self, text):
        self.prompt = text
        self.prompt_input.setText(self.prompt)
        self.prompt_input.adjustSize()

    def on_dream_away(self, show = False, save = True):
        stability_api = client.StabilityInference(
            key=settings.STABILITY_API_KEY,
            verbose=True,
        )

        answers = stability_api.generate(
            prompt = str(self.prompt),
            height = int(self.height),
            width = int(self.width)
        )

        for resp in answers:
            for artifact in resp.artifacts:
                if artifact.finish_reason == generation.FILTER:
                    warnings.warn(
                        "Your request activated the API's safety filters and could not be processed."
                        "Please modify the prompt and try again.")
                if artifact.type == generation.ARTIFACT_IMAGE:
                    img = Image.open(io.BytesIO(artifact.binary))
                    if show:
                        img.show(img)
                    if save:
                        img.save(str(_default_dream_file), 'PNG')

    def __init__(self, parent=None):
        super(o3de_dreamstudioDialog, self).__init__(parent)

        # set main layout
        self.main_layout = QVBoxLayout(self)
        self.main_layout.setSpacing(20)

        # create pulldown to set run locla or remote on dreamstudio.ai
        interface_combobox_group = QGroupBox("Select Interface Type", self)
        interface_combobox_layout = QVBoxLayout()
        interface_types = ['Dream Studio API', 'Run Local (not implemented)']
        self.interface_combobox = QComboBox(self)
        self.interface_combobox.setEditable(True)
        self.interface_combobox.addItems(interface_types)
        interface_combobox_layout.addWidget(self.interface_combobox)
        interface_combobox_group.setLayout(interface_combobox_layout)
        self.main_layout.addWidget(interface_combobox_group)

        self.intro_label = QLabel()
        self.intro_label.setOpenExternalLinks(True)
        self.intro_label.setText(f'1. Make a DreamStudio account at:\r'
                                f'<a href=\"https://beta.dreamstudio.ai/\">beta.dreamstudio.ai</a> website<br/>\r'
                                f'2. Copy your API key:\r'
                                f'<a href=\"https://beta.dreamstudio.ai/membership?tab=apiKeys/\">dreamstudio user API key</a><br/>\r'
                                f'3. Paste here ...')
        self.main_layout.addWidget(self.intro_label, 0, Qt.AlignCenter)

        # Line edit to hold the API key
        self.api_label = QLabel('API Key')
        self.main_layout.addWidget(self.api_label, 0, Qt.AlignLeft)

        self.api_key_input = QLineEdit(self)
        self.api_key_input.setText(self.api_key_store)
        self.api_key_input.setClearButtonEnabled(True)
        # listening to signals using a slot as the handler
        self.api_key_input.textChanged.connect(self.on_api_input_text_changed)
        self.main_layout.addWidget(self.api_key_input, 0, Qt.AlignCenter)

        print(self.api_key_input.sizePolicy())

        # height and width
        self.height_input = QLineEdit(self)
        self.height_input.setText(str(self.height))
        self.height_input.textChanged.connect(self.on_height_input_text_changed)
        self.main_layout.addWidget(self.height_input, 0, Qt.AlignCenter)

        self.width_input = QLineEdit(self)
        self.width_input.setText(str(self.width))
        self.width_input.textChanged.connect(self.on_width_input_text_changed)
        self.main_layout.addWidget(self.width_input, 0, Qt.AlignCenter)

        self.prompt_input = QLineEdit(self)
        self.prompt_input.resize(500, 200)
        self.prompt_input.setText(str(self.prompt))
        self.prompt_input.textChanged.connect(self.on_prompt_text_changed)
        self.main_layout.addWidget(self.prompt_input, 0, Qt.AlignCenter)

        self.dream_button = QPushButton('Dream Away', self)
        self.dream_button.clicked.connect(self.on_dream_away)
        self.main_layout.addWidget(self.dream_button, 0, Qt.AlignCenter)

        self.about_label = QLabel()
        self.about_label.setWordWrap(True)
        self.about_label.setOpenExternalLinks(True)
        self.about_label.setText('The O3DE DreamStudioAI Gem was creaded by <a href=\"https://github.com/HogJonny-AMZN/\">HogJonny-AMZN</a>. Please let me know if you have ideas, enahncement requests, find any bugs, or would like to contribute. Enjoy Dreaming!')
        self.main_layout.addWidget(self.about_label, 0, Qt.AlignCenter)

        self.help_text = str("For help getting started,"
            "visit the <a href=\"https://o3de.org/docs/tools-ui/\">UI Development</a> documentation<br/>"
            "or come ask a question in the <a href=\"https://discord.gg/R77Wss3kHe\">sig-ui-ux channel</a> on Discord")

        self.help_label = QLabel()
        self.help_label.setTextFormat(Qt.RichText)
        self.help_label.setText(self.help_text)
        self.help_label.setOpenExternalLinks(True)

        self.main_layout.addWidget(self.help_label, 0, Qt.AlignCenter)

        self.setLayout(self.main_layout)


if __name__ == "__main__":
    # Create a new instance of the tool if launched from the Python Scripts window,
    # which allows for quick iteration without having to close/re-launch the Editor
    test_dialog = o3de_dreamstudioDialog()
    test_dialog.setWindowTitle("o3de_dreamstudio")
    test_dialog.show()
    test_dialog.adjustSize()
