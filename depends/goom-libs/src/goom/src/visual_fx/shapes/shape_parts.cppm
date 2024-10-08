module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

module Goom.VisualFx.ShapesFx:ShapeParts;

import Goom.Color.RandomColorMaps;
import Goom.Color.RandomColorMapsGroups;
import Goom.Draw.GoomDrawBase;
import Goom.Utils.Timer;
import Goom.Utils.StepSpeed;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Utils.Math.ParametricFunctions2d;
import Goom.Utils.Math.Paths;
import Goom.Utils.Math.Transform2d;
import Goom.Utils.Math.TValues;
import Goom.VisualFx.FxHelper;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;
import :ShapePaths;

using GOOM::COLOR::GetUnweightedRandomColorMaps;
using GOOM::COLOR::WeightedRandomColorMaps;
using GOOM::DRAW::MultiplePixels;
using GOOM::UTILS::StepSpeed;
using GOOM::UTILS::MATH::AngleParams;
using GOOM::UTILS::MATH::CircleFunction;
using GOOM::UTILS::MATH::CirclePath;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::IsEven;
using GOOM::UTILS::MATH::IsOdd;
using GOOM::UTILS::MATH::NumberRange;
using GOOM::UTILS::MATH::SMALL_FLOAT;
using GOOM::UTILS::MATH::Transform2d;
using GOOM::UTILS::MATH::TransformedPath;
using GOOM::UTILS::MATH::TValue;
using GOOM::UTILS::MATH::TWO_PI;

namespace GOOM::VISUAL_FX::SHAPES
{

class ShapePart
{
public:
  struct Params
  {
    uint32_t shapePartNum{};
    uint32_t totalNumShapeParts{};
    float minRadiusFraction{};
    float maxRadiusFraction{};
    int32_t minShapeDotRadius{};
    int32_t maxShapeDotRadius{};
    uint32_t maxNumShapePaths{};
    float tMinMaxLerp{};
    Point2dInt shapePathsTargetPoint;
    uint32_t shapePathsMinNumSteps{};
    uint32_t shapePathsMaxNumSteps{};
  };

  ShapePart(FxHelper& fxHelper, const Params& params, PixelChannelType defaultAlpha) noexcept;
  ShapePart(const ShapePart&) noexcept           = delete;
  ShapePart(ShapePart&&) noexcept                = default;
  ~ShapePart() noexcept                          = default;
  auto operator=(const ShapePart&) -> ShapePart& = delete;
  auto operator=(ShapePart&&) -> ShapePart&      = delete;

  auto SetWeightedMainColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept -> void;
  auto SetWeightedLowColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept -> void;
  auto SetWeightedInnerColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept -> void;

  auto SetShapePathsTargetPoint(const Point2dInt& targetPoint) -> void;

  auto SetShapePathsMinMaxNumSteps(const MinMaxValues<uint32_t>& minMaxShapePathsNumSteps) noexcept
      -> void;

  auto Start() noexcept -> void;

  struct DrawParams
  {
    float brightnessAttenuation{};
    bool firstShapePathAtMeetingPoint{};
    bool varyDotRadius{};
    MultiplePixels meetingPointColors;
  };
  auto Draw(const DrawParams& drawParams) noexcept -> void;

  auto Update() noexcept -> void;
  auto ResetTs(float val) noexcept -> void;

  auto DoRandomChanges() noexcept -> void;
  auto UseRandomShapePathsNumSteps() noexcept -> void;
  auto UseFixedShapePathsNumSteps(float tMinMaxLerp) noexcept -> void;
  auto UseEvenShapePartNumsForDirection(bool val) -> void;
  [[nodiscard]] static auto GetNewRandomMinMaxLerpT(const GoomRand& goomRand,
                                                    float oldTMinMaxLerp) noexcept -> float;

