cmake_minimum_required(VERSION 3.20)

project(visualization.goom-pp)

message(STATUS "Cmake version = \"${CMAKE_VERSION}\"")
message(STATUS "Cmake generator = \"${CMAKE_GENERATOR}\"")

set(GOOM_TARGET_NAME "visualization.goom-pp")
set(GOOM_ADDON_PREFIX "GOOM")

option(ENABLE_TESTING "Enable unit testing" OFF)
option(USE_MAGIC_ENUM "Use magic enum" OFF)

set(GOOM_LIBS_TARGET_NAME "goom")
set(GOOM_LIBS_RELDIR depends/goom-libs)
set(GOOM_LIBS_DIR ${PROJECT_SOURCE_DIR}/${GOOM_LIBS_RELDIR})
set(GOOM_BUILD_ARTIFACTS_DIR ${CMAKE_BINARY_DIR}/goom-libs-artifacts)
set(GOOM_INCLUDE_DIRS "${GOOM_LIBS_DIR}/src/goom/include")

if (NOT WIN32)
    set(GOOM_BIN_LIBS
        ${GOOM_BUILD_ARTIFACTS_DIR}/lib/${CMAKE_BUILD_TYPE}/libgoom.a
        ${GOOM_BUILD_ARTIFACTS_DIR}/lib/${CMAKE_BUILD_TYPE}/libvivid.a)
else ()
    set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE})
    set(GOOM_BIN_LIBS
        ${GOOM_BUILD_ARTIFACTS_DIR}/lib/${CMAKE_CONFIGURATION_TYPES}/goom.lib
        ${GOOM_BUILD_ARTIFACTS_DIR}/lib/${CMAKE_CONFIGURATION_TYPES}/vivid.lib)
endif ()

if (NOT KODI_ROOT_DIR)
    set(KODI_ROOT_DIR ${PROJECT_SOURCE_DIR}/..)
else ()
    get_filename_component(KODI_ROOT_DIR "${KODI_ROOT_DIR}" ABSOLUTE)
endif ()

if (IS_THIS_MAIN_CMAKE_SCRIPT)
    set(GOOM_BUILD_DIR "${CMAKE_BINARY_DIR}")
    set(CMAKE_INSTALL_PREFIX ${KODI_ROOT_DIR}/kodi/kodi-build/addons)
else ()
    message(STATUS "This cmake script is being run from the kodi addons cmake script.")
    message(STATUS "KODI_ROOT_DIR = \"${KODI_ROOT_DIR}\".")
    message(STATUS "CMAKE_INSTALL_PREFIX = \"${CMAKE_INSTALL_PREFIX}\".")
    if (CMAKE_INSTALL_LIBDIR)
        message(STATUS "CMAKE_INSTALL_LIBDIR = \"${CMAKE_INSTALL_LIBDIR}\".")
        message(WARNING "Do not set 'CMAKE_INSTALL_LIBDIR'."
                " This will be set to empty so kodi can reset it later.")
    endif ()
    # Hack - 'CMAKE_BINARY_DIR' points to a subdirectory below the parent build dir.
    #        Not sure how to cleanly get the parent directory.
    set(GOOM_BUILD_DIR "${CMAKE_BINARY_DIR}/../../..")
    cmake_path(NORMAL_PATH GOOM_BUILD_DIR OUTPUT_VARIABLE GOOM_BUILD_DIR)
endif ()

if (NOT EXISTS ${GOOM_BUILD_DIR})
    message(FATAL_ERROR "Could not find goom build directory \"${GOOM_BUILD_DIR}\".")
endif ()
if (NOT EXISTS ${CMAKE_INSTALL_PREFIX})
    message(FATAL_ERROR "Could not find install directory \"${CMAKE_INSTALL_PREFIX}\".")
endif ()
set(CMAKE_SCRIPTS_PATH "${PROJECT_SOURCE_DIR}/cmake")


if (IS_THIS_MAIN_CMAKE_SCRIPT)
    # Executing the Kodi addon cmake script here allows this script to be the main cmake script.
    # This fits in with the way CLion works.
    if (EXISTS "${CMAKE_BINARY_DIR}/CMakeCache.txt")
        message(STATUS "In build mode: Found cache file ${CMAKE_BINARY_DIR}/CMakeCache.txt.")
    else ()
        message(STATUS "In config mode: Executing kodi addons cmake.")
        execute_process(COMMAND cmake
                        -D ADDONS_TO_BUILD=${GOOM_TARGET_NAME}
                        -D ADDON_SRC_PREFIX=${KODI_ROOT_DIR}
                        -D CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
                        -D PACKAGE_ZIP=1
                        ${KODI_ROOT_DIR}/kodi/cmake/addons
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                        RESULT_VARIABLE CMAKE_RESULT)
        if (NOT CMAKE_RESULT EQUAL "0")
            file(REMOVE "${CMAKE_BINARY_DIR}/CMakeCache.txt")
            message(FATAL_ERROR "cmake kodi-addons failed.")
        endif ()
    endif ()
endif ()


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INSTALL_MESSAGE LAZY)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SCRIPTS_PATH})

