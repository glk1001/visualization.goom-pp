#pragma once

#include "color/color_adjustment.h"
#include "color/color_maps.h"
#include "draw/goom_draw.h"
#include "goom_config.h"
#include "goom_graphic.h"

#include <memory>

namespace GOOM::VISUAL_FX::FLYING_STARS
{

class StarColors
{
public:
  struct ColorMapsSet
  {
    std::shared_ptr<const COLOR::IColorMap> currentMainColorMap{};
    std::shared_ptr<const COLOR::IColorMap> currentLowColorMap{};
    std::shared_ptr<const COLOR::IColorMap> dominantMainColormap{};
    std::shared_ptr<const COLOR::IColorMap> dominantLowColormap{};
  };
  enum class ColorMode
  {
    MIX_COLORS,
    REVERSE_MIX_COLORS,
    SINE_MIX_COLORS,
    _num // unused, and marks the enum end
  };
  struct ColorProperties
  {
    ColorMapsSet colorMapsSet;
    ColorMode colorMode{};
    bool reverseWithinClusterMix{};
    bool similarLowColors{};
  };

  StarColors(const std::shared_ptr<const ColorProperties>& colorProperties,
             float withinClusterT) noexcept;

  struct MixedColorsParams
  {
    float brightness{};
    float lengthT{};
  };
  [[nodiscard]] auto GetMixedColors(const MixedColorsParams& mixedColorsParams) const noexcept
      -> DRAW::MultiplePixels;

private:
  std::shared_ptr<const ColorProperties> m_colorProperties;
  Pixel m_withinClusterMainColor;
  Pixel m_withinClusterLowColor;

  static constexpr auto GAMMA = 1.0F / 2.0F;
  COLOR::ColorAdjustment m_colorAdjust{
      {GAMMA, COLOR::ColorAdjustment::INCREASED_CHROMA_FACTOR}
  };
  [[nodiscard]] auto GetColorCorrection(float brightness, const Pixel& color) const noexcept
      -> Pixel;

  [[nodiscard]] auto GetColors(float t) const noexcept -> DRAW::MultiplePixels;
  [[nodiscard]] auto GetReversedMixColors(float t) const noexcept -> DRAW::MultiplePixels;
  [[nodiscard]] auto GetSineMixColors() const noexcept -> DRAW::MultiplePixels;
  [[nodiscard]] auto GetFinalMixedColors(const MixedColorsParams& mixedColorsParams,
                                         const DRAW::MultiplePixels& colors) const noexcept
      -> DRAW::MultiplePixels;
  [[nodiscard]] auto GetFinalTMix(float lengthT) const noexcept -> float;
};

} //namespace GOOM::VISUAL_FX::FLYING_STARS
