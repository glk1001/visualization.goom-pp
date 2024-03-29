#pragma once

#include "color/color_adjustment.h"
#include "color/color_maps.h"
#include "color/random_color_maps.h"
#include "goom/goom_config.h"
#include "goom/goom_graphic.h"
#include "utils/math/goom_rand_base.h"
#include "visual_fx/ifs_dancers_fx.h"

#include <cmath>
#include <cstdint>
#include <set>
#include <utility>

namespace GOOM::VISUAL_FX::IFS
{

class Colorizer
{
public:
  Colorizer(const UTILS::MATH::IGoomRand& goomRand, PixelChannelType defaultAlpha);

  [[nodiscard]] auto GetWeightedColorMaps() const -> const COLOR::WeightedRandomColorMaps&;
  auto SetWeightedColorMaps(const COLOR::WeightedRandomColorMaps& weightedColorMaps) -> void;

  auto InitColorMaps() -> void;

  auto GetColorMaps() const -> const COLOR::RandomColorMaps&;

  auto GetColorMode() const -> VISUAL_FX::IfsDancersFx::ColorMode;
  auto SetForcedColorMode(VISUAL_FX::IfsDancersFx::ColorMode val) -> void;
  auto ChangeColorMode() -> void;

  auto ChangeColorMaps() -> void;

  auto SetMaxHitCount(uint32_t val) -> void;

  struct MixProperties
  {
    float brightness;
    float tMix;
    float tX;
    float tY;
  };
  [[nodiscard]] auto GetMixedColor(const Pixel& baseColor,
                                   uint32_t hitCount,
                                   const MixProperties& mixProperties) const -> Pixel;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;

  COLOR::WeightedRandomColorMaps m_colorMaps;
  COLOR::ColorMapSharedPtr m_mixerColorMapPtr1         = nullptr;
  COLOR::ColorMapSharedPtr m_previousMixerColorMapPtr1 = nullptr;
  COLOR::ColorMapSharedPtr m_mixerColorMapPtr2         = nullptr;
  COLOR::ColorMapSharedPtr m_previousMixerColorMapPtr2 = nullptr;
  auto UpdateMixerMaps() -> void;
  [[nodiscard]] auto GetNextColorMapTypes() const noexcept
      -> const std::set<COLOR::WeightedRandomColorMaps::ColorMapTypes>&;
  mutable uint32_t m_countSinceColorMapChange              = 0;
  static constexpr uint32_t MIN_COLOR_MAP_CHANGE_COMPLETED = 500;
  static constexpr uint32_t MAX_COLOR_MAP_CHANGE_COMPLETED = 1000;
  uint32_t m_colorMapChangeCompleted                       = MIN_COLOR_MAP_CHANGE_COMPLETED;

  VISUAL_FX::IfsDancersFx::ColorMode m_colorMode = VISUAL_FX::IfsDancersFx::ColorMode::MAP_COLORS;
  VISUAL_FX::IfsDancersFx::ColorMode m_forcedColorMode  = VISUAL_FX::IfsDancersFx::ColorMode::_NULL;
  uint32_t m_maxHitCount                                = 0;
  float m_logMaxHitCount                                = 0.0;
  static constexpr float MIN_T_AWAY_FROM_BASE_COLOR     = 0.0F;
  static constexpr float MAX_T_AWAY_FROM_BASE_COLOR     = 0.4F;
  static constexpr float INITIAL_T_AWAY_FROM_BASE_COLOR = 0.0F;
  float m_tAwayFromBaseColor = INITIAL_T_AWAY_FROM_BASE_COLOR; // in [0, 1]
  UTILS::MATH::Weights<VISUAL_FX::IfsDancersFx::ColorMode> m_colorModeWeights;
  auto GetNextColorMode() const -> VISUAL_FX::IfsDancersFx::ColorMode;
  [[nodiscard]] auto GetMixedColorInfo(const Pixel& baseColor,
                                       float logAlpha,
                                       const MixProperties& mixProperties) const
      -> std::pair<Pixel, float>;
  [[nodiscard]] auto GetNextMixerMapColor(float t, float tX, float tY) const -> Pixel;
  [[nodiscard]] auto GetSineMixColor(float tX, float tY) const -> Pixel;
  [[nodiscard]] auto GetMapColorsTBaseMix() const -> float;
  [[nodiscard]] auto GetFinalMixedColor(const Pixel& baseColor,
                                        float tBaseMix,
                                        const Pixel& mixColor) const -> Pixel;

  static constexpr float GAMMA = 2.2F;
  COLOR::ColorAdjustment m_colorAdjust{
      {GAMMA, COLOR::ColorAdjustment::INCREASED_CHROMA_FACTOR}
  };
};

inline auto Colorizer::GetColorMaps() const -> const COLOR::RandomColorMaps&
{
  return m_colorMaps;
}

inline auto Colorizer::GetColorMode() const -> VISUAL_FX::IfsDancersFx::ColorMode
{
  return m_colorMode;
}

inline auto Colorizer::SetForcedColorMode(const VISUAL_FX::IfsDancersFx::ColorMode val) -> void
{
  m_forcedColorMode = val;
}

inline auto Colorizer::SetMaxHitCount(const uint32_t val) -> void
{
  m_maxHitCount    = val;
  m_logMaxHitCount = std::log(static_cast<float>(m_maxHitCount));
}

inline auto Colorizer::GetWeightedColorMaps() const -> const COLOR::WeightedRandomColorMaps&
{
  return m_colorMaps;
}

} // namespace GOOM::VISUAL_FX::IFS
