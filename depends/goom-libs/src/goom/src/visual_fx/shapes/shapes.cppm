module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

module Goom.VisualFx.ShapesFx:Shapes;

import Goom.Color.ColorMaps;
import Goom.Color.RandomColorMaps;
import Goom.Draw.GoomDrawBase;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Utils.Math.TValues;
import Goom.VisualFx.FxHelper;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;
import :ShapeParts;

namespace GOOM::VISUAL_FX::SHAPES
{

class Shape
{
public:
  struct Params
  {
    float minRadiusFraction{};
    float maxRadiusFraction{};
    int32_t minShapeDotRadius{};
    int32_t maxShapeDotRadius{};
    uint32_t maxNumShapePaths{};
    Point2dInt zoomMidpoint;
    uint32_t minNumShapePathSteps{};
    uint32_t maxNumShapePathSteps{};
  };

  Shape(FxHelper& fxHelper, const Params& params, PixelChannelType defaultAlpha) noexcept;

  auto SetWeightedMainColorMaps(const COLOR::WeightedRandomColorMaps& weightedMaps) noexcept
      -> void;
  auto SetWeightedLowColorMaps(const COLOR::WeightedRandomColorMaps& weightedMaps) noexcept -> void;
  auto SetWeightedInnerColorMaps(const COLOR::WeightedRandomColorMaps& weightedMaps) noexcept
      -> void;

  auto SetVaryDotRadius(bool val) -> void;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;
  auto SetShapePathsMinMaxNumSteps(const MinMaxValues<uint32_t>& minMaxShapePathsNumSteps) noexcept
      -> void;

  auto Start() noexcept -> void;

  auto Draw() noexcept -> void;
  auto Update() noexcept -> void;

  [[nodiscard]] auto GetTotalNumShapePaths() const noexcept -> uint32_t;
  [[nodiscard]] auto GetNumShapeParts() const noexcept -> size_t;
  [[nodiscard]] auto GetShapePart(size_t shapePartNum) const noexcept -> const ShapePart&;

  [[nodiscard]] auto HasFirstShapePathJustHitStartBoundary() const noexcept -> bool;
  [[nodiscard]] auto HasFirstShapePathJustHitEndBoundary() const noexcept -> bool;
  [[nodiscard]] auto FirstShapePathAtMeetingPoint() const noexcept -> bool;
  [[nodiscard]] auto FirstShapePathsCloseToMeeting() const noexcept -> bool;

  auto DoRandomChanges() noexcept -> void;
  auto SetShapeNumSteps() noexcept -> void;
  auto SetFixedShapeNumSteps() noexcept -> void;

private:
  const UTILS::MATH::GoomRand* m_goomRand;

  static constexpr uint32_t NUM_SHAPE_PARTS = 10;
  std::vector<ShapePart> m_shapeParts;
  [[nodiscard]] static auto GetInitialShapeParts(FxHelper& fxHelper,
                                                 const Params& params,
                                                 PixelChannelType defaultAlpha) noexcept
      -> std::vector<ShapePart>;
  [[nodiscard]] auto GetFirstShapePathPositionT() const noexcept -> float;

  bool m_varyDotRadius                                        = false;
  COLOR::ConstColorMapSharedPtr m_meetingPointMainColorMapPtr = nullptr;
  COLOR::ConstColorMapSharedPtr m_meetingPointLowColorMapPtr  = nullptr;
  static constexpr uint32_t NUM_MEETING_POINT_COLOR_STEPS     = 50;
  TValue m_meetingPointColorsT{
      {.stepType = TValue::StepType::CONTINUOUS_REVERSIBLE,
       .numSteps = NUM_MEETING_POINT_COLOR_STEPS}
  };
  [[nodiscard]] auto GetCurrentMeetingPointColors() const noexcept -> DRAW::MultiplePixels;
  [[nodiscard]] auto GetBrightnessAttenuation() const noexcept -> float;

  static constexpr float STARTING_FIXED_T_MIN_MAX_LERP = 0.5F;
  float m_fixedTMinMaxLerp                             = STARTING_FIXED_T_MIN_MAX_LERP;
  auto SetRandomShapeNumSteps() noexcept -> void;
};

} // namespace GOOM::VISUAL_FX::SHAPES

