@echo off
REM
REM Copyright (c) Contributors to the Open 3D Engine Project.
REM For complete copyright and license terms please see the LICENSE at the root of this distribution.
REM
REM SPDX-License-Identifier: Apache-2.0 OR MIT
REM
REM

:: Sets up environment for Lumberyard DCC tools and code access

:: Set up window
TITLE O3DE StudioTools Core
:: Use obvious color to prevent confusion (Grey with Yellow Text)
COLOR 8E

:: Skip initialization if already completed
IF "%DCCSI_ENV_INIT%"=="1" GOTO :END_OF_FILE

:: Store current dir
%~d0
cd %~dp0
PUSHD %~dp0

::SETLOCAL ENABLEDELAYEDEXPANSION

:: if the user has set up a custom env call it
IF EXIST "%~dp0Dev.bat" CALL %~dp0Dev.bat

echo.
echo _____________________________________________________________________
echo.
echo ~    O3DE StudioTools (DCCsi), Core Environment ...
echo _____________________________________________________________________
echo.

IF "%DCCSI_CORE_ENV%"=="" (set DCCSI_CORE_ENV=%~dp0)
echo     DCCSI_CORE_ENV = %DCCSI_CORE_ENV%

:: add to the PATH
SET PATH=%DCCSI_CORE_ENV%;%PATH%

:: Constant Vars (Global)
:: global debug flag (propogates)
:: The intent here is to set and globally enter a debug mode
IF "%DCCSI_GDEBUG%"=="" (set DCCSI_GDEBUG=false)
echo     DCCSI_GDEBUG = %DCCSI_GDEBUG%
:: initiates earliest debugger connection
:: we support attaching to WingIDE... PyCharm and VScode in the future
IF "%DCCSI_DEV_MODE%"=="" (set DCCSI_DEV_MODE=false)
echo     DCCSI_DEV_MODE = %DCCSI_DEV_MODE%
:: sets debugger, options: WING, PYCHARM
IF "%DCCSI_GDEBUGGER%"=="" (set DCCSI_GDEBUGGER=WING)
echo     DCCSI_GDEBUGGER = %DCCSI_GDEBUGGER%
:: Default level logger will handle
:: Override this to control the setting
:: CRITICAL:50
:: ERROR:40
:: WARNING:30
:: INFO:20
:: DEBUG:10
:: NOTSET:0
IF "%DCCSI_LOGLEVEL%"=="" (set DCCSI_LOGLEVEL=20)
echo     DCCSI_LOGLEVEL = %DCCSI_LOGLEVEL%

IF "%DCCSI_TESTS%"=="" (set DCCSI_TESTS=false)
echo     DCCSI_TESTS = %DCCSI_TESTS%

:: You can define the project name
IF "%O3DE_PROJECT%"=="" (
    for %%a in (%CD%..\..\..\..) do set O3DE_PROJECT=%%~na
        )
echo     O3DE_PROJECT = %O3DE_PROJECT%

:: set up the default project path (dccsi)
:: if not set we also use the DCCsi path as stand-in
CD /D ..\..\..\
IF "%PATH_O3DE_PROJECT%"=="" (set "PATH_O3DE_PROJECT=%CD%")
echo     PATH_O3DE_PROJECT = %PATH_O3DE_PROJECT%

:: Save current directory and change to target directory
IF "%ABS_PATH%"=="" (set "ABS_PATH=%PATH_O3DE_PROJECT%")
pushd %ABS_PATH%

IF "%O3DE_DEV%"=="" (
    echo       ~ O3DE_DEV is not set aka engine_root
    echo       ~ set O3DE_DEV in Dev.bat
    echo       ~ example: set "O3DE_DEV=C:\O3DE\0.0.0.0"
    set O3DE_DEV=C:\not\set\o3de
        )

rem IF "%O3DE_DEV%"=="" (set "O3DE_DEV=< not set >")
rem echo     O3DE_DEV = %O3DE_DEV%
rem :: Restore original directory
rem popd

IF "%PATH_O3DE_USERHOME%"=="" (set "PATH_O3DE_USERHOME=%userprofile%\.o3de")
echo     PATH_O3DE_USERHOME = %PATH_O3DE_USERHOME%

:: We need to know where the engine 3rdParty folder is to add access for PySide2, etc
:: Adding 3rdParty location, default is something like c:\users\< user>\.o3de\3rdparty
IF "%PATH_O3DE_3RDPARTY%"=="" (set "PATH_O3DE_3RDPARTY=%PATH_O3DE_USERHOME%\3rdparty")
echo     PATH_O3DE_3RDPARTY = %PATH_O3DE_3RDPARTY%

IF "%PATH_O3DE_USER_LOGS%"=="" (set "PATH_O3DE_USER_LOGS==%PATH_O3DE_USERHOME%\Logs")
echo     PATH_O3DE_USER_LOGS = %PATH_O3DE_USER_LOGS%

:: O3DE Technical Art Gems Location
set "PATH_O3DE_TECHART_GEMS=%ABS_PATH%\..\.."
FOR /F "delims=" %%F IN ("%PATH_O3DE_TECHART_GEMS%") DO SET "PATH_O3DE_TECHART_GEMS=%%~fF"
echo     PATH_O3DE_TECHART_GEMS = %PATH_O3DE_TECHART_GEMS%

:: dcc scripting interface gem path
:: currently know relative path to this gem
set "PATH_DCCSIG=%PATH_O3DE_TECHART_GEMS%\O3DE\StudioTools"
echo     PATH_DCCSIG = %PATH_DCCSIG%

set "PATH_DCCSIG_TOOLS=%PATH_DCCSIG%\Tools"
echo     PATH_DCCSIG_TOOLS = %PATH_DCCSIG_TOOLS%

:: Change to DCCsi root dir
CD /D %PATH_DCCSIG_TOOLS%

:: dccsi DCC tools path
set "PATH_DCCSI_TOOLS_DCC=%PATH_O3DE_TECHART_GEMS%\DCC"
echo     PATH_DCCSI_TOOLS_DCC = %PATH_DCCSI_TOOLS_DCC%

:: dccsi IDE tools path
set "PATH_DCCSI_TOOLS_IDE=%PATH_O3DE_TECHART_GEMS%\IDE"
echo     PATH_DCCSI_TOOLS_IDE = %PATH_DCCSI_TOOLS_IDE%

:: temp log location specific to this gem
set "DCCSI_LOG_PATH=%PATH_O3DE_USER_LOGS%\dccsi"
echo     DCCSI_LOG_PATH = %DCCSI_LOG_PATH%

:: This is the default bin path pattern for installer builds
IF "%O3DE_BUILD_FOLDER%"=="" (set "O3DE_BUILD_FOLDER=bin\Windows\profile\Default")
echo     O3DE_BUILD_FOLDER = %O3DE_BUILD_FOLDER%

:: for reference the nightly engine sdk bin path looks like:
:: C:\O3DE\0.0.0.0\bin\Windows\profile\Default
IF "%PATH_O3DE_BIN%"=="" (set "PATH_O3DE_BIN=%O3DE_DEV%\%O3DE_BUILD_FOLDER%")
echo     PATH_O3DE_BIN = %PATH_O3DE_BIN%

::ENDLOCAL

:: Set flag so we don't initialize dccsi environment twice
SET DCCSI_ENV_INIT=1
GOTO END_OF_FILE

:: Return to starting directory
POPD

:END_OF_FILE
