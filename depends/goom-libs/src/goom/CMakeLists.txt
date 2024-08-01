cmake_minimum_required(VERSION 3.28)

project(Goom LANGUAGES CXX)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g -O2 -Wno-disabled-optimization")
#set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

if (NOT GLM_INCLUDE_DIR)
    message(FATAL_ERROR "\"GLM_INCLUDE_DIR\" must be defined.")
endif ()
if (NOT blend2d_SOURCE_DIR)
    message(FATAL_ERROR "\"blend2d_SOURCE_DIR\" must be defined.")
endif ()
if (NOT goom_SOURCE_DIR)
    message(FATAL_ERROR "\"goom_SOURCE_DIR\" must be defined.")
endif ()
if (NOT "${goom_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}/src")
    message(FATAL_ERROR "\"goom_SOURCE_DIR\" != \"${CMAKE_CURRENT_SOURCE_DIR}/src\".")
endif ()

if (WIN32)
    add_definitions(-D_WIN32PC)
endif ()

if (DEFINED GOOM_LOGGING)
    MESSAGE(STATUS "Goom: Logging by default is on.")
else ()
    add_definitions(-DNO_LOGGING)
    MESSAGE(STATUS "Goom: Logging turned off.")
endif ()

include(src/lsys/cmake/GetLSysSources.cmake)
set(LSys_SOURCE_DIR src/lsys/)
LSys_get_modules("${LSys_SOURCE_DIR}" LSys_modules)
LSys_get_source_files("${LSys_SOURCE_DIR}" LSys_source_files)
LSys_set_some_warnings_off("${LSys_SOURCE_DIR}")

include(src/particles/cmake/GetParticlesSources.cmake)
set(Particles_SOURCE_DIR src/particles/)
Particles_get_modules("${Particles_SOURCE_DIR}" Particles_modules)
Particles_get_source_files("${Particles_SOURCE_DIR}" Particles_source_files)

set(GoomLib_modules
    include/goom/assert_utils.cppm
    include/goom/circular_buffer.cppm
    include/goom/compiler_versions.cppm
    include/goom/frame_data.cppm
    include/goom/goom_buffer_saver.cppm
    include/goom/goom_config_paths.cppm
    include/goom/goom_control.cppm
    include/goom/goom_graphic.cppm
    include/goom/goom_paths.cppm
    include/goom/goom_types.cppm
    include/goom/goom_utils.cppm
    include/goom/goom_version.cppm
    include/goom/point2d.cppm
    include/goom/sound_info.cppm
    include/goom/spimpl.cppm
)
set(GoomLib_source_files
    src/goom_control.cpp
    src/goom_graphic.cpp
    src/goom_logger.cpp
    src/goom_utils.cpp
    src/goom_version.cpp
    src/sound_info.cpp
)

set(GoomColor_modules
    src/color/color_data/extra_color_groups.cppm
    src/color/color_adjustment.cppm
    src/color/color_maps_base.cppm
    src/color/color_maps.cppm
    src/color/color_maps_grids.cppm
    src/color/color_utils.cppm
    src/color/random_color_maps.cppm
    src/color/random_color_maps_groups.cppm
)

set(GoomColor_source_files
    src/color/color_data/color_data_maps.cpp
    src/color/color_data/extra_color_groups.cpp
    src/color/color_adjustment.cpp
    src/color/color_maps.cpp
    src/color/color_maps_grids.cpp
    src/color/color_utils.cpp
    src/color/random_color_maps.cpp
    src/color/random_color_maps_groups.cpp
)