include(${CMAKE_SCRIPTS_PATH}/CPM.cmake)
include(${CMAKE_SCRIPTS_PATH}/StandardProjectSettings.cmake)
include(${CMAKE_SCRIPTS_PATH}/PreventInSourceBuilds.cmake)
include(${CMAKE_SCRIPTS_PATH}/StandardProjectSettings.cmake)
include(${CMAKE_SCRIPTS_PATH}/PreventInSourceBuilds.cmake)
include(${CMAKE_SCRIPTS_PATH}/Cache.cmake)
include(${CMAKE_SCRIPTS_PATH}/CompilerWarnings.cmake)
include(${CMAKE_SCRIPTS_PATH}/StaticAnalyzers.cmake)
include(${CMAKE_SCRIPTS_PATH}/CheckMinimumToolRequirements.cmake)


if (CMAKE_CXX_COMPILER_ID MATCHES ".*MSVC")
    set(POS_INDEP_CODE "OFF")
else ()
    set(POS_INDEP_CODE "ON")
endif ()
if (CORE_SYSTEM_NAME STREQUAL "osx")
    set(CMAKE_SHARED_LINKER_FLAGS "-undefined dynamic_lookup -flat_namespace")
endif ()


CPMAddPackage("gh:fmtlib/fmt#9.1.0")

CPMAddPackage(NAME "LSys"
              GITHUB_REPOSITORY "glk1001/LSys"
              GIT_TAG "master"
              FORCE True
              )
set_target_properties(lsys-gen PROPERTIES EXCLUDE_FROM_ALL True)

find_package(Kodi REQUIRED)

if (NOT WIN32 AND (APP_RENDER_SYSTEM STREQUAL "gl" OR NOT APP_RENDER_SYSTEM))
    find_package(OpenGl REQUIRED)
    set(DEPLIBS ${OPENGL_LIBRARIES})
    add_definitions(${OPENGL_DEFINITIONS})
else ()
    find_package(OpenGLES 3.2 REQUIRED)
    set(DEPLIBS ${OPENGLES_LIBRARIES})
    add_definitions(${OPENGLES_DEFINITIONS})
endif ()


# Let the Goom code know we're building from Kodi. There are Kodi
# headers that the Main goom code needs only when doing a Kodi build.
add_definitions(-DIS_KODI_BUILD)

# Set GOOM_DEBUG for non-release builds.
string(TOUPPER "${CMAKE_BUILD_TYPE}" UPPER_CMAKE_BUILD_TYPE)
if (UPPER_CMAKE_BUILD_TYPE STREQUAL "RELEASE")
    MESSAGE(STATUS "Goom: GOOM_DEBUG turned off.")
else ()
    add_definitions(-DGOOM_DEBUG)
    MESSAGE(STATUS "Goom: GOOM_DEBUG turned on.")
endif ()

# Include the Goom libs cmake files here, before the (horrible) global
# compiler settings (needed for Kodi addon builder).
add_subdirectory(${GOOM_LIBS_RELDIR})

get_project_warnings(PROJECT_WARNINGS)
add_compile_options(${PROJECT_WARNINGS})

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    # glm has many 'unsafe-buffer-usage' warnings.
    message(STATUS "Turning off 'unsafe-buffer-usage' for Clang.")
    add_compile_options(-Wno-unknown-warning-option -Wno-unsafe-buffer-usage)
endif ()

CPMAddPackage("gh:g-truc/glm#0.9.9.8")
set(GLM_INCLUDE_DIR "${glm_SOURCE_DIR}")


# Setup the includes and other dependencies the addon builder needs.
include_directories(BEFORE
                    ${GOOM_LIBS_DIR}
                    ${GOOM_INCLUDE_DIRS}
                    )
include_directories(BEFORE
                    SYSTEM
                    ${GOOM_LIBS_DIR}/lib/span/include
                    ${GOOM_LIBS_DIR}/lib/span/include/tcb
                    ${GOOM_LIBS_DIR}/src/vivid/include
                    ${GOOM_LIBS_DIR}/lib/fmt/include
                    ${fmt_SOURCE_DIR}/include
                    ${GLM_INCLUDE_DIR}
                    ${KODI_INCLUDE_DIR}
                    ${KODI_FIXED_INCLUDE_DIR}
                    )

set(${GOOM_ADDON_PREFIX}_SOURCES src/Main.cpp
    src/build_time.cpp
    src/gl_renderer.cpp
    src/goom_buffer_producer.cpp
    src/shader_with_effects.cpp
    src/kodi_shader_with_effects.cpp)
set(${GOOM_ADDON_PREFIX}_HEADERS src/Main.h
    src/build_time.h
    src/circular_buffer.h
    src/gl_render_types.h
    src/gl_renderer.h
    src/goom_buffer_producer.h
    src/shader_with_effects.h
    src/kodi_shader_with_effects.h)

list(APPEND DEPLIBS ${GOOM_BIN_LIBS} LSys::lib)

# Let kodi set this:
set(CMAKE_INSTALL_LIBDIR "")

