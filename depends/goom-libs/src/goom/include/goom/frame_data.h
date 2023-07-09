#pragma once

#include "goom_graphic.h"
#include "point2d.h"

namespace GOOM
{

struct FilterPosArrays
{
  std_spn::span<Point2dFlt> filterSrcePos{};
  bool filterSrcePosNeedsUpdating = false;
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
  // TODO - blending params, contrast, other command/effects
  float lerpFactor          = 0.0F;
  float brightness          = 1.0F;
  float hueShift            = 0.0F;
  float chromaFactor        = 1.0F;
  float baseColorMultiplier = 1.0F;
};
struct FrameData
{
  MiscData miscData{};
  FilterPosArrays filterPosArrays{};
  ImageArrays imageArrays{};
};

} // namespace GOOM