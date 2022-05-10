#include "shape_parts.h"

#undef NO_LOGGING

#include "color/random_colormaps.h"
#include "color/random_colormaps_manager.h"
#include "goom/logging.h"
#include "goom_config.h"
#include "point2d.h"
#include "shape_paths.h"
#include "utils/math/goom_rand_base.h"
#include "utils/t_values.h"

#include <cassert>

namespace GOOM::VISUAL_FX::SHAPES
{

using COLOR::GetAllMapsUnweighted;
using COLOR::RandomColorMapsManager;
using COLOR::COLOR_DATA::ColorMapName;
using UTILS::Logging;
using UTILS::TValue;
using UTILS::MATH::AngleParams;
using UTILS::MATH::CircleFunction;
using UTILS::MATH::CirclePath;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::IsEven;
using UTILS::MATH::IsOdd;
using UTILS::MATH::LerpedPath;
using UTILS::MATH::Transform2d;
using UTILS::MATH::TransformedPath;
using UTILS::MATH::TWO_PI;

ShapePart::ShapePart(const IGoomRand& goomRand,
                     const PluginInfo& goomInfo,
                     RandomColorMapsManager& colorMapsManager,
                     const Params& params) noexcept
  : m_goomRand{goomRand},
    m_goomInfo{goomInfo},
    m_colorMapsManager{colorMapsManager},
    m_shapePathsTargetPoint{params.shapePathsTargetPoint},
    m_shapePartNum{params.shapePartNum},
    m_totalNumShapeParts{params.totalNumShapeParts},
    m_minRadiusFraction{params.minRadiusFraction},
    m_maxRadiusFraction{params.maxRadiusFraction},
    m_maxNumShapePaths{params.maxNumShapePaths},
    m_minShapePathSpeed{params.minShapePathSpeed},
    m_maxShapePathSpeed{params.maxShapePathSpeed},
    m_minShapeDotRadius{params.minShapeDotRadius},
    m_maxShapeDotRadius{params.maxShapeDotRadius},
    m_allColorsT{TValue::StepType::CONTINUOUS_REVERSIBLE, GetShapePathSpeed(params.tMinMaxLerp)}
{
  assert(m_totalNumShapeParts > 0);
  assert(m_shapePartNum < m_totalNumShapeParts);
  assert(0.0F <= m_minRadiusFraction);
  assert(m_minRadiusFraction < m_maxRadiusFraction);
  assert(1 <= m_minShapeDotRadius);
  assert(m_minShapeDotRadius <= m_maxShapeDotRadius);
  assert(m_maxNumShapePaths >= MIN_NUM_SHAPE_PATHS);
  assert(m_minShapePathSpeed > 0.0F);
  assert(m_minShapePathSpeed < m_maxShapePathSpeed);
  assert(m_maxShapePathSpeed < 1.0F);
}

auto ShapePart::GetInitialColorInfo() const noexcept -> ColorInfo
{
  return {GetAllMapsUnweighted(m_goomRand),
          ColorMapName::_NULL,
          GetAllMapsUnweighted(m_goomRand),
          ColorMapName::_NULL,
          GetAllMapsUnweighted(m_goomRand),
          ColorMapName::_NULL,
          m_goomRand.GetRandInRange(MIN_INNER_COLOR_MIX_T, MAX_INNER_COLOR_MIX_T)};
}

auto ShapePart::SetShapePathsTargetPoint(const Point2dInt& targetPoint) -> void
{
  if (m_shapePathsTargetPoint == targetPoint)
  {
    m_needToUpdateTargetPoint = false;
    return;
  }

  m_needToUpdateTargetPoint = true;
  m_newShapePathsTargetPoint = targetPoint;
}

auto ShapePart::SetMinMaxShapePathSpeeds(const float minShapePathSpeed,
                                         const float maxShapePathSpeed) noexcept -> void
{
  m_minShapePathSpeed = minShapePathSpeed;
  m_maxShapePathSpeed = maxShapePathSpeed;
  assert(m_minShapePathSpeed > 0.0F);
  assert(m_minShapePathSpeed < m_maxShapePathSpeed);
  assert(m_maxShapePathSpeed < 1.0F);
}

auto ShapePart::UpdateShapePathTargets() noexcept -> void
{
  if (not m_needToUpdateTargetPoint)
  {
    return;
  }
  if (not m_shapePaths.at(0).HasJustHitStartBoundary())
  {
    return;
  }

  m_oldShapePathsTargetPoint =
      lerp(m_oldShapePathsTargetPoint, m_shapePathsTargetPoint, m_oldToNewLerpT());
  m_shapePathsTargetPoint = m_newShapePathsTargetPoint;

  std::for_each(begin(m_shapePaths), end(m_shapePaths),
                [this](ShapePath& shapePath) { UpdateShapePathTransform(shapePath); });

  ResetTs(0.0F);

  m_oldToNewLerpT.Reset(0.0F);
  m_needToUpdateTargetPoint = false;
}

inline auto ShapePart::UpdateShapePathTransform(ShapePath& shapePath) const noexcept -> void
{
  auto& basePath = dynamic_cast<LerpedPath&>(shapePath.GetIPath());
  auto& oldPath = dynamic_cast<TransformedPath&>(basePath.GetPath1());
  auto& newPath = dynamic_cast<TransformedPath&>(basePath.GetPath2());

  Transform2d oldTransform = oldPath.GetTransform();
  Transform2d newTransform = newPath.GetTransform();

  oldTransform.SetTranslation(Vec2dFlt{m_oldShapePathsTargetPoint.ToFlt()});
  newTransform.SetTranslation(Vec2dFlt{m_shapePathsTargetPoint.ToFlt()});

  oldPath.SetTransform(oldTransform);
  newPath.SetTransform(newTransform);
}

inline auto ShapePart::GetTransform2d(const Vec2dFlt& targetPoint,
                                      const float radius,
                                      const float scale,
                                      const float rotate) noexcept -> Transform2d
{
  const auto centre = Vec2dFlt{
      targetPoint.x - (scale * radius * std::cos(rotate)),
      targetPoint.y - (scale * radius * std::sin(rotate)),
  };
  return Transform2d{rotate, scale, centre};
}

auto ShapePart::GetRandomizedShapePaths() noexcept -> std::vector<ShapePath>
{
  const uint32_t numShapePaths =
      m_goomRand.GetRandInRange(MIN_NUM_SHAPE_PATHS, m_maxNumShapePaths + 1);

  static constexpr float MIN_MIN_SCALE = 0.9F;
  static constexpr float MAX_MIN_SCALE = 1.0F;
  static constexpr float MIN_MAX_SCALE = 1.0F + UTILS::MATH::SMALL_FLOAT;
  static constexpr float MAX_MAX_SCALE = 1.5F;
  static constexpr float PROB_SCALE_EQUALS_ONE = 0.9F;
  const bool probScaleEqualsOne = m_goomRand.ProbabilityOf(PROB_SCALE_EQUALS_ONE);
  const auto minScale =
      float{probScaleEqualsOne ? 1.0F : m_goomRand.GetRandInRange(MIN_MIN_SCALE, MAX_MIN_SCALE)};
  const auto maxScale =
      float{probScaleEqualsOne ? 1.0F : m_goomRand.GetRandInRange(MIN_MAX_SCALE, MAX_MAX_SCALE)};

  return GetShapePaths(numShapePaths, minScale, maxScale);
}

inline auto ShapePart::GetShapePaths(const uint32_t numShapePaths,
                                     const float minScale,
                                     const float maxScale) noexcept -> std::vector<ShapePath>
{

  const Vec2dFlt targetPointFlt{m_shapePathsTargetPoint.ToFlt()};
  const Vec2dFlt oldTargetPointFlt{m_oldShapePathsTargetPoint.ToFlt()};

  static constexpr float MIN_ANGLE = 0.0F;
  static constexpr float MAX_ANGLE = TWO_PI;
  auto stepFraction = TValue{TValue::StepType::SINGLE_CYCLE, numShapePaths};

  const auto radius = float{GetCircleRadius()};
  const auto direction = CircleFunction::Direction{GetCircleDirection()};
  const auto speed = float{GetShapePathSpeed(m_fixedTMinMaxLerp)};

  std::vector<ShapePath> shapePaths{};

  for (uint32_t i = 0; i < numShapePaths; ++i)
  {
    const auto rotate = float{STD20::lerp(MIN_ANGLE, MAX_ANGLE, stepFraction())};
    const auto scale = float{STD20::lerp(minScale, maxScale, stepFraction())};
    const auto circlePath = GetCirclePath(radius, direction, speed);

    const auto newTransform = GetTransform2d(targetPointFlt, radius, scale, rotate);
    const auto newBasePath = std::make_shared<TransformedPath>(circlePath.GetClone(), newTransform);

    const auto oldTransform = GetTransform2d(oldTargetPointFlt, radius, scale, rotate);
    const auto oldBasePath = std::make_shared<TransformedPath>(circlePath.GetClone(), oldTransform);

    const auto basePath = std::make_shared<LerpedPath>(oldBasePath, newBasePath, m_oldToNewLerpT);
    const auto colorInfo = MakeShapePathColorInfo();

    shapePaths.emplace_back(basePath, colorInfo);

    if (SqDistance(shapePaths.at(i).GetIPath().GetStartPos(),
                   lerp(m_oldShapePathsTargetPoint, m_shapePathsTargetPoint, m_oldToNewLerpT())) >
        4)
    {
      const Point2dInt lerpedTarget =
          lerp(m_oldShapePathsTargetPoint, m_shapePathsTargetPoint, m_oldToNewLerpT());
      LogError("shapePaths.at({}).GetIPath().GetStartPos() = {}, {}", i,
               shapePaths.at(i).GetIPath().GetStartPos().x,
               shapePaths.at(i).GetIPath().GetStartPos().y);
      LogError("m_oldShapesTargetPoint = {}, {}", m_oldShapePathsTargetPoint.x,
               m_shapePathsTargetPoint.y);
      LogError("m_shapesTargetPoint = {}, {}", m_shapePathsTargetPoint.x,
               m_shapePathsTargetPoint.y);
      LogError("lerpedTarget = {}, {}", lerpedTarget.x, lerpedTarget.y);
      LogError("targetPointFlt = {}, {}", targetPointFlt.x, targetPointFlt.y);
      LogError("radius = {}", radius);
      LogError("rotate = {}", rotate);
      LogError("std::cos(rotate) = {}", std::cos(rotate));
      LogError("std::sin(rotate) = {}", std::sin(rotate));
      LogError("scale = {}", scale);
      LogError("speed = {}", speed);
    }
    assert(SqDistance(
               shapePaths.at(i).GetIPath().GetStartPos(),
               lerp(m_oldShapePathsTargetPoint, m_shapePathsTargetPoint, m_oldToNewLerpT())) <= 4);

    stepFraction.Increment();
  }

  return shapePaths;
}

inline auto ShapePart::MakeShapePathColorInfo() noexcept -> ShapePath::ColorInfo
{
  return ShapePath::ColorInfo{
      m_colorMapsManager.AddDefaultColorMapInfo(m_goomRand),
      m_colorMapsManager.AddDefaultColorMapInfo(m_goomRand),
      m_colorMapsManager.AddDefaultColorMapInfo(m_goomRand),
  };
}

inline auto ShapePart::GetCircleRadius() const noexcept -> float
{
  const auto minDimension = static_cast<float>(
      std::min(m_goomInfo.GetScreenInfo().width, m_goomInfo.GetScreenInfo().height));
  const auto minRadius = float{m_minRadiusFraction * minDimension};
  const auto maxRadius = float{m_maxRadiusFraction * minDimension};
  const auto t = static_cast<float>(m_shapePartNum) / static_cast<float>(m_totalNumShapeParts - 1);

  return STD20::lerp(minRadius, maxRadius, t);
}

inline auto ShapePart::GetCircleDirection() const noexcept -> CircleFunction::Direction
{
  if (m_useEvenShapePartNumsForDirection)
  {
    return IsEven(m_shapePartNum) ? CircleFunction::Direction::COUNTER_CLOCKWISE
                                  : CircleFunction::Direction::CLOCKWISE;
  }
  return IsOdd(m_shapePartNum) ? CircleFunction::Direction::COUNTER_CLOCKWISE
                               : CircleFunction::Direction::CLOCKWISE;
}

inline auto ShapePart::GetCirclePath(const float radius,
                                     const CircleFunction::Direction direction,
                                     const float speed) noexcept -> CirclePath
{
  auto positionT = std::make_unique<TValue>(TValue::StepType::CONTINUOUS_REVERSIBLE, speed);

  const auto params = ShapeFunctionParams{radius, AngleParams{}, direction};

  return CirclePath{std::move(positionT), GetCircleFunction(params)};
}

inline auto ShapePart::GetCircleFunction(const ShapeFunctionParams& params) -> CircleFunction
{
  static constexpr Vec2dFlt CENTRE_POS{0.0F, 0.0F};

  return {CENTRE_POS, params.radius, params.angleParams, params.direction};
}

auto ShapePart::SetWeightedMainColorMaps(
    const std::shared_ptr<COLOR::RandomColorMaps> weightedMaps) noexcept -> void
{
  m_colorInfo.mainColorMaps = weightedMaps;
  m_colorInfo.mainColormapName =
      weightedMaps->GetRandomColorMapName(weightedMaps->GetRandomGroup());

  std::for_each(begin(m_shapePaths), end(m_shapePaths),
                [this](const ShapePath& shapePath) { shapePath.UpdateMainColorInfo(*this); });
}

auto ShapePart::SetWeightedLowColorMaps(
    const std::shared_ptr<COLOR::RandomColorMaps> weightedMaps) noexcept -> void
{
  m_colorInfo.lowColorMaps = weightedMaps;
  m_colorInfo.lowColormapName = weightedMaps->GetRandomColorMapName(weightedMaps->GetRandomGroup());

  std::for_each(begin(m_shapePaths), end(m_shapePaths),
                [this](const ShapePath& shapePath) { shapePath.UpdateLowColorInfo(*this); });
}

auto ShapePart::SetWeightedInnerColorMaps(
    const std::shared_ptr<COLOR::RandomColorMaps> weightedMaps) noexcept -> void
{
  m_colorInfo.innerColorMix =
      m_goomRand.GetRandInRange(MIN_INNER_COLOR_MIX_T, MAX_INNER_COLOR_MIX_T);

  m_colorInfo.innerColorMaps = weightedMaps;
  m_colorInfo.innerColormapName =
      weightedMaps->GetRandomColorMapName(weightedMaps->GetRandomGroup());

  std::for_each(begin(m_shapePaths), end(m_shapePaths),
                [this](const ShapePath& shapePath) { shapePath.UpdateInnerColorInfo(*this); });
}

auto ShapePart::UpdateMainColorMapId(
    const COLOR::RandomColorMapsManager::ColorMapId mainColorMapId) noexcept -> void
{
  const std::shared_ptr<COLOR::RandomColorMaps>& mainColorMaps = m_colorInfo.mainColorMaps;

  const COLOR::COLOR_DATA::ColorMapName colormapName =
      not m_useRandomColorNames
          ? m_colorInfo.mainColormapName
          : mainColorMaps->GetRandomColorMapName(mainColorMaps->GetRandomGroup());

  m_colorMapsManager.UpdateColorMapInfo(mainColorMapId,
                                        {mainColorMaps, colormapName, COLOR_MAP_TYPES});
}

auto ShapePart::UpdateLowColorMapId(
    const COLOR::RandomColorMapsManager::ColorMapId lowColorMapId) noexcept -> void
{
  const std::shared_ptr<COLOR::RandomColorMaps>& lowColorMaps = m_colorInfo.lowColorMaps;

  const COLOR::COLOR_DATA::ColorMapName colormapName =
      not m_useRandomColorNames
          ? m_colorInfo.lowColormapName
          : lowColorMaps->GetRandomColorMapName(lowColorMaps->GetRandomGroup());

  m_colorMapsManager.UpdateColorMapInfo(lowColorMapId,
                                        {lowColorMaps, colormapName, COLOR_MAP_TYPES});
}

auto ShapePart::UpdateInnerColorMapId(
    const COLOR::RandomColorMapsManager::ColorMapId innerColorMapId) noexcept -> void
{
  const std::shared_ptr<COLOR::RandomColorMaps>& innerColorMaps = m_colorInfo.innerColorMaps;

  const COLOR::COLOR_DATA::ColorMapName colormapName =
      not m_useRandomColorNames
          ? m_colorInfo.innerColormapName
          : innerColorMaps->GetRandomColorMapName(innerColorMaps->GetRandomGroup());

  m_colorMapsManager.UpdateColorMapInfo(innerColorMapId,
                                        {innerColorMaps, colormapName, COLOR_MAP_TYPES});
}

void ShapePart::DoRandomChanges() noexcept
{
  if (not m_shapePaths.at(0).HasJustHitAnyBoundary())
  {
    return;
  }

  SetRandomizedShapePaths();
  SetShapePathsSpeed();
  ChangeAllColorMapsNow();
  ChangeAllColorsT();
}

inline auto ShapePart::ChangeAllColorMapsNow() noexcept -> void
{
  static constexpr float PROB_USE_RANDOM_COLOR_NAMES = 0.2F;
  m_useRandomColorNames = m_goomRand.ProbabilityOf(PROB_USE_RANDOM_COLOR_NAMES);
  m_colorMapsManager.ChangeAllColorMapsNow();
}

inline auto ShapePart::ChangeAllColorsT() noexcept -> void
{
  const float t = m_goomRand.GetRandInRange(0.0F, 1.0F);
  m_allColorsT.SetStepSize(STD20::lerp(MIN_COLOR_MAP_SPEED, MAX_COLOR_MAP_SPEED, t));
}

inline auto ShapePart::SetShapePathsSpeed() noexcept -> void
{
  m_currentTMinMaxLerp = m_useRandomShapePathsSpeed
                             ? GetNewRandomMinMaxLerpT(m_goomRand, m_currentTMinMaxLerp)
                             : m_fixedTMinMaxLerp;
  const float newSpeed = GetShapePathSpeed(m_currentTMinMaxLerp);
  std::for_each(begin(m_shapePaths), end(m_shapePaths),
                [&newSpeed](ShapePath& path) { path.SetStepSize(newSpeed); });

  m_dotRadiusT.SetNumSteps(
      STD20::lerp(MIN_DOT_RADIUS_STEPS, MAX_DOT_RADIUS_STEPS, m_currentTMinMaxLerp));
}

inline auto ShapePart::GetShapePathSpeed(const float tMinMaxLerp) const noexcept -> float
{
  return STD20::lerp(m_minShapePathSpeed, m_maxShapePathSpeed, tMinMaxLerp);
}

} // namespace GOOM::VISUAL_FX::SHAPES