  [[nodiscard]] auto GetNumShapePaths() const noexcept -> uint32_t;
  [[nodiscard]] auto GetShapePath(size_t shapePathNum) const noexcept -> const ShapePath&;
  [[nodiscard]] auto GetCurrentShapeDotRadius(bool varyRadius) const noexcept -> int32_t;
  [[nodiscard]] auto GetFirstShapePathPositionT() const noexcept -> float;
  [[nodiscard]] auto GetFirstShapePathTDistanceFromClosestBoundary() const noexcept -> float;
  [[nodiscard]] auto AreShapePathsCloseToMeeting() const noexcept -> bool;

private:
  FxHelper* m_fxHelper;
  PixelChannelType m_defaultAlpha;

  float m_currentTMinMaxLerp;
  StepSpeed m_shapePathsStepSpeed;
  auto SetShapePathsNumSteps() noexcept -> void;

  int32_t m_minShapeDotRadius;
  int32_t m_maxShapeDotRadius;
  static constexpr int32_t EXTREME_MAX_DOT_RADIUS_MULTIPLIER = 5;
  int32_t m_extremeMaxShapeDotRadius = EXTREME_MAX_DOT_RADIUS_MULTIPLIER * m_maxShapeDotRadius;
  bool m_useExtremeMaxShapeDotRadius = false;
  static constexpr auto MIN_MAX_DOT_RADIUS_STEPS = MinMaxValues{.minValue = 100U, .maxValue = 200U};
  static constexpr auto INITIAL_DOT_RADIUS_SPEED = 0.5F;
  StepSpeed m_dotRadiusStepSpeed{MIN_MAX_DOT_RADIUS_STEPS, INITIAL_DOT_RADIUS_SPEED};
  TValue m_dotRadiusT{
      {.stepType = TValue::StepType::CONTINUOUS_REVERSIBLE,
       .numSteps = m_dotRadiusStepSpeed.GetCurrentNumSteps()}
  };
  [[nodiscard]] auto GetMaxDotRadius(bool varyRadius) const noexcept -> int32_t;

  static constexpr auto INNER_COLOR_MIX_T_RANGE = NumberRange{0.1F, 0.9F};
  struct ColorInfo
  {
    WeightedRandomColorMaps mainColorMaps;
    WeightedRandomColorMaps lowColorMaps;
    WeightedRandomColorMaps innerColorMaps;
    float innerColorMix;
  };
  [[nodiscard]] auto GetInitialColorInfo() const noexcept -> ColorInfo;
  auto ChangeAllColorMapsNow() noexcept -> void;
  ColorInfo m_colorInfo = GetInitialColorInfo();
  auto UpdateShapesMainColorMaps() noexcept -> void;
  auto UpdateShapesLowColorMaps() noexcept -> void;
  auto UpdateShapesInnerColorMaps() noexcept -> void;
  auto ChangeAllShapesColorMapsNow() noexcept -> void;

  bool m_megaColorChangeMode = false;
  auto DoMegaColorChange() noexcept -> void;
  static constexpr uint32_t MEGA_COLOR_CHANGE_ON_TIME         = 100;
  static constexpr uint32_t MEGA_COLOR_CHANGE_ON_FAILED_TIME  = 10;
  static constexpr uint32_t MEGA_COLOR_CHANGE_OFF_TIME        = 1000;
  static constexpr uint32_t MEGA_COLOR_CHANGE_OFF_FAILED_TIME = 20;
  UTILS::OnOffTimer m_megaColorChangeOnOffTimer{
      m_fxHelper->GetGoomTime(),
      {.numOnCount               = MEGA_COLOR_CHANGE_ON_TIME,
                       .numOnCountAfterFailedOff = MEGA_COLOR_CHANGE_ON_FAILED_TIME,
                       .numOffCount              = MEGA_COLOR_CHANGE_OFF_TIME,
                       .numOffCountAfterFailedOn = MEGA_COLOR_CHANGE_OFF_FAILED_TIME}
  };
  auto StartMegaColorChangeOnOffTimer() noexcept -> void;
  [[nodiscard]] auto SetMegaColorChangeOn() noexcept -> bool;
  [[nodiscard]] auto SetMegaColorChangeOff() noexcept -> bool;

