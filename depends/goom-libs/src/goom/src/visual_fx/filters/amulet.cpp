#include "amulet.h"

#include "utils/goomrand.h"
#include "utils/name_value_pairs.h"

#undef NDEBUG
#include <cassert>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

using UTILS::GetRandInRange;
using UTILS::NameValuePairs;
using UTILS::NumberRange;
using UTILS::ProbabilityOf;

constexpr float DEFAULT_AMPLITUDE = 1.0F;
constexpr NumberRange<float> AMPLITUDE_RANGE = {0.1F, 1.51F};

constexpr float PROB_XY_AMPLITUDES_EQUAL = 0.98F;

Amulet::Amulet() noexcept : m_params{DEFAULT_AMPLITUDE, DEFAULT_AMPLITUDE}
{
}

void Amulet::SetRandomParams()
{
  m_params.xAmplitude = GetRandInRange(AMPLITUDE_RANGE);
  m_params.yAmplitude = ProbabilityOf(PROB_XY_AMPLITUDES_EQUAL) ? m_params.xAmplitude
                                                                : GetRandInRange(AMPLITUDE_RANGE);
}

auto Amulet::GetSpeedCoefficientsEffectNameValueParams() const -> NameValuePairs
{
  return NameValuePairs();
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif