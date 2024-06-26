@ECHO OFF

SETLOCAL

SET SCRIPT_PATH=%~dp0
SET VIS_GOOM_ROOT_PATH=%SCRIPT_PATH%\..
SET VIS_GOOM_PARENT=%VIS_GOOM_ROOT_PATH%\..

SET BUILD_TYPE=Release
SET SUFFIX=master
SET BUILD_DIR=depends\goom-libs\build-cl-%BUILD_TYPE%-%SUFFIX%

PUSHD %VIS_GOOM_ROOT_PATH%
IF ErrorLevel 1 Exit /b 1

RMDIR /Q/S %BUILD_DIR%
MKDIR %BUILD_DIR%

CD %BUILD_DIR%

cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
      -DENABLE_TESTING=1 ^
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ^
      ..
POPD

