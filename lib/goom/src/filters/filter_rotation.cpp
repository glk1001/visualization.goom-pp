#include "filter_rotation.h"

#include "goomutils/goomrand.h"
#include "goomutils/name_value_pairs.h"

#undef NDEBUG
#include <cassert>
#include <string>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::GetRandInRange;
using UTILS::NameValuePairs;
using UTILS::NumberRange;
using UTILS::ProbabilityOf;

constexpr float DEFAULT_ROTATE_SPEED = 0.0F;
constexpr NumberRange<float> ROTATE_SPEED_RANGE = {-0.5F, +0.5F};
constexpr float PROB_EQUAL_XY_ROTATION = 0.8F;

constexpr float DEFAULT_FACTOR = 1.0F;
constexpr NumberRange<float> FACTOR_RANGE = {0.3F, 1.0F};
constexpr float PROB_NO_FACTORS = 0.5F;
constexpr float PROB_EQUAL_XY_FACTORS = 0.8F;
constexpr float PROB_BALANCED_FACTORS = 0.98F;


Rotation::Rotation() noexcept
  : m_params{DEFAULT_ROTATE_SPEED, DEFAULT_ROTATE_SPEED, DEFAULT_FACTOR,
             DEFAULT_FACTOR,       DEFAULT_FACTOR,       DEFAULT_FACTOR}
{
}

void Rotation::SetRandomParams()
{
  m_params.xRotateSpeed = GetRandInRange(ROTATE_SPEED_RANGE);
  m_params.yRotateSpeed = ProbabilityOf(PROB_EQUAL_XY_ROTATION)
                              ? m_params.xRotateSpeed
                              : GetRandInRange(ROTATE_SPEED_RANGE);

  if (ProbabilityOf(PROB_NO_FACTORS))
  {
    m_params.xxFactor = 1.0F;
    m_params.xyFactor = 1.0F;
    m_params.yxFactor = 1.0F;
    m_params.yyFactor = 1.0F;
  }
  else if (ProbabilityOf(PROB_BALANCED_FACTORS))
  {
    m_params.xxFactor = GetRandInRange(FACTOR_RANGE);
    m_params.xyFactor = 1.0F - m_params.xxFactor;
    m_params.yxFactor = GetRandInRange(FACTOR_RANGE);
    m_params.yyFactor = 1.0F - m_params.yxFactor;
  }
  else
  {
    m_params.xxFactor = GetRandInRange(FACTOR_RANGE);
    m_params.xyFactor = GetRandInRange(FACTOR_RANGE);
    m_params.yxFactor = GetRandInRange(FACTOR_RANGE);
    m_params.yyFactor = GetRandInRange(FACTOR_RANGE);
  }

  if (ProbabilityOf(PROB_EQUAL_XY_FACTORS))
  {
    m_params.yxFactor = m_params.xxFactor;
    m_params.yyFactor = m_params.xyFactor;
  }
}

auto Rotation::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  if (!IsActive())
  {
    return {GetPair(paramGroup, "rotation", std::string{"None"})};
  }

  const std::string fullParamGroup = GetFullParamGroup({paramGroup, "rotate"});
  return {
      GetPair(fullParamGroup, "x speed", m_params.xRotateSpeed),
      GetPair(fullParamGroup, "y speed", m_params.yRotateSpeed),
      GetPair(fullParamGroup, "xx factor", m_params.xxFactor),
      GetPair(fullParamGroup, "xy factor", m_params.xyFactor),
      GetPair(fullParamGroup, "yx factor", m_params.yxFactor),
      GetPair(fullParamGroup, "yy factor", m_params.yyFactor),
  };
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
