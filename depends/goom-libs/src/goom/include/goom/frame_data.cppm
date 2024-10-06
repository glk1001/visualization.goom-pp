module;

#include <cstdint>
#include <memory>
#include <span>

export module Goom.Lib.FrameData;

import Goom.FilterFx.GpuFilterEffects.GpuZoomFilterEffect;
import Goom.FilterFx.FilterModes;
import Goom.FilterFx.FilterSettings;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.GoomGraphic;
import Goom.Lib.Point2d;

using GOOM::FILTER_FX::GpuZoomFilterMode;
using GOOM::FILTER_FX::NormalizedCoords;
using GOOM::FILTER_FX::TextureWrapType;
using GOOM::FILTER_FX::GPU_FILTER_EFFECTS::IGpuParams;
using GOOM::UTILS::MATH::NumberRange;

export namespace GOOM
{

inline constexpr auto MIN_NORMALIZED_COORD   = NormalizedCoords::MIN_COORD;
inline constexpr auto MAX_NORMALIZED_COORD   = NormalizedCoords::MAX_COORD;
inline constexpr auto NORMALIZED_COORD_WIDTH = NormalizedCoords::COORD_WIDTH;

struct FilterPosArrays
{
  std::span<Point2dFlt> filterDestPos;
  float filterPosBuffersLerpFactor                 = 0.0F;
  static constexpr auto POS1_POS2_MIX_FREQ_RANGE   = NumberRange{0.001F, 0.010F};
  static constexpr auto DEFAULT_POS1_POS2_MIX_FREQ = POS1_POS2_MIX_FREQ_RANGE.max;
  float filterPos1Pos2FreqMixFreq                  = 0.0F;
  bool filterDestPosNeedsUpdating                  = false;
};
struct ImageArrays
{
  PixelBuffer mainImagePixelBuffer;
  bool mainImagePixelBufferNeedsUpdating = false;
  PixelBuffer lowImagePixelBuffer;
  bool lowImagePixelBufferNeedsUpdating = false;
};
struct MiscData
{
  TextureWrapType textureWrapType{};
  // TODO(glk) - blending params, contrast, other command/effects
  float brightness          = 1.0F;
  float hueShift            = 0.0F;
  float chromaFactor        = 1.0F;
  float baseColorMultiplier = 1.0F;
  float prevFrameTMix       = 0.0F;
  float gamma               = 1.0F;
  uint64_t goomTime         = 0U;
};
struct GpuFilterEffectData
{
  bool filterNeedsUpdating = false;
  GpuZoomFilterMode srceFilterMode{};
  GpuZoomFilterMode destFilterMode{};
  const IGpuParams* srceFilterParams = nullptr;
  const IGpuParams* destFilterParams = nullptr;
  IGpuParams::FilterTimingInfo filterTimingInfo{};
  float srceDestLerpFactor{};
  float gpuLerpFactor{};
  Point2dFlt midpoint{};
  float maxZoomAdjustment{};
};
struct FrameData
{
  GpuFilterEffectData* gpuFilterEffectData{};
  MiscData miscData{};
  FilterPosArrays filterPosArrays{};
  ImageArrays imageArrays{};
};

} // namespace GOOM

namespace GOOM
{

} // namespace GOOM
