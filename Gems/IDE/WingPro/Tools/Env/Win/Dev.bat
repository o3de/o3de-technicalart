@echo off

REM
REM Copyright (c) Contributors to the Open 3D Engine Project.
REM For complete copyright and license terms please see the LICENSE at the root of this distribution.
REM
REM SPDX-License-Identifier: Apache-2.0 OR MIT
REM
REM

IF NOT "%DCCSI_WING_ENV_DEV_INIT%"=="1" (
	echo.
	echo _____________________________________________________________________
	echo.
	echo ~   O3DE StudioTools, WingPro Env Dev ...
	echo ~   WingPro\Tools\Env\Dev.bat
	echo _____________________________________________________________________
	echo.
)

set DCCSI_GDEBUG=True

set DCCSI_DEV_MODE=False

set DCCSI_GDEBUGGER=WING

:: Default level logger will handle
:: Override this to control the setting
:: CRITICAL:50
:: ERROR:40
:: WARNING:30
:: INFO:20
:: DEBUG:10
:: NOTSET:0
set DCCSI_LOGLEVEL=10

set "O3DE_BUILD_FOLDER=Build\bin\profile"

:: Set flag so we don't initialize dccsi environment twice
SET DCCSI_WING_ENV_DEV_INIT=1
GOTO END_OF_FILE

:: Return to starting directory
POPD

:END_OF_FILE