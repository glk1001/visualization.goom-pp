module;

#include <cmath>
#include <format>
#include <string>

module Goom.FilterFx.AfterEffects.TheEffects.Rotation;

import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Lib.Point2d;

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;
using UTILS::MATH::PI;

static constexpr auto ROTATE_SPEED_RANGE          = NumberRange{-0.5F, +0.5F};
static constexpr auto PROB_EQUAL_XY_ROTATE_SPEEDS = 0.8F;

static constexpr auto ANGLE_RANGE = NumberRange{(1.0F / 8.0F) * PI, (3.0F / 8.0F) * PI};

Rotation::Rotation(const GoomRand& goomRand) noexcept
  : m_goomRand{&goomRand}, m_params{GetRandomParams()}
{
}

auto Rotation::GetRandomParams() const noexcept -> Params
{
  const auto xRotateSpeed = m_goomRand->GetRandInRange<ROTATE_SPEED_RANGE>();
  auto yRotateSpeed       = m_goomRand->ProbabilityOf<PROB_EQUAL_XY_ROTATE_SPEEDS>()
                                ? xRotateSpeed
                                : m_goomRand->GetRandInRange<ROTATE_SPEED_RANGE>();

  if (((xRotateSpeed < 0.0F) && (yRotateSpeed > 0.0F)) ||
      ((xRotateSpeed > 0.0F) && (yRotateSpeed < 0.0F)))
  {
    yRotateSpeed = -yRotateSpeed;
  }

  const auto angle    = m_goomRand->GetRandInRange<ANGLE_RANGE>();
  const auto sinAngle = std::sin(angle);
  const auto cosAngle = std::cos(angle);

  return {.xRotateSpeed = xRotateSpeed,
          .yRotateSpeed = yRotateSpeed,
          .sinAngle     = sinAngle,
          .cosAngle     = cosAngle};
}

auto Rotation::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  const auto fullParamGroup = GetFullParamGroup({paramGroup, "rotation"});
  return {GetPair(fullParamGroup,
                  "params",
                  std::format("({:.2f},{:.2f}), {:.2f}, {:.2f}",
                              m_params.xRotateSpeed,
                              m_params.yRotateSpeed,
                              m_params.sinAngle,
                              m_params.cosAngle))};
}

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