namespace GOOM::VISUAL_FX::SHAPES
{

inline auto Shape::SetVaryDotRadius(const bool val) -> void
{
  m_varyDotRadius = val;
}

inline auto Shape::GetNumShapeParts() const noexcept -> size_t
{
  return m_shapeParts.size();
}

inline auto Shape::GetShapePart(const size_t shapePartNum) const noexcept -> const ShapePart&
{
  return m_shapeParts.at(shapePartNum);
}

inline auto Shape::HasFirstShapePathJustHitStartBoundary() const noexcept -> bool
{
  if (0 == m_shapeParts.front().GetNumShapePaths())
  {
    return false;
  }

  return m_shapeParts.front().GetShapePath(0).HasJustHitStartBoundary();
}

inline auto Shape::HasFirstShapePathJustHitEndBoundary() const noexcept -> bool
{
  if (0 == m_shapeParts.front().GetNumShapePaths())
  {
    return false;
  }

  return m_shapeParts.front().GetShapePath(0).HasJustHitEndBoundary();
}

inline auto Shape::FirstShapePathAtMeetingPoint() const noexcept -> bool
{
  return HasFirstShapePathJustHitStartBoundary() or HasFirstShapePathJustHitEndBoundary();
}

inline auto Shape::FirstShapePathsCloseToMeeting() const noexcept -> bool
{
  return m_shapeParts.front().AreShapePathsCloseToMeeting();
}

inline auto Shape::GetFirstShapePathPositionT() const noexcept -> float
{
  if (0 == m_shapeParts.front().GetNumShapePaths())
  {
    return 1.0F;
  }

  return m_shapeParts.front().GetFirstShapePathPositionT();
}

inline auto Shape::SetShapeNumSteps() noexcept -> void
{
  if (static constexpr auto PROB_FIXED_NUM_STEPS = 0.95F;
      m_goomRand->ProbabilityOf<PROB_FIXED_NUM_STEPS>())
  {
    SetFixedShapeNumSteps();
  }
  else
  {
    SetRandomShapeNumSteps();
  }
}

using COLOR::WeightedRandomColorMaps;
using DRAW::MultiplePixels;

Shape::Shape(FxHelper& fxHelper, const Params& params, const PixelChannelType defaultAlpha) noexcept
  : m_goomRand{&fxHelper.GetGoomRand()},
    m_shapeParts{GetInitialShapeParts(fxHelper, params, defaultAlpha)}
{
}

auto Shape::GetInitialShapeParts(FxHelper& fxHelper,
                                 const Params& params,
                                 const PixelChannelType defaultAlpha) noexcept
    -> std::vector<ShapePart>
{
  auto shapeParts = std::vector<ShapePart>{};

  for (auto i = 0U; i < NUM_SHAPE_PARTS; ++i)
  {
    static constexpr auto T_MIN_MAX_LERP = 0.5F;
    const auto shapePartParams           = ShapePart::Params{
                  .shapePartNum          = i,
                  .totalNumShapeParts    = NUM_SHAPE_PARTS,
                  .minRadiusFraction     = params.minRadiusFraction,
                  .maxRadiusFraction     = params.maxRadiusFraction,
                  .minShapeDotRadius     = params.minShapeDotRadius,
                  .maxShapeDotRadius     = params.maxShapeDotRadius,
                  .maxNumShapePaths      = params.maxNumShapePaths,
                  .tMinMaxLerp           = T_MIN_MAX_LERP,
                  .shapePathsTargetPoint = params.zoomMidpoint,
                  .shapePathsMinNumSteps = params.minNumShapePathSteps,
                  .shapePathsMaxNumSteps = params.maxNumShapePathSteps,
    };
    shapeParts.emplace_back(fxHelper, shapePartParams, defaultAlpha);
  }

  return shapeParts;
}

auto Shape::SetWeightedMainColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept -> void
{
  m_meetingPointMainColorMapPtr =
      weightedMaps.GetRandomColorMapSharedPtr(WeightedRandomColorMaps::GetAllColorMapsTypes());

  std::ranges::for_each(m_shapeParts,
                        [&weightedMaps](ShapePart& shapePart)
                        { shapePart.SetWeightedMainColorMaps(weightedMaps); });
}

auto Shape::SetWeightedLowColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept -> void
{
  m_meetingPointLowColorMapPtr =
      weightedMaps.GetRandomColorMapSharedPtr(WeightedRandomColorMaps::GetAllColorMapsTypes());

  std::ranges::for_each(m_shapeParts,
                        [&weightedMaps](ShapePart& shapePart)
                        { shapePart.SetWeightedLowColorMaps(weightedMaps); });
}

auto Shape::SetWeightedInnerColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept -> void
{
  std::ranges::for_each(m_shapeParts,
                        [&weightedMaps](ShapePart& shapePart)
                        { shapePart.SetWeightedInnerColorMaps(weightedMaps); });
}

auto Shape::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  if (static constexpr auto PROB_ACCEPT_NEW_MIDPOINT = 0.8F;
      not m_goomRand->ProbabilityOf<PROB_ACCEPT_NEW_MIDPOINT>())
  {
    return;
  }

