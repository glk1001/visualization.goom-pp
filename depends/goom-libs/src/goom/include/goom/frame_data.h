#pragma once

#include "goom_graphic.h"
#include "point2d.h"

#include <span> // NOLINT(misc-include-cleaner): Waiting for C++20.

namespace GOOM
{

struct FilterPosArrays
{
  // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
  std_spn::span<Point2dFlt> filterSrcePos{};
  bool filterSrcePosNeedsUpdating = false;
  // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
  std_spn::span<Point2dFlt> filterDestPos{};
  bool filterDestPosNeedsUpdating = false;
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
  float filterPosBuffersLerpFactor = 0.0F;
  float brightness                 = 1.0F;
  float hueShift                   = 0.0F;
  float chromaFactor               = 1.0F;
  float baseColorMultiplier        = 1.0F;
};
struct FrameData
{
  MiscData miscData{};
  FilterPosArrays filterPosArrays{};
  ImageArrays imageArrays{};
};

} // namespace GOOM
