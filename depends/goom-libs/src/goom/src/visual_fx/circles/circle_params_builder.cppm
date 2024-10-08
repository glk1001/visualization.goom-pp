module;

#include <algorithm>
#include <cstdint>
#include <vector>

export module Goom.VisualFx.CirclesFx.CircleParamsBuilder;

import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.VisualFx.CirclesFx.Circle;
import Goom.VisualFx.FxHelper;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;

using GOOM::UTILS::MATH::NumberRange;
using GOOM::UTILS::MATH::UNIT_RANGE;

export namespace GOOM::VISUAL_FX::CIRCLES
{

class CircleParamsBuilder
{
public:
  CircleParamsBuilder(uint32_t numCircles, const FxHelper& fxHelper) noexcept;

  enum class CircleStartModes : UnderlyingEnumType
  {
    SAME_RADIUS,
    REDUCING_RADIUS,
    FOUR_CORNERED_IN_MAIN,
  };
  auto SetCircleStartMode(CircleStartModes mode) noexcept -> void;

  enum class CircleTargetModes : UnderlyingEnumType
  {
    FOUR_CORNERS,
    SIMILAR_TARGETS,
  };
  auto SetCircleTargetMode(CircleTargetModes mode) noexcept -> void;

  auto SetMainCircleStartCentre(const Point2dInt& circleCentreStart) noexcept -> void;
  auto SetMainCircleCentreTarget(const Point2dInt& circleCentreTarget) noexcept -> void;
  [[nodiscard]] auto GetMainCircleCentreTarget() const noexcept -> const Point2dInt&;

  [[nodiscard]] auto GetCircleParams() const noexcept -> std::vector<Circle::Params>;
  [[nodiscard]] auto GetCircleParamsTargetsOnly(const std::vector<Circle::Params>& circleParams)
      const noexcept -> std::vector<Circle::Params>;

private:
  uint32_t m_numCircles;
  const FxHelper* m_fxHelper;
  uint32_t m_screenWidth         = m_fxHelper->GetDimensions().GetWidth();
  uint32_t m_screenHeight        = m_fxHelper->GetDimensions().GetHeight();
  Point2dInt m_screenCentre      = MidpointFromOrigin(GetPoint2dInt(m_screenWidth, m_screenHeight));
  Point2dInt m_topLeftCorner     = GetPoint2dInt(0U, 0U);
  Point2dInt m_topRightCorner    = GetPoint2dInt(m_screenWidth - 1, 0U);
  Point2dInt m_bottomLeftCorner  = GetPoint2dInt(0U, m_screenHeight - 1);
  Point2dInt m_bottomRightCorner = GetPoint2dInt(m_screenWidth - 1, m_screenHeight - 1);
  Point2dInt m_mainCircleCentreStart   = m_screenCentre;
  Point2dInt m_mainCircleCentreTarget  = m_screenCentre;
  CircleStartModes m_circleStartMode   = CircleStartModes::SAME_RADIUS;
  CircleTargetModes m_circleTargetMode = CircleTargetModes::FOUR_CORNERS;

  auto SetCircleCentreStarts(std::vector<Circle::Params>& circleParams) const noexcept -> void;
  [[nodiscard]] auto GetCircleCentreStarts(float mainCircleRadius) const noexcept
      -> std::vector<Point2dInt>;
  [[nodiscard]] auto GetAllSameCircleCentreStarts(float mainCircleRadius) const noexcept
      -> std::vector<Point2dInt>;
  [[nodiscard]] auto GetReducingRadiusCircleCentreStarts(float mainCircleRadius) const noexcept
      -> std::vector<Point2dInt>;
  [[nodiscard]] auto GetFourCornersCircleCentreStarts(float mainCircleRadius) const noexcept
      -> std::vector<Point2dInt>;
  [[nodiscard]] auto GetCircleRadii(float mainCircleRadius) const noexcept -> std::vector<float>;
  [[nodiscard]] auto GetSameCircleRadii(float mainCircleRadius) const noexcept
      -> std::vector<float>;
  [[nodiscard]] auto GetReducingCircleRadii(float mainCircleRadius) const noexcept
      -> std::vector<float>;
  [[nodiscard]] auto GetFourCornersCircleRadii(float mainCircleRadius) const noexcept
      -> std::vector<float>;
  [[nodiscard]] auto GetMainCircleRadius() const noexcept -> float;
  [[nodiscard]] auto GetCircleRadiusReducer() const noexcept -> float;

