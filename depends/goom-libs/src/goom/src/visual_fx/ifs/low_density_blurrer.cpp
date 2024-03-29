#include "low_density_blurrer.h"

#include "color/color_maps.h"
#include "colorizer.h"
#include "draw/goom_draw.h"
#include "fractal.h"
#include "goom/goom_graphic.h"
#include "goom/point2d.h"
#include "utils/graphics/image_bitmaps.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace GOOM::VISUAL_FX::IFS
{

using COLOR::ColorMaps;
using COLOR::GetColorAverage;
using DRAW::IGoomDraw;
using DRAW::MultiplePixels;
using UTILS::GRAPHICS::ImageBitmap;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::Sq;
using UTILS::MATH::U_HALF;

LowDensityBlurrer::LowDensityBlurrer(IGoomDraw& draw,
                                     const IGoomRand& goomRand,
                                     const uint32_t width,
                                     const Colorizer& colorizer,
                                     const SmallImageBitmaps& smallBitmaps) noexcept
  : m_draw{&draw},
    m_goomRand{&goomRand},
    m_width{width},
    m_smallBitmaps{&smallBitmaps},
    m_colorizer{&colorizer}
{
}

auto LowDensityBlurrer::SetWidth(const uint32_t val) noexcept -> void
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4389) // '!=' mismatch. Not sure why?
#endif
  static constexpr auto VALID_WIDTHS = std::array{3, 5, 7};
  USED_FOR_DEBUGGING(VALID_WIDTHS);
  Expects(std::find(cbegin(VALID_WIDTHS), cend(VALID_WIDTHS), val) != cend(VALID_WIDTHS));
#ifdef _MSC_VER
#pragma warning(pop)
#endif

  m_width        = val;
  m_widthSquared = Sq(static_cast<size_t>(m_width));
}

auto LowDensityBlurrer::SetColorMode(const BlurrerColorMode colorMode) noexcept -> void
{
  m_colorMode = colorMode;

  static constexpr auto PROB_USE_BITMAPS = 0.1F;
  m_currentImageBitmap = GetImageBitmap(m_goomRand->ProbabilityOf(PROB_USE_BITMAPS));
}

auto LowDensityBlurrer::GetImageBitmap(const bool useBitmaps) const noexcept -> const ImageBitmap*
{
  if (!useBitmaps)
  {
    return nullptr;
  }

  static constexpr auto MIN_RES = 3U;
  static constexpr auto MAX_RES = 7U;
  const auto bitmapRes          = m_goomRand->GetRandInRange(MIN_RES, MAX_RES);

  return &m_smallBitmaps->GetImageBitmap(SmallImageBitmaps::ImageNames::SPHERE, bitmapRes);
}

auto LowDensityBlurrer::DoBlur(std::vector<IfsPoint>& lowDensityPoints,
                               const uint32_t maxLowDensityCount) noexcept -> void
{
  SetPointColors(lowDensityPoints, maxLowDensityCount);
  DrawPoints(lowDensityPoints);
}

inline auto LowDensityBlurrer::SetPointColors(std::vector<IfsPoint>& lowDensityPoints,
                                              const uint32_t maxLowDensityCount) const noexcept
    -> void
{
  const auto logMaxLowDensityCount = std::log(static_cast<float>(maxLowDensityCount));

  const auto tStep     = 1.0F / static_cast<float>(lowDensityPoints.size());
  const auto halfWidth = U_HALF * m_width;
  auto t               = 0.0F;
  for (auto& point : lowDensityPoints)
  {
    if ((point.GetX() < halfWidth) or (point.GetY() < halfWidth) or
        (point.GetX() >= (m_draw->GetDimensions().GetWidth() - halfWidth)) or
        (point.GetY() >= (m_draw->GetDimensions().GetHeight() - halfWidth)))
    {
      point.SetCount(0); // just signal that no need to set buff
      continue;
    }

    point.SetColor(GetPointColor(point, t, GetNeighbours(point), logMaxLowDensityCount));

    t += tStep;
  }
}

auto LowDensityBlurrer::GetNeighbours(const IfsPoint& point) const noexcept -> std::vector<Pixel>
{
  auto neighbours = std::vector<Pixel>(m_widthSquared);

  const auto neighX0 = static_cast<int32_t>(point.GetX() - (m_width / 2));
  auto neighY        = static_cast<int32_t>(point.GetY() - (m_width / 2));
  auto n             = 0U;

  for (auto i = 0U; i < m_width; ++i)
  {
    auto neighX = neighX0;
    for (auto j = 0U; j < m_width; ++j)
    {
      neighbours[n] = m_draw->GetPixel({neighX, neighY});
      ++n;
      ++neighX;
    }
    ++neighY;
  }

  return neighbours;
}

