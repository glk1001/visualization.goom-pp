#include "rotation.h"

#include "utils/math/misc.h"
#include "utils/name_value_pairs.h"

#include <cmath>
#include <string>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using STD20::pi;
using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;

static constexpr float DEFAULT_ROTATE_SPEED = 0.0F;
static constexpr IGoomRand::NumberRange<float> ROTATE_SPEED_RANGE = {-0.5F, +0.5F};
static constexpr float PROB_EQUAL_XY_ROTATE_SPEEDS = 0.8F;

static constexpr float DEFAULT_ROTATE_ANGLE = pi / 4.0F;
static constexpr IGoomRand::NumberRange<float> ANGLE_RANGE = {(1.0F / 8.0F) * pi,
                                                              (3.0F / 8.0F) * pi};

Rotation::Rotation(const IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand},
    m_params{DEFAULT_ROTATE_SPEED, DEFAULT_ROTATE_SPEED, std::sin(DEFAULT_ROTATE_ANGLE),
             std::cos(DEFAULT_ROTATE_ANGLE)}
{
}

auto Rotation::SetRandomParams() -> void
{
  const float xRotateSpeed = m_goomRand.GetRandInRange(ROTATE_SPEED_RANGE);
  float yRotateSpeed = m_goomRand.ProbabilityOf(PROB_EQUAL_XY_ROTATE_SPEEDS)
                           ? xRotateSpeed
                           : m_goomRand.GetRandInRange(ROTATE_SPEED_RANGE);

  if (((xRotateSpeed < 0.0F) && (yRotateSpeed > 0.0F)) ||
      ((xRotateSpeed > 0.0F) && (yRotateSpeed < 0.0F)))
  {
    yRotateSpeed = -yRotateSpeed;
  }

  const float angle = m_goomRand.GetRandInRange(ANGLE_RANGE);
  const float sinAngle = std::sin(angle);
  const float cosAngle = std::cos(angle);

  SetParams({xRotateSpeed, yRotateSpeed, sinAngle, cosAngle});
}

auto Rotation::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  const std::string fullParamGroup = GetFullParamGroup({paramGroup, "rotation"});
  return {
      GetPair(fullParamGroup, "speed", Point2dFlt{m_params.xRotateSpeed, m_params.yRotateSpeed}),
      GetPair(fullParamGroup, "sinAngle", m_params.sinAngle),
      GetPair(fullParamGroup, "cosAngle", m_params.cosAngle),
  };
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS