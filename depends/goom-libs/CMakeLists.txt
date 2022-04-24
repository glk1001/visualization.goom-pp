cmake_minimum_required(VERSION 3.15)

project(visualization.goom.lib)

if(NOT DEFINED GOOM_BUILD_ARTIFACTS_DIR)
  set(GOOM_BUILD_ARTIFACTS_DIR "${CMAKE_BINARY_DIR}")
endif()
if (NOT DEFINED CMAKE_SCRIPTS_PATH)
  set(CMAKE_SCRIPTS_PATH "../../cmake")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${GOOM_BUILD_ARTIFACTS_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${GOOM_BUILD_ARTIFACTS_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${GOOM_BUILD_ARTIFACTS_DIR}/bin)

if(WIN32)
  set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE})
endif()

set(CPP_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

string(TOUPPER "${CMAKE_BUILD_TYPE}" UPPER_CMAKE_BUILD_TYPE)
if (UPPER_CMAKE_BUILD_TYPE STREQUAL "RELEASE")
  MESSAGE(STATUS "Goom: GOOM_DEBUG turned off.")
else()
  add_definitions(-DGOOM_DEBUG)
  MESSAGE(STATUS "Goom: GOOM_DEBUG turned on.")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES ".*MSVC" AND NOT MSVC_TOOLSET_VERSION STREQUAL 143)
  option(WARNINGS_AS_ERRORS "OLDER MSVC: '[[maybe_unused]]' not working for lambdas" OFF)
else()
  option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  # Older clangs don't like some variations of "Circle(Circle&&) noexcept = default"
  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(MIN_CLANG_VERSION 13)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(MIN_CLANG_VERSION 12)
  endif()
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${MIN_CLANG_VERSION})
    message(FATAL_ERROR "${CMAKE_CXX_COMPILER_ID} version = '${CMAKE_CXX_COMPILER_VERSION}'. Minimum acceptable version is '${MIN_CLANG_VERSION}'.")
  endif()
  option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
  if(ENABLE_BUILD_WITH_TIME_TRACE)
    target_compile_options(project_options INTERFACE -ftime-trace)
  endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES ".*GNU")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11)
    message(FATAL_ERROR "GCC version = '${CMAKE_CXX_COMPILER_VERSION}'. Minimum acceptable version is '11'.")
  endif()
endif()


# Don't trust older build environments yet.
include(CheckIncludeFileCXX)
CHECK_INCLUDE_FILE_CXX(optional OPTIONAL_INCLUDE)
if (NOT OPTIONAL_INCLUDE)
  message(FATAL_ERROR "Could not find C++17 'optional' header. Is libc++ up to date?")
endif()
CHECK_INCLUDE_FILE_CXX(filesystem FILESYSTEM_INCLUDE)
if (NOT FILESYSTEM_INCLUDE)
  message(FATAL_ERROR "Could not find C++17 'filesystem' header. Is libc++ up to date?")
endif()


include(${CMAKE_SCRIPTS_PATH}/CPM.cmake)
include(${CMAKE_SCRIPTS_PATH}/StandardProjectSettings.cmake)
include(${CMAKE_SCRIPTS_PATH}/PreventInSourceBuilds.cmake)
include(${CMAKE_SCRIPTS_PATH}/Cache.cmake)
include(${CMAKE_SCRIPTS_PATH}/Linker.cmake)
include(${CMAKE_SCRIPTS_PATH}/CompilerWarnings.cmake)
include(${CMAKE_SCRIPTS_PATH}/Sanitizers.cmake)
include(${CMAKE_SCRIPTS_PATH}/StaticAnalyzers.cmake)

option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" OFF)
option(ENABLE_TESTING "Enable Test Builds" ON)
option(ENABLE_FUZZING "Enable Fuzzing Builds" OFF)


if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_COMMON_CXX_WARNINGS /W2)
  # Windows linker seems to be broken - possibly with multi-threading.
  set(POS_INDEP_CODE "OFF")
else()
  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(CMAKE_COMMON_CXX_WARNINGS ${CMAKE_COMMON_CXX_WARNINGS} -Wno-c++17-extensions)
  endif()
  set(POS_INDEP_CODE "ON")
endif()


if (NOT DEFINED GLM_INCLUDE_DIR)
  CPMAddPackage("gh:g-truc/glm#0.9.9.8")
  set(GLM_INCLUDE_DIR "${glm_SOURCE_DIR}")
