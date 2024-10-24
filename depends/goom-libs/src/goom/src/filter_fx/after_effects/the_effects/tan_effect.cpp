module;

// #undef NO_LOGGING

#include "goom/goom_logger.h"

#include <format>
#include <string>

module Goom.FilterFx.AfterEffects.TheEffects.TanEffect;

import Goom.FilterFx.CommonTypes;
import Goom.Utils.EnumUtils;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.Point2d;

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

using UTILS::EnumToString;
using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::NUM;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;

static constexpr auto COT_MIX_RANGE   = NumberRange{0.6F, 1.6F};
static constexpr auto TAN_ONLY_WEIGHT = 500.0F;
static constexpr auto COT_ONLY_WEIGHT = 1.0F;
static constexpr auto COT_MIX_WEIGHT  = 50.0F;

static constexpr auto AMPLITUDE_RANGE          = NumberRange{0.10F, 1.11F};
static constexpr auto PROB_XY_AMPLITUDES_EQUAL = 1.00F;

static constexpr auto LIMITING_FACTOR_RANGE = NumberRange{0.10F, 0.85F};

TanEffect::TanEffect(const GoomRand& goomRand)
  : m_goomRand{&goomRand},
    m_tanEffectWeights{
      *m_goomRand,
      {
        {.key=TanType::TAN_ONLY, .weight=TAN_ONLY_WEIGHT},
        {.key=TanType::COT_ONLY, .weight=COT_ONLY_WEIGHT},
        {.key=TanType::COT_MIX,  .weight=COT_MIX_WEIGHT},
      }
    },
    m_params{GetRandomParams()}
{
}

auto TanEffect::GetRandomParams() const noexcept -> Params
{
  const auto tanType = m_tanEffectWeights.GetRandomWeighted();
  const auto cotMix  = m_goomRand->GetRandInRange<COT_MIX_RANGE>();

  const auto xAmplitude = m_goomRand->GetRandInRange<AMPLITUDE_RANGE>();
  const auto yAmplitude = m_goomRand->ProbabilityOf<PROB_XY_AMPLITUDES_EQUAL>()
                              ? xAmplitude
                              : m_goomRand->GetRandInRange<AMPLITUDE_RANGE>();

  const auto limitingFactor = m_goomRand->GetRandInRange<LIMITING_FACTOR_RANGE>();

  LogInfo("tanType = {}, cotMix = {}", EnumToString(tanType), cotMix); // NOLINT
  LogInfo("xAmplitude = {}, yAmplitude = {}", xAmplitude, yAmplitude); // NOLINT
  LogInfo("limitingFactor = {}", limitingFactor); // NOLINT

  return {
      .tanType        = tanType,
      .cotMix         = cotMix,
      .amplitude      = {xAmplitude, yAmplitude},
      .limitingFactor = limitingFactor
  };
}

auto TanEffect::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  const auto fullParamGroup = GetFullParamGroup({paramGroup, "tan effect"});
  return {GetPair(fullParamGroup,
                  "params",
                  std::format("{}, {:.2f}, ({:.2f},{:.2f}), {:.2f}",
                              EnumToString(m_params.tanType),
                              m_params.cotMix,
                              m_params.amplitude.x,
                              m_params.amplitude.y,
                              m_params.limitingFactor))};
}

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