set(GoomControl_modules
    src/control/goom_drawable_states/goom_drawables.cppm
    src/control/goom_drawable_states/goom_drawables_data.cppm
    src/control/goom_drawable_states/goom_favourite_states_handler.cppm
    src/control/goom_drawable_states/goom_forced_state_handler.cppm
    src/control/goom_drawable_states/goom_random_state_handler.cppm
    src/control/goom_drawable_states/goom_state_handler.cppm
    src/control/all_standard_visual_fx.cppm
    src/control/goom_all_visual_fx.cppm
    src/control/goom_effects.cppm
    src/control/goom_lock.cppm
    src/control/goom_message_displayer.cppm
    src/control/goom_music_settings_reactor.cppm
    src/control/goom_sound_events.cppm
    src/control/goom_state_dump.cppm
    src/control/goom_state_monitor.cppm
    src/control/goom_title_displayer.cppm
    src/control/state_and_filter_consts.cppm
    src/control/visual_fx_color_maps.cppm
    src/control/visual_fx_color_matched_sets.cppm
    src/control/visual_fx_weighted_color_maps.cppm
)

set(GoomControl_source_files
    src/control/goom_drawable_states/goom_drawables.cpp
    src/control/goom_drawable_states/goom_drawables_data.cpp
    src/control/goom_drawable_states/goom_favourite_states_handler.cpp
    src/control/goom_drawable_states/goom_forced_state_handler.cpp
    src/control/goom_drawable_states/goom_random_state_handler.cpp
    src/control/goom_all_visual_fx.cpp
    src/control/goom_message_displayer.cpp
    src/control/goom_music_settings_reactor.cpp
    src/control/goom_sound_events.cpp
    src/control/goom_state_dump.cpp
    src/control/goom_state_monitor.cpp
    src/control/goom_title_displayer.cpp
)

set(GoomDraw_modules
    src/draw/shape_drawers/bitmap_drawer.cppm
    src/draw/shape_drawers/circle_drawer.cppm
    src/draw/shape_drawers/drawer_utils.cppm
    src/draw/shape_drawers/line_draw_thick.cppm
    src/draw/shape_drawers/line_draw_wu.cppm
    src/draw/shape_drawers/line_drawer.cppm
    src/draw/shape_drawers/line_drawer_moving_noise.cppm
    src/draw/shape_drawers/line_drawer_noisy_pixels.cppm
    src/draw/shape_drawers/line_drawer_with_effects.cppm
    src/draw/shape_drawers/pixel_drawer.cppm
    src/draw/shape_drawers/text_drawer.cppm
    src/draw/goom_draw.cppm
    src/draw/goom_draw_to_buffer.cppm
    src/draw/goom_draw_to_container.cppm
    src/draw/goom_draw_to_many.cppm
)

set(GoomDraw_source_files
    src/draw/shape_drawers/bitmap_drawer.cpp
    src/draw/shape_drawers/circle_drawer.cpp
    src/draw/shape_drawers/line_draw_thick.cpp
    src/draw/shape_drawers/line_draw_wu.cpp
    src/draw/shape_drawers/line_drawer_moving_noise.cpp
    src/draw/shape_drawers/line_drawer_noisy_pixels.cpp
    src/draw/shape_drawers/text_drawer.cpp
    src/draw/goom_draw_to_buffer.cpp
    src/draw/goom_draw_to_container.cpp
    src/draw/goom_draw_to_many.cpp
)