endif()

find_package(Freetype)
if (FREETYPE_FOUND)
  message(STATUS "Found FreeType package - using supplied include directories.")
else()
  message(WARNING "Could not find FreeType package - disabling Freetype.")
  add_definitions(-DNO_FREETYPE_INSTALLED)
endif ()

if(NOT DEFINED fmt_SOURCE_DIR)
  CPMAddPackage("gh:fmtlib/fmt#8.0.1")
endif()

# CPMAddPackage has trouble with 'magic_enum' on Windows. So turn off by default.
if (NOT USE_MAGIC_ENUM)
  set(magic_enum_SOURCE_DIR "")
else()
  CPMAddPackage("gh:Neargye/magic_enum#v0.7.3")
endif()
if(magic_enum_SOURCE_DIR)
  message(STATUS "Found magic_enum package - using supplied include directories.")
  set(magic_enum_INCLUDE_DIR "${magic_enum_SOURCE_DIR}/include")
else()
  message(WARNING "Could not find magic_enum package - disabling magic_enum.")
  add_definitions(-DNO_MAGIC_ENUM_AVAILABLE)
  set(magic_enum_INCLUDE_DIR "")
endif()

if(ENABLE_TESTING)
  find_package(Catch2 REQUIRED)
endif()


set(THIRD_PARTY_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/lib)
set(GOOM_ROOT_DIR ${CMAKE_SOURCE_DIR}/src)


message(STATUS "Goom-libs: CMAKE_CXX_COMPILER_ID = \"${CMAKE_CXX_COMPILER_ID}\".")
message(STATUS "Goom-libs: CMAKE_CXX_COMPILER_VERSION = \"${CMAKE_CXX_COMPILER_VERSION}\".")
message(STATUS "Goom-libs: CMAKE_BUILD_TYPE = \"${CMAKE_BUILD_TYPE}\".")
message(STATUS "Goom-libs: WARNINGS_AS_ERRORS = \"${WARNINGS_AS_ERRORS}\".")
message(STATUS "Goom-libs: CMAKE_MODULE_PATH = \"${CMAKE_MODULE_PATH}\".")
message(STATUS "Goom-libs: CMAKE_SCRIPTS_PATH = \"${CMAKE_SCRIPTS_PATH}\".")
message(STATUS "Goom-libs: CMAKE_LIBRARY_OUTPUT_DIRECTORY = \"${CMAKE_LIBRARY_OUTPUT_DIRECTORY}\".")
message(STATUS "Goom-libs: C++ standard = \"${CPP_STANDARD}\".")
message(STATUS "Goom-libs: C++ standard required = \"${CMAKE_CXX_STANDARD_REQUIRED}\".")
message(STATUS "Goom-libs: C++ extensions = \"${CMAKE_CXX_EXTENSIONS}\".")
message(STATUS "Goom-libs: ENABLE_TESTING = \"${ENABLE_TESTING}\".")
message(STATUS "Goom-libs: GLM_INCLUDE_DIR = \"${GLM_INCLUDE_DIR}\".")
message(STATUS "Goom-libs: THIRD_PARTY_INCLUDE_DIR = \"${THIRD_PARTY_INCLUDE_DIR}\".")
message(STATUS "Goom-libs: FREETYPE_INCLUDE_DIRS = \"${FREETYPE_INCLUDE_DIRS}\".")
message(STATUS "Goom-libs: magic_enum_INCLUDE_DIR = \"${magic_enum_INCLUDE_DIR}\".")
message(STATUS "Goom-libs: fmt_SOURCE_DIR = \"${fmt_SOURCE_DIR}\".")
message(STATUS "Goom-libs: GOOM_ROOT_DIR = \"${GOOM_ROOT_DIR}\".")
message(STATUS "Goom-libs: CMAKE_ARCHIVE_OUTPUT_DIRECTORY = \"${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}\".")
message(STATUS "Goom-libs: CMAKE_LIBRARY_OUTPUT_DIRECTORY = \"${CMAKE_LIBRARY_OUTPUT_DIRECTORY}\".")
message(STATUS "Goom-libs: CMAKE_RUNTIME_OUTPUT_DIRECTORY = \"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}\".")


if(ENABLE_TESTING)
  include(CTest)
  include(Catch)
endif()

add_subdirectory(src)