  auto SetCircleCentreTargets(std::vector<Circle::Params>& circleParams) const noexcept -> void;
  [[nodiscard]] auto GetCircleCentreTargets(const Point2dInt& target) const noexcept
      -> std::vector<Point2dInt>;
  [[nodiscard]] auto GetSimilarCircleCentreTargets(const Point2dInt& target) const noexcept
      -> std::vector<Point2dInt>;
  [[nodiscard]] auto GetFourCornersCircleCentreTargets(const Point2dInt& target) const noexcept
      -> std::vector<Point2dInt>;
};

} // namespace GOOM::VISUAL_FX::CIRCLES

namespace GOOM::VISUAL_FX::CIRCLES
{

inline auto CircleParamsBuilder::SetCircleStartMode(const CircleStartModes mode) noexcept -> void
{
  m_circleStartMode = mode;
}

inline auto CircleParamsBuilder::SetCircleTargetMode(const CircleTargetModes mode) noexcept -> void
{
  m_circleTargetMode = mode;
}

inline auto CircleParamsBuilder::SetMainCircleStartCentre(
    const Point2dInt& circleCentreStart) noexcept -> void
{
  m_mainCircleCentreStart = circleCentreStart;
}

inline auto CircleParamsBuilder::GetMainCircleCentreTarget() const noexcept -> const Point2dInt&
{
  return m_mainCircleCentreTarget;
}

inline auto CircleParamsBuilder::SetMainCircleCentreTarget(
    const Point2dInt& circleCentreTarget) noexcept -> void
{
  m_mainCircleCentreTarget = circleCentreTarget;
}

} // namespace GOOM::VISUAL_FX::CIRCLES

module :private;

