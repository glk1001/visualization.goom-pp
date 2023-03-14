#include "star_colors.h"

#include "color/color_adjustment.h"
#include "color/color_maps.h"
#include "color/color_utils.h"
#include "goom_config.h"

namespace GOOM::VISUAL_FX::FLYING_STARS
{

using COLOR::GetLightenedColor;
using COLOR::IColorMap;
using DRAW::GetLowColor;
using DRAW::GetMainColor;
using DRAW::MultiplePixels;

StarColors::StarColors(const std::shared_ptr<const ColorProperties>& colorProperties,
                       const float withinClusterT) noexcept
  : m_colorProperties{colorProperties},
    m_withinClusterMainColor{
        m_colorProperties->colorMapsSet.dominantMainColormap->GetColor(withinClusterT)},
    m_withinClusterLowColor{
        m_colorProperties->colorMapsSet.dominantLowColormap->GetColor(withinClusterT)}
{
}

auto StarColors::GetMixedColors(const MixedColorsParams& mixedColorsParams) const noexcept
    -> DRAW::MultiplePixels
{
  MultiplePixels starColors;

  switch (m_colorProperties->colorMode)
  {
    case ColorMode::SINE_MIX_COLORS:
      starColors = GetSineMixColors();
      break;
    case ColorMode::MIX_COLORS:
      starColors = GetColors(mixedColorsParams.lengthT);
      break;
    case ColorMode::REVERSE_MIX_COLORS:
      starColors = GetReversedMixColors(mixedColorsParams.lengthT);
      break;
    default:
      FailFast();
  }

  return GetFinalMixedColors(mixedColorsParams, starColors);
}

inline auto StarColors::GetFinalMixedColors(const MixedColorsParams& mixedColorsParams,
                                            const MultiplePixels& colors) const noexcept
    -> MultiplePixels
{
  const auto tMix = GetFinalTMix(mixedColorsParams.lengthT);

  const auto mixedMainColor = GetColorCorrection(
      mixedColorsParams.brightness,
      IColorMap::GetColorMix(m_withinClusterMainColor, GetMainColor(colors), tMix));

  static constexpr auto LIGHTEN_POWER = 10.0F;
  const auto mixedLowColor            = GetLightenedColor(
      IColorMap::GetColorMix(m_withinClusterLowColor, GetLowColor(colors), tMix), LIGHTEN_POWER);

  if (m_colorProperties->similarLowColors)
  {
    return {mixedMainColor, mixedLowColor};
  }

  static constexpr auto MAIN_LOW_MIX_T = 0.4F;
  const auto remixedLowColor =
      GetColorCorrection(mixedColorsParams.brightness,
                         IColorMap::GetColorMix(mixedMainColor, mixedLowColor, MAIN_LOW_MIX_T));

  return {mixedMainColor, remixedLowColor};
}

inline auto StarColors::GetFinalTMix(const float lengthT) const noexcept -> float
{
  static constexpr auto MIN_MIX = 0.2F;
  static constexpr auto MAX_MIX = 0.8F;
  const auto tMix               = STD20::lerp(MIN_MIX, MAX_MIX, lengthT);

  if (m_colorProperties->reverseWithinClusterMix)
  {
    return 1.0F - tMix;
  }

  return tMix;
}

inline auto StarColors::GetColors(const float t) const noexcept -> MultiplePixels
{
  return {m_colorProperties->colorMapsSet.currentMainColorMap->GetColor(t),
          m_colorProperties->colorMapsSet.currentLowColorMap->GetColor(t)};
}

inline auto StarColors::GetReversedMixColors(const float t) const noexcept -> MultiplePixels
{
  return GetColors(1.0F - t);
}

inline auto StarColors::GetSineMixColors() const noexcept -> MultiplePixels
{
  static constexpr auto FREQ         = 20.0F;
  static constexpr auto T_MIX_FACTOR = 0.5F;
  static constexpr auto T_STEP       = 0.1F;
  static auto s_t                    = 0.0F;

  const auto tSin = T_MIX_FACTOR * (1.0F + std::sin(FREQ * s_t));

  auto starColors =
      MultiplePixels{m_colorProperties->colorMapsSet.currentMainColorMap->GetColor(tSin),
                     m_colorProperties->colorMapsSet.currentLowColorMap->GetColor(tSin)};

  s_t += T_STEP;

  return starColors;
}

inline auto StarColors::GetColorCorrection(const float brightness,
                                           const Pixel& color) const noexcept -> Pixel
{
  return m_colorAdjust.GetAdjustment(brightness, color);
}

} // namespace GOOM::VISUAL_FX::FLYING_STARS