set(GoomFilterFx_modules
    src/filter_fx/after_effects/the_effects/hypercos.cppm
    src/filter_fx/after_effects/the_effects/image_velocity.cppm
    src/filter_fx/after_effects/the_effects/noise.cppm
    src/filter_fx/after_effects/the_effects/planes.cppm
    src/filter_fx/after_effects/the_effects/rotation.cppm
    src/filter_fx/after_effects/the_effects/tan_effect.cppm
    src/filter_fx/after_effects/the_effects/xy_lerp_effect.cppm
    src/filter_fx/after_effects/after_effects.cppm
    src/filter_fx/after_effects/after_effects_states.cppm
    src/filter_fx/after_effects/after_effects_types.cppm
    src/filter_fx/after_effects/zoom_vector_after_effects.cppm
    src/filter_fx/filter_effects/adjustment_effects/amulet.cppm
    src/filter_fx/filter_effects/adjustment_effects/complex_rational.cppm
    src/filter_fx/filter_effects/adjustment_effects/complex_utils.cppm
    src/filter_fx/filter_effects/adjustment_effects/crystal_ball.cppm
    src/filter_fx/filter_effects/adjustment_effects/distance_field.cppm
    src/filter_fx/filter_effects/adjustment_effects/exp_reciprocal.cppm
    src/filter_fx/filter_effects/adjustment_effects/flow_field.cppm
    src/filter_fx/filter_effects/adjustment_effects/func_of_func.cppm
    src/filter_fx/filter_effects/adjustment_effects/image_zoom_adjustment.cppm
    src/filter_fx/filter_effects/adjustment_effects/mobius.cppm
    src/filter_fx/filter_effects/adjustment_effects/newton.cppm
    src/filter_fx/filter_effects/adjustment_effects/perlin_noise.cppm
    src/filter_fx/filter_effects/adjustment_effects/scrunch.cppm
    src/filter_fx/filter_effects/adjustment_effects/speedway.cppm
    src/filter_fx/filter_effects/adjustment_effects/uniform_zoom_adjustment_effect.cppm
    src/filter_fx/filter_effects/adjustment_effects/wave.cppm
    src/filter_fx/filter_effects/adjustment_effects/y_only.cppm
    src/filter_fx/filter_effects/zoom_adjustment_effect_factory.cppm
    src/filter_fx/filter_effects/zoom_vector_effects.cppm
    src/filter_fx/filter_utils/goom_lerp_data.cppm
    src/filter_fx/filter_utils/image_displacement.cppm
    src/filter_fx/filter_utils/image_displacement_list.cppm
    src/filter_fx/filter_utils/utils.cppm
    src/filter_fx/common_types.cppm
    src/filter_fx/filter_buffers.cppm
    src/filter_fx/filter_consts.cppm
    src/filter_fx/filter_buffers_service.cppm
    src/filter_fx/filter_settings.cppm
    src/filter_fx/filter_settings_service.cppm
    src/filter_fx/filter_speed.cppm
    src/filter_fx/filter_zoom_vector.cppm
    src/filter_fx/normalized_coords.cppm
    src/filter_fx/zoom_adjustment_effect.cppm
    src/filter_fx/zoom_vector.cppm
)

set(GoomFilterFx_source_files
    src/filter_fx/after_effects/the_effects/hypercos.cpp
    src/filter_fx/after_effects/the_effects/image_velocity.cpp
    src/filter_fx/after_effects/the_effects/noise.cpp
    src/filter_fx/after_effects/the_effects/planes.cpp
    src/filter_fx/after_effects/the_effects/rotation.cpp
    src/filter_fx/after_effects/the_effects/tan_effect.cpp
    src/filter_fx/after_effects/the_effects/xy_lerp_effect.cpp
    src/filter_fx/after_effects/after_effects.cpp
    src/filter_fx/after_effects/after_effects_states.cpp
    src/filter_fx/after_effects/zoom_vector_after_effects.cpp
    src/filter_fx/filter_effects/adjustment_effects/amulet.cpp
    src/filter_fx/filter_effects/adjustment_effects/flow_field.cpp
    src/filter_fx/filter_effects/adjustment_effects/complex_rational.cpp
    src/filter_fx/filter_effects/adjustment_effects/complex_utils.cpp
    src/filter_fx/filter_effects/adjustment_effects/crystal_ball.cpp
    src/filter_fx/filter_effects/adjustment_effects/distance_field.cpp
    src/filter_fx/filter_effects/adjustment_effects/exp_reciprocal.cpp
    src/filter_fx/filter_effects/adjustment_effects/func_of_func.cpp
    src/filter_fx/filter_effects/adjustment_effects/image_zoom_adjustment.cpp
    src/filter_fx/filter_effects/adjustment_effects/mobius.cpp
    src/filter_fx/filter_effects/adjustment_effects/newton.cpp
    src/filter_fx/filter_effects/adjustment_effects/perlin_noise.cpp
    src/filter_fx/filter_effects/adjustment_effects/scrunch.cpp
    src/filter_fx/filter_effects/adjustment_effects/speedway.cpp
    src/filter_fx/filter_effects/adjustment_effects/wave.cpp
    src/filter_fx/filter_effects/adjustment_effects/y_only.cpp
    src/filter_fx/filter_effects/zoom_adjustment_effect_factory.cpp
    src/filter_fx/filter_effects/zoom_vector_effects.cpp
    src/filter_fx/filter_utils/image_displacement.cpp
    src/filter_fx/filter_utils/image_displacement_list.cpp
    src/filter_fx/filter_utils/utils.cpp
    src/filter_fx/filter_buffers.cpp
    src/filter_fx/filter_buffers_service.cpp
    src/filter_fx/filter_settings_service.cpp
    src/filter_fx/filter_zoom_vector.cpp
)