build_addon(${GOOM_TARGET_NAME} ${GOOM_ADDON_PREFIX} DEPLIBS)
add_dependencies(${GOOM_TARGET_NAME} ${GOOM_LIBS_TARGET_NAME} vivid)


# Force the build time into the compiled binaries.
set(GOOM_BUILD_TIME_FILES
    ${GOOM_LIBS_DIR}/src/goom/src/utils/build_time.cpp
    ${PROJECT_SOURCE_DIR}/src/build_time.cpp
    )
add_custom_target(goom_build_time ALL DEPENDS ${GOOM_BUILD_TIME_FILES})
add_custom_command(TARGET goom_build_time
                   PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E touch ${GOOM_BUILD_TIME_FILES}
                   COMMENT "Touching build files."
                   VERBATIM
                   )


include(CPack)


get_target_property(goom_addon_compile_options ${GOOM_TARGET_NAME} COMPILE_OPTIONS)
get_target_property(goom_addon_compile_features ${GOOM_TARGET_NAME} COMPILE_FEATURES)

message(STATUS "Visualization.goom: CORE_SYSTEM_NAME = \"${CORE_SYSTEM_NAME}\".")
message(STATUS "Visualization.goom: CMAKE_CXX_COMPILER_ID = \"${CMAKE_CXX_COMPILER_ID}\".")
message(STATUS
        "Visualization.goom: CMAKE_CXX_COMPILER_VERSION = \"${CMAKE_CXX_COMPILER_VERSION}\".")
message(STATUS "Visualization.goom: CMAKE_BUILD_TYPE = \"${CMAKE_BUILD_TYPE}\".")
message(STATUS "Visualization.goom: WARNINGS_AS_ERRORS = \"${WARNINGS_AS_ERRORS}\".")
if (WIN32)
    message(STATUS "Visualization.goom: CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION ="
            " \"${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}\".")
endif ()
message(STATUS "Visualization.goom: CMAKE_SOURCE_DIR   = \"${CMAKE_SOURCE_DIR}\".")
message(STATUS "Visualization.goom: PROJECT_SOURCE_DIR = \"${PROJECT_SOURCE_DIR}\".")
message(STATUS "Visualization.goom: CMAKE_BINARY_DIR   = \"${CMAKE_BINARY_DIR}\".")
message(STATUS "Visualization.goom: CMAKE_MODULE_PATH = \"${CMAKE_MODULE_PATH}\".")
message(STATUS "Visualization.goom: CMAKE_SCRIPTS_PATH = \"${CMAKE_SCRIPTS_PATH}\".")
message(STATUS "Visualization.goom: CMAKE_INSTALL_PREFIX = \"${CMAKE_INSTALL_PREFIX}\".")
message(STATUS "Visualization.goom: CMAKE_INSTALL_LIBDIR = \"${CMAKE_INSTALL_LIBDIR}\".")
message(STATUS "Visualization.goom: CMAKE_INSTALL_DATADIR = \"${CMAKE_INSTALL_DATADIR}\".")
message(STATUS "Visualization.goom: C++ standard = \"${CMAKE_CXX_STANDARD}\".")
message(STATUS "Visualization.goom: C++ standard required = \"${CMAKE_CXX_STANDARD_REQUIRED}\".")
message(STATUS "Visualization.goom: C++ extensions = \"${CMAKE_CXX_EXTENSIONS}\".")
message(STATUS "Visualization.goom: C++ compile features = \"${goom_addon_compile_features}\".")
message(STATUS "Visualization.goom: C++ compile options = \"${goom_addon_compile_options}\".")
message(STATUS "Visualization.goom: ENABLE_TESTING = \"${ENABLE_TESTING}\".")
message(STATUS "Visualization.goom: IS_THIS_MAIN_CMAKE_SCRIPT = \"${IS_THIS_MAIN_CMAKE_SCRIPT}\".")
message(STATUS "Visualization.goom: GOOM_LIBS_DIR = \"${GOOM_LIBS_DIR}\".")
message(STATUS "Visualization.goom: GOOM_BUILD_DIR = \"${GOOM_BUILD_DIR}\".")
message(STATUS "Visualization.goom: GOOM_INCLUDE_DIRS = \"${GOOM_INCLUDE_DIRS}\".")
message(STATUS "Visualization.goom: GOOM_BIN_LIBS = \"${GOOM_BIN_LIBS}\".")
message(STATUS "Visualization.goom: GOOM_BUILD_ARTIFACTS_DIR = \"${GOOM_BUILD_ARTIFACTS_DIR}\".")
message(STATUS "Visualization.goom: GLM_INCLUDE_DIR = \"${GLM_INCLUDE_DIR}\".")
message(STATUS "Visualization.goom: KODI_ROOT_DIR = \"${KODI_ROOT_DIR}\".")
message(STATUS "Visualization.goom: KODI_CONFIG_DIR = \"${KODI_CONFIG_DIR}\".")
message(STATUS "Visualization.goom: KODI_INCLUDE_DIR = \"${KODI_INCLUDE_DIR}\".")
message(STATUS "Visualization.goom: KODI_FIXED_INCLUDE_DIR = \"${KODI_FIXED_INCLUDE_DIR}\".")
