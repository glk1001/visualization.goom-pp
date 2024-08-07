cmake_minimum_required(VERSION 3.28)

project(visualization.goom-pp.lib)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


if (NOT DEFINED GOOM_CMAKE_SCRIPTS_PATH)
    set(GOOM_CMAKE_SCRIPTS_PATH "../../cmake")
endif ()
if (NOT DEFINED GOOM_LIBS_DIR)
    set(GOOM_LIBS_DIR ${PROJECT_SOURCE_DIR})
endif ()

# Set GOOM_DEBUG for non-release builds.
string(TOUPPER "${CMAKE_BUILD_TYPE}" UPPER_CMAKE_BUILD_TYPE)
if (UPPER_CMAKE_BUILD_TYPE STREQUAL "RELEASE")
    MESSAGE(STATUS "Goom: GOOM_DEBUG turned off.")
else ()
    add_definitions(-DGOOM_DEBUG)
    MESSAGE(STATUS "Goom: GOOM_DEBUG turned on.")
endif ()
string(REPLACE "-DNDEBUG" ""
       CMAKE_CXX_FLAGS_${UPPER_CMAKE_BUILD_TYPE}
       ${CMAKE_CXX_FLAGS_${UPPER_CMAKE_BUILD_TYPE}})


include(${GOOM_CMAKE_SCRIPTS_PATH}/CPM.cmake)
include(${GOOM_CMAKE_SCRIPTS_PATH}/StandardProjectSettings.cmake)
include(${GOOM_CMAKE_SCRIPTS_PATH}/PreventInSourceBuilds.cmake)
include(${GOOM_CMAKE_SCRIPTS_PATH}/Cache.cmake)
include(${GOOM_CMAKE_SCRIPTS_PATH}/CompilerWarnings.cmake)
include(${GOOM_CMAKE_SCRIPTS_PATH}/Linker.cmake)
include(${GOOM_CMAKE_SCRIPTS_PATH}/CheckMinimumToolRequirements.cmake)


set(BUILD_SHARED_LIBS OFF)
include(../../ProjectOptions.cmake)
option(ENABLE_TESTING "Enable Test Builds" ON)


if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Windows linker seems to be broken - possibly with multi-threading.
    set(POS_INDEP_CODE "OFF")
else ()
    set(POS_INDEP_CODE "ON")
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    option(ENABLE_BUILD_WITH_TIME_TRACE
           "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
    if (ENABLE_BUILD_WITH_TIME_TRACE)
        target_compile_options(project_options INTERFACE -ftime-trace)
    endif ()
endif ()


if (NOT DEFINED GLM_INCLUDE_DIR)
    CPMAddPackage("gh:g-truc/glm#1.0.1")
    set(GLM_INCLUDE_DIR "${glm_SOURCE_DIR}")
endif ()

set(ASMJIT_EMBED True)
set(BLEND2D_STATIC True)
set(BLEND2D_NO_INSTALL True)
add_definitions(-DBL_STATIC)
CPMAddPackage(NAME "asmjit"
              GITHUB_REPOSITORY "glk1001/asmjit"
              GIT_TAG "disable_fno_enforce_eh_specs"
              FORCE False
              DOWNLOAD_ONLY True
)
set(ASMJIT_DIR "${asmjit_SOURCE_DIR}")
CPMAddPackage(NAME "blend2d"
              GITHUB_REPOSITORY "glk1001/blend2d"
              GIT_TAG "disable_fno_enforce_eh_specs"
              FORCE False
)
set_target_properties(blend2d PROPERTIES POSITION_INDEPENDENT_CODE ${POS_INDEP_CODE})

set(magic_enum_INCLUDE_DIR "${GOOM_LIBS_DIR}/lib/magic_enum")

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    message(WARNING "Freetype on Windows/Mac is too hard to install - disabling Freetype.")
    add_definitions(-DNO_FREETYPE_INSTALLED)
else ()
    find_package(Freetype)
    if (FREETYPE_FOUND)
        message(STATUS "Found Freetype package - using supplied include directories.")
    else ()
        message(WARNING "Could not find Freetype package - disabling Freetype.")
        add_definitions(-DNO_FREETYPE_INSTALLED)
    endif ()
endif ()

set(goom_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/goom/src)
set(GOOM_LIB_BUILD_TIME_FILE ${goom_SOURCE_DIR}/utils/build_time.cppm PARENT_SCOPE)

add_subdirectory(src)

message(STATUS "Goom-libs: Cmake version                  = \"${CMAKE_VERSION}\"")
message(STATUS "Goom-libs: Cmake generator                = \"${CMAKE_GENERATOR}\"")
message(STATUS "Goom-libs: CMAKE_CXX_COMPILER_ID          = \"${CMAKE_CXX_COMPILER_ID}\".")
message(STATUS "Goom-libs: CMAKE_CXX_COMPILER_VERSION     = \"${CMAKE_CXX_COMPILER_VERSION}\".")
message(STATUS "Goom-libs: CMAKE_BUILD_TYPE               = \"${CMAKE_BUILD_TYPE}\".")
message(STATUS "Goom-libs: CMAKE_CXX_FLAGS_${UPPER_CMAKE_BUILD_TYPE}"
        " = \"${CMAKE_CXX_FLAGS_${UPPER_CMAKE_BUILD_TYPE}}\".")
message(STATUS "Goom-libs: WARNINGS_AS_ERRORS             = \"${vis_goom_pp_WARNINGS_AS_ERRORS}\".")
message(STATUS "Goom-libs: CMAKE_MODULE_PATH              = \"${CMAKE_MODULE_PATH}\".")
message(STATUS "Goom-libs: GOOM_CMAKE_SCRIPTS_PATH        = \"${GOOM_CMAKE_SCRIPTS_PATH}\".")
message(STATUS "Goom-libs: GENERATOR_IS_MULTI_CONFIG      = \"${GENERATOR_IS_MULTI_CONFIG}\".")
message(STATUS "Goom-libs: C++ standard                   = \"${CMAKE_CXX_STANDARD}\".")
message(STATUS "Goom-libs: C++ standard required          = \"${CMAKE_CXX_STANDARD_REQUIRED}\".")
message(STATUS "Goom-libs: C++ extensions                 = \"${CMAKE_CXX_EXTENSIONS}\".")
message(STATUS "Goom-libs: BUILD_SHARED_LIBS              = \"${BUILD_SHARED_LIBS}\".")
message(STATUS "Goom-libs: ENABLE_TESTING                 = \"${ENABLE_TESTING}\".")
message(STATUS "Goom-libs: GOOM_LIBS_DIR                  = \"${GOOM_LIBS_DIR}\".")
message(STATUS "Goom-libs: goom_SOURCE_DIR                = \"${goom_SOURCE_DIR}\".")
message(STATUS "Goom-libs: blend2d_SOURCE_DIR             = \"${blend2d_SOURCE_DIR}\".")
message(STATUS "Goom-libs: GLM_INCLUDE_DIR                = \"${GLM_INCLUDE_DIR}\".")
message(STATUS "Goom-libs: FREETYPE_INCLUDE_DIRS          = \"${FREETYPE_INCLUDE_DIRS}\".")
message(STATUS "Goom-libs: magic_enum_INCLUDE_DIR         = \"${magic_enum_INCLUDE_DIR}\".")
