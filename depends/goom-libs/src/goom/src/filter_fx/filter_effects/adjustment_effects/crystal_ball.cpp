module Goom.FilterFx.FilterEffects.AdjustmentEffects.CrystalBall;

import Goom.FilterFx.CommonTypes;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::NameValuePairs;
using UTILS::MATH::GoomRand;

static constexpr auto AMPLITUDE_RANGE_MODE0 = AmplitudeRange{
    .xRange = {0.001F, 0.501F},
    .yRange = {0.001F, 0.501F},
};
static constexpr auto AMPLITUDE_RANGE_MODE1 = AmplitudeRange{
    .xRange = {0.500F, 1.001F},
    .yRange = {0.500F, 1.001F},
};

static constexpr auto SQ_DIST_MULT_RANGE_MODE0 = SqDistMultRange{
    .xRange = {0.001F, 0.051F},
    .yRange = {0.001F, 0.051F},
};
static constexpr auto SQ_DIST_MULT_RANGE_MODE1 = SqDistMultRange{
    .xRange = {0.050F, 0.101F},
    .yRange = {0.050F, 0.101F},
};

static constexpr auto SQ_DIST_OFFSET_RANGE_MODE0 = SqDistOffsetRange{
    .xRange = {0.001F, 0.11F},
    .yRange = {0.001F, 0.11F},
};
static constexpr auto SQ_DIST_OFFSET_RANGE_MODE1 = SqDistOffsetRange{
    .xRange = {0.100F, 1.01F},
    .yRange = {0.100F, 1.01F},
};

static constexpr auto PROB_XY_AMPLITUDES_EQUAL     = 1.00F;
static constexpr auto PROB_XY_SQ_DIST_MULT_EQUAL   = 1.00F;
static constexpr auto PROB_XY_SQ_DIST_OFFSET_EQUAL = 1.00F;

CrystalBall::CrystalBall(const Modes mode, const GoomRand& goomRand) noexcept
  : m_mode{mode}, m_goomRand{&goomRand}, m_params{GetMode0RandomParams()}
{
}

auto CrystalBall::SetRandomParams() noexcept -> void
{
  if (m_mode == Modes::MODE0)
  {
    SetMode0RandomParams();
  }
  else
  {
    SetMode1RandomParams();
  }
}

auto CrystalBall::GetMode0RandomParams() const noexcept -> Params
{
  return GetRandomParams(
      AMPLITUDE_RANGE_MODE0, SQ_DIST_MULT_RANGE_MODE0, SQ_DIST_OFFSET_RANGE_MODE0);
}

auto CrystalBall::GetMode1RandomParams() const noexcept -> Params
{
  return GetRandomParams(
      AMPLITUDE_RANGE_MODE1, SQ_DIST_MULT_RANGE_MODE1, SQ_DIST_OFFSET_RANGE_MODE1);
}

auto CrystalBall::GetRandomParams(const AmplitudeRange& amplitudeRange,
                                  const SqDistMultRange& sqDistMultRange,
                                  const SqDistOffsetRange& sqDistOffsetRange) const noexcept
    -> Params
{
  const auto xAmplitude = m_goomRand->GetRandInRange(amplitudeRange.xRange);
  const auto yAmplitude = m_goomRand->ProbabilityOf<PROB_XY_AMPLITUDES_EQUAL>()
                              ? xAmplitude
                              : m_goomRand->GetRandInRange(amplitudeRange.yRange);

  const auto xSqDistMult = m_goomRand->GetRandInRange(sqDistMultRange.xRange);
  const auto ySqDistMult = m_goomRand->ProbabilityOf<PROB_XY_SQ_DIST_MULT_EQUAL>()
                               ? xSqDistMult
                               : m_goomRand->GetRandInRange(sqDistMultRange.yRange);

  const auto xSqDistOffset = m_goomRand->GetRandInRange(sqDistOffsetRange.xRange);
  const auto ySqDistOffset = m_goomRand->ProbabilityOf<PROB_XY_SQ_DIST_OFFSET_EQUAL>()
                                 ? xSqDistOffset
                                 : m_goomRand->GetRandInRange(sqDistOffsetRange.yRange);

  return {
      .amplitude    = {        xAmplitude,         yAmplitude},
      .sqDistMult   = {  .x = xSqDistMult,   .y = ySqDistMult},
      .sqDistOffset = {.x = xSqDistOffset, .y = ySqDistOffset}
  };
}

auto CrystalBall::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  return NameValuePairs{};
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
