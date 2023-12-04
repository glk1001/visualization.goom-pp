#pragma once

#include "goom_graphic.h"
#include "point2d.h"

#include <cstdint>
#include <span> // NOLINT(misc-include-cleaner): Waiting for C++20.

namespace GOOM
{

struct FilterPosArrays
{
  // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
  std_spn::span<Point2dFlt> filterDestPos{};
  float filterPosBuffersLerpFactor                 = 0.0F;
  static constexpr auto MIN_POS1_POS2_MIX_FREQ     = 0.001F;
  static constexpr auto MAX_POS1_POS2_MIX_FREQ     = 0.010F;
  static constexpr auto DEFAULT_POS1_POS2_MIX_FREQ = 0.01F;
  bool useFilterPosBlender                         = true;
  float filterPos1Pos2FreqMixFreq                  = 0.0F;
  bool filterDestPosNeedsUpdating                  = false;
};
struct ImageArrays
{
  GOOM::PixelBuffer mainImagePixelBuffer{};
  bool mainImagePixelBufferNeedsUpdating = false;
  GOOM::PixelBuffer lowImagePixelBuffer{};
  bool lowImagePixelBufferNeedsUpdating = false;
};
struct MiscData
{
  // TODO(glk) - blending params, contrast, other command/effects
  float brightness          = 1.0F;
  float hueShift            = 0.0F;
  float chromaFactor        = 1.0F;
  float baseColorMultiplier = 1.0F;
  float gamma               = 1.0F;
  uint64_t goomTime         = 0U;
};
struct FrameData
{
  MiscData miscData{};
  FilterPosArrays filterPosArrays{};
  ImageArrays imageArrays{};
};

} // namespace GOOM
