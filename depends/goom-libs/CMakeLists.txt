cmake_minimum_required(VERSION 3.20)

project(visualization.goom-pp.lib)

# set(LSys_SOURCE_DIR "/home/greg/Prj/github/LSys")
# set(Particles_SOURCE_DIR "/home/greg/Prj/github/particles")

if (NOT DEFINED GOOM_BUILD_ARTIFACTS_DIR)
    set(GOOM_BUILD_ARTIFACTS_DIR "${CMAKE_BINARY_DIR}/goom-libs-artifacts")
endif ()
if (NOT DEFINED CMAKE_SCRIPTS_PATH)
    set(CMAKE_SCRIPTS_PATH "../../cmake")
endif ()
if (NOT DEFINED GOOM_LIBS_DIR)
    set(GOOM_LIBS_DIR ${PROJECT_SOURCE_DIR})
endif ()

if (WIN32)
    # 'GENERATOR_IS_MULTI_CONFIG' does not seem reliable for Visual Studio, so we use this string
    # matching hack.
    string(TOLOWER "${CMAKE_GENERATOR}" CMAKE_GENERATOR_LOWER)
    message("CMAKE_GENERATOR_LOWER = \"${CMAKE_GENERATOR_LOWER}\".")
    message("GENERATOR_IS_MULTI_CONFIG = \"${GENERATOR_IS_MULTI_CONFIG}\".")
    if (CMAKE_GENERATOR_LOWER MATCHES ".*nmake.*")
        # Nmake generators don't build libs with a '/${CMAKE_BUILD_TYPE}' suffix.
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${GOOM_BUILD_ARTIFACTS_DIR}/lib/${CMAKE_BUILD_TYPE})
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${GOOM_BUILD_ARTIFACTS_DIR}/lib/${CMAKE_BUILD_TYPE})
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${GOOM_BUILD_ARTIFACTS_DIR}/bin/${CMAKE_BUILD_TYPE})
    else ()
        set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE})
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${GOOM_BUILD_ARTIFACTS_DIR}/lib)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${GOOM_BUILD_ARTIFACTS_DIR}/lib)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${GOOM_BUILD_ARTIFACTS_DIR}/bin)
    endif ()
elseif (NOT GENERATOR_IS_MULTI_CONFIG)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${GOOM_BUILD_ARTIFACTS_DIR}/lib/${CMAKE_BUILD_TYPE})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${GOOM_BUILD_ARTIFACTS_DIR}/lib/${CMAKE_BUILD_TYPE})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${GOOM_BUILD_ARTIFACTS_DIR}/bin/${CMAKE_BUILD_TYPE})
else ()
    set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${GOOM_BUILD_ARTIFACTS_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${GOOM_BUILD_ARTIFACTS_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${GOOM_BUILD_ARTIFACTS_DIR}/bin)
endif ()


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

string(TOUPPER "${CMAKE_BUILD_TYPE}" UPPER_CMAKE_BUILD_TYPE)
if (UPPER_CMAKE_BUILD_TYPE STREQUAL "RELEASE")
    MESSAGE(STATUS "Goom: GOOM_DEBUG turned off.")
else ()
    add_definitions(-DGOOM_DEBUG)
    MESSAGE(STATUS "Goom: GOOM_DEBUG turned on.")
endif ()


include(${CMAKE_SCRIPTS_PATH}/CPM.cmake)
include(${CMAKE_SCRIPTS_PATH}/StandardProjectSettings.cmake)
include(${CMAKE_SCRIPTS_PATH}/PreventInSourceBuilds.cmake)
include(${CMAKE_SCRIPTS_PATH}/Cache.cmake)
include(${CMAKE_SCRIPTS_PATH}/CompilerWarnings.cmake)
include(${CMAKE_SCRIPTS_PATH}/Hardening.cmake)
include(${CMAKE_SCRIPTS_PATH}/Linker.cmake)
include(${CMAKE_SCRIPTS_PATH}/Sanitizers.cmake)
include(${CMAKE_SCRIPTS_PATH}/StaticAnalyzers.cmake)
include(${CMAKE_SCRIPTS_PATH}/CheckMinimumToolRequirements.cmake)


set(BUILD_SHARED_LIBS OFF)
include(../../ProjectOptions.cmake)
if (NO_SAN)
    set(vis_goom_pp_ENABLE_SANITIZER_ADDRESS "OFF")
    set(vis_goom_pp_ENABLE_SANITIZER_LEAK "OFF")
    set(vis_goom_pp_ENABLE_SANITIZER_UNDEFINED "OFF")
    set(vis_goom_pp_ENABLE_SANITIZER_THREAD "OFF")
    set(vis_goom_pp_ENABLE_SANITIZER_MEMORY "OFF")
    set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
endif ()
option(ENABLE_TESTING "Enable Test Builds" ON)
option(ENABLE_FUZZING "Enable Fuzzing Builds" OFF)
vis_goom_pp_enable_hardening(global ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})


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
    CPMAddPackage("gh:g-truc/glm#0.9.9.8")
    set(GLM_INCLUDE_DIR "${glm_SOURCE_DIR}")
endif ()
if (NOT blend2d_SOURCE_DIR)
    set(ASMJIT_EMBED True)
    set(BLEND2D_STATIC True)
    set(BLEND2D_NO_INSTALL True)
    add_definitions(-DBL_STATIC)
    CPMAddPackage(NAME "asmjit"
                  GITHUB_REPOSITORY "asmjit/asmjit"
                  GIT_TAG "master"
                  FORCE False
                  DOWNLOAD_ONLY True
    )
    set(ASMJIT_DIR "${asmjit_SOURCE_DIR}")
    CPMAddPackage(NAME "blend2d"
                  GITHUB_REPOSITORY "blend2d/blend2d"
                  GIT_TAG "master"
                  FORCE False
    )
    set_target_properties(blend2d PROPERTIES POSITION_INDEPENDENT_CODE ${POS_INDEP_CODE})
