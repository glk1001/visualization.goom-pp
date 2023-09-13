cmake_minimum_required(VERSION 3.20)

project(GoomTests LANGUAGES CXX)

set(GOOM_LIB_TESTS_NAME goom_lib_tests)

if (WIN32)
    add_definitions(-D_WIN32PC)
endif ()

find_package(Threads)

CPMAddPackage(NAME Catch2 GITHUB_REPOSITORY catchorg/Catch2 VERSION 3.3.2)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_BINARY_DIR}/_deps/catch2-src/extras")


add_executable(${GOOM_LIB_TESTS_NAME}
               src/test_main.cpp
               src/test_pixels.cpp
               src/color/test_color_maps_grids.cpp
               src/color/test_color_utils.cpp
               src/draw/test_draw.cpp
               src/filters/test_filter_buffers.cpp
               src/filters/test_filter_zoom_vector.cpp
               src/filters/test_normalized_coords.cpp
               src/sound/test_sound_info.cpp
               src/utils/graphics/test_pixel_utils.cpp
               src/utils/math/test_goom_rand.cpp
               src/utils/math/test_misc.cpp
               src/utils/math/test_randutils.cpp
               src/utils/test_enum_utils.cpp
               src/utils/test_parallel_utils.cpp
               src/utils/test_strutils.cpp
               src/utils/test_t_values.cpp
               src/utils/test_timer.cpp
)

target_include_directories(${GOOM_LIB_TESTS_NAME}
                           PRIVATE
                           ${GOOM_LIBS_DIR}/src/goom/include
                           ${GOOM_LIBS_DIR}/src/goom/include/goom
                           ${GOOM_LIBS_DIR}/src/goom/src
)
target_include_directories(${GOOM_LIB_TESTS_NAME}
                           SYSTEM PRIVATE
                           ${GOOM_LIBS_DIR}/lib/vivid/include
                           ${GOOM_LIBS_DIR}/lib/libdivide
                           ${GOOM_LIBS_DIR}/lib/span/include
                           ${GOOM_LIBS_DIR}/lib/fmt/include
                           ${fmt_SOURCE_DIR}/include
                           ${magic_enum_INCLUDE_DIR}
                           ${GLM_INCLUDE_DIR}
)

target_link_libraries(${GOOM_LIB_TESTS_NAME}
                      PRIVATE
                      goom
                      vivid
                      LSys::lib
                      particles::lib
                      Catch2::Catch2
                      ${CMAKE_THREAD_LIBS_INIT}
)
if (FREETYPE_FOUND)
    target_link_libraries(${GOOM_LIB_TESTS_NAME}
                          PRIVATE
                          Freetype::Freetype
    )
endif ()
if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_link_libraries(${GOOM_LIB_TESTS_NAME}
                          PRIVATE
                          m
                          pthread
                          stdc++
    )
endif ()

vis_goom_pp_set_project_warnings(vis_goom_pp_WARNINGS_AS_ERRORS ${GOOM_LIB_TESTS_NAME})
vis_goom_pp_enable_sanitizers(${GOOM_LIB_TESTS_NAME}
                              ${vis_goom_pp_ENABLE_SANITIZER_ADDRESS}
                              ${vis_goom_pp_ENABLE_SANITIZER_LEAK}
                              ${vis_goom_pp_ENABLE_SANITIZER_UNDEFINED}
                              ${vis_goom_pp_ENABLE_SANITIZER_THREAD}
                              ${vis_goom_pp_ENABLE_SANITIZER_MEMORY}
)
vis_goom_pp_configure_linker(${GOOM_LIB_TESTS_NAME})

include(Catch)

#catch_discover_tests(${GOOM_LIB_TESTS_NAME})
add_test(NAME ${GOOM_LIB_TESTS_NAME}
         COMMAND ${GOOM_LIB_TESTS_NAME})


get_target_property(${GOOM_LIB_TESTS_NAME} ${GOOM_LIB_TESTS_NAME} COMPILE_OPTIONS)
get_target_property(${GOOM_LIB_TESTS_NAME} ${GOOM_LIB_TESTS_NAME} COMPILE_FEATURES)
get_target_property(${GOOM_LIB_TESTS_NAME} ${GOOM_LIB_TESTS_NAME} LINK_OPTIONS)

message(STATUS "Goom Lib Tests: C++ standard = \"${CMAKE_CXX_STANDARD}\".")
message(STATUS
        "Goom Lib Tests: C++ compile features = \"${${GOOM_LIB_TESTS_NAME}_compile_features}\".")
message(STATUS
        "Goom Lib Tests: C++ compile options = \"${${GOOM_LIB_TESTS_NAME}_compile_options}\".")
message(STATUS "Goom Lib Tests: C++ link options = \"${${GOOM_LIB_TESTS_NAME}_link_options}\".")
message(STATUS "Goom Lib Tests: Catch2_SOURCE_DIR = \"${Catch2_SOURCE_DIR}\".")