  uint32_t m_shapePartNum;
  static constexpr auto MIN_NUM_SHAPE_PATHS = 4U;
  uint32_t m_maxNumShapePaths;
  uint32_t m_totalNumShapeParts;
  std::vector<ShapePath> m_shapePaths;
  bool m_useEvenShapePartNumsForDirection = true;

  Point2dInt m_shapePathsTargetPoint;
  Point2dInt m_newShapePathsTargetPoint;
  bool m_needToUpdateTargetPoint = false;
  auto UpdateShapePathTargets() noexcept -> void;
  auto UpdateShapePathTransform(ShapePath& shapePath) const noexcept -> void;

  auto IncrementTs() noexcept -> void;
  auto SetRandomizedShapePaths() noexcept -> void;
  [[nodiscard]] auto GetRandomizedShapePaths() noexcept -> std::vector<ShapePath>;
  [[nodiscard]] auto GetShapePaths(uint32_t numShapePaths,
                                   const MinMaxValues<float>& minMaxValues) noexcept
      -> std::vector<ShapePath>;
  [[nodiscard]] static auto GetTransform2d(const Vec2dFlt& targetPoint,
                                           float radius,
                                           float scale,
                                           float rotate) noexcept -> UTILS::MATH::Transform2d;
  struct ShapeFunctionParams
  {
    float radius{};
    AngleParams angleParams;
    CircleFunction::Direction direction{};
  };
  float m_minRadiusFraction;
  float m_maxRadiusFraction;
  static constexpr auto DEFAULT_NUM_STEPS = 100U;
  TValue m_radiusFractionT{
      {.stepType = TValue::StepType::CONTINUOUS_REVERSIBLE, .numSteps = DEFAULT_NUM_STEPS}
  };
  [[nodiscard]] auto GetCircleRadius() const noexcept -> float;
  [[nodiscard]] auto GetCircleDirection() const noexcept -> CircleFunction::Direction;
  [[nodiscard]] auto GetShapePathColorInfo() const noexcept -> ShapePath::ColorInfo;
  [[nodiscard]] static auto GetCirclePath(float radius,
                                          CircleFunction::Direction direction,
                                          uint32_t numSteps) noexcept -> CirclePath;
  [[nodiscard]] static auto GetCircleFunction(const ShapeFunctionParams& params) -> CircleFunction;
};

static_assert(std::is_nothrow_move_constructible_v<ShapePart>);

} // namespace GOOM::VISUAL_FX::SHAPES