set(GoomUtils_modules
    src/utils/graphics/bezier_drawer.cppm
    src/utils/graphics/blend2d_to_goom.cppm
    src/utils/graphics/blend2d_utils.cppm
    src/utils/graphics/camera.cppm
    src/utils/graphics/image_bitmaps.cppm
    src/utils/graphics/line_clipper.cppm
    src/utils/graphics/pixel_blend.cppm
    src/utils/graphics/pixel_utils.cppm
    src/utils/graphics/point_utils.cppm
    src/utils/graphics/small_image_bitmaps.cppm
    src/utils/graphics/test_patterns.cppm
    src/utils/math/rand/rand_gen.cppm
    src/utils/math/rand/randutils.cppm
    src/utils/math/damping_functions.cppm
    src/utils/math/goom_rand.cppm
    src/utils/math/incremented_values.cppm
    src/utils/math/misc.cppm
    src/utils/math/parametric_functions2d.cppm
    src/utils/math/paths.cppm
    src/utils/math/t_values.cppm
    src/utils/math/transform2d.cppm
    src/utils/text/drawable_text.cppm
    src/utils/array_utils.cppm
    src/utils/buffer_saver.cppm
    src/utils/build_time.cppm
    src/utils/date_utils.cppm
    src/utils/debugging_logger.cppm
    src/utils/enum_utils.cppm
    src/utils/format_utils.cppm
    src/utils/goom_time.cppm
    src/utils/name_value_pairs.cppm
    src/utils/parallel_utils.cppm
    src/utils/step_speed.cppm
    src/utils/stopwatch.cppm
    src/utils/strutils.cppm
    src/utils/thread_pool.cppm
    src/utils/timer.cppm
)

set(GoomUtils_source_files
    src/utils/graphics/bezier_drawer.cpp
    src/utils/graphics/blend2d_to_goom.cpp
    src/utils/graphics/blend2d_utils.cpp
    src/utils/graphics/camera.cpp
    src/utils/graphics/image_bitmaps.cpp
    src/utils/graphics/small_image_bitmaps.cpp
    src/utils/graphics/stb_image.h
    src/utils/graphics/test_patterns.cpp
    src/utils/math/rand/rand_gen.cpp
    src/utils/math/rand/xoshiro.hpp
    src/utils/math/parametric_functions2d.cpp
    src/utils/math/paths.cpp
    src/utils/text/drawable_text.cpp
    src/utils/timer.cpp
)

