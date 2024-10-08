module;

#include <string>

module Goom.FilterFx.AfterEffects.TheEffects.Noise;

import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;

static constexpr auto NOISE_FACTOR_RANGE = NumberRange{0.001F, 0.100F};

Noise::Noise(const GoomRand& goomRand) noexcept : m_goomRand{&goomRand}, m_params{GetRandomParams()}
{
}

auto Noise::GetRandomParams() const noexcept -> Params
{
  return {m_goomRand->GetRandInRange<NOISE_FACTOR_RANGE>()};
}

auto Noise::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  const auto fullParamGroup = GetFullParamGroup({paramGroup, "noise"});
  return {
      GetPair(fullParamGroup, "noise factor", m_params.noiseFactor),
  };
}

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
