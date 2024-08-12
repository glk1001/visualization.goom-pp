module;

#include <cmath>
#include <cstdint>

module Goom.FilterFx.FilterEffects.AdjustmentEffects.DipoleFlowField;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Lib.Point2d;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{
using UTILS::NameValuePairs;
using UTILS::MATH::GetRandSeed;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;
using UTILS::MATH::Sq;

namespace
{

constexpr auto DEFAULT_AMPLITUDE = Amplitude{1.0F, 1.0F};
constexpr auto AMPLITUDE_RANGE   = NumberRange{0.5F, 3.0F};

constexpr auto DEFAULT_LERP_TO_ONE_T_S = LerpToOneTs{.xLerpT = 0.5F, .yLerpT = 0.5F};
constexpr auto LERP_TO_ONE_T_RANGE     = NumberRange{0.0F, 1.0F};

constexpr auto DEFAULT_ANGLE_FREQUENCY_FACTOR = 1.0F;
constexpr auto ANGLE_FREQUENCY_FACTOR_RANGE   = NumberRange{0.5F, 1.5F};

constexpr auto PROB_XY_AMPLITUDES_EQUAL        = 0.98F;
constexpr auto PROB_LERP_TO_ONE_T_S_EQUAL      = 0.95F;
constexpr auto PROB_XY_ANGLE_FREQUENCIES_EQUAL = 0.5F;
constexpr auto PROB_MULTIPLY_VELOCITY          = 0.2F;

} // namespace

DipoleFlowField::DipoleFlowField(const GoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_params{.amplitude = DEFAULT_AMPLITUDE,
             .lerpToOneTs = DEFAULT_LERP_TO_ONE_T_S,
             .angleFrequencyFactor = {.x = DEFAULT_ANGLE_FREQUENCY_FACTOR,
                                        .y = DEFAULT_ANGLE_FREQUENCY_FACTOR},
             .multiplyVelocity = false}
{
  SetupAngles();
}

auto DipoleFlowField::SetupAngles() noexcept -> void
{
  const auto setupFunc = [](const uint32_t x, const uint32_t y) -> FlowFieldGrid::PolarCoords
  {
    const auto xFlt =
        2.0F * (-0.5F + (static_cast<float>(x) / static_cast<float>(FlowFieldGrid::GRID_WIDTH)));
    const auto yFlt =
        2.0F * (-0.5F + (static_cast<float>(y) / static_cast<float>(FlowFieldGrid::GRID_HEIGHT)));

    const auto newX   = 2.0F * (xFlt * yFlt);
    const auto newY   = Sq(yFlt) - Sq(xFlt);
    const auto angle  = std::atan2(newY, newX);
    const auto radius = std::sqrt(Sq(newX) + Sq(newY));

    return {.angle = angle, .radius = radius};
  };

  m_gridArray.Initialize(setupFunc);
}

auto DipoleFlowField::GetVelocity(const Vec2dFlt& baseZoomAdjustment,
                                  const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto gridPolarCoords = m_gridArray.GetPolarCoords(coords);

  const auto x =
      m_params.amplitude.x *
      (gridPolarCoords.radius * std::cos(m_params.angleFrequencyFactor.x * gridPolarCoords.angle));
  const auto y =
      m_params.amplitude.y *
      (gridPolarCoords.radius * std::sin(m_params.angleFrequencyFactor.y * gridPolarCoords.angle));

  if (not m_params.multiplyVelocity)
  {
    return {.x = baseZoomAdjustment.x + x, .y = baseZoomAdjustment.y + y};
    // return {.x = baseZoomAdjustment.x + (2.0F * coords.GetX()*coords.GetY()),
    //         .y = baseZoomAdjustment.y + (Sq(coords.GetY()) - Sq(coords.GetX()))};
  }

  return {.x = coords.GetX() * x, .y = coords.GetY() * y};
}

auto DipoleFlowField::SetRandomParams() noexcept -> void
{
  const auto xAmplitude = m_goomRand->GetRandInRange<AMPLITUDE_RANGE>();
  const auto yAmplitude = m_goomRand->ProbabilityOf<PROB_XY_AMPLITUDES_EQUAL>()
                              ? xAmplitude
                              : m_goomRand->GetRandInRange<AMPLITUDE_RANGE>();

  const auto xLerpToOneT = m_goomRand->GetRandInRange<LERP_TO_ONE_T_RANGE>();
  const auto yLerpToOneT = m_goomRand->ProbabilityOf<PROB_LERP_TO_ONE_T_S_EQUAL>()
                               ? xLerpToOneT
                               : m_goomRand->GetRandInRange<LERP_TO_ONE_T_RANGE>();

  const auto xAngleFrequencyFactor = m_goomRand->GetRandInRange<ANGLE_FREQUENCY_FACTOR_RANGE>();
  const auto yAngleFrequencyFactor =
      m_goomRand->ProbabilityOf<PROB_XY_ANGLE_FREQUENCIES_EQUAL>()
          ? xAngleFrequencyFactor
          : m_goomRand->GetRandInRange<ANGLE_FREQUENCY_FACTOR_RANGE>();

  const auto multiplyVelocity = m_goomRand->ProbabilityOf<PROB_MULTIPLY_VELOCITY>();

  SetParams({
      .amplitude            = {                xAmplitude,                 yAmplitude},
      .lerpToOneTs          = {     .xLerpT = xLerpToOneT,      .yLerpT = yLerpToOneT},
      .angleFrequencyFactor = {.x = xAngleFrequencyFactor, .y = yAngleFrequencyFactor},
      .multiplyVelocity     = multiplyVelocity
  });
}

auto DipoleFlowField::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  return {};
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
