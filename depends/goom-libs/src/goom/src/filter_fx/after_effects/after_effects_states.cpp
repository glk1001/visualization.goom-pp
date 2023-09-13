#include "after_effects_states.h"

#include "after_effects_types.h"
#include "filter_fx/filter_consts.h"
#include "goom/goom_time.h"
#include "utils/math/goom_rand_base.h"
#include "utils/timer.h"

#include <cstdint>
#include <memory>

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

using UTILS::Timer;
using UTILS::MATH::IGoomRand;

static constexpr auto DEFAULT_HYPERCOS_OVERLAY_EFFECT = false;
static constexpr auto DEFAULT_IMAGE_VELOCITY_EFFECT   = false;
static constexpr auto DEFAULT_NOISE_EFFECT            = false;
static constexpr auto DEFAULT_PLANE_EFFECT            = false;
static constexpr auto DEFAULT_ROTATION_EFFECT         = false;
static constexpr auto DEFAULT_TAN_EFFECT              = false;
static constexpr auto DEFAULT_XY_LERP_EFFECT          = false;

class AfterEffectsStates::AfterEffectState
{
public:
  struct AfterEffectProperties
  {
    float probabilityOfEffectRepeated;
    uint32_t effectOffTime;
  };

  AfterEffectState(const GoomTime& goomTime,
                   const IGoomRand& goomRand,
                   bool turnedOn,
                   const AfterEffectProperties& effectProperties) noexcept;

  void UpdateState(float effectProbability);
  void SetState(bool value);
  void CheckPendingOffTimerReset();

  [[nodiscard]] auto IsTurnedOn() const -> bool;

private:
  const IGoomRand* m_goomRand;
  float m_probabilityOfEffectRepeated;
  bool m_turnedOn;
  Timer m_offTimer;
  bool m_pendingOffTimerReset = false;
};

AfterEffectsStates::AfterEffectsStates(const GoomTime& goomTime,
                                       const IGoomRand& goomRand,
                                       const AfterEffectsProbabilityMap& repeatProbabilities,
                                       const AfterEffectsOffTimeMap& offTimes) noexcept
  : m_hypercosOverlayEffect{std::make_unique<AfterEffectState>(
        goomTime,
        goomRand,
        DEFAULT_HYPERCOS_OVERLAY_EFFECT,
        AfterEffectState::AfterEffectProperties{repeatProbabilities[AfterEffectsTypes::HYPERCOS],
                                                offTimes[AfterEffectsTypes::HYPERCOS]})},
    m_imageVelocityEffect{std::make_unique<AfterEffectState>(
        goomTime,
        goomRand,
        DEFAULT_IMAGE_VELOCITY_EFFECT,
        AfterEffectState::AfterEffectProperties{
            repeatProbabilities[AfterEffectsTypes::IMAGE_VELOCITY],
            offTimes[AfterEffectsTypes::IMAGE_VELOCITY]})},
    m_noiseEffect{std::make_unique<AfterEffectState>(
        goomTime,
        goomRand,
        DEFAULT_NOISE_EFFECT,
        AfterEffectState::AfterEffectProperties{repeatProbabilities[AfterEffectsTypes::NOISE],
                                                offTimes[AfterEffectsTypes::NOISE]})},
    m_planeEffect{std::make_unique<AfterEffectState>(
        goomTime,
        goomRand,
        DEFAULT_PLANE_EFFECT,
        AfterEffectState::AfterEffectProperties{repeatProbabilities[AfterEffectsTypes::PLANES],
                                                offTimes[AfterEffectsTypes::PLANES]})},
    m_rotationEffect{std::make_unique<AfterEffectState>(
        goomTime,
        goomRand,
        DEFAULT_ROTATION_EFFECT,
        AfterEffectState::AfterEffectProperties{repeatProbabilities[AfterEffectsTypes::ROTATION],
                                                offTimes[AfterEffectsTypes::ROTATION]})},
    m_tanEffect{std::make_unique<AfterEffectState>(
        goomTime,
        goomRand,
        DEFAULT_TAN_EFFECT,
        AfterEffectState::AfterEffectProperties{repeatProbabilities[AfterEffectsTypes::TAN_EFFECT],
                                                offTimes[AfterEffectsTypes::TAN_EFFECT]})},
    m_xyLerpEffect{std::make_unique<AfterEffectState>(
        goomTime,
        goomRand,
        DEFAULT_XY_LERP_EFFECT,
        AfterEffectState::AfterEffectProperties{
            repeatProbabilities[AfterEffectsTypes::XY_LERP_EFFECT],
            offTimes[AfterEffectsTypes::XY_LERP_EFFECT]})}
{
}

AfterEffectsStates::~AfterEffectsStates() noexcept = default;

auto AfterEffectsStates::UpdateAfterEffectsSettingsFromStates(
    AfterEffectsSettings& afterEffectsSettings) const -> void
{
  afterEffectsSettings.hypercosOverlayMode = m_hypercosOverlayMode;

  afterEffectsSettings.isActive[AfterEffectsTypes::HYPERCOS] =
      m_hypercosOverlayEffect->IsTurnedOn();
  afterEffectsSettings.isActive[AfterEffectsTypes::IMAGE_VELOCITY] =
      m_imageVelocityEffect->IsTurnedOn();
  afterEffectsSettings.isActive[AfterEffectsTypes::NOISE]          = m_noiseEffect->IsTurnedOn();
  afterEffectsSettings.isActive[AfterEffectsTypes::PLANES]         = m_planeEffect->IsTurnedOn();
  afterEffectsSettings.isActive[AfterEffectsTypes::ROTATION]       = m_rotationEffect->IsTurnedOn();
  afterEffectsSettings.isActive[AfterEffectsTypes::TAN_EFFECT]     = m_tanEffect->IsTurnedOn();
  afterEffectsSettings.isActive[AfterEffectsTypes::XY_LERP_EFFECT] = m_xyLerpEffect->IsTurnedOn();
}

