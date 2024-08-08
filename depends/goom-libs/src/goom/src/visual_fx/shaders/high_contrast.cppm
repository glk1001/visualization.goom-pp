module;

#include <cmath>
#include <cstdint>

module Goom.VisualFx.ShaderFx:HighContrast;

import Goom.Utils.Timer;
import Goom.Utils.Math.TValues;
import Goom.Utils.Math.GoomRand;
import Goom.PluginInfo;

using GOOM::UTILS::Timer;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::NumberRange;
using GOOM::UTILS::MATH::TValue;

namespace GOOM::VISUAL_FX::SHADERS
{

class HighContrast
{
public:
  static constexpr float DEFAULT_CONTRAST   = 1.0F;
  static constexpr float DEFAULT_BRIGHTNESS = 1.0F;

  HighContrast(const PluginInfo& goomInfo, const GoomRand& goomRand) noexcept;

  auto Start() -> void;

  auto ChangeHighContrast() -> void;
  auto UpdateHighContrast() -> void;
  [[nodiscard]] auto GetCurrentContrast() const -> float;
  [[nodiscard]] auto GetCurrentBrightness() const -> float;
  [[nodiscard]] auto GetCurrentContrastMinChannelValue() const -> float;

private:
  const PluginInfo* m_goomInfo;
  const GoomRand* m_goomRand;

  float m_currentContrast                = DEFAULT_CONTRAST;
  float m_currentBrightness              = DEFAULT_BRIGHTNESS;
  float m_currentContrastMinChannelValue = 0.0F;
  float m_maxContrastMinChannelValue     = 0.0F;
  auto ResetValues() -> void;

  static constexpr uint32_t NUM_HIGH_CONTRAST_ON_STEPS  = 250;
  static constexpr uint32_t HIGH_CONTRAST_ON_DELAY_TIME = 100;
  static constexpr uint32_t HIGH_CONTRAST_ON_TIME =
      (2 * NUM_HIGH_CONTRAST_ON_STEPS) + HIGH_CONTRAST_ON_DELAY_TIME;
  TValue m_highContrastT{
      {.stepType = TValue::StepType::CONTINUOUS_REVERSIBLE, .numSteps = NUM_HIGH_CONTRAST_ON_STEPS},
      {{.t0 = 1.0F, .delayTime = HIGH_CONTRAST_ON_DELAY_TIME}}
  };
  Timer m_highContrastOnTimer{m_goomInfo->GetTime(), HIGH_CONTRAST_ON_TIME, true};
  static constexpr uint32_t HIGH_CONTRAST_OFF_TIME = 300;
  Timer m_highContrastOffTimer{m_goomInfo->GetTime(), HIGH_CONTRAST_OFF_TIME, false};
};

} // namespace GOOM::VISUAL_FX::SHADERS

namespace GOOM::VISUAL_FX::SHADERS
{

inline auto HighContrast::GetCurrentContrast() const -> float
{
  return m_currentContrast;
}

inline auto HighContrast::GetCurrentBrightness() const -> float
{
  return m_currentBrightness;
}

inline auto HighContrast::GetCurrentContrastMinChannelValue() const -> float
{
  return m_currentContrastMinChannelValue;
}

HighContrast::HighContrast(const PluginInfo& goomInfo, const GoomRand& goomRand) noexcept
  : m_goomInfo{&goomInfo}, m_goomRand{&goomRand}
{
}

auto HighContrast::Start() -> void
{
  m_highContrastT.Reset();
  m_highContrastOnTimer.ResetToZero();
  m_highContrastOffTimer.ResetToZero();

  ResetValues();
}

inline auto HighContrast::ResetValues() -> void
{
  m_currentContrast                = DEFAULT_CONTRAST;
  m_currentBrightness              = DEFAULT_BRIGHTNESS;
  m_currentContrastMinChannelValue = 0.0F;
  m_maxContrastMinChannelValue     = 0.0F;
}

auto HighContrast::ChangeHighContrast() -> void
{
  if (!m_highContrastOffTimer.Finished())
  {
    return;
  }
  if (!m_highContrastOnTimer.Finished())
  {
    return;
  }

  if (static constexpr float PROB_CONTRAST = 0.001F;
      (0 == m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom()) &&
      m_goomRand->ProbabilityOf<PROB_CONTRAST>())
  {
    m_highContrastT.Reset();
    m_highContrastOnTimer.ResetToZero();
    static constexpr auto CONTRAST_MIN_CHAN_RANGE = NumberRange{-0.5F, -0.2F};
    m_maxContrastMinChannelValue = m_goomRand->GetRandInRange<CONTRAST_MIN_CHAN_RANGE>();
  }
}

auto HighContrast::UpdateHighContrast() -> void
{
  m_highContrastT.Increment();

  if (!m_highContrastOffTimer.Finished())
  {
    return;
  }

  if (!m_highContrastOnTimer.Finished())
  {
    static constexpr auto HIGH_CONTRAST = 1.01F;
    m_currentContrast = std::lerp(DEFAULT_CONTRAST, HIGH_CONTRAST, m_highContrastT());
    m_currentContrastMinChannelValue =
        std::lerp(0.0F, m_maxContrastMinChannelValue, m_highContrastT());
    static constexpr auto CONTRAST_BRIGHTNESS = 1.1F;
    m_currentBrightness = std::lerp(DEFAULT_BRIGHTNESS, CONTRAST_BRIGHTNESS, m_highContrastT());

    return;
  }

  if (m_highContrastOnTimer.JustFinished())
  {
    m_highContrastOffTimer.ResetToZero();
    ResetValues();
    return;
  }

  ChangeHighContrast();
}

} // namespace GOOM::VISUAL_FX::SHADERS
