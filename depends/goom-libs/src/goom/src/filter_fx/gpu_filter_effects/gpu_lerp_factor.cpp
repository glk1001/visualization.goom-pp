module;

module Goom.FilterFx.GpuFilterEffects.GpuLerpFactor;

import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Lerper;

namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
{

using UTILS::MATH::GoomRand;
using UTILS::MATH::Lerper;
using UTILS::MATH::NumberRange;

GpuLerpFactor::GpuLerpFactor(const GoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_lerpFactor{
        DEFAULT_NUM_GPU_LERP_FACTOR_STEPS, 0.0F, 1.0F, Lerper<float>::LerperType::CONTINUOUS}
{
}

auto GpuLerpFactor::ResetToZero() noexcept -> void
{
  m_lerpFactor.ResetT();
}

auto GpuLerpFactor::ResetToOne() noexcept -> void
{
  m_lerpFactor.ResetT(1.0F);
}

auto GpuLerpFactor::ResetNumSteps() noexcept -> void
{
  const auto newNumSteps = m_goomRand->GetRandInRange<NUM_GPU_LERP_FACTOR_STEPS_RANGE>();
  m_lerpFactor.SetNumSteps(newNumSteps);
}

auto GpuLerpFactor::ResetNumStepsToDefault() noexcept -> void
{
  m_lerpFactor.SetNumSteps(DEFAULT_NUM_GPU_LERP_FACTOR_STEPS);
}

auto GpuLerpFactor::MultiplyLerpIncrement(const float factor) noexcept -> void
{
  m_lerpFactor.SetStepSize(m_lerpFactor.GetStepSize() * factor);
}

auto GpuLerpFactor::GoUpABit() noexcept -> void
{
  const auto tRangeLeftUp = 1.0F - m_lerpFactor.GetT();
  const auto fracUp       = m_goomRand->GetRandInRange<FRAC_MOVE_RANGE>();
  m_lerpFactor.ResetT(m_lerpFactor.GetT() + (fracUp * tRangeLeftUp));
}

auto GpuLerpFactor::GoDownABit() noexcept -> void
{
  const auto tRangeLeftDown = m_lerpFactor.GetT();
  const auto fracDown       = m_goomRand->GetRandInRange<FRAC_MOVE_RANGE>();
  m_lerpFactor.ResetT(m_lerpFactor.GetT() - (fracDown * tRangeLeftDown));
}

auto GpuLerpFactor::SpeedUpABit() noexcept -> void
{
  const auto currentNumSteps = m_lerpFactor.GetNumSteps();
  const auto newSmallerNumSteps =
      m_goomRand->GetRandInRange(NumberRange{NUM_GPU_LERP_FACTOR_STEPS_RANGE.min, currentNumSteps});
  m_lerpFactor.SetNumSteps(newSmallerNumSteps);
}

auto GpuLerpFactor::SlowDownABit() noexcept -> void
{
  const auto currentNumSteps = m_lerpFactor.GetNumSteps();
  const auto newLargerNumSteps =
      m_goomRand->GetRandInRange(NumberRange{currentNumSteps, NUM_GPU_LERP_FACTOR_STEPS_RANGE.max});
  m_lerpFactor.SetNumSteps(newLargerNumSteps);
}

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