auto AfterEffectsStates::TurnPlaneEffectOn() -> void
{
  if constexpr (ALL_AFTER_EFFECTS_TURNED_OFF)
  {
    return;
  }

  m_planeEffect->SetState(true);
}

auto AfterEffectsStates::SetDefaults() -> void
{
  m_hypercosOverlayMode = HypercosOverlayMode::NONE;
}

auto AfterEffectsStates::ResetAllStates(const AfterEffectsProbabilities& effectsProbabilities)
    -> void
{
  if constexpr (ALL_AFTER_EFFECTS_TURNED_OFF)
  {
    return;
  }

  m_hypercosOverlayMode = effectsProbabilities.hypercosModeWeights.GetRandomWeighted();

  ResetStandardStates(effectsProbabilities);
}

auto AfterEffectsStates::ResetStandardStates(const AfterEffectsProbabilities& effectsProbabilities)
    -> void
{
  if constexpr (ALL_AFTER_EFFECTS_TURNED_OFF)
  {
    return;
  }

  m_hypercosOverlayEffect->UpdateState(
      effectsProbabilities.probabilities[AfterEffectsTypes::HYPERCOS]);
  m_imageVelocityEffect->UpdateState(
      effectsProbabilities.probabilities[AfterEffectsTypes::IMAGE_VELOCITY]);
  m_noiseEffect->UpdateState(effectsProbabilities.probabilities[AfterEffectsTypes::NOISE]);
  m_planeEffect->UpdateState(effectsProbabilities.probabilities[AfterEffectsTypes::PLANES]);
  m_rotationEffect->UpdateState(effectsProbabilities.probabilities[AfterEffectsTypes::ROTATION]);
  m_tanEffect->UpdateState(effectsProbabilities.probabilities[AfterEffectsTypes::TAN_EFFECT]);
  m_xyLerpEffect->UpdateState(
      effectsProbabilities.probabilities[AfterEffectsTypes::XY_LERP_EFFECT]);

  // TODO(glk) - sort out this hypercos special case hack.
  if (HypercosOverlayMode::NONE == m_hypercosOverlayMode)
  {
    m_hypercosOverlayEffect->SetState(false);
  }
  if (not m_hypercosOverlayEffect->IsTurnedOn())
  {
    m_hypercosOverlayMode = HypercosOverlayMode::NONE;
  }
}

auto AfterEffectsStates::CheckForPendingOffTimers() -> void
{
  if constexpr (ALL_AFTER_EFFECTS_TURNED_OFF)
  {
    return;
  }

  m_hypercosOverlayEffect->CheckPendingOffTimerReset();
  m_imageVelocityEffect->CheckPendingOffTimerReset();
  m_noiseEffect->CheckPendingOffTimerReset();
  m_planeEffect->CheckPendingOffTimerReset();
  m_rotationEffect->CheckPendingOffTimerReset();
  m_tanEffect->CheckPendingOffTimerReset();
  m_xyLerpEffect->CheckPendingOffTimerReset();
}

inline AfterEffectsStates::AfterEffectState::AfterEffectState(
    const GoomTime& goomTime,
    const UTILS::MATH::IGoomRand& goomRand,
    const bool turnedOn,
    const AfterEffectProperties& effectProperties) noexcept
  : m_goomRand{&goomRand},
    m_probabilityOfEffectRepeated{effectProperties.probabilityOfEffectRepeated},
    m_turnedOn{turnedOn},
    m_offTimer{goomTime, effectProperties.effectOffTime, true}
{
}

inline auto AfterEffectsStates::AfterEffectState::UpdateState(const float effectProbability) -> void
{
  if (not m_offTimer.Finished())
  {
    return;
  }
  if (m_pendingOffTimerReset)
  {
    return;
  }

  SetState(m_goomRand->ProbabilityOf(effectProbability));
}

inline auto AfterEffectsStates::AfterEffectState::SetState(const bool value) -> void
{
  const auto previouslyTurnedOn = m_turnedOn;
  m_turnedOn                    = value;

  if (previouslyTurnedOn && m_turnedOn)
  {
    m_turnedOn = m_goomRand->ProbabilityOf(m_probabilityOfEffectRepeated);
  }
  if (previouslyTurnedOn && (not m_turnedOn))
  {
    m_pendingOffTimerReset = true;
  }
}

inline auto AfterEffectsStates::AfterEffectState::CheckPendingOffTimerReset() -> void
{
  if (not m_pendingOffTimerReset)
  {
    return;
  }

  // Wait a while before allowing effect back on.
  m_offTimer.ResetToZero();
  m_pendingOffTimerReset = false;
}

inline auto AfterEffectsStates::AfterEffectState::IsTurnedOn() const -> bool
{
  return m_turnedOn;
}

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
