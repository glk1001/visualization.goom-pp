module;

#include <algorithm>
#include <cstdint>

export module Goom.Utils.Graphics.PointUtils;

import Goom.Utils.Graphics.LineClipper;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;

using GOOM::UTILS::MATH::NumberRange;

export namespace GOOM::UTILS::GRAPHICS
{

[[nodiscard]] constexpr auto IsZero(const Point2dInt& point) noexcept -> bool;

[[nodiscard]] constexpr auto GetMinSideLength(const Rectangle2dInt& rectangle2D) noexcept
    -> uint32_t;

[[nodiscard]] constexpr auto GetCentrePoint(const Rectangle2dInt& rectangle2D) noexcept
    -> Point2dInt;

[[nodiscard]] auto GetPointClippedToRectangle(
    const Point2dInt& point,
    const Rectangle2dInt& clipRectangle,
    const Point2dInt& connectingPointInsideClipRectangle) noexcept -> Point2dInt;

[[nodiscard]] auto GetRandomPoint(const UTILS::MATH::GoomRand& goomRand,
                                  const Rectangle2dInt& rectangle2D) noexcept -> Point2dInt;

inline constexpr auto DEFAULT_CLOSE_TO_WEIGHT_POINT_T = 0.3F; // not very close
[[nodiscard]] auto GetRandomPoint(
    const UTILS::MATH::GoomRand& goomRand,
    const Rectangle2dInt& rectangle2D,
    const Point2dInt& weightPoint,
    float closeToWeightPointT = DEFAULT_CLOSE_TO_WEIGHT_POINT_T) noexcept -> Point2dInt;

} // namespace GOOM::UTILS::GRAPHICS

namespace GOOM::UTILS::GRAPHICS
{

constexpr auto IsZero(const Point2dInt& point) noexcept -> bool
{
  return (0 == point.x) and (0 == point.y);
}

constexpr auto GetMinSideLength(const Rectangle2dInt& rectangle2D) noexcept -> uint32_t
{
  Expects(rectangle2D.topLeft.x <= rectangle2D.bottomRight.x);
  Expects(rectangle2D.topLeft.y <= rectangle2D.bottomRight.y);

  return static_cast<uint32_t>(std::min(rectangle2D.bottomRight.x - rectangle2D.topLeft.x,
                                        rectangle2D.bottomRight.y - rectangle2D.topLeft.y));
}

constexpr auto GetCentrePoint(const Rectangle2dInt& rectangle2D) noexcept -> Point2dInt
{
  Expects(rectangle2D.topLeft.x <= rectangle2D.bottomRight.x);
  Expects(rectangle2D.topLeft.y <= rectangle2D.bottomRight.y);

  return {.x = UTILS::MATH::I_HALF * (rectangle2D.topLeft.x + rectangle2D.bottomRight.x),
          .y = UTILS::MATH::I_HALF * (rectangle2D.topLeft.y + rectangle2D.bottomRight.y)};
}

inline auto GetPointClippedToRectangle(
    const Point2dInt& point,
    const Rectangle2dInt& clipRectangle,
    const Point2dInt& connectingPointInsideClipRectangle) noexcept -> Point2dInt
{
  const auto lineClipper = LineClipper{clipRectangle};
  const auto clippedLine = lineClipper.GetClippedLine(LineFlt{
      .point1 = ToPoint2dFlt(connectingPointInsideClipRectangle), .point2 = ToPoint2dFlt(point)});
  Expects(clippedLine.clipResult != LineClipper::ClipResult::REJECTED);

  return ToPoint2dInt(clippedLine.line.point2);
}

inline auto GetRandomPoint(const UTILS::MATH::GoomRand& goomRand,
                           const Rectangle2dInt& rectangle2D) noexcept -> Point2dInt
{
  Expects(rectangle2D.topLeft.x <= rectangle2D.bottomRight.x);
  Expects(rectangle2D.topLeft.y <= rectangle2D.bottomRight.y);

  return {
      .x = goomRand.GetRandInRange(NumberRange{rectangle2D.topLeft.x, rectangle2D.bottomRight.x}),
      .y = goomRand.GetRandInRange(NumberRange{rectangle2D.topLeft.y, rectangle2D.bottomRight.y}),
  };
}

inline auto GetRandomPoint(const UTILS::MATH::GoomRand& goomRand,
                           const Rectangle2dInt& rectangle2D,
                           const Point2dInt& weightPoint,
                           const float closeToWeightPointT) noexcept -> Point2dInt
{
  const auto unweightedPoint = GetRandomPoint(goomRand, rectangle2D);

  return lerp(unweightedPoint, weightPoint, closeToWeightPointT);
}

} // namespace GOOM::UTILS::GRAPHICS
