#undef NO_LOGGING // NOLINT: This maybe be defined on command line.

#include "raindrop_positions.h"

#include "goom/goom_config.h"
#include "goom/point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/math/parametric_functions2d.h"
#include "utils/math/paths.h"
#include "utils/t_values.h"
#include "visual_fx/fx_helper.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX::RAINDROPS
{

using UTILS::IncrementedValue;
using UTILS::TValue;
using UTILS::MATH::TWO_PI;

RaindropPositions::RaindropPositions(const FxHelper& fxHelper,
                                     const uint32_t numRaindrops,
                                     const Params& params) noexcept
  : m_fxHelper{&fxHelper},
    m_params{params},
    m_raindropPositions{GetNewRaindropPositions(numRaindrops)},
    m_raindropPaths{GetNewRaindropPaths()}
{
  SetNewTargetRectangleWeightPoint(m_params.targetRectangleWeightPoint);
}

auto RaindropPositions::UpdatePositions() noexcept -> void
{
  m_rectangleWeightPointT.Increment();
  m_raindropPositionT.Increment();
  std::for_each(
      begin(m_raindropPaths), end(m_raindropPaths), [](auto& path) { path->IncrementT(); });
}

auto RaindropPositions::GetCurrentRectangleWeightPoint() const noexcept -> Point2dInt
{
  return lerp(m_params.sourceRectangleWeightPoint,
              m_params.targetRectangleWeightPoint,
              m_rectangleWeightPointT());
}

auto RaindropPositions::SetNewTargetRectangleWeightPoint(
    const Point2dInt& targetRectangleWeightPoint) noexcept -> void
{
  m_params.sourceRectangleWeightPoint = GetCurrentRectangleWeightPoint();
  m_params.targetRectangleWeightPoint =
      GetAcceptableTargetRectangleWeightPoint(targetRectangleWeightPoint);
  m_rectangleWeightPointT.SetNumSteps(
      m_fxHelper->GetGoomRand().GetRandInRange(MIN_WEIGHT_POINT_STEPS, MAX_WEIGHT_POINT_STEPS + 1));
  m_rectangleWeightPointT.Reset();
}

auto RaindropPositions::GetAcceptableTargetRectangleWeightPoint(
    const Point2dInt& requestedTargetRectangleWeightPoint) const noexcept -> Point2dInt
{
  return lerp(
      requestedTargetRectangleWeightPoint, m_screenCentre, WEIGHT_POINT_CLOSE_TO_SCREEN_CENTRE_T);
}

auto RaindropPositions::GetNewRaindropPositions(const uint32_t numRaindrops) const noexcept
    -> std::vector<Point2dInt>
{
  return GetPositionsOnConcentricCircles(
      numRaindrops,
      GetCurrentRectangleWeightPoint(),
      static_cast<uint32_t>(m_params.enclosingRadius /
                            static_cast<float>(m_params.numConcentricCircles)),
      static_cast<uint32_t>(m_params.enclosingRadius));
}

auto RaindropPositions::GetPositionsOnConcentricCircles(const uint32_t numPositions,
                                                        const Point2dInt& centre,
                                                        const uint32_t minRadius,
                                                        const uint32_t maxRadius) const noexcept
    -> std::vector<Point2dInt>
{
  Expects(numPositions > 0);
  Expects(minRadius <= maxRadius);

  auto positions = std::vector<Point2dInt>{};
  positions.reserve(numPositions);

  IncrementedValue<uint32_t> radius{
      minRadius, maxRadius, TValue::StepType::SINGLE_CYCLE, m_params.numConcentricCircles - 1};

  const auto numPositionsPerCircle = numPositions / m_params.numConcentricCircles;
  Expects((numPositionsPerCircle * m_params.numConcentricCircles) == numPositions);

  Expects(numPositionsPerCircle >= m_params.numConcentricCircles);
  auto numPositionsInThisCircle = numPositionsPerCircle - (m_params.numConcentricCircles / 2);
  auto numPositionsSoFar        = 0U;
  for (auto circleNum = 1U; circleNum <= m_params.numConcentricCircles; ++circleNum)
  {
    if (m_params.numConcentricCircles == circleNum)
    {
      Expects(radius() == maxRadius);
      numPositionsInThisCircle = numPositions - numPositionsSoFar;
    }
    const auto circlePositions = GetPositionsOnCircle(numPositionsInThisCircle, centre, radius());
    positions.insert(positions.end(), circlePositions.begin(), circlePositions.end());
    radius.Increment();
    numPositionsSoFar += numPositionsInThisCircle;
    ++numPositionsInThisCircle;
  }
  Ensures(positions.size() == numPositions);

  return positions;
}

auto RaindropPositions::GetPositionsOnCircle(const uint32_t num,
                                             const Point2dInt& centre,
                                             const uint32_t radius) const noexcept
    -> std::vector<Point2dInt>
{
  auto angle     = IncrementedValue<float>{0.0F, TWO_PI, TValue::StepType::SINGLE_CYCLE, num};
  auto positions = std::vector<Point2dInt>{};
  positions.reserve(num);

  const auto startAngle = m_raindropPositionT() * TWO_PI;
  for (auto i = 0U; i < num; ++i)
  {
    const auto nextAngle = startAngle + angle();
    positions.emplace_back(centre +
                           ToVec2dInt(Vec2dFlt{static_cast<float>(radius) * std::sin(nextAngle),
                                               static_cast<float>(radius) * std::cos(nextAngle)}));
    angle.Increment();
  }
  Ensures(positions.size() == num);

  return positions;
}

auto RaindropPositions::ResetPositions(const uint32_t newNumConcentricCircles,
                                       const uint32_t newNumRaindrops) noexcept -> void
{
  if ((newNumConcentricCircles == m_params.numConcentricCircles) and
      (newNumRaindrops == m_raindropPositions.size()))
  {
    return;
  }

  if ((newNumConcentricCircles != m_params.numConcentricCircles) or
      (newNumRaindrops > m_raindropPositions.size()))
  {
    m_params.numConcentricCircles = newNumConcentricCircles;
    m_raindropPositions           = GetNewRaindropPositions(newNumRaindrops);
    m_raindropPaths               = GetNewRaindropPaths();
  }
  else
  {
    const auto numExcessElements =
        static_cast<int64_t>(m_raindropPositions.size() - newNumRaindrops);
    m_raindropPositions.erase(end(m_raindropPositions) - numExcessElements,
                              end(m_raindropPositions));
    m_raindropPaths.erase(end(m_raindropPaths) - numExcessElements, end(m_raindropPaths));
  }

  Ensures(m_raindropPositions.size() == newNumRaindrops);
}

auto RaindropPositions::GetNewRaindropPaths() const noexcept
    -> std::vector<std::unique_ptr<UTILS::MATH::IPath>>
{
  Expects(not m_raindropPositions.empty());

  using UTILS::MATH::AngleParams;
  using UTILS::MATH::OscillatingFunction;
  using UTILS::MATH::ParametricPath;

  using OscillatingPath = ParametricPath<OscillatingFunction>;

  static constexpr auto FREQ               = 5.0F;
  static constexpr auto OSCILLATING_PARAMS = std::array{
      OscillatingFunction::Params{12.0F, FREQ, FREQ},
      OscillatingFunction::Params{12.1F, FREQ, FREQ},
      OscillatingFunction::Params{12.2F, FREQ, FREQ},
      OscillatingFunction::Params{12.3F, FREQ, FREQ},
      OscillatingFunction::Params{12.4F, FREQ, FREQ}
  };

  static constexpr auto NUM_OSCILLATING_PATH_INCREMENTS = NUM_RAINDROP_POSITION_INCREMENTS;
  const auto oscillatingPathDelayPoints                 = m_raindropDelayPoints;
  const auto oscillatingPathAngleT                      = TValue{
      TValue::NumStepsProperties{UTILS::TValue::StepType::CONTINUOUS_REPEATABLE,
                                 NUM_OSCILLATING_PATH_INCREMENTS},
      oscillatingPathDelayPoints
  };

  auto paths = std::vector<std::unique_ptr<UTILS::MATH::IPath>>{};
  //for (const auto& centrePos : m_raindropPositions)
  for (auto dropNum = 0U; dropNum < m_raindropPositions.size(); ++dropNum)
  {
    const auto centrePos      = m_raindropPositions.at(dropNum);
    const auto toPos          = ToPoint2dFlt(m_raindropPositions.at(
        (dropNum + (m_raindropPositions.size() / 2)) % m_raindropPositions.size()));
    const auto startAndEndPos = UTILS::MATH::StartAndEndPos{ToPoint2dFlt(centrePos), toPos};
    const auto params         = OSCILLATING_PARAMS.at(dropNum % OSCILLATING_PARAMS.size());
    paths.emplace_back(std::make_unique<OscillatingPath>(
        std::make_unique<TValue>(
            TValue::NumStepsProperties{UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE,
                                       NUM_OSCILLATING_PATH_INCREMENTS},
            oscillatingPathDelayPoints),
        oscillatingPathAngleT,
        startAndEndPos,
        params));
  }

  Ensures(paths.size() == m_raindropPositions.size());

  return paths;
}

} // namespace GOOM::VISUAL_FX::RAINDROPS