set(GoomVisualFx_modules
    src/visual_fx/circles/bitmap_getter.cppm
    src/visual_fx/circles/circle.cppm
    src/visual_fx/circles/circle_params_builder.cppm
    src/visual_fx/circles/circles.cppm
    src/visual_fx/circles/dot_diameters.cppm
    src/visual_fx/circles/dot_drawer.cppm
    src/visual_fx/circles/dot_paths.cppm
    src/visual_fx/circles/helper.cppm
    src/visual_fx/circles_fx.cppm
    src/visual_fx/flying_stars/star_colors.cppm
    src/visual_fx/flying_stars/star_colors_maker.cppm
    src/visual_fx/flying_stars/star_drawer.cppm
    src/visual_fx/flying_stars/star_maker.cppm
    src/visual_fx/flying_stars/star_types_container.cppm
    src/visual_fx/flying_stars/stars.cppm
    src/visual_fx/flying_stars_fx.cppm
    src/visual_fx/fx_utils/dot_drawer.cppm
    src/visual_fx/fx_utils/lines.cppm
    src/visual_fx/fx_utils/random_pixel_blender.cppm
    src/visual_fx/ifs/colorizer.cppm
    src/visual_fx/ifs/fractal.cppm
    src/visual_fx/ifs/fractal_hits.cppm
    src/visual_fx/ifs/ifs_points.cppm
    src/visual_fx/ifs/ifs_types.cppm
    src/visual_fx/ifs/low_density_blurrer.cppm
    src/visual_fx/ifs/similitudes.cppm
    src/visual_fx/l_systems/l_system.cppm
    src/visual_fx/l_systems/lsys_colors.cppm
    src/visual_fx/l_systems/lsys_draw.cppm
    src/visual_fx/l_systems/lsys_geom.cppm
    src/visual_fx/l_systems/lsys_paths.cppm
    src/visual_fx/l_systems/line_drawer_manager.cppm
    src/visual_fx/lines/line_morph.cppm
    src/visual_fx/lines/line_types.cppm
    src/visual_fx/particles/attractor_effect.cppm
    src/visual_fx/particles/fountain_effect.cppm
    src/visual_fx/particles/tunnel_effect.cppm
    src/visual_fx/raindrops/raindrops.cppm
    src/visual_fx/raindrops/raindrop_positions.cppm
    src/visual_fx/shaders/high_contrast.cppm
    src/visual_fx/shaders/hue_shift_lerper.cppm
    src/visual_fx/shaders/shader_object_lerper.cppm
    src/visual_fx/shapes/bezier_flower.cppm
    src/visual_fx/shapes/shape_parts.cppm
    src/visual_fx/shapes/shape_paths.cppm
    src/visual_fx/shapes/shapes.cppm
    src/visual_fx/tentacles/circles_tentacle_layout.cppm
    src/visual_fx/tentacles/tentacle_driver.cppm
    src/visual_fx/tentacles/tentacle_plotter.cppm
    src/visual_fx/tentacles/tentacle2d.cppm
    src/visual_fx/tentacles/tentacle3d.cppm
    src/visual_fx/tubes/tube_data.cppm
    src/visual_fx/tubes/tubes.cppm
    src/visual_fx/circles_fx.cppm
    src/visual_fx/flying_stars_fx.cppm
    src/visual_fx/fx_helper.cppm
    src/visual_fx/fx_utils.cppm
    src/visual_fx/goom_dots_fx.cppm
    src/visual_fx/goom_visual_fx.cppm
    src/visual_fx/ifs_dancers_fx.cppm
    src/visual_fx/image_fx.cppm
    src/visual_fx/l_system_fx.cppm
    src/visual_fx/lines_fx.cppm
    src/visual_fx/particles_fx.cppm
    src/visual_fx/raindrops_fx.cppm
    src/visual_fx/shader_fx.cppm
    src/visual_fx/shapes_fx.cppm
    src/visual_fx/tentacles_fx.cppm
    src/visual_fx/tubes_fx.cppm
)

set(GoomVisualFx_source_files
    src/visual_fx/particles/attractor_effect.cpp
    src/visual_fx/particles/fountain_effect.cpp
    src/visual_fx/particles/tunnel_effect.cpp
    src/visual_fx/circles_fx.cpp
    src/visual_fx/flying_stars_fx.cpp
    src/visual_fx/goom_dots_fx.cpp
    src/visual_fx/ifs_dancers_fx.cpp
    src/visual_fx/image_fx.cpp
    src/visual_fx/l_system_fx.cpp
    src/visual_fx/lines_fx.cpp
    src/visual_fx/particles_fx.cpp
    src/visual_fx/raindrops_fx.cpp
    src/visual_fx/shader_fx.cpp
    src/visual_fx/shapes_fx.cpp
    src/visual_fx/tentacles_fx.cpp
    src/visual_fx/tubes_fx.cpp
)

