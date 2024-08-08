module;

#include <array>
#include <blend2d.h> // NOLINT(misc-include-cleaner): Blend2d insists on this.
#include <blend2d/context.h>
#include <blend2d/gradient.h>
#include <blend2d/rgba.h>

module Goom.Utils.Graphics.Blend2dUtils;

import Goom.Color.ColorUtils;
import Goom.Draw.GoomDrawBase;
import Goom.Utils.Graphics.Blend2dToGoom;
import Goom.Lib.GoomGraphic;
import Goom.Lib.Point2d;

namespace GOOM::UTILS::GRAPHICS
{

using COLOR::GetBrighterColor;
using DRAW::GetLowColor;
using DRAW::GetMainColor;
using DRAW::MultiplePixels;

namespace
{

auto GetRadialGradient(const Point2dInt& centre,
                       const double radius,
                       const Pixel& color,
                       const float brightness) noexcept -> BLGradient
{
  auto radialGradient = BLGradient{BLRadialGradientValues(static_cast<double>(centre.x),
                                                          static_cast<double>(centre.y),
                                                          static_cast<double>(centre.x),
                                                          static_cast<double>(centre.y),
                                                          radius)};
  struct RadialStop
  {
    double offset;
    float brightnessFactor;
  };
  static constexpr auto RADIAL_STOPS = std::array{
      RadialStop{.offset = 0.0, .brightnessFactor = 0.25F},
      RadialStop{.offset = 0.5, .brightnessFactor = 0.50F},
      RadialStop{.offset = 1.0, .brightnessFactor = 1.00F},
  };
  for (const auto& stop : RADIAL_STOPS)
  {
    radialGradient.addStop(stop.offset,
                           BLRgba32(Blend2dToGoom::GetBlend2dColor(
                               GetBrighterColor(stop.brightnessFactor * brightness, color))));
  }

  return radialGradient;
}

} // namespace

auto FillCircleWithGradient(Blend2dContexts& blend2DContexts,
                            const MultiplePixels& colors,
                            const float brightness,
                            const Point2dInt& centre,
                            const double radius) noexcept -> void
{
  auto radialGradient = GetRadialGradient(centre, radius, GetMainColor(colors), brightness);
  blend2DContexts.mainBlend2dContext.fillCircle(
      static_cast<double>(centre.x), static_cast<double>(centre.y), radius, radialGradient);

  radialGradient = GetRadialGradient(centre, radius, GetLowColor(colors), brightness);
  blend2DContexts.mainBlend2dContext.fillCircle(
      static_cast<double>(centre.x), static_cast<double>(centre.y), radius, radialGradient);
}

} // namespace GOOM::UTILS::GRAPHICS
