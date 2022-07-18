@ECHO OFF

SETLOCAL

SET SCRIPT_PATH=%~dp0
SET VIS_GOOM_PARENT=%SCRIPT_PATH%\..
SET BUILD_TYPE=Release
SET BUILD_DIR=%SCRIPT_PATH%\build-%BUILD_TYPE%

PUSHD %SCRIPT_PATH%
IF ErrorLevel 1 Exit /b 1

RMDIR /Q/S %BUILD_DIR%
MKDIR %BUILD_DIR%

CD %BUILD_DIR%

cmake -DADDONS_TO_BUILD=visualization.goom ^
      -DADDON_SRC_PREFIX=%VIS_GOOM_PARENT% ^
      -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
      -DCMAKE_INSTALL_PREFIX=%VIS_GOOM_PARENT%\xbmc\kodi-build\addons ^
      -DPACKAGE_ZIP=1 ^
      -DBUILD_ARGS_ext="-DENABLE_TESTING=0;-DUSE_MAGIC_ENUM=0" ^
      %VIS_GOOM_PARENT%\xbmc\cmake\addons

POPD