add_library(goom
            STATIC
            ${LSys_source_files}
            ${Particles_source_files}
            ${GoomLib_source_files}
            ${GoomColor_source_files}
            ${GoomControl_source_files}
            ${GoomDraw_source_files}
            ${GoomFilterFx_source_files}
            ${GoomUtils_source_files}
            ${GoomVisualFx_source_files}
)

add_library(goom::lib ALIAS goom)

target_sources(goom
               PUBLIC
               FILE_SET public_modules TYPE CXX_MODULES FILES
               ${GoomLib_modules}
)
target_sources(goom
               PUBLIC  # Seems like there is not way (yet) to make this PRIVATE
               FILE_SET private_modules TYPE CXX_MODULES FILES
               ${LSys_modules}
               ${Particles_modules}
               ${GoomColor_modules}
               ${GoomControl_modules}
               ${GoomDraw_modules}
               ${GoomFilterFx_modules}
               ${GoomUtils_modules}
               ${GoomVisualFx_modules}
               src/plugin_info.cppm
)

target_link_libraries(goom
                      INTERFACE
                      libs::vivid
                      blend2d::blend2d
)

target_include_directories(goom
                           PUBLIC
                           ${GOOM_LIBS_DIR}/src/goom/include
                           PRIVATE
                           ${GOOM_LIBS_DIR}/src/goom/src/utils
                           ${GOOM_LIBS_DIR}/src/goom/src
)

target_include_directories(goom
                           SYSTEM PUBLIC
                           ${GOOM_LIBS_DIR}/lib/vivid
                           ${GOOM_LIBS_DIR}/src/vivid/include
                           ${GOOM_LIBS_DIR}/lib/mdspan
                           ${GOOM_LIBS_DIR}/lib/perlin_noise
                           ${magic_enum_INCLUDE_DIR}
                           ${GLM_INCLUDE_DIR}
                           PRIVATE
                           ${GOOM_LIBS_DIR}/src/bezier/include
                           ${LSys_SOURCE_DIR}/include/lsys
                           ${blend2d_SOURCE_DIR}/src
                           ${FREETYPE_INCLUDE_DIRS}
)

vis_goom_pp_set_project_warnings(vis_goom_pp_WARNINGS_AS_ERRORS goom)

set_target_properties(goom
                      PROPERTIES
                      POSITION_INDEPENDENT_CODE ${POS_INDEP_CODE}
)

get_target_property(goom_compile_options goom COMPILE_OPTIONS)
get_target_property(goom_compile_features goom COMPILE_FEATURES)

message(STATUS "Goom: C++ standard               = \"${CMAKE_CXX_STANDARD}\".")
message(STATUS "Goom: C++ compile features       = \"${goom_compile_features}\".")
message(STATUS "Goom: C++ compile options        = \"${goom_compile_options}\".")
message(STATUS "Goom: SOME_WARNINGS_OFF_FOR_LSYS = \"${SOME_WARNINGS_OFF_FOR_LSYS}\".")
message(STATUS "Goom: GOOM_LIBS_DIR              = \"${GOOM_LIBS_DIR}\".")
message(STATUS "Goom: goom_SOURCE_DIR            = \"${goom_SOURCE_DIR}\".")
message(STATUS "Goom: blend2d_SOURCE_DIR         = \"${blend2d_SOURCE_DIR}\".")
message(STATUS "Goom: GLM_INCLUDE_DIR            = \"${GLM_INCLUDE_DIR}\".")
message(STATUS "Goom: FREETYPE_INCLUDE_DIRS      = \"${FREETYPE_INCLUDE_DIRS}\".")
message(STATUS "Goom: magic_enum_INCLUDE_DIR     = \"${magic_enum_INCLUDE_DIR}\".")
