module;

#include <cstdint>

export module Goom.FilterFx.GpuFilterEffects.GpuLerpFactor;

import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Lerper;

using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::Lerper;
using GOOM::UTILS::MATH::NumberRange;

export namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
{

class GpuLerpFactor
{
public:
  GpuLerpFactor() = default;
  explicit GpuLerpFactor(const GoomRand& goomRand) noexcept;

  auto operator()() const noexcept -> float;

  auto Increment() noexcept -> void;

  auto ResetToZero() noexcept -> void;
  auto ResetToOne() noexcept -> void;
  auto ResetNumSteps() noexcept -> void;
  auto ResetNumStepsToDefault() noexcept -> void;

  [[nodiscard]] auto GetNumSteps() noexcept -> uint32_t;

  auto MultiplyLerpIncrement(float factor) noexcept -> void;
  auto GoUpABit() noexcept -> void;
  auto GoDownABit() noexcept -> void;
  auto SpeedUpABit() noexcept -> void;
  auto SlowDownABit() noexcept -> void;

private:
  const GoomRand* m_goomRand = nullptr;

  static constexpr auto FRAC_MOVE_RANGE                   = NumberRange(0.05F, 0.25F);
  static constexpr auto NUM_GPU_LERP_FACTOR_STEPS_RANGE   = NumberRange(300U, 1000U);
  static constexpr auto DEFAULT_NUM_GPU_LERP_FACTOR_STEPS = NUM_GPU_LERP_FACTOR_STEPS_RANGE.max;
  Lerper<float> m_lerpFactor{};
};

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS

namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
{

inline auto GpuLerpFactor::operator()() const noexcept -> float
{
  return m_lerpFactor();
}

inline auto GpuLerpFactor::Increment() noexcept -> void
{
  m_lerpFactor.Increment();
}

inline auto GpuLerpFactor::GetNumSteps() noexcept -> uint32_t
{
  return m_lerpFactor.GetNumSteps();
}

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
