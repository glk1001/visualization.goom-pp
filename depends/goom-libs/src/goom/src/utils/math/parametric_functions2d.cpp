module;

#include <cmath>
#include <cstdint>

module Goom.Utils.Math.ParametricFunctions2d;

import Goom.Utils.Math.Misc;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;

namespace GOOM::UTILS::MATH
{

auto IParametricFunction2d::GetPointData(const float t) const noexcept -> PointData
{
  return {.point = GetPoint(t), .normalAngle = HALF_PI};
}

CircleFunction::CircleFunction(const Vec2dFlt& centrePos,
                               const float radius,
                               const AngleParams& angleParams,
                               const Direction direction) noexcept
  : m_centrePos{centrePos}, m_radius{radius}, m_angleParams{angleParams}, m_direction{direction}
{
}

auto CircleFunction::GetPointAtAngle(float angle) const noexcept -> Point2dFlt
{
  if (m_direction == Direction::CLOCKWISE)
  {
    angle = -angle;
  }

  return Point2dFlt{.x = m_radius * std::cos(angle), .y = -m_radius * std::sin(angle)} +
         m_centrePos;
}

SpiralFunction::SpiralFunction(const Vec2dFlt& centrePos,
                               const uint32_t numTurns,
                               const Direction direction,
                               const MinMaxValues<float>& minMaxRadius) noexcept
  : m_centrePos{centrePos},
    m_minRadius{minMaxRadius.minValue},
    m_maxRadius{minMaxRadius.maxValue},
    m_angleFactor{direction == Direction::COUNTER_CLOCKWISE
                      ? -(static_cast<float>(numTurns) * TWO_PI)
                      : +(static_cast<float>(numTurns) * TWO_PI)}
{
}

LissajousFunction::LissajousFunction(const Vec2dFlt& centrePos,
                                     const AngleParams& angleParams,
                                     const Params& params) noexcept
  : m_centrePos{centrePos}, m_angleParams{angleParams}, m_params{params}
{
}

auto LissajousFunction::GetPointAtAngle(const float angle) const noexcept -> Point2dFlt
{
  return Point2dFlt{.x = +m_params.a * std::cos(m_params.kX * angle),
                    .y = -m_params.b * std::sin(m_params.kY * angle)} +
         m_centrePos;
}

HypotrochoidFunction::HypotrochoidFunction(const Vec2dFlt& centrePos,
                                           const AngleParams& angleParams,
                                           const Params& params) noexcept
  : m_centrePos{centrePos},
    m_angleParams{angleParams},
    m_params{params},
    m_rDiff{m_params.bigR - m_params.smallR},
    m_numCusps{GetNumCusps({.bigR = m_params.bigR, .smallR = m_params.smallR})}
{
  Expects(params.bigR > 0.0F);
  Expects(params.smallR > 0.0F);
  Expects(params.amplitude > 0.0F);
  Expects(angleParams.startAngleInRadians <= angleParams.endAngleInRadians);
}

auto HypotrochoidFunction::GetNumCusps(const BigAndSmallR& bigAndSmallR) noexcept -> float
{
  const auto intBigR   = static_cast<int32_t>(bigAndSmallR.bigR + SMALL_FLOAT);
  const auto intSmallR = static_cast<int32_t>(bigAndSmallR.smallR + SMALL_FLOAT);

  if ((0 == intBigR) || (0 == intSmallR))
  {
    return 1.0F;
  }

  // NOLINTNEXTLINE(bugprone-integer-division)
  return static_cast<float>(Lcm(intSmallR, intBigR) / static_cast<int64_t>(intBigR));
}

auto HypotrochoidFunction::GetPointAtAngle(const float angle) const noexcept -> Point2dFlt
{
  const auto angleArg2 = (m_rDiff / m_params.smallR) * angle;

  const auto point = Point2dFlt{
      .x = +(m_rDiff * std::cos(angle)) + (m_params.height * std::cos(angleArg2)),
      .y = -(m_rDiff * std::sin(angle)) + (m_params.height * std::sin(angleArg2)),
  };

  return Scale(point, m_params.amplitude) + m_centrePos;
}

EpicycloidFunction::EpicycloidFunction(const Vec2dFlt& centrePos,
                                       const AngleParams& angleParams,
                                       const Params& params) noexcept
  : m_centrePos{centrePos},
    m_angleParams{angleParams},
    m_params{params},
    m_numCusps{GetNumCusps(m_params.k)}
{
  Expects(params.k > 0.0F);
  Expects(params.smallR > 0.0F);
  Expects(params.amplitude > 0.0F);
  Expects(angleParams.startAngleInRadians <= angleParams.endAngleInRadians);
}

auto EpicycloidFunction::GetNumCusps([[maybe_unused]] const float k) noexcept -> float
{
  // From 'https://en.wikipedia.org/wiki/Epicycloid'
  if (const RationalNumber frac = FloatToIrreducibleFraction(k); frac.isRational)
  {
    return static_cast<float>(frac.numerator);
  }

  // k is irrational. Curve never closes, so return 'large' number.
  static constexpr auto LARGE_NUM_CUSPS = 20.0F;
  return LARGE_NUM_CUSPS;
}

auto EpicycloidFunction::GetPointAtAngle(const float angle) const noexcept -> Point2dFlt
{
  const auto angleArg2 = (m_params.k + 1.0F) * angle;

  const auto point = Point2dFlt{
      .x = +(m_params.smallR * (m_params.k + 1.0F) * std::cos(angle)) -
           +(m_params.smallR * std::cos(angleArg2)),
      .y = -(m_params.smallR * (m_params.k + 1.0F) * std::sin(angle)) +
           +(m_params.smallR * std::sin(angleArg2)),
  };

  return Scale(point, m_params.amplitude) + m_centrePos;
}

SineFunction::SineFunction(const StartAndEndPos& startAndEndPos, const Params& params) noexcept
  : m_startPos{startAndEndPos.startPos},
    m_endPos{startAndEndPos.endPos},
    m_params{params},
    m_distance{Distance(m_startPos, m_endPos)},
    m_rotateAngle{std::asin((m_endPos.y - m_startPos.y) / m_distance)}
{
}

auto SineFunction::GetPoint(const float t) const noexcept -> Point2dFlt
{
  const auto y        = 100.0F * std::sin(m_params.freq * TWO_PI * t);
  const auto x        = m_distance * t;
  const auto newPoint = Rotate(Point2dFlt{.x = x, .y = y}, m_rotateAngle);

  return (Point2dFlt{.x = newPoint.x, .y = (m_params.amplitude * newPoint.y)} +
          ToVec2dFlt(m_startPos));
}

OscillatingFunction::OscillatingFunction(const StartAndEndPos& startAndEndPos,
                                         const Params& params) noexcept
  : m_params{params},
    m_startPos{GetAdjustedStartPos(startAndEndPos.startPos)},
    m_endPos{startAndEndPos.endPos}
{
}

OscillatingFunction::OscillatingFunction(const UTILS::MATH::TValue& angleT,
                                         const StartAndEndPos& startAndEndPos,
                                         const Params& params) noexcept
  : m_usingAngleT{true},
    m_angleT{angleT},
    m_params{params},
    m_startPos{GetAdjustedStartPos(startAndEndPos.startPos)},
    m_endPos{startAndEndPos.endPos}
{
}

auto OscillatingFunction::Increment() noexcept -> void
{
  if (m_usingAngleT)
  {
    m_angleT.Increment();
  }
}

auto OscillatingFunction::GetPoint(const float t) const noexcept -> Point2dFlt
{
  const auto linearPoint = lerp(m_startPos, m_endPos, t);
  return GetOscillatingPoint(linearPoint, t);
}

inline auto OscillatingFunction::GetOscillatingPoint(const Point2dFlt& linearPoint,
                                                     const float t) const noexcept -> Point2dFlt
{
  return linearPoint + GetOscillatingOffset(t);
}

inline auto OscillatingFunction::GetOscillatingOffset(float t) const noexcept -> Vec2dFlt
{
  if (not m_allowOscillatingPath)
  {
    return {.x = 0, .y = 0};
  }

  if (m_usingAngleT)
  {
    t = m_angleT();
  }

  return {
      .x = m_params.oscillatingAmplitude * std::cos(m_params.xOscillatingFreq * t * TWO_PI),
      .y = m_params.oscillatingAmplitude * std::sin(m_params.yOscillatingFreq * t * TWO_PI),
  };
}

auto OscillatingFunction::GetAdjustedStartPos(const Point2dFlt& startPos) const noexcept
    -> Point2dFlt
{
  return startPos - GetOscillatingOffset(0.0F);
}

} // namespace GOOM::UTILS::MATH