endif ()
if (NOT DEFINED LSys_SOURCE_DIR)
    CPMAddPackage(NAME "LSys"
                  GITHUB_REPOSITORY "glk1001/LSys"
                  GIT_TAG "master"
                  FORCE True
    )
    set_target_properties(lsys-gen PROPERTIES EXCLUDE_FROM_ALL True)
endif ()
if (NOT DEFINED Particles_SOURCE_DIR)
    CPMAddPackage(NAME "particles"
                  GITHUB_REPOSITORY "glk1001/particles"
                  GIT_TAG "public_cleaned"
                  FORCE True
    )
    set_target_properties(cpuTest PROPERTIES EXCLUDE_FROM_ALL True)
endif ()

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

CPMAddPackage("gh:Neargye/magic_enum#v0.9.5")
if (magic_enum_SOURCE_DIR)
    message(STATUS "Found magic_enum package - using supplied include directories.")
    set(magic_enum_INCLUDE_DIR "${magic_enum_SOURCE_DIR}/include/magic_enum")
else ()
    message(FATAL_ERROR "Could not find magic_enum package.")
endif ()


message(STATUS "Goom-libs: CMAKE_CXX_COMPILER_ID          = \"${CMAKE_CXX_COMPILER_ID}\".")
message(STATUS "Goom-libs: CMAKE_CXX_COMPILER_VERSION     = \"${CMAKE_CXX_COMPILER_VERSION}\".")
message(STATUS "Goom-libs: CMAKE_BUILD_TYPE               = \"${CMAKE_BUILD_TYPE}\".")
message(STATUS "Goom-libs: WARNINGS_AS_ERRORS             = \"${vis_goom_pp_WARNINGS_AS_ERRORS}\".")
message(STATUS "Goom-libs: CMAKE_MODULE_PATH              = \"${CMAKE_MODULE_PATH}\".")
message(STATUS "Goom-libs: CMAKE_SCRIPTS_PATH             = \"${CMAKE_SCRIPTS_PATH}\".")
message(STATUS "Goom-libs: CMAKE_GENERATOR                = \"${CMAKE_GENERATOR}\".")
message(STATUS "Goom-libs: GENERATOR_IS_MULTI_CONFIG      = \"${GENERATOR_IS_MULTI_CONFIG}\".")
message(STATUS "Goom-libs: CMAKE_ARCHIVE_OUTPUT_DIRECTORY = \"${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}\".")
message(STATUS "Goom-libs: CMAKE_LIBRARY_OUTPUT_DIRECTORY = \"${CMAKE_LIBRARY_OUTPUT_DIRECTORY}\".")
message(STATUS "Goom-libs: CMAKE_RUNTIME_OUTPUT_DIRECTORY = \"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}\".")
message(STATUS "Goom-libs: C++ standard                   = \"${CMAKE_CXX_STANDARD}\".")
message(STATUS "Goom-libs: C++ standard required          = \"${CMAKE_CXX_STANDARD_REQUIRED}\".")
message(STATUS "Goom-libs: C++ extensions                 = \"${CMAKE_CXX_EXTENSIONS}\".")
message(STATUS "Goom-libs: BUILD_SHARED_LIBS              = \"${BUILD_SHARED_LIBS}\".")
message(STATUS "Goom-libs: ENABLE_TESTING                 = \"${ENABLE_TESTING}\".")
message(STATUS "Goom-libs: GOOM_LIBS_DIR                  = \"${GOOM_LIBS_DIR}\".")
message(STATUS "Goom-libs: blend2d_SOURCE_DIR             = \"${blend2d_SOURCE_DIR}\".")
message(STATUS "Goom-libs: LSys_SOURCE_DIR                = \"${LSys_SOURCE_DIR}\".")
message(STATUS "Goom-libs: particles_SOURCE_DIR           = \"${particles_SOURCE_DIR}\".")
message(STATUS "Goom-libs: GLM_INCLUDE_DIR                = \"${GLM_INCLUDE_DIR}\".")
message(STATUS "Goom-libs: FREETYPE_INCLUDE_DIRS          = \"${FREETYPE_INCLUDE_DIRS}\".")
message(STATUS "Goom-libs: magic_enum_INCLUDE_DIR         = \"${magic_enum_INCLUDE_DIR}\".")
message(STATUS "Goom-libs: NO_SAN                         = \"${NO_SAN}\".")
message(STATUS "Goom-libs: ENABLE_SANITIZER_ADDRESS       = \"${vis_goom_pp_ENABLE_SANITIZER_ADDRESS}\".")
message(STATUS "Goom-libs: ENABLE_SANITIZER_LEAK          = \"${vis_goom_pp_ENABLE_SANITIZER_LEAK}\".")
message(STATUS "Goom-libs: ENABLE_SANITIZER_UNDEFINED     = \"${vis_goom_pp_ENABLE_SANITIZER_UNDEFINED}\".")
message(STATUS "Goom-libs: ENABLE_SANITIZER_THREAD        = \"${vis_goom_pp_ENABLE_SANITIZER_THREAD}\".")
message(STATUS "Goom-libs: ENABLE_SANITIZER_MEMORY        = \"${vis_goom_pp_ENABLE_SANITIZER_MEMORY}\".")

add_subdirectory(src)
