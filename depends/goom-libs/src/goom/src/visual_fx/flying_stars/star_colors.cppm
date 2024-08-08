module;

#include <cmath>

module Goom.VisualFx.FlyingStarsFx:StarColors;

import Goom.Color.ColorAdjustment;
import Goom.Color.ColorMaps;
import Goom.Color.ColorUtils;
import Goom.Draw.GoomDrawBase;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;

namespace GOOM::VISUAL_FX::FLYING_STARS
{

class StarColors
{
public:
  struct ColorMapsSet
  {
    COLOR::ConstColorMapSharedPtr currentMainColorMapPtr;
    COLOR::ConstColorMapSharedPtr currentLowColorMapPtr;
    COLOR::ConstColorMapSharedPtr dominantMainColorMapPtr;
    COLOR::ConstColorMapSharedPtr dominantLowColorMapPtr;
  };
  enum class ColorMode : UnderlyingEnumType
  {
    MIX_COLORS,
    REVERSE_MIX_COLORS,
    SINE_MIX_COLORS,
  };
  struct ColorProperties
  {
    ColorMapsSet colorMapsSet;
    ColorMode colorMode{};
    bool reverseWithinClusterMix{};
    bool similarLowColors{};
  };

  StarColors(const ColorProperties& colorProperties, float withinClusterT) noexcept;

  [[nodiscard]] auto GetColorProperties() const noexcept -> const ColorProperties&;
  [[nodiscard]] auto GetWithinClusterT() const noexcept -> float;

  struct MixedColorsParams
  {
    float brightness{};
    float lengthT{};
  };
  [[nodiscard]] auto GetMixedColors(const MixedColorsParams& mixedColorsParams) const noexcept
      -> DRAW::MultiplePixels;

private:
  ColorProperties m_colorProperties;
  float m_withinClusterT;
  Pixel m_withinClusterMainColor;
  Pixel m_withinClusterLowColor;

  static constexpr auto GAMMA = 1.4F;
  COLOR::ColorAdjustment m_colorAdjust{
      {.gamma = GAMMA, .alterChromaFactor = COLOR::ColorAdjustment::INCREASED_CHROMA_FACTOR}
  };
  [[nodiscard]] auto GetColorCorrection(float brightness,
                                        const Pixel& color) const noexcept -> Pixel;

  [[nodiscard]] auto GetColors(float t) const noexcept -> DRAW::MultiplePixels;
  [[nodiscard]] auto GetReversedMixColors(float t) const noexcept -> DRAW::MultiplePixels;
  [[nodiscard]] auto GetSineMixColors() const noexcept -> DRAW::MultiplePixels;
  [[nodiscard]] auto GetFinalMixedColors(const MixedColorsParams& mixedColorsParams,
                                         const DRAW::MultiplePixels& colors) const noexcept
      -> DRAW::MultiplePixels;
  [[nodiscard]] auto GetFinalTMix(float lengthT) const noexcept -> float;
};

} //namespace GOOM::VISUAL_FX::FLYING_STARS

namespace GOOM::VISUAL_FX::FLYING_STARS
{

inline auto StarColors::GetColorProperties() const noexcept -> const ColorProperties&
{
  return m_colorProperties;
}

inline auto StarColors::GetWithinClusterT() const noexcept -> float
{
  return m_withinClusterT;
}

} //namespace GOOM::VISUAL_FX::FLYING_STARS

namespace GOOM::VISUAL_FX::FLYING_STARS
{

using COLOR::ColorMaps;
using COLOR::GetLightenedColor;
using DRAW::GetLowColor;
using DRAW::GetMainColor;
using DRAW::MultiplePixels;

StarColors::StarColors(const ColorProperties& colorProperties, const float withinClusterT) noexcept
  : m_colorProperties{colorProperties},
    m_withinClusterT{withinClusterT},
    m_withinClusterMainColor{
        m_colorProperties.colorMapsSet.dominantMainColorMapPtr->GetColor(withinClusterT)},
    m_withinClusterLowColor{
        m_colorProperties.colorMapsSet.dominantLowColorMapPtr->GetColor(withinClusterT)}
{
}

auto StarColors::GetMixedColors(const MixedColorsParams& mixedColorsParams) const noexcept
    -> DRAW::MultiplePixels
{
  MultiplePixels starColors;

  switch (m_colorProperties.colorMode)
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
      ColorMaps::GetColorMix(m_withinClusterMainColor, GetMainColor(colors), tMix));

  static constexpr auto LIGHTEN_POWER = 10.0F;
  const auto mixedLowColor            = GetLightenedColor(
      ColorMaps::GetColorMix(m_withinClusterLowColor, GetLowColor(colors), tMix), LIGHTEN_POWER);

  if (m_colorProperties.similarLowColors)
  {
    return {.color1 = mixedMainColor, .color2 = mixedLowColor};
  }

  static constexpr auto MAIN_LOW_MIX_T = 0.4F;
  const auto remixedLowColor =
      GetColorCorrection(mixedColorsParams.brightness,
                         ColorMaps::GetColorMix(mixedMainColor, mixedLowColor, MAIN_LOW_MIX_T));

  return {.color1 = mixedMainColor, .color2 = remixedLowColor};
}

inline auto StarColors::GetFinalTMix(const float lengthT) const noexcept -> float
{
  static constexpr auto MIN_MIX = 0.2F;
  static constexpr auto MAX_MIX = 0.8F;
  const auto tMix               = std::lerp(MIN_MIX, MAX_MIX, lengthT);

  if (m_colorProperties.reverseWithinClusterMix)
  {
    return 1.0F - tMix;
  }

  return tMix;
}

inline auto StarColors::GetColors(const float t) const noexcept -> MultiplePixels
{
  return {.color1 = m_colorProperties.colorMapsSet.currentMainColorMapPtr->GetColor(t),
          .color2 = m_colorProperties.colorMapsSet.currentLowColorMapPtr->GetColor(t)};
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

  auto starColors = MultiplePixels{
      .color1 = m_colorProperties.colorMapsSet.currentMainColorMapPtr->GetColor(tSin),
      .color2 = m_colorProperties.colorMapsSet.currentLowColorMapPtr->GetColor(tSin)};

  s_t += T_STEP;

  return starColors;
}

inline auto StarColors::GetColorCorrection(const float brightness,
                                           const Pixel& color) const noexcept -> Pixel
{
  return m_colorAdjust.GetAdjustment(brightness, color);
}

} // namespace GOOM::VISUAL_FX::FLYING_STARS
