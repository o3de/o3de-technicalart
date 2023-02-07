"""
Copyright (c) Contributors to the Open 3D Engine Project.
For complete copyright and license terms please see the LICENSE at the root of this distribution.

SPDX-License-Identifier: Apache-2.0 OR MIT
"""
# -------------------------------------------------------------------------
"""urdf_exporter_gem\\editor\\scripts\\urdf_exporter_gem_dialog.py
Generated from O3DE PythonToolGem Template"""

from PySide2.QtCore import Qt
from PySide2.QtCore import QProcess
from PySide2.QtWidgets import QCheckBox, QComboBox, QDialog, QFormLayout, QGridLayout, QGroupBox, QLabel,  QPushButton, QVBoxLayout, QWidget, QLineEdit, QFileDialog
from functools import partial
import export as URDF
from pathlib import Path
import re


class urdf_exporter_gemDialog(QDialog):
    def __init__(self, parent=None):
        """ 
        This is the start of the QT O3DE URDF tool window panel.
        """
        # Window Functions
        super(urdf_exporter_gemDialog, self).__init__(parent)
        
        self.ros2_path = '/opt/ros/humble/'
        self.ros2_ws_src_path = '~/ros2_ws/src/'
        self.ros2_ws_path = '~/ros2_ws/'
        self.title_label_info = ''
        self.rvis_command = ''
        self.gazebo_command = ''
        self.main_text_window()
        self.main_layout = QVBoxLayout()
        self.main_layout.setAlignment(Qt.AlignCenter)
        self.main_layout.addWidget(self.form_group_box)
        self.setLayout(self.main_layout)

        self.form_group_box = QGroupBox()
        self.main_layout = QFormLayout(self)
        self.main_layout.setAlignment(Qt.AlignCenter)

    def start_export_button(self):
        """
        This function will grab all the form information and send to export.
        """
        # User entered robot name
        robot_name = self.name_input.text()
        # Send infomration to export, recieve message on status
        robot_found, message, rviz_launch_message, gazebo_launch_message = URDF.search_for_base_link(robot_name, self.ros2_path, self.ros2_ws_src_path)
        # Status from export scene search
        if not robot_found:
            self.rvis_button.setDisabled(True)
            self.gazebo_button.setDisabled(True)
            self.title_label_info.setText(message)
        else:
            self.rvis_button.setDisabled(False)
            self.gazebo_button.setDisabled(False)
            self.title_label_info.setText(rviz_launch_message)
            self.rvis_command = rviz_launch_message
            self.gazebo_command = gazebo_launch_message

    def select_folder(self, ui_type):
        """
        This function will ask user to select folder paths and store thus path var
        
        Args:
            ui_type (String): Type of folder selected by UI

        """
        file_dialog = QFileDialog()
        if ui_type is 'ros2':
            ros2_path = file_dialog.getExistingDirectory(self, 'Select ROS2 Directory')
            self.title_label_info.setText(ros2_path)
            self.ros2_path_line.setText(ros2_path)
            # Create Path from string
            self.ros2_path = Path(ros2_path)
        else:
            ros2_ws_src_path = file_dialog.getExistingDirectory(self, 'Select ROS2 WS SRC Directory')
            self.title_label_info.setText(ros2_ws_src_path)
            self.ros2_ws_path_line.setText(ros2_ws_src_path)
            # Create Path from string
            self.ros2_ws_src_path = Path(ros2_ws_src_path)
            self.ros2_ws_path = Path(self.ros2_ws_src_path).parents[0]
            print(self.ros2_ws_path)
            # Check to see if this is a valid Directoy
            if self.ros2_ws_path.is_dir():
                # If Valid Directory enable save button
                self.save_button.setDisabled(False)

    def check_string(self):
        """
        This function will check to see if the robots name string is lower case and valid.
        """   
        update_text = re.sub(r'[A-Z]+', '', self.name_input.text())
        self.name_input.setText(update_text)

    def update_info_text(self, message):
        """
        This function will update info text from other scripts

        Args:
            message (String): Message for title label text
        """
        self.title_label_info.setText(message)

    def start_rvis_process(self):
        """
        This function will start the rVis launch application subprocess
        """
        URDF.launch_rviz(self.rvis_command)

    def start_gazebo_process(self):
        """
        This function will start the Gazebo launch application subprocess
        """
        URDF.launch_gazebo(self.gazebo_command)

    def main_text_window(self):
        """
        This is the main game text output window and main user text edit input
        """
        self.form_group_box = QGroupBox()
        self.main_layout = QFormLayout()
        self.main_layout.setAlignment(Qt.AlignCenter)
        self.main_layout.update()

        # Title
        self.title_lable = QLabel("Export your selected Visual Robot to URDF XML")
        self.title_lable.setStyleSheet('text-align: center; font: 10pt')
        self.title_lable.setFixedWidth(300)
        self.main_layout.addRow(self.title_lable)

        self.title_lable_base_link = QLabel("The root entity must be named base_link.")
        self.title_lable_base_link.setStyleSheet('text-align: center; font: 10pt; color: rgb(255, 208, 0)')
        self.title_lable_base_link.setFixedWidth(300)
        self.main_layout.addRow(self.title_lable_base_link)

        self.title_lable_note = QLabel("all names including mesh names need to be lower case.")
        self.title_lable_note.setStyleSheet('text-align: center; font: 10pt; color: rgb(255, 208, 0)')
        self.title_lable_note.setFixedWidth(300)
        self.main_layout.addRow(self.title_lable_note)
        
        # Select ROS2 Path
        self.ros2_path_lable = QLabel("ROS2 Location")
        self.ros2_path_lable.setFixedWidth(300)
        self.main_layout.addRow(self.ros2_path_lable)

        self.ros2_path_line = QLineEdit(self)
        self.ros2_path_line.setPlaceholderText("/opt/ros/humble/")
        self.ros2_path_line.setClearButtonEnabled(True)
        self.ros2_path_line.setFixedWidth(260)

        self.ros2_path_button = QPushButton("SET")
        self.ros2_path_button.setFixedWidth(40)
        self.ros2_path_button.setDisabled(False)
        self.ros2_path_button.clicked.connect(lambda:self.select_folder('ros2'))
        self.main_layout.addRow(self.ros2_path_line, self.ros2_path_button)

        # Select ROS2 WorkSpace Path
        self.ros2_ws_path_lable = QLabel("ROS2 Workpsace Source Location")
        self.ros2_ws_path_lable.setFixedWidth(300)
        self.main_layout.addRow(self.ros2_ws_path_lable)

        self.ros2_ws_path_line = QLineEdit(self)
        self.ros2_ws_path_line.setPlaceholderText("/ros2_ws/src/")
        self.ros2_ws_path_line.setClearButtonEnabled(True)
        self.ros2_ws_path_line.setFixedWidth(260)

        self.ros2_ws_path_button = QPushButton("SET")
        self.ros2_ws_path_button.setFixedWidth(40)
        self.ros2_ws_path_button.setDisabled(False)
        self.ros2_ws_path_button.clicked.connect(lambda:self.select_folder('ros2_ws'))
        self.main_layout.addRow(self.ros2_ws_path_line, self.ros2_ws_path_button)
        
        # Enter Robot Description Name
        self.robot_name_lable = QLabel("Enter your Robot Name")
        self.robot_name_lable.setFixedWidth(300)
        self.main_layout.addRow(self.robot_name_lable)

        self.name_input = QLineEdit(self)
        self.name_input.setPlaceholderText("my_robot")
        self.name_input.setClearButtonEnabled(True)
        self.name_input.setFixedWidth(300)
        self.name_input.textChanged.connect(lambda:self.check_string())
        self.main_layout.addRow(self.name_input)

        # Export the Robot Description Package Files and Build
        self.save_button = QPushButton("EXPORT ROBOT PACKAGE")
        self.save_button.setFixedWidth(300)
        self.save_button.setDisabled(True)
        self.save_button.clicked.connect(self.start_export_button)
        self.main_layout.addRow(self.save_button)

        # Launch rIVZ
        self.rvis_button = QPushButton("LAUNCH RVIZ")
        self.rvis_button.setFixedWidth(300)
        self.rvis_button.setDisabled(True)
        self.rvis_button.clicked.connect(self.start_rvis_process)
        self.main_layout.addRow(self.rvis_button)

        # Launch Gazebo
        self.gazebo_button = QPushButton("LAUNCH GAZEBO")
        self.gazebo_button.setFixedWidth(300)
        self.gazebo_button.setDisabled(True)
        self.gazebo_button.clicked.connect(self.start_gazebo_process)
        self.main_layout.addRow(self.gazebo_button)

        self.title_label_info = QLabel("INFO:")
        self.title_label_info.setStyleSheet('text-align: center; background-color: rgb(10, 10, 10); color: rgb(170, 170, 170); padding: 6px; font: 10pt')
        self.title_label_info.setFixedWidth(300)
        self.main_layout.addRow(self.title_label_info)

        # Will add a help page when its done.
        #self.helpText = str("For help getting started,"
            #"visit the <a href=\"https://o3de.org/">URDF EXPORTER FOR O3DE</a> documentation<br/>"

        #self.helpLabel = QLabel()
        #self.helpLabel.setTextFormat(Qt.RichText)
        #self.helpLabel.setText(self.helpText)
        #self.helpLabel.setOpenExternalLinks(True)

        spacer = QLabel("")
        spacer.setStyleSheet('min-width: 5em')
        self.main_layout.addRow(spacer)
        self.form_group_box.setLayout(self.main_layout)
        self.form_group_box.setAlignment(Qt.AlignCenter)


if __name__ == "__main__":
    # Create a new instance of the tool if launched from the Python Scripts window,
    # which allows for quick iteration without having to close/re-launch the Editor
    test_dialog = urdf_exporter_gemDialog()
    test_dialog.setWindowTitle("O3DE URDF EXPORTER")
    test_dialog.show()
    test_dialog.resize(300, 200)