namespace GOOM::VISUAL_FX::SHAPES
{

inline auto ShapePart::GetNumShapePaths() const noexcept -> uint32_t
{
  return static_cast<uint32_t>(m_shapePaths.size());
}

inline auto ShapePart::GetShapePath(const size_t shapePathNum) const noexcept -> const ShapePath&
{
  return m_shapePaths.at(shapePathNum);
}

inline auto ShapePart::ResetTs(const float val) noexcept -> void
{
  std::ranges::for_each(m_shapePaths, [&val](ShapePath& path) { path.ResetT(val); });
}

inline auto ShapePart::GetNewRandomMinMaxLerpT(const UTILS::MATH::GoomRand& goomRand,
                                               const float oldTMinMaxLerp) noexcept -> float
{
  static constexpr auto SMALL_OFFSET = 0.2F;
  return goomRand.GetRandInRange(NumberRange{std::max(0.0F, -SMALL_OFFSET + oldTMinMaxLerp),
                                             std::min(1.0F, oldTMinMaxLerp + SMALL_OFFSET)});
}

inline auto ShapePart::UseEvenShapePartNumsForDirection(const bool val) -> void
{
  m_useEvenShapePartNumsForDirection = val;
}

inline auto ShapePart::SetRandomizedShapePaths() noexcept -> void
{
  m_shapePaths = GetRandomizedShapePaths();
}

ShapePart::ShapePart(FxHelper& fxHelper,
                     const Params& params,
                     const PixelChannelType defaultAlpha) noexcept
  : m_fxHelper{&fxHelper},
    m_defaultAlpha{defaultAlpha},
    m_currentTMinMaxLerp{params.tMinMaxLerp},
    m_shapePathsStepSpeed{
        {.minValue=params.shapePathsMinNumSteps, .maxValue=params.shapePathsMaxNumSteps}, params.tMinMaxLerp},
    m_minShapeDotRadius{params.minShapeDotRadius},
    m_maxShapeDotRadius{params.maxShapeDotRadius},
    m_shapePartNum{params.shapePartNum},
    m_maxNumShapePaths{params.maxNumShapePaths},
    m_totalNumShapeParts{params.totalNumShapeParts},
    m_shapePathsTargetPoint{params.shapePathsTargetPoint},
    m_minRadiusFraction{params.minRadiusFraction},
    m_maxRadiusFraction{params.maxRadiusFraction}
{
  Expects(0 < params.totalNumShapeParts);
  Expects(params.shapePartNum < params.totalNumShapeParts);
  Expects(0.0F <= params.minRadiusFraction);
  Expects(params.minRadiusFraction < params.maxRadiusFraction);
  Expects(0.0F <= params.tMinMaxLerp);
  Expects(params.tMinMaxLerp <= 1.0F);
  Expects(1 <= params.minShapeDotRadius);
  Expects(params.minShapeDotRadius <= params.maxShapeDotRadius);
  Expects(MIN_NUM_SHAPE_PATHS <= params.maxNumShapePaths);
  Expects(0 < params.shapePathsMinNumSteps);
  Expects(params.shapePathsMinNumSteps < params.shapePathsMaxNumSteps);
}

auto ShapePart::GetInitialColorInfo() const noexcept -> ColorInfo
{
  return {.mainColorMaps  = GetUnweightedRandomColorMaps(m_fxHelper->GetGoomRand(), m_defaultAlpha),
          .lowColorMaps   = GetUnweightedRandomColorMaps(m_fxHelper->GetGoomRand(), m_defaultAlpha),
          .innerColorMaps = GetUnweightedRandomColorMaps(m_fxHelper->GetGoomRand(), m_defaultAlpha),
          .innerColorMix  = m_fxHelper->GetGoomRand().GetRandInRange<INNER_COLOR_MIX_T_RANGE>()};
}

auto ShapePart::SetShapePathsTargetPoint(const Point2dInt& targetPoint) -> void
{
  if (m_shapePathsTargetPoint == targetPoint)
  {
    m_needToUpdateTargetPoint = false;
    return;
  }

  m_needToUpdateTargetPoint  = true;
  m_newShapePathsTargetPoint = targetPoint;
}

auto ShapePart::SetShapePathsMinMaxNumSteps(
    const MinMaxValues<uint32_t>& minMaxShapePathsNumSteps) noexcept -> void
{
  m_shapePathsStepSpeed.SetMinMaxNumSteps(minMaxShapePathsNumSteps);
}

auto ShapePart::UpdateShapePathTargets() noexcept -> void
{
  m_radiusFractionT.Increment();

  static constexpr auto MIN_MIN_RADIUS_FRACTION = 0.05F;
  static constexpr auto MAX_MIN_RADIUS_FRACTION = 0.2F;
  static constexpr auto MIN_MAX_RADIUS_FRACTION = 0.3F;
  static constexpr auto MAX_MAX_RADIUS_FRACTION = 0.5F;
  m_minRadiusFraction =
      std::lerp(MAX_MIN_RADIUS_FRACTION, MIN_MIN_RADIUS_FRACTION, m_radiusFractionT());
  m_maxRadiusFraction =
      std::lerp(MAX_MAX_RADIUS_FRACTION, MIN_MAX_RADIUS_FRACTION, m_radiusFractionT());

  if (not m_needToUpdateTargetPoint)
  {
    return;
  }
  if (not m_shapePaths.at(0).HasJustHitEndBoundary())
  {
    return;
  }

  m_shapePathsTargetPoint = m_newShapePathsTargetPoint;

  std::ranges::for_each(m_shapePaths,
                        [this](ShapePath& shapePath) { UpdateShapePathTransform(shapePath); });

  ResetTs(0.0F);

  m_needToUpdateTargetPoint = false;
}

inline auto ShapePart::UpdateShapePathTransform(ShapePath& shapePath) const noexcept -> void
{
  auto& basePath = dynamic_cast<TransformedPath&>(shapePath.GetIPath());

  auto newTransform = basePath.GetTransform();
  newTransform.SetTranslation(ToVec2dFlt(m_shapePathsTargetPoint));

  basePath.SetTransform(newTransform);
}

inline auto ShapePart::GetTransform2d(const Vec2dFlt& targetPoint,
                                      const float radius,
                                      const float scale,
                                      const float rotate) noexcept -> Transform2d
{
  const auto centre = Vec2dFlt{
      .x = targetPoint.x - (scale * radius * std::cos(rotate)),
      .y = targetPoint.y - (scale * radius * std::sin(rotate)),
  };
  return Transform2d{rotate, centre, scale};
}

auto ShapePart::GetRandomizedShapePaths() noexcept -> std::vector<ShapePath>
{
  const auto numShapePaths = m_fxHelper->GetGoomRand().GetRandInRange(
      NumberRange{MIN_NUM_SHAPE_PATHS, m_maxNumShapePaths});

  static constexpr auto MIN_SCALE_RANGE       = NumberRange{0.9F, 1.0F};
  static constexpr auto MAX_SCALE_RANGE       = NumberRange{1.0F + SMALL_FLOAT, 1.5F};
  static constexpr auto PROB_SCALE_EQUALS_ONE = 0.9F;

  const auto probScaleEqualsOne = m_fxHelper->GetGoomRand().ProbabilityOf<PROB_SCALE_EQUALS_ONE>();
  const auto minScale =
      probScaleEqualsOne ? 1.0F : m_fxHelper->GetGoomRand().GetRandInRange<MIN_SCALE_RANGE>();
  const auto maxScale =
      probScaleEqualsOne ? 1.0F : m_fxHelper->GetGoomRand().GetRandInRange<MAX_SCALE_RANGE>();

  return GetShapePaths(numShapePaths, {.minValue = minScale, .maxValue = maxScale});
}

auto ShapePart::GetShapePaths(const uint32_t numShapePaths,
                              const MinMaxValues<float>& minMaxValues) noexcept
    -> std::vector<ShapePath>
{
  const auto targetPointFlt = ToVec2dFlt(m_shapePathsTargetPoint);

  static constexpr auto MIN_ANGLE = 0.0F;
  static constexpr auto MAX_ANGLE = TWO_PI;
  auto stepFraction               = TValue{
                    {.stepType = TValue::StepType::SINGLE_CYCLE, .numSteps = numShapePaths}
  };

  const auto radius    = GetCircleRadius();
  const auto direction = GetCircleDirection();
  const auto numSteps  = m_shapePathsStepSpeed.GetCurrentNumSteps();

  auto shapePaths = std::vector<ShapePath>{};

  for (auto i = 0U; i < numShapePaths; ++i)
  {
    const auto rotate     = std::lerp(MIN_ANGLE, MAX_ANGLE, stepFraction());
    const auto scale      = std::lerp(minMaxValues.minValue, minMaxValues.maxValue, stepFraction());
    const auto circlePath = GetCirclePath(radius, direction, numSteps);

    const auto newTransform = GetTransform2d(targetPointFlt, radius, scale, rotate);
    const auto basePath = std::make_shared<TransformedPath>(circlePath.GetClone(), newTransform);

    const auto colorInfo = GetShapePathColorInfo();

    shapePaths.emplace_back(*m_fxHelper, basePath, colorInfo);

    static constexpr auto CLOSE_ENOUGH = 4;
    if (SqDistance(shapePaths.at(i).GetIPath().GetStartPos(), m_shapePathsTargetPoint) >
        CLOSE_ENOUGH)
    {
      //      LogError(m_fxHelper->GetGoomLogger(),
      //               "shapePaths.at({}).GetIPath().GetStartPos() = {}, {}",
      //               i,
      //               shapePaths.at(i).GetIPath().GetStartPos().x,
      //               shapePaths.at(i).GetIPath().GetStartPos().y);
      //      LogError(m_fxHelper->GetGoomLogger(),
      //               "m_shapesTargetPoint = {}, {}",
      //               m_shapePathsTargetPoint.x,
      //               m_shapePathsTargetPoint.y);
      //      LogError(
      //          m_fxHelper->GetGoomLogger(),
      //          "targetPointFlt = {}, {}", targetPointFlt.x, targetPointFlt.y);
      //      LogError(m_fxHelper->GetGoomLogger(), "radius = {}", radius);
      //      LogError(m_fxHelper->GetGoomLogger(), "rotate = {}", rotate);
      //      LogError(m_fxHelper->GetGoomLogger(), "std::cos(rotate) = {}", std::cos(rotate));
      //      LogError(m_fxHelper->GetGoomLogger(), "std::sin(rotate) = {}", std::sin(rotate));
      //      LogError(m_fxHelper->GetGoomLogger(), "scale = {}", scale);
      //      LogError(m_fxHelper->GetGoomLogger(), "numSteps = {}", numSteps);
    }
    Ensures(SqDistance(shapePaths.at(i).GetIPath().GetStartPos(), m_shapePathsTargetPoint) <=
            CLOSE_ENOUGH);

    stepFraction.Increment();
  }

  return shapePaths;
}

inline auto ShapePart::GetShapePathColorInfo() const noexcept -> ShapePath::ColorInfo
{
  const auto& colorMapTypes = WeightedRandomColorMaps::GetAllColorMapsTypes();

  return ShapePath::ColorInfo{
      .mainColorMapPtr  = m_colorInfo.mainColorMaps.GetRandomColorMapSharedPtr(colorMapTypes),
      .lowColorMapPtr   = m_colorInfo.lowColorMaps.GetRandomColorMapSharedPtr(colorMapTypes),
      .innerColorMapPtr = m_colorInfo.innerColorMaps.GetRandomColorMapSharedPtr(colorMapTypes),
  };
}

inline auto ShapePart::GetCircleRadius() const noexcept -> float
{
  const auto minDimension = std::min(m_fxHelper->GetDimensions().GetFltWidth(),
                                     m_fxHelper->GetDimensions().GetFltHeight());
  const auto minRadius    = m_minRadiusFraction * minDimension;
  const auto maxRadius    = m_maxRadiusFraction * minDimension;
  const auto t = static_cast<float>(m_shapePartNum) / static_cast<float>(m_totalNumShapeParts - 1);

  return std::lerp(minRadius, maxRadius, t);
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
                                     const uint32_t numSteps) noexcept -> CirclePath
{
  auto positionT = std::make_unique<TValue>(TValue::NumStepsProperties{
      .stepType = TValue::StepType::CONTINUOUS_REVERSIBLE, .numSteps = numSteps});

  const auto params =
      ShapeFunctionParams{.radius = radius, .angleParams = AngleParams{}, .direction = direction};

  return CirclePath{std::move(positionT), GetCircleFunction(params)};
}

inline auto ShapePart::GetCircleFunction(const ShapeFunctionParams& params) -> CircleFunction
{
  static constexpr auto CENTRE_POS = Vec2dFlt{.x = 0.0F, .y = 0.0F};

  return {CENTRE_POS, params.radius, params.angleParams, params.direction};
}

auto ShapePart::SetWeightedMainColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept
    -> void
{
  m_colorInfo.mainColorMaps = weightedMaps;
  UpdateShapesMainColorMaps();
}

auto ShapePart::SetWeightedLowColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept
    -> void
{
  m_colorInfo.lowColorMaps = weightedMaps;
  UpdateShapesLowColorMaps();
}

auto ShapePart::SetWeightedInnerColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept
    -> void
{
  m_colorInfo.innerColorMix  = m_fxHelper->GetGoomRand().GetRandInRange<INNER_COLOR_MIX_T_RANGE>();
  m_colorInfo.innerColorMaps = weightedMaps;

  UpdateShapesInnerColorMaps();
}

auto ShapePart::UpdateShapesMainColorMaps() noexcept -> void
{
  std::ranges::for_each(m_shapePaths,
                        [this](ShapePath& shapePath)
                        { shapePath.UpdateMainColorInfo(m_colorInfo.mainColorMaps); });
}

auto ShapePart::UpdateShapesLowColorMaps() noexcept -> void
{
  std::ranges::for_each(m_shapePaths,
                        [this](ShapePath& shapePath)
                        { shapePath.UpdateLowColorInfo(m_colorInfo.lowColorMaps); });
}

auto ShapePart::UpdateShapesInnerColorMaps() noexcept -> void
{
  std::ranges::for_each(m_shapePaths,
                        [this](ShapePath& shapePath)
                        { shapePath.UpdateInnerColorInfo(m_colorInfo.innerColorMaps); });
}

auto ShapePart::ChangeAllShapesColorMapsNow() noexcept -> void
{
  UpdateShapesMainColorMaps();
  UpdateShapesLowColorMaps();
  UpdateShapesInnerColorMaps();
}

auto ShapePart::Start() noexcept -> void
{
  SetRandomizedShapePaths();
  StartMegaColorChangeOnOffTimer();
}

auto ShapePart::GetCurrentShapeDotRadius(const bool varyRadius) const noexcept -> int32_t
{
  if (not varyRadius)
  {
    return m_minShapeDotRadius;
  }

  const auto maxShapeDotRadius =
      m_useExtremeMaxShapeDotRadius ? m_extremeMaxShapeDotRadius : m_maxShapeDotRadius;

  return static_cast<int32_t>(std::lerp(m_minShapeDotRadius, maxShapeDotRadius, m_dotRadiusT()));
}

auto ShapePart::GetFirstShapePathPositionT() const noexcept -> float
{
  if (0 == GetNumShapePaths())
  {
    return 1.0F;
  }

  return GetShapePath(0).GetCurrentT();
}

auto ShapePart::GetFirstShapePathTDistanceFromClosestBoundary() const noexcept -> float
{
  const auto positionT = GetFirstShapePathPositionT();

  if (positionT < UTILS::MATH::HALF)
  {
    return positionT;
  }

  return 1.0F - positionT;
}

auto ShapePart::AreShapePathsCloseToMeeting() const noexcept -> bool
{
  static constexpr auto T_MEETING_CUTOFF = 0.05F;
  const auto positionT                   = GetFirstShapePathPositionT();

  return (T_MEETING_CUTOFF > positionT) || (positionT > (1.0F - T_MEETING_CUTOFF));
}

auto ShapePart::UseFixedShapePathsNumSteps(const float tMinMaxLerp) noexcept -> void
{
  m_currentTMinMaxLerp = tMinMaxLerp;
  m_shapePathsStepSpeed.SetSpeed(m_currentTMinMaxLerp);
  m_dotRadiusStepSpeed.SetSpeed(m_currentTMinMaxLerp);
}

auto ShapePart::UseRandomShapePathsNumSteps() noexcept -> void
{
  m_currentTMinMaxLerp = GetNewRandomMinMaxLerpT(m_fxHelper->GetGoomRand(), m_currentTMinMaxLerp);
  m_shapePathsStepSpeed.SetSpeed(m_currentTMinMaxLerp);
  m_dotRadiusStepSpeed.SetSpeed(m_currentTMinMaxLerp);
}

auto ShapePart::SetShapePathsNumSteps() noexcept -> void
{
  std::ranges::for_each(m_shapePaths,
                        [this](ShapePath& path)
                        { path.SetNumSteps(m_shapePathsStepSpeed.GetCurrentNumSteps()); });

  m_dotRadiusT.SetNumSteps(m_dotRadiusStepSpeed.GetCurrentNumSteps());
}

void ShapePart::DoRandomChanges() noexcept
{
  DoMegaColorChange();

  if (not m_shapePaths.at(0).HasJustHitAnyBoundary())
  {
    return;
  }

  SetRandomizedShapePaths();
  SetShapePathsNumSteps();
  ChangeAllColorMapsNow();
}

inline auto ShapePart::DoMegaColorChange() noexcept -> void
{
  if (not m_megaColorChangeMode)
  {
    return;
  }

  ChangeAllShapesColorMapsNow();
}

inline auto ShapePart::StartMegaColorChangeOnOffTimer() noexcept -> void
{
  m_megaColorChangeOnOffTimer.Reset();
  m_megaColorChangeOnOffTimer.SetActions(
      {.onAction  = [this]() { return SetMegaColorChangeOn(); },
       .offAction = [this]() { return SetMegaColorChangeOff(); }});
  m_megaColorChangeOnOffTimer.StartOffTimer();
}

inline auto ShapePart::SetMegaColorChangeOn() noexcept -> bool
{
  if (static constexpr auto PROB_MEGA_COLOR_CHANGE_ON = 0.1F;
      not m_fxHelper->GetGoomRand().ProbabilityOf<PROB_MEGA_COLOR_CHANGE_ON>())
  {
    return false;
  }
  m_megaColorChangeMode = true;
  return true;
}

inline auto ShapePart::SetMegaColorChangeOff() noexcept -> bool
{
  if (static constexpr auto PROB_MEGA_COLOR_CHANGE_OFF = 0.9F;
      not m_fxHelper->GetGoomRand().ProbabilityOf<PROB_MEGA_COLOR_CHANGE_OFF>())
  {
    return false;
  }
  m_megaColorChangeMode = false;
  return true;
}

inline auto ShapePart::ChangeAllColorMapsNow() noexcept -> void
{
  ChangeAllShapesColorMapsNow();

  static constexpr auto PROB_USE_EXTREME_MAX_DOT_RADIUS = 0.5F;
  m_useExtremeMaxShapeDotRadius =
      m_fxHelper->GetGoomRand().ProbabilityOf<PROB_USE_EXTREME_MAX_DOT_RADIUS>();
}

auto ShapePart::Update() noexcept -> void
{
  IncrementTs();
  UpdateShapePathTargets();
}

inline auto ShapePart::IncrementTs() noexcept -> void
{
  std::ranges::for_each(m_shapePaths, [](ShapePath& path) { path.IncrementT(); });

  m_dotRadiusT.Increment();
  m_megaColorChangeOnOffTimer.Update();
}

auto ShapePart::Draw(const DrawParams& drawParams) noexcept -> void
{
  const auto shapePathParams = ShapePath::DrawParams{
      .brightnessAttenuation        = drawParams.brightnessAttenuation,
      .firstShapePathAtMeetingPoint = drawParams.firstShapePathAtMeetingPoint,
      .maxRadius                    = GetMaxDotRadius(drawParams.varyDotRadius),
      .innerColorMix                = m_colorInfo.innerColorMix,
      .meetingPointColors           = drawParams.meetingPointColors,
  };

  for (auto& shapePath : m_shapePaths)
  {
    shapePath.Draw(shapePathParams);
  }
}

inline auto ShapePart::GetMaxDotRadius(const bool varyRadius) const noexcept -> int32_t
{
  auto maxRadius = GetCurrentShapeDotRadius(varyRadius);

  if (AreShapePathsCloseToMeeting())
  {
    const auto tDistanceFromOne        = GetFirstShapePathTDistanceFromClosestBoundary();
    static constexpr auto EXTRA_RADIUS = 10.0F;
    static constexpr auto EXPONENT     = 2.0F;
    maxRadius += static_cast<int32_t>(std::pow(tDistanceFromOne, EXPONENT) * EXTRA_RADIUS);
  }

  return maxRadius;
}

} // namespace GOOM::VISUAL_FX::SHAPES