  std::ranges::for_each(m_shapeParts,
                        [&zoomMidpoint](ShapePart& shapePart)
                        { shapePart.SetShapePathsTargetPoint(zoomMidpoint); });
}

auto Shape::SetShapePathsMinMaxNumSteps(
    const MinMaxValues<uint32_t>& minMaxShapePathsNumSteps) noexcept -> void
{
  std::ranges::for_each(m_shapeParts,
                        [&minMaxShapePathsNumSteps](ShapePart& shapePart)
                        { shapePart.SetShapePathsMinMaxNumSteps(minMaxShapePathsNumSteps); });
}

auto Shape::Start() noexcept -> void
{
  SetFixedShapeNumSteps();

  std::ranges::for_each(m_shapeParts, [](ShapePart& shapePart) { shapePart.Start(); });
}

auto Shape::Draw() noexcept -> void
{
  const auto shapePartParams = ShapePart::DrawParams{
      .brightnessAttenuation        = GetBrightnessAttenuation(),
      .firstShapePathAtMeetingPoint = FirstShapePathAtMeetingPoint(),
      .varyDotRadius                = m_varyDotRadius,
      .meetingPointColors           = GetCurrentMeetingPointColors(),
  };
  std::ranges::for_each(
      m_shapeParts, [&shapePartParams](ShapePart& shapePart) { shapePart.Draw(shapePartParams); });

  if (FirstShapePathAtMeetingPoint())
  {
    m_meetingPointColorsT.Increment();
  }
}

inline auto Shape::GetCurrentMeetingPointColors() const noexcept -> MultiplePixels
{
  return {.color1 = m_meetingPointMainColorMapPtr->GetColor(m_meetingPointColorsT()),
          .color2 = m_meetingPointLowColorMapPtr->GetColor(m_meetingPointColorsT())};
}

inline auto Shape::GetBrightnessAttenuation() const noexcept -> float
{
  if (not FirstShapePathsCloseToMeeting())
  {
    return 1.0F;
  }

  const auto distanceFromOne =
      1.0F - GetShapePart(0).GetFirstShapePathTDistanceFromClosestBoundary();

  const auto minBrightness       = 2.0F / static_cast<float>(GetTotalNumShapePaths());
  static constexpr auto EXPONENT = 25.0F;
  return std::lerp(1.0F, minBrightness, std::pow(distanceFromOne, EXPONENT));
}

auto Shape::Update() noexcept -> void
{
  std::ranges::for_each(m_shapeParts, [](ShapePart& shapePart) { shapePart.Update(); });
}

auto Shape::DoRandomChanges() noexcept -> void
{
  static constexpr auto PROB_USE_EVEN_PART_NUMS_FOR_DIRECTION = 0.5F;
  const auto useEvenPartNumsForDirection =
      m_goomRand->ProbabilityOf<PROB_USE_EVEN_PART_NUMS_FOR_DIRECTION>();

  std::ranges::for_each(m_shapeParts,
                        [&useEvenPartNumsForDirection](ShapePart& shapePart)
                        {
                          shapePart.UseEvenShapePartNumsForDirection(useEvenPartNumsForDirection);
                          shapePart.DoRandomChanges();
                        });
}

auto Shape::SetFixedShapeNumSteps() noexcept -> void
{
  m_fixedTMinMaxLerp   = ShapePart::GetNewRandomMinMaxLerpT(*m_goomRand, m_fixedTMinMaxLerp);
  const auto positionT = GetFirstShapePathPositionT();

  std::ranges::for_each(m_shapeParts,
                        [this, &positionT](ShapePart& shapePart)
                        {
                          shapePart.UseFixedShapePathsNumSteps(m_fixedTMinMaxLerp);
                          shapePart.ResetTs(positionT);
                        });
}

auto Shape::SetRandomShapeNumSteps() noexcept -> void
{
  std::ranges::for_each(m_shapeParts,
                        [](ShapePart& shapePart) { shapePart.UseRandomShapePathsNumSteps(); });
}

auto Shape::GetTotalNumShapePaths() const noexcept -> uint32_t
{
  auto total = 0U;

  const auto numShapeParts = GetNumShapeParts();
  for (auto i = 0U; i < numShapeParts; ++i)
  {
    const auto numShapePaths = m_shapeParts.at(i).GetNumShapePaths();
    for (auto j = 0U; j < numShapePaths; ++j)
    {
      ++total;
    }
  }

  return total;
}

} // namespace GOOM::VISUAL_FX::SHAPES