inline auto LowDensityBlurrer::DrawPoints(const std::vector<IfsPoint>& lowDensityPoints) noexcept
    -> void
{
  std::for_each(cbegin(lowDensityPoints),
                cend(lowDensityPoints),
                [this](const auto& point)
                {
                  if (point.GetCount() != 0)
                  {
                    DrawPoint(point);
                  }
                });
}

inline auto LowDensityBlurrer::DrawPoint(const IfsPoint& point) noexcept -> void
{
  const auto pt = GetPoint2dInt(point.GetX(), point.GetY());

  if (nullptr == m_currentImageBitmap)
  {
    m_pixelDrawer.DrawPixels(pt, MultiplePixels{point.GetColor(), point.GetColor()});
  }
  else
  {
    const auto getColor = [&point]([[maybe_unused]] const Point2dInt& bitmapPoint,
                                   [[maybe_unused]] const Pixel& bgnd) { return point.GetColor(); };
    m_bitmapDrawer.Bitmap(pt, *m_currentImageBitmap, {getColor, getColor});
  }
}

auto LowDensityBlurrer::GetPointColor(const IfsPoint& point,
                                      const float t,
                                      const std::vector<Pixel>& neighbours,
                                      const float logMaxLowDensityCount) const noexcept -> Pixel
{
  const auto logAlpha =
      point.GetCount() <= 1
          ? 1.0F
          : (std::log(static_cast<float>(point.GetCount())) / logMaxLowDensityCount);

  const auto brightness = GetBrightness();

  Pixel pointColor{};
  switch (m_colorMode)
  {
    using enum BlurrerColorMode;

    case SINGLE_NO_NEIGHBOURS:
      pointColor = m_singleColor;
      break;
    case SINGLE_WITH_NEIGHBOURS:
      pointColor = ColorMaps::GetColorMix(
          m_singleColor, GetColorAverage(neighbours.size(), neighbours), m_neighbourMixFactor);
      break;
    case SIMI_NO_NEIGHBOURS:
      pointColor = point.GetSimi()->GetColor();
      break;
    case SIMI_WITH_NEIGHBOURS:
    {
      const auto simiColor = point.GetSimi()->GetColor();
      const auto mixedPointColor =
          GetMixedPointColor(simiColor, point, neighbours, brightness, logAlpha);
      pointColor = mixedPointColor;
      break;
    }
    case SMOOTH_NO_NEIGHBOURS:
      pointColor = point.GetSimi()->GetColorMap().GetColor(t);
      break;
    case SMOOTH_WITH_NEIGHBOURS:
    {
      const auto simiSmoothColor = point.GetSimi()->GetColorMap().GetColor(t);
      const auto mixedPointColor =
          GetMixedPointColor(simiSmoothColor, point, neighbours, brightness, logAlpha);
      pointColor = mixedPointColor;
      break;
    }
  }

  return m_colorAdjust.GetAdjustment(brightness * logAlpha, pointColor);
}

inline auto LowDensityBlurrer::GetBrightness() const noexcept -> float
{
  static constexpr auto NO_NEIGHBOUR_BRIGHTNESS = 1.5F;
  static constexpr auto NEIGHBOUR_BRIGHTNESS    = 3.1F;
  static constexpr auto BITMAP_BRIGHTNESS_CUT   = 0.5F;

  const auto brightness = m_currentImageBitmap == nullptr ? 1.0F : BITMAP_BRIGHTNESS_CUT;

  switch (m_colorMode)
  {
    using enum BlurrerColorMode;

    case SINGLE_NO_NEIGHBOURS:
    case SIMI_NO_NEIGHBOURS:
      return brightness * NO_NEIGHBOUR_BRIGHTNESS;
    case SINGLE_WITH_NEIGHBOURS:
    case SIMI_WITH_NEIGHBOURS:
    case SMOOTH_NO_NEIGHBOURS:
    case SMOOTH_WITH_NEIGHBOURS:
      return brightness * NEIGHBOUR_BRIGHTNESS;
  }
}

inline auto LowDensityBlurrer::GetMixedPointColor(const Pixel& baseColor,
                                                  const IfsPoint& point,
                                                  const std::vector<Pixel>& neighbours,
                                                  const float brightness,
                                                  const float logAlpha) const noexcept -> Pixel
{
  const auto fx = static_cast<float>(point.GetX()) / m_draw->GetDimensions().GetFltWidth();
  const auto fy = static_cast<float>(point.GetY()) / m_draw->GetDimensions().GetFltHeight();

  const auto neighbourhoodAverageColor = GetColorAverage(neighbours.size(), neighbours);

  const auto baseAndNeighbourhoodMixedColor =
      ColorMaps::GetColorMix(baseColor, neighbourhoodAverageColor, m_neighbourMixFactor);

  return m_colorizer->GetMixedColor(
      baseAndNeighbourhoodMixedColor, point.GetCount(), {brightness, logAlpha, fx, fy});
}

} // namespace GOOM::VISUAL_FX::IFS
