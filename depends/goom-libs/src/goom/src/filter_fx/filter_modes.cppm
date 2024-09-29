module;

#include <string_view>

export module Goom.FilterFx.FilterModes;

import Goom.Utils.EnumUtils;
import Goom.Lib.GoomTypes;

export namespace GOOM::FILTER_FX
{

enum class GpuZoomFilterMode : UnderlyingEnumType
{
  GPU_NONE_MODE,
  GPU_AMULET_MODE,
  GPU_WAVE_MODE,
  GPU_VORTEX_MODE,
  GPU_REFLECTING_POOL_MODE,
  GPU_BEAUTIFUL_FIELD_MODE,
  GPU_UP_DOWN_MODE,
};

enum class ZoomFilterMode : UnderlyingEnumType
{
  AMULET_MODE = 0,
  COMPLEX_RATIONAL_MODE,
  CRYSTAL_BALL_MODE0,
  CRYSTAL_BALL_MODE1,
  DISTANCE_FIELD_MODE0,
  DISTANCE_FIELD_MODE1,
  DISTANCE_FIELD_MODE2,
  EXP_RECIPROCAL_MODE,
  FLOW_FIELD_MODE,
  HYPERCOS_MODE0,
  HYPERCOS_MODE1,
  HYPERCOS_MODE2,
  HYPERCOS_MODE3,
  IMAGE_DISPLACEMENT_MODE,
  JULIA_MODE,
  MOBIUS_MODE,
  NEWTON_MODE,
  NORMAL_MODE,
  PERLIN_NOISE_MODE,
  SCRUNCH_MODE,
  SPEEDWAY_MODE0,
  SPEEDWAY_MODE1,
  SPEEDWAY_MODE2,
  WATER_MODE,
  WAVE_SQ_DIST_ANGLE_EFFECT_MODE0,
  WAVE_SQ_DIST_ANGLE_EFFECT_MODE1,
  WAVE_ATAN_ANGLE_EFFECT_MODE0,
  WAVE_ATAN_ANGLE_EFFECT_MODE1,
  Y_ONLY_MODE,
};

auto GetFilterModeName(ZoomFilterMode filterMode) noexcept -> std::string_view;
auto GetGpuFilterModeName(GpuZoomFilterMode gpuFilterMode) noexcept -> std::string_view;

} // namespace GOOM::FILTER_FX

namespace GOOM::FILTER_FX
{

using UTILS::EnumMap;
using UTILS::NUM;

inline auto GetGpuFilterModeName(const GpuZoomFilterMode gpuFilterMode) noexcept -> std::string_view
{
  using enum GpuZoomFilterMode;
  static constexpr auto GPU_FILTER_MODE_NAMES = EnumMap<GpuZoomFilterMode, std::string_view>{{{
      {GPU_NONE_MODE, "GPU None"},
      {GPU_AMULET_MODE, "GPU Amulet"},
      {GPU_WAVE_MODE, "GPU Wave"},
      {GPU_VORTEX_MODE, "GPU Vortex"},
      {GPU_REFLECTING_POOL_MODE, "GPU Reflecting Pool"},
      {GPU_BEAUTIFUL_FIELD_MODE, "GPU Beautiful Field"},
      {GPU_UP_DOWN_MODE, "GPU Up Down"},
  }}};
  static_assert(GPU_FILTER_MODE_NAMES.size() == NUM<GpuZoomFilterMode>);

  return GPU_FILTER_MODE_NAMES[gpuFilterMode];
}

inline auto GetFilterModeName(const ZoomFilterMode filterMode) noexcept -> std::string_view
{
  using enum ZoomFilterMode;
  static constexpr auto FILTER_MODE_NAMES = EnumMap<ZoomFilterMode, std::string_view>{{{
      {AMULET_MODE, "Amulet"},
      {COMPLEX_RATIONAL_MODE, "Complex Rational"},
      {CRYSTAL_BALL_MODE0, "Crystal Ball Mode 0"},
      {CRYSTAL_BALL_MODE1, "Crystal Ball Mode 1"},
      {DISTANCE_FIELD_MODE0, "Distance Field Mode 0"},
      {DISTANCE_FIELD_MODE1, "Distance Field Mode 1"},
      {DISTANCE_FIELD_MODE2, "Distance Field Mode 2"},
      {EXP_RECIPROCAL_MODE, "Exp Reciprocal"},
      {FLOW_FIELD_MODE, "Flow Field"},
      {HYPERCOS_MODE0, "Hypercos Mode 0"},
      {HYPERCOS_MODE1, "Hypercos Mode 1"},
      {HYPERCOS_MODE2, "Hypercos Mode 2"},
      {HYPERCOS_MODE3, "Hypercos Mode 3"},
      {IMAGE_DISPLACEMENT_MODE, "Image Displacement"},
      {JULIA_MODE, "Julia"},
      {MOBIUS_MODE, "Mobius"},
      {NEWTON_MODE, "Newton"},
      {NORMAL_MODE, "Normal"},
      {PERLIN_NOISE_MODE, "Perlin Noise"},
      {SCRUNCH_MODE, "Scrunch"},
      {SPEEDWAY_MODE0, "Speedway Mode 0"},
      {SPEEDWAY_MODE1, "Speedway Mode 1"},
      {SPEEDWAY_MODE2, "Speedway Mode 2"},
      {WATER_MODE, "Water"},
      {WAVE_SQ_DIST_ANGLE_EFFECT_MODE0, "Wave Sq Dist Mode 0"},
      {WAVE_SQ_DIST_ANGLE_EFFECT_MODE1, "Wave Sq Dist Mode 1"},
      {WAVE_ATAN_ANGLE_EFFECT_MODE0, "Wave Atan Mode 0"},
      {WAVE_ATAN_ANGLE_EFFECT_MODE1, "Wave Atan Mode 1"},
      {Y_ONLY_MODE, "Y Only"},
  }}};
  static_assert(FILTER_MODE_NAMES.size() == NUM<ZoomFilterMode>);

  return FILTER_MODE_NAMES[filterMode];
}

} // namespace GOOM::FILTER_FX
