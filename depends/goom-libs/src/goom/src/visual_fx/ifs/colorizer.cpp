#include "colorizer.h"

#include "color/color_maps.h"
#include "color/random_color_maps.h"
#include "color/random_color_maps_groups.h"
#include "goom/goom_config.h"
#include "goom/goom_graphic.h"
#include "math/goom_rand_base.h"
#include "visual_fx/ifs_dancers_fx.h"

#include <cmath>
#include <cstdint>
#include <set>
#include <utility>

namespace GOOM::VISUAL_FX::IFS
{

using COLOR::ColorMaps;
using COLOR::GetUnweightedRandomColorMaps;
using COLOR::WeightedRandomColorMaps;
using UTILS::MATH::IGoomRand;
using VISUAL_FX::IfsDancersFx;

static constexpr auto MAP_COLORS_WEIGHT            = 20.0F;
static constexpr auto MEGA_MAP_COLOR_CHANGE_WEIGHT = 15.0F;
static constexpr auto MIX_COLORS_WEIGHT            = 20.0F;
static constexpr auto MEGA_MIX_COLOR_CHANGE_WEIGHT = 15.0F;
static constexpr auto REVERSE_MIX_COLORS_WEIGHT    = 20.0F;
static constexpr auto SINGLE_COLORS_WEIGHT         = 05.0F;
static constexpr auto SINE_MIX_COLORS_WEIGHT       = 05.0F;
static constexpr auto SINE_MAP_COLORS_WEIGHT       = 05.0F;

Colorizer::Colorizer(const IGoomRand& goomRand, const PixelChannelType defaultAlpha)
  : m_goomRand{&goomRand},
    m_colorMaps{GetUnweightedRandomColorMaps(*m_goomRand, defaultAlpha)},
    m_colorModeWeights{
        *m_goomRand,
        {
            { IfsDancersFx::ColorMode::MAP_COLORS,            MAP_COLORS_WEIGHT },
            { IfsDancersFx::ColorMode::MEGA_MAP_COLOR_CHANGE, MEGA_MAP_COLOR_CHANGE_WEIGHT },
            { IfsDancersFx::ColorMode::MIX_COLORS,            MIX_COLORS_WEIGHT },
            { IfsDancersFx::ColorMode::MEGA_MIX_COLOR_CHANGE, MEGA_MIX_COLOR_CHANGE_WEIGHT },
            { IfsDancersFx::ColorMode::REVERSE_MIX_COLORS,    REVERSE_MIX_COLORS_WEIGHT },
            { IfsDancersFx::ColorMode::SINGLE_COLORS,         SINGLE_COLORS_WEIGHT },
            { IfsDancersFx::ColorMode::SINE_MIX_COLORS,       SINE_MIX_COLORS_WEIGHT },
            { IfsDancersFx::ColorMode::SINE_MAP_COLORS,       SINE_MAP_COLORS_WEIGHT },
        }
    }
{
}

auto Colorizer::SetWeightedColorMaps(const WeightedRandomColorMaps& weightedColorMaps) -> void
{
  m_colorMaps = weightedColorMaps;
}

auto Colorizer::InitColorMaps() -> void
{
  const auto& colorMapTypes = GetNextColorMapTypes();

  m_mixerColorMapPtr1 = m_colorMaps.GetRandomColorMapSharedPtr(colorMapTypes);
  m_mixerColorMapPtr2 = m_colorMaps.GetRandomColorMapSharedPtr(colorMapTypes);

  m_previousMixerColorMapPtr1 = m_mixerColorMapPtr1;
  m_previousMixerColorMapPtr2 = m_mixerColorMapPtr2;
}

auto Colorizer::UpdateMixerMaps() -> void
{
  const auto& colorMapTypes = GetNextColorMapTypes();

  m_previousMixerColorMapPtr1 = m_mixerColorMapPtr1;
  m_mixerColorMapPtr1         = m_colorMaps.GetRandomColorMapSharedPtr(colorMapTypes);

  m_previousMixerColorMapPtr2 = m_mixerColorMapPtr2;
  m_mixerColorMapPtr2         = m_colorMaps.GetRandomColorMapSharedPtr(colorMapTypes);
}

inline auto Colorizer::GetNextColorMapTypes() const noexcept
    -> const std::set<WeightedRandomColorMaps::ColorMapTypes>&
{
  static constexpr auto PROB_NO_EXTRA_COLOR_MAP_TYPES = 0.9F;
  return m_goomRand->ProbabilityOf(PROB_NO_EXTRA_COLOR_MAP_TYPES)
             ? WeightedRandomColorMaps::GetNoColorMapsTypes()
             : WeightedRandomColorMaps::GetAllColorMapsTypes();
}

auto Colorizer::ChangeColorMode() -> void
{
  if (m_forcedColorMode != IfsDancersFx::ColorMode::_NULL)
  {
    m_colorMode = m_forcedColorMode;
  }
  else
  {
    m_colorMode = GetNextColorMode();
  }
}

inline auto Colorizer::GetNextColorMode() const -> IfsDancersFx::ColorMode
{
  return m_colorModeWeights.GetRandomWeighted();
}

auto Colorizer::ChangeColorMaps() -> void
{
  UpdateMixerMaps();

  m_colorMapChangeCompleted =
      m_goomRand->GetRandInRange(MIN_COLOR_MAP_CHANGE_COMPLETED, MAX_COLOR_MAP_CHANGE_COMPLETED);
  m_tAwayFromBaseColor =
      m_goomRand->GetRandInRange(MIN_T_AWAY_FROM_BASE_COLOR, MAX_T_AWAY_FROM_BASE_COLOR);
  m_countSinceColorMapChange = m_colorMapChangeCompleted;
}

auto Colorizer::GetMixedColor(const Pixel& baseColor,
                              const uint32_t hitCount,
                              const MixProperties& mixProperties) const -> Pixel
{
  const auto logAlpha =
      m_maxHitCount <= 1 ? 1.0F : (std::log(static_cast<float>(hitCount)) / m_logMaxHitCount);

  const auto [mixColor, tBaseMix] = GetMixedColorInfo(baseColor, logAlpha, mixProperties);

  return m_colorAdjust.GetAdjustment(mixProperties.brightness * logAlpha,
                                     GetFinalMixedColor(baseColor, tBaseMix, mixColor));
}

auto Colorizer::GetMixedColorInfo(const Pixel& baseColor,
                                  const float logAlpha,
                                  const MixProperties& mixProperties) const
    -> std::pair<Pixel, float>
{
  switch (m_colorMode)
  {
    case IfsDancersFx::ColorMode::MAP_COLORS:
    case IfsDancersFx::ColorMode::MEGA_MAP_COLOR_CHANGE:
      return {GetNextMixerMapColor(
                  mixProperties.brightness * logAlpha, mixProperties.tX, mixProperties.tY),
              GetMapColorsTBaseMix()};

    case IfsDancersFx::ColorMode::MIX_COLORS:
    case IfsDancersFx::ColorMode::REVERSE_MIX_COLORS:
    case IfsDancersFx::ColorMode::MEGA_MIX_COLOR_CHANGE:
      return {GetNextMixerMapColor(mixProperties.tMix, mixProperties.tX, mixProperties.tY),
              1.0F - m_tAwayFromBaseColor};

    case IfsDancersFx::ColorMode::SINGLE_COLORS:
      return {baseColor, 1.0F - m_tAwayFromBaseColor};

    case IfsDancersFx::ColorMode::SINE_MIX_COLORS:
    case IfsDancersFx::ColorMode::SINE_MAP_COLORS:
      return {GetSineMixColor(mixProperties.tX, mixProperties.tY), 1.0F - m_tAwayFromBaseColor};

    default:
      FailFast();
  }
}

auto Colorizer::GetNextMixerMapColor(const float t, const float tX, const float tY) const -> Pixel
{
  Expects(m_mixerColorMapPtr1 != nullptr);
  Expects(m_mixerColorMapPtr2 != nullptr);
  Expects(m_previousMixerColorMapPtr1 != nullptr);
  Expects(m_previousMixerColorMapPtr2 != nullptr);

  const auto nextColor = ColorMaps::GetColorMix(
      m_mixerColorMapPtr1->GetColor(tX), m_mixerColorMapPtr2->GetColor(tY), t);
  if (0 == m_countSinceColorMapChange)
  {
    return nextColor;
  }

  const auto tTransition = static_cast<float>(m_countSinceColorMapChange) /
                           static_cast<float>(m_colorMapChangeCompleted);
  --m_countSinceColorMapChange;
  const auto prevNextColor = ColorMaps::GetColorMix(
      m_previousMixerColorMapPtr1->GetColor(tX), m_previousMixerColorMapPtr2->GetColor(tY), t);
  return ColorMaps::GetColorMix(nextColor, prevNextColor, tTransition);
}

inline auto Colorizer::GetMapColorsTBaseMix() const -> float
{
  if (m_colorMode == IfsDancersFx::ColorMode::MAP_COLORS)
  {
    return 1.0F - m_tAwayFromBaseColor;
  }

  static constexpr auto MIN_T_BASE_MIX = 0.3F;
  static constexpr auto MAX_T_BASE_MIX = 0.5F;
  return m_goomRand->GetRandInRange(MIN_T_BASE_MIX, MAX_T_BASE_MIX);
}

inline auto Colorizer::GetSineMixColor(const float tX, const float tY) const -> Pixel
{
  static constexpr auto INITIAL_FREQ = 20.0F;
  static constexpr auto T_MIX_FACTOR = 0.5F;
  static constexpr auto Z_STEP       = 0.1F;
  static const auto s_FREQ           = INITIAL_FREQ;
  static auto s_z                    = 0.0F;

  const auto mixColor =
      GetNextMixerMapColor(T_MIX_FACTOR * (1.0F + std::sin(s_FREQ * s_z)), tX, tY);

  s_z += Z_STEP;

  return mixColor;
}

inline auto Colorizer::GetFinalMixedColor(const Pixel& baseColor,
                                          const float tBaseMix,
                                          const Pixel& mixColor) const -> Pixel
{
  if (m_colorMode == IfsDancersFx::ColorMode::REVERSE_MIX_COLORS)
  {
    return ColorMaps::GetColorMix(mixColor, baseColor, tBaseMix);
  }

  return ColorMaps::GetColorMix(baseColor, mixColor, tBaseMix);
}

} // namespace GOOM::VISUAL_FX::IFS
