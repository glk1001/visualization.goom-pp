#pragma once

#include "after_effects_types.h"
#include "goom/goom_time.h"
#include "goom/goom_types.h"
#include "the_effects/rotation.h"
#include "utils/enum_utils.h"
#include "utils/math/goom_rand_base.h"

#include <cstdint>
#include <memory>

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

enum class HypercosOverlayMode : UnderlyingEnumType
{
  NONE,
  MODE0,
  MODE1,
  MODE2,
  MODE3,
  _num // unused, and marks the enum end
};

class AfterEffectsStates
{
public:
  using AfterEffectsOffTimeMap     = UTILS::EnumMap<AfterEffectsTypes, uint32_t>;
  using AfterEffectsProbabilityMap = UTILS::EnumMap<AfterEffectsTypes, float>;

  AfterEffectsStates(const GoomTime& goomTime,
                     const UTILS::MATH::IGoomRand& goomRand,
                     const AfterEffectsProbabilityMap& repeatProbabilities,
                     const AfterEffectsOffTimeMap& offTimes) noexcept;
  AfterEffectsStates(const AfterEffectsStates&) noexcept = delete;
  AfterEffectsStates(AfterEffectsStates&&) noexcept      = delete;
  ~AfterEffectsStates() noexcept;
  auto operator=(const AfterEffectsStates&) -> AfterEffectsStates& = delete;
  auto operator=(AfterEffectsStates&&) -> AfterEffectsStates&      = delete;

  auto SetDefaults() -> void;
  auto CheckForPendingOffTimers() -> void;

  struct AfterEffectsProbabilities
  {
    UTILS::MATH::Weights<HypercosOverlayMode> hypercosModeWeights;
    AfterEffectsProbabilityMap probabilities{};
  };
  auto ResetAllStates(const AfterEffectsProbabilities& effectsProbabilities) -> void;
  auto ResetStandardStates(const AfterEffectsProbabilities& effectsProbabilities) -> void;

  using AfterEffectsActiveMap = UTILS::EnumMap<AfterEffectsTypes, bool>;
  struct AfterEffectsSettings
  {
    HypercosOverlayMode hypercosOverlayMode{};
    AfterEffectsActiveMap isActive{};
    RotationAdjustments rotationAdjustments;
  };
  auto UpdateAfterEffectsSettingsFromStates(AfterEffectsSettings& afterEffectsSettings) const
      -> void;

  auto TurnPlaneEffectOn() -> void;

private:
  class AfterEffectState;

  HypercosOverlayMode m_hypercosOverlayMode = HypercosOverlayMode::NONE;
  std::unique_ptr<AfterEffectState> m_hypercosOverlayEffect;
  std::unique_ptr<AfterEffectState> m_imageVelocityEffect;
  std::unique_ptr<AfterEffectState> m_noiseEffect;
  std::unique_ptr<AfterEffectState> m_planeEffect;
  std::unique_ptr<AfterEffectState> m_rotationEffect;
  std::unique_ptr<AfterEffectState> m_tanEffect;
  std::unique_ptr<AfterEffectState> m_xyLerpEffect;
};

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