namespace GOOM::VISUAL_FX::CIRCLES
{

using UTILS::MATH::HALF;
using UTILS::MATH::U_FIFTH;

static constexpr auto FIXED_NUM_CIRCLES = 5U;

CircleParamsBuilder::CircleParamsBuilder(const uint32_t numCircles,
                                         const FxHelper& fxHelper) noexcept
  : m_numCircles{numCircles}, m_fxHelper{&fxHelper}
{
}

auto CircleParamsBuilder::GetCircleParamsTargetsOnly(
    const std::vector<Circle::Params>& circleParams) const noexcept -> std::vector<Circle::Params>
{
  auto newCircleParams = circleParams;

  SetCircleCentreTargets(newCircleParams);

  return newCircleParams;
}

auto CircleParamsBuilder::GetCircleParams() const noexcept -> std::vector<Circle::Params>
{
  auto circleParams = std::vector<Circle::Params>(m_numCircles);

  SetCircleCentreStarts(circleParams);
  SetCircleCentreTargets(circleParams);

  return circleParams;
}

inline auto CircleParamsBuilder::SetCircleCentreStarts(
    std::vector<Circle::Params>& circleParams) const noexcept -> void
{
  const auto mainCircleRadius   = GetMainCircleRadius();
  const auto circleRadii        = GetCircleRadii(mainCircleRadius);
  const auto circleCentreStarts = GetCircleCentreStarts(mainCircleRadius);

  for (auto i = 0U; i < m_numCircles; ++i)
  {
    circleParams[i].toTargetParams.circleRadius   = circleRadii[i];
    circleParams[i].fromTargetParams.circleRadius = circleRadii[i];

    circleParams[i].toTargetParams.circleCentreStart   = circleCentreStarts[i];
    circleParams[i].fromTargetParams.circleCentreStart = circleCentreStarts[i];
  }
}

inline auto CircleParamsBuilder::GetMainCircleRadius() const noexcept -> float
{
  static constexpr auto RADIUS_MARGIN_RANGE = NumberRange{100.0F, 200.0F};
  const auto radiusMargin = m_fxHelper->GetGoomRand().GetRandInRange<RADIUS_MARGIN_RANGE>();

  const auto maxRadius = 0.5F * static_cast<float>(std::min(m_screenWidth, m_screenHeight));

  return maxRadius - radiusMargin;
}

auto CircleParamsBuilder::GetCircleRadii(const float mainCircleRadius) const noexcept
    -> std::vector<float>
{
  if (m_circleStartMode == CircleStartModes::SAME_RADIUS)
  {
    return GetSameCircleRadii(mainCircleRadius);
  }
  if (m_circleStartMode == CircleStartModes::REDUCING_RADIUS)
  {
    return GetReducingCircleRadii(mainCircleRadius);
  }
  return GetFourCornersCircleRadii(mainCircleRadius);
}

auto CircleParamsBuilder::GetSameCircleRadii(const float mainCircleRadius) const noexcept
    -> std::vector<float>
{
  auto sameRadii = std::vector<float>(m_numCircles);
  std::ranges::fill(sameRadii, mainCircleRadius);
  return sameRadii;
}

auto CircleParamsBuilder::GetReducingCircleRadii(const float mainCircleRadius) const noexcept
    -> std::vector<float>
{
  const auto radiusReducer = GetCircleRadiusReducer();

  auto reducingRadii = std::vector<float>(m_numCircles);

  reducingRadii.at(0) = mainCircleRadius;
  for (auto i = 1U; i < m_numCircles; ++i)
  {
    reducingRadii[i] = radiusReducer * reducingRadii[i - 1];
  }

  return reducingRadii;
}

inline auto CircleParamsBuilder::GetCircleRadiusReducer() const noexcept -> float
{
  static constexpr auto RADIUS_REDUCER_RANGE = UNIT_RANGE;
  return m_fxHelper->GetGoomRand().GetRandInRange<RADIUS_REDUCER_RANGE>();
}

inline auto CircleParamsBuilder::GetFourCornersCircleRadii(
    const float mainCircleRadius) const noexcept -> std::vector<float>
{
  Expects(FIXED_NUM_CIRCLES == m_numCircles);

  const auto innerCircleRadius = HALF * mainCircleRadius;

  auto fourCornersRadii = std::vector<float>(m_numCircles);

  fourCornersRadii.at(0) = mainCircleRadius;
  for (auto i = 1U; i < m_numCircles; ++i)
  {
    fourCornersRadii.at(i) = innerCircleRadius;
  }

  return fourCornersRadii;
}

inline auto CircleParamsBuilder::GetCircleCentreStarts(const float mainCircleRadius) const noexcept
    -> std::vector<Point2dInt>
{
  switch (m_circleStartMode)
  {
    case CircleStartModes::SAME_RADIUS:
      return GetAllSameCircleCentreStarts(mainCircleRadius);
    case CircleStartModes::REDUCING_RADIUS:
      return GetReducingRadiusCircleCentreStarts(mainCircleRadius);
    case CircleStartModes::FOUR_CORNERED_IN_MAIN:
      return GetFourCornersCircleCentreStarts(mainCircleRadius);
  }
}

inline auto CircleParamsBuilder::GetAllSameCircleCentreStarts(
    [[maybe_unused]] const float mainCircleRadius) const noexcept -> std::vector<Point2dInt>
{
  auto circleCentreStarts = std::vector<Point2dInt>(m_numCircles);
  std::ranges::fill(circleCentreStarts, m_mainCircleCentreStart);
  return circleCentreStarts;
}

inline auto CircleParamsBuilder::GetReducingRadiusCircleCentreStarts(
    [[maybe_unused]] const float mainCircleRadius) const noexcept -> std::vector<Point2dInt>
{
  auto circleCentreStarts = std::vector<Point2dInt>(m_numCircles);
  std::ranges::fill(circleCentreStarts, m_mainCircleCentreStart);
  return circleCentreStarts;
}

inline auto CircleParamsBuilder::GetFourCornersCircleCentreStarts(
    const float mainCircleRadius) const noexcept -> std::vector<Point2dInt>
{
  Expects(FIXED_NUM_CIRCLES == m_numCircles);

  const auto innerCircleRadius = HALF * mainCircleRadius;
  const auto offset            = static_cast<int32_t>(
      m_fxHelper->GetGoomRand().GetRandInRange<NumberRange{0.5F, 1.0F}>() * innerCircleRadius);

  auto circleCentreStarts = std::vector<Point2dInt>(m_numCircles);

  circleCentreStarts.at(0) = m_mainCircleCentreStart;
  circleCentreStarts.at(1) = {.x = m_mainCircleCentreStart.x,
                              .y = m_mainCircleCentreStart.y - offset};
  circleCentreStarts.at(2) = {.x = m_mainCircleCentreStart.x + offset,
                              .y = m_mainCircleCentreStart.y};
  circleCentreStarts.at(3) = {.x = m_mainCircleCentreStart.x,
                              .y = m_mainCircleCentreStart.y + offset};
  circleCentreStarts.at(4) = {.x = m_mainCircleCentreStart.x - offset,
                              .y = m_mainCircleCentreStart.y};

  return circleCentreStarts;
}

inline auto CircleParamsBuilder::SetCircleCentreTargets(
    std::vector<Circle::Params>& circleParams) const noexcept -> void
{
  const auto circleCentreTargets = GetCircleCentreTargets(m_mainCircleCentreTarget);

  for (auto i = 0U; i < m_numCircles; ++i)
  {
    circleParams[i].toTargetParams.circleCentreTarget   = circleCentreTargets.at(i);
    circleParams[i].fromTargetParams.circleCentreTarget = circleCentreTargets.at(i);
  }
}

inline auto CircleParamsBuilder::GetCircleCentreTargets(const Point2dInt& target) const noexcept
    -> std::vector<Point2dInt>
{
  if (m_circleTargetMode == CircleTargetModes::SIMILAR_TARGETS)
  {
    return GetSimilarCircleCentreTargets(target);
  }
  return GetFourCornersCircleCentreTargets(target);
}

inline auto CircleParamsBuilder::GetSimilarCircleCentreTargets(
    const Point2dInt& target) const noexcept -> std::vector<Point2dInt>
{
  auto circleCentreTargets = std::vector<Point2dInt>(m_numCircles);

  static constexpr auto MIN_OFFSET = 0U;
  const auto maxOffset             = U_FIFTH * std::min(m_screenWidth, m_screenHeight);

  for (auto& circleCentre : circleCentreTargets)
  {
    circleCentre =
        target +
        GetVec2dInt(m_fxHelper->GetGoomRand().GetRandInRange(NumberRange{MIN_OFFSET, maxOffset}),
                    m_fxHelper->GetGoomRand().GetRandInRange(NumberRange{MIN_OFFSET, maxOffset}));
  }

  return circleCentreTargets;
}

inline auto CircleParamsBuilder::GetFourCornersCircleCentreTargets(
    const Point2dInt& target) const noexcept -> std::vector<Point2dInt>
{
  Expects(FIXED_NUM_CIRCLES == m_numCircles);

  auto circleCentreTargets = std::vector<Point2dInt>(m_numCircles);

  static constexpr auto T_RANGE = NumberRange{0.05F, 0.95F};
  const auto t                  = m_fxHelper->GetGoomRand().GetRandInRange<T_RANGE>();

  circleCentreTargets.at(0) = target;
  circleCentreTargets.at(1) = lerp(m_topLeftCorner, circleCentreTargets.at(0), t);
  circleCentreTargets.at(2) = lerp(m_topRightCorner, circleCentreTargets.at(0), t);
  circleCentreTargets.at(3) = lerp(m_bottomLeftCorner, circleCentreTargets.at(0), t);
  circleCentreTargets.at(4) = lerp(m_bottomRightCorner, circleCentreTargets.at(0), t);

  return circleCentreTargets;
}

} // namespace GOOM::VISUAL_FX::CIRCLES
