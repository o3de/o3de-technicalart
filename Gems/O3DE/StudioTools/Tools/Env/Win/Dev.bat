@echo off

REM
REM Copyright (c) Contributors to the Open 3D Engine Project.
REM For complete copyright and license terms please see the LICENSE at the root of this distribution.
REM
REM SPDX-License-Identifier: Apache-2.0 OR MIT
REM
REM

IF NOT "%DCCSI_ENV_DEV_INIT%"=="1" (
	echo.
	echo _____________________________________________________________________
	echo.
	echo ~    O3DE StudioTools, User Dev ...
	echo ~    StudioTools\Tools\win\Dev.bat
	echo _____________________________________________________________________
	echo.
)

:: developers may have multiple repos, on different branch states
:: this allows a dev to explicitly set the root engine path
:: e.g. the nightly installer default is C:\O3DE\0.0.0.0
:: set O3DE_DEV=C:\depot\o3de-dev

:: the build folder for o3de binaries is not always known
:: where it is in an installer release may be stable
:: but for example, the nightly installer is pre-built and does not
:: have a build folder location, just the binaries location
:: C:\O3DE\0.0.0.0\bin\Windows\profile\Default

:: For devs, my o3de engine-centric location
set "O3DE_DEV=C:\depot\o3de-dev"
echo     O3DE_DEV = %O3DE_DEV%

:: engine developers may build in an engine centric way
:: and configure a cmake build anywhere, this allows devs to set their
:: build folder name (assuming it is in the engine root)

:: this allows the full path to your build folder to be explicitly set
:: this would work for also pointing to a project centric build location
set "O3DE_BUILD_FOLDER=build\bin\profile"

:: this allows the full path to your bin folder to be explicitly set
set "PATH_O3DE_BIN=%O3DE_DEV%\%O3DE_BUILD_FOLDER%"
echo     PATH_O3DE_BIN = %PATH_O3DE_BIN%

:: this will enable additional debug behaviour within code
:: this will in many cases cascade into settings like LogLevel
:: note: expect things to become more verbose
set DCCSI_GDEBUG=True

:: this generally enables behaviour such as auto-start/connect debugger in entry points
:: or may enable local tests that execute during operation
set DCCSI_DEV_MODE=True

:: This specifies the debugger to use for DEV_MODE
:: note: currently only wing is supported (expanding this is future work)
set DCCSI_GDEBUGGER=WING

:: even though you can enable/disable debug logic
:: this allows the logging level to be explicitly set (more verbose)
:: I commonly set this to 10 even when I disbale the global debug flag
set DCCSI_LOGLEVEL=10

:: Set flag so we don't initialize dccsi environment twice
SET DCCSI_ENV_DEV_INIT=1
GOTO END_OF_FILE

:: Return to starting directory
POPD

:END_OF_FILE
