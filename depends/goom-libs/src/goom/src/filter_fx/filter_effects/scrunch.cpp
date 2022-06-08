#include "scrunch.h"

#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;

static constexpr float X_DEFAULT_AMPLITUDE = 0.1F;
static constexpr float Y_DEFAULT_AMPLITUDE = 1.0F;
static constexpr IGoomRand::NumberRange<float> X_AMPLITUDE_RANGE = {0.05F, 0.2F};
static constexpr IGoomRand::NumberRange<float> Y_AMPLITUDE_RANGE = {0.50F, 2.0F};

Scrunch::Scrunch(const IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand}, m_params{X_DEFAULT_AMPLITUDE, Y_DEFAULT_AMPLITUDE}
{
}

auto Scrunch::SetRandomParams() -> void
{
  const float xAmplitude = m_goomRand.GetRandInRange(X_AMPLITUDE_RANGE);
  const float yAmplitude = m_goomRand.GetRandInRange(Y_AMPLITUDE_RANGE);

  SetParams({xAmplitude, yAmplitude});
}

auto Scrunch::GetSpeedCoefficientsEffectNameValueParams() const -> NameValuePairs
{
  return NameValuePairs();
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS