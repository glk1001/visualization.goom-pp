module;

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

module Goom.VisualFx.TentaclesFx:TentacleDriver;

import Goom.Color.ColorAdjustment;
import Goom.Color.ColorMaps;
import Goom.Draw.GoomDrawBase;
import Goom.Utils.Math.DampingFunctions;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.IncrementedValues;
import Goom.Utils.Math.Misc;
import Goom.Utils.Math.Paths;
import Goom.Utils.Math.TValues;
import Goom.Utils.GoomTime;
import Goom.VisualFx.VisualFxBase;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomGraphic;
import Goom.Lib.Point2d;
import :CirclesTentacleLayout;
import :Tentacle2d;
import :Tentacle3d;
import :TentaclePlotter;

using GOOM::COLOR::ColorMaps;
using GOOM::COLOR::ConstColorMapSharedPtr;
using GOOM::DRAW::GetLowColor;
using GOOM::DRAW::GetMainColor;
using GOOM::DRAW::IGoomDraw;
using GOOM::DRAW::MultiplePixels;
using GOOM::UTILS::GoomTime;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::IncrementedValue;
using GOOM::UTILS::MATH::NumberRange;
using GOOM::UTILS::MATH::SineWaveMultiplier;
using GOOM::UTILS::MATH::TValue;

namespace GOOM::VISUAL_FX::TENTACLES
{

class TentacleDriver
{
public:
  TentacleDriver(IGoomDraw& draw,
                 const GoomRand& goomRand,
                 const GoomTime& goomTime,
                 const CirclesTentacleLayout& tentacleLayout,
                 PixelChannelType defaultAlpha) noexcept;

  auto SetWeightedColorMaps(const IVisualFx::WeightedColorMaps& weightedColorMaps) noexcept -> void;
  auto ChangeTentacleColorMaps() -> void;

  auto StartIterating() -> void;

  auto SetTentaclesEndCentrePos(const Point2dInt& newEndCentrePos) noexcept -> void;

  auto MultiplyIterZeroYValWaveFreq(float value) -> void;
  auto SetDominantColorMaps(const ConstColorMapSharedPtr& dominantMainColorMap,
                            const ConstColorMapSharedPtr& dominantLowColorMap) -> void;

  auto Update() -> void;

private:
  using StepType = UTILS::MATH::TValue::StepType;

  const GoomRand* m_goomRand;
  const GoomTime* m_goomTime;
  Point2dInt m_screenCentre;

  ColorMaps m_colorMaps;
  ConstColorMapSharedPtr m_dominantMainColorMapPtr = nullptr;
  ConstColorMapSharedPtr m_dominantLowColorMapPtr  = nullptr;

  struct IterationParams
  {
    uint32_t numNodes;
    float length;
    float iterZeroYValWaveFreq;
    SineWaveMultiplier iterZeroYValWave;
  };
  IterationParams m_tentacleParams;
  TentaclePlotter m_tentaclePlotter;

  struct TentacleAndAttributes
  {
    Tentacle3D tentacle3D;
    ConstColorMapSharedPtr mainColorMapPtr = nullptr;
    ConstColorMapSharedPtr lowColorMapPtr  = nullptr;
    Pixel currentMainColor;
    Pixel currentLowColor;
  };
  static constexpr auto NUM_CURRENT_COLOR_STEPS = 500U;
  TValue m_currentColorT{
      {.stepType = StepType::CONTINUOUS_REVERSIBLE, .numSteps = NUM_CURRENT_COLOR_STEPS}
  };
  static constexpr auto NUM_NODE_T_OFFSET_STEPS = 10U;
  TValue m_nodeTOffset{
      {.stepType = StepType::CONTINUOUS_REVERSIBLE, .numSteps = NUM_NODE_T_OFFSET_STEPS}
  };
  std::vector<TentacleAndAttributes> m_tentacles;
  [[nodiscard]] static auto GetTentacles(const GoomRand& goomRand,
                                         const CirclesTentacleLayout& tentacleLayout,
                                         const IterationParams& tentacleParams) noexcept
      -> std::vector<TentacleAndAttributes>;
  [[nodiscard]] static auto CreateNewTentacle2D(const GoomRand& goomRand,
                                                const IterationParams& tentacleParams) noexcept
      -> std::unique_ptr<Tentacle2D>;
  uint32_t m_tentacleGroupSize = static_cast<uint32_t>(m_tentacles.size());
  [[nodiscard]] auto GetMixedColors(float dominantT,
                                    float nodeT,
                                    const TentacleAndAttributes& tentacleAndAttributes,
                                    float brightness) const -> MultiplePixels;
  static constexpr float GAMMA = 1.8F;
  COLOR::ColorAdjustment m_colorAdjust{
      {.gamma = GAMMA, .alterChromaFactor = COLOR::ColorAdjustment::INCREASED_CHROMA_FACTOR}
  };

  static constexpr auto COLOR_SEGMENT_MIX_T_RANGE   = NumberRange{0.7F, 1.0F};
  static constexpr auto DEFAULT_COLOR_SEGMENT_MIX_T = 0.9F;
  float m_mainColorSegmentMixT                      = DEFAULT_COLOR_SEGMENT_MIX_T;
  float m_lowColorSegmentMixT                       = DEFAULT_COLOR_SEGMENT_MIX_T;
  auto ChangeSegmentMixes() noexcept -> void;

  auto CheckForTimerEvents() -> void;

  Point2dInt m_previousEndCentrePos             = m_screenCentre;
  Point2dInt m_targetEndCentrePos               = m_screenCentre;
  static constexpr auto NUM_STEPS_TO_END_TARGET = 100U;
  TValue m_endCentrePosT{
      {.stepType = StepType::SINGLE_CYCLE, .numSteps = NUM_STEPS_TO_END_TARGET}
  };
  [[nodiscard]] auto GetAcceptableEndCentrePos(
      const Point2dInt& requestedEndCentrePos) const noexcept -> Point2dInt;
  auto UpdateTentaclesEndCentrePosOffsets() noexcept -> void;
  [[nodiscard]] static auto GetNewRadiusEndCentrePosOffset(
      float radiusScale,
      const Point2dFlt& oldTentacleEndPos,
      const Point2dInt& newCentreEndPosOffset) noexcept -> V3dFlt;
  static constexpr auto MIN_END_RADIUS       = 10.0F;
  static constexpr auto MAX_END_RADIUS       = 150.0F;
  static constexpr auto NUM_END_RADIUS_STEPS = 50U;
  IncrementedValue<float> m_endRadius{
      MIN_END_RADIUS, MAX_END_RADIUS, StepType::CONTINUOUS_REVERSIBLE, NUM_END_RADIUS_STEPS};
  auto UpdateTentaclesEndPos() noexcept -> void;

  auto PreDrawUpdateTentacles() noexcept -> void;
  auto DrawTentacles() noexcept -> void;
  auto PostDrawUpdateTentacles() noexcept -> void;
  auto IterateTentacle(Tentacle3D& tentacle) const noexcept -> void;
  bool m_useThickLines = true;
  [[nodiscard]] auto GetLineThickness(uint32_t tentacleNum) const noexcept -> uint8_t;
};

} // namespace GOOM::VISUAL_FX::TENTACLES

namespace GOOM::VISUAL_FX::TENTACLES
{

inline auto TentacleDriver::SetDominantColorMaps(
    const COLOR::ConstColorMapSharedPtr& dominantMainColorMap,
    const COLOR::ConstColorMapSharedPtr& dominantLowColorMap) -> void
{
  m_dominantMainColorMapPtr = dominantMainColorMap;
  m_dominantLowColorMapPtr  = dominantLowColorMap;
}

static constexpr auto MAIN_BRIGHTNESS_FACTOR = 0.5F;
static constexpr auto LOW_BRIGHTNESS_FACTOR  = 1.0F;
static constexpr auto PROB_LOW_MIX_SAME      = 0.5F;

static constexpr size_t CHANGE_CURRENT_COLOR_MAP_GROUP_EVERY_N_UPDATES = 400U;

static constexpr auto RADIUS_FACTOR_RANGE     = NumberRange{1.0F, 1.000001F};
static constexpr auto MIN_TENTACLE_GROUP_SIZE = 10U;
static constexpr auto TENTACLE_2D_X_MIN       = 0.0;
static constexpr auto TENTACLE_2D_Y_MIN       = 0.065736;
static constexpr auto TENTACLE_2D_Y_MAX       = 10000.0;
static constexpr auto TENTACLE_LENGTH         = 120.0F;
static constexpr auto NUM_TENTACLE_NODES      = 100U;
static constexpr auto MAX_LINE_THICKNESS      = 5U;
static constexpr auto PROB_THICK_LINES        = 0.9F;

static constexpr auto MIN_SINE_FREQUENCY          = 1.0F;
static constexpr auto MAX_SINE_FREQUENCY          = 3.1F;
static constexpr auto BASE_Y_WEIGHT_FACTOR_RANGE  = NumberRange{0.8F, 1.1F};
static constexpr auto ITER_ZERO_LERP_FACTOR       = 0.9;
static constexpr auto MIN_SINE_X0                 = 0.0F;
static const auto ITER_ZERO_Y_VAL_WAVE_ZERO_START = SineWaveMultiplier{
    SineWaveMultiplier::SineProperties{
                                       .frequency = MIN_SINE_FREQUENCY, .lower = -20.0F, .upper = +20.0F, .x0 = MIN_SINE_X0}
};

constexpr auto GetMatchingBaseYWeights(const float freq) noexcept -> Tentacle2D::BaseYWeights
{
  constexpr auto FREQUENCIES = std::array{
      1.0F,
      1.7F,
      2.3F,
      3.1F,
  };
  static_assert(FREQUENCIES.front() == MIN_SINE_FREQUENCY);
  static_assert(FREQUENCIES.back() == MAX_SINE_FREQUENCY);
  constexpr auto CORRESPONDING_BASE_Y_WEIGHTS = std::array{
      0.60F,
      0.70F,
      0.75F,
      0.80F,
  };
  constexpr auto HIGHEST_BASE_Y_WEIGHT = 0.85F;

  for (auto i = 0U; i < FREQUENCIES.size(); ++i)
  {
    if (freq <= FREQUENCIES.at(i))
    {
      return {.previous = CORRESPONDING_BASE_Y_WEIGHTS.at(i),
              .current  = 1.0F - CORRESPONDING_BASE_Y_WEIGHTS.at(i)};
    }
  }

  return {.previous = HIGHEST_BASE_Y_WEIGHT, .current = 1.0F - HIGHEST_BASE_Y_WEIGHT};
}

TentacleDriver::TentacleDriver(IGoomDraw& draw,
                               const GoomRand& goomRand,
                               const UTILS::GoomTime& goomTime,
                               const CirclesTentacleLayout& tentacleLayout,
                               const PixelChannelType defaultAlpha) noexcept
  : m_goomRand{&goomRand},
    m_goomTime{&goomTime},
    m_screenCentre{draw.GetDimensions().GetCentrePoint()},
    m_colorMaps{defaultAlpha},
    m_tentacleParams{.numNodes             = NUM_TENTACLE_NODES,
                     .length               = TENTACLE_LENGTH,
                     .iterZeroYValWaveFreq = MIN_SINE_FREQUENCY,
                     .iterZeroYValWave     = ITER_ZERO_Y_VAL_WAVE_ZERO_START},
    m_tentaclePlotter{draw, *m_goomRand},
    m_tentacles{GetTentacles(*m_goomRand, tentacleLayout, m_tentacleParams)}
{
}

auto TentacleDriver::GetTentacles(const GoomRand& goomRand,
                                  const CirclesTentacleLayout& tentacleLayout,
                                  const IterationParams& tentacleParams) noexcept
    -> std::vector<TentacleAndAttributes>
{
  const auto numTentacles = tentacleLayout.GetNumTentacles();

  auto tentacles = std::vector<TentacleAndAttributes>{};
  tentacles.reserve(numTentacles);

  for (auto i = 0U; i < numTentacles; ++i)
  {
    auto tentacle2D = CreateNewTentacle2D(goomRand, tentacleParams);
    tentacle2D->SetIterZeroLerpFactor(ITER_ZERO_LERP_FACTOR);

    auto tentacle = Tentacle3D{std::move(tentacle2D)};

    tentacle.SetStartPos(tentacleLayout.GetStartPoints().at(i));
    tentacle.SetEndPos(tentacleLayout.GetEndPoints().at(i));

    tentacles.emplace_back(TentacleAndAttributes{.tentacle3D      = std::move(tentacle),
                                                 .mainColorMapPtr = ConstColorMapSharedPtr{nullptr},
                                                 .lowColorMapPtr  = ConstColorMapSharedPtr{nullptr},
                                                 .currentMainColor = BLACK_PIXEL,
                                                 .currentLowColor  = BLACK_PIXEL});
  }

  return tentacles;
}

auto TentacleDriver::CreateNewTentacle2D(const GoomRand& goomRand,
                                         const IterationParams& tentacleParams) noexcept
    -> std::unique_ptr<Tentacle2D>
{
  const auto tentacleLen = tentacleParams.length;
  Ensures(tentacleLen >= 1.0F);
  const auto tent2dXMax = TENTACLE_2D_X_MIN + static_cast<double>(tentacleLen);
  Ensures(tent2dXMax >= 1.0);

  const auto dimensions = Tentacle2D::Dimensions{
      .xDimensions = {.min = TENTACLE_2D_X_MIN,        .max = tent2dXMax},
      .yDimensions = {.min = TENTACLE_2D_Y_MIN, .max = TENTACLE_2D_Y_MAX},
  };

  auto baseYWeights = GetMatchingBaseYWeights(tentacleParams.iterZeroYValWaveFreq);
  baseYWeights.previous *= goomRand.GetRandInRange<BASE_Y_WEIGHT_FACTOR_RANGE>();
  baseYWeights.current = 1.0F - baseYWeights.previous;

  return std::make_unique<Tentacle2D>(tentacleParams.numNodes, dimensions, baseYWeights);
}

auto TentacleDriver::SetWeightedColorMaps(
    const IVisualFx::WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  const auto baseMainColorMapName = weightedColorMaps.mainColorMaps.GetRandomColorMapName();
  const auto baseLowColorMapName  = weightedColorMaps.lowColorMaps.GetRandomColorMapName();

  static constexpr auto MIN_SATURATION       = 0.2F;
  static constexpr auto SATURATION_STEP_SIZE = 0.1F;
  auto saturation                            = IncrementedValue<float>{
      MIN_SATURATION, 1.0F, TValue::StepType::CONTINUOUS_REVERSIBLE, SATURATION_STEP_SIZE};
  static constexpr auto MIN_LIGHTNESS       = 0.2F;
  static constexpr auto LIGHTNESS_STEP_SIZE = 0.1F;
  auto lightness                            = IncrementedValue<float>{
      MIN_LIGHTNESS, 1.0F, TValue::StepType::CONTINUOUS_REVERSIBLE, LIGHTNESS_STEP_SIZE};

  std::ranges::for_each(
      m_tentacles,
      [this, &baseMainColorMapName, &baseLowColorMapName, &saturation, &lightness](auto& tentacle)
      {
        const auto tintProperties =
            ColorMaps::TintProperties{.saturation = saturation(), .lightness = lightness()};

        tentacle.mainColorMapPtr =
            m_colorMaps.GetTintedColorMapPtr(baseMainColorMapName, tintProperties);
        tentacle.lowColorMapPtr =
            m_colorMaps.GetTintedColorMapPtr(baseLowColorMapName, tintProperties);

        saturation.Increment();
        lightness.Increment();
      });

  m_currentColorT.Reset();
}

auto TentacleDriver::StartIterating() -> void
{
  std::ranges::for_each(m_tentacles, [](auto& tentacle) { tentacle.tentacle3D.StartIterating(); });

  m_endCentrePosT.Reset();
}

auto TentacleDriver::MultiplyIterZeroYValWaveFreq(const float value) -> void
{
  const auto newFreq = std::clamp(
      value * m_tentacleParams.iterZeroYValWaveFreq, MIN_SINE_FREQUENCY, MAX_SINE_FREQUENCY);
  m_tentacleParams.iterZeroYValWave.SetFrequency(newFreq);

  for (auto& tentacle : m_tentacles)
  {
    tentacle.tentacle3D.SetBaseYWeights(GetMatchingBaseYWeights(newFreq));
  }
}

auto TentacleDriver::CheckForTimerEvents() -> void
{
  if ((m_goomTime->GetCurrentTime() % CHANGE_CURRENT_COLOR_MAP_GROUP_EVERY_N_UPDATES) != 0U)
  {
    return;
  }

  ChangeTentacleColorMaps();

  m_tentaclePlotter.UpdateCameraPosition();
}

auto TentacleDriver::ChangeTentacleColorMaps() -> void
{
  ChangeSegmentMixes();

  m_tentacleGroupSize = m_goomRand->GetRandInRange(
      NumberRange{MIN_TENTACLE_GROUP_SIZE, static_cast<uint32_t>(m_tentacles.size() - 1)});

  m_useThickLines = m_goomRand->ProbabilityOf<PROB_THICK_LINES>();
}

auto TentacleDriver::SetTentaclesEndCentrePos(const Point2dInt& newEndCentrePos) noexcept -> void
{
  m_targetEndCentrePos   = GetAcceptableEndCentrePos(newEndCentrePos);
  m_previousEndCentrePos = lerp(m_previousEndCentrePos, m_targetEndCentrePos, m_endCentrePosT());
  m_endCentrePosT.Reset();
}

inline auto TentacleDriver::GetAcceptableEndCentrePos(
    const Point2dInt& requestedEndCentrePos) const noexcept -> Point2dInt
{
  static constexpr auto CLOSE_TO_SCREEN_CENTRE_T = 0.2F;
  return lerp(requestedEndCentrePos, m_screenCentre, CLOSE_TO_SCREEN_CENTRE_T);
}

auto TentacleDriver::UpdateTentaclesEndCentrePosOffsets() noexcept -> void
{
  const auto endCentrePos = lerp(m_previousEndCentrePos, m_targetEndCentrePos, m_endCentrePosT());
  const auto endCentrePosOffset = endCentrePos - ToVec2dInt(m_screenCentre);
  const auto radiusScale        = m_goomRand->GetRandInRange<RADIUS_FACTOR_RANGE>();

  std::ranges::for_each(m_tentacles,
                        [&endCentrePosOffset, &radiusScale](auto& tentacle)
                        {
                          const auto newRadiusCentreEndPosOffset = GetNewRadiusEndCentrePosOffset(
                              radiusScale, tentacle.tentacle3D.GetEndPos(), endCentrePosOffset);

                          tentacle.tentacle3D.SetEndPosOffset(newRadiusCentreEndPosOffset);
                        });
}

auto TentacleDriver::UpdateTentaclesEndPos() noexcept -> void
{
  std::ranges::for_each(m_tentacles,
                        [this](auto& tentacle)
                        {
                          const auto tentacleEndPos = tentacle.tentacle3D.GetEndPos();
                          const auto endRadius      = std::sqrt(UTILS::MATH::Sq(tentacleEndPos.x) +
                                                           UTILS::MATH::Sq(tentacleEndPos.y));
                          const auto newTentacleEndPos =
                              Scale(tentacle.tentacle3D.GetEndPos(), m_endRadius() / endRadius);

                          tentacle.tentacle3D.SetEndPos(newTentacleEndPos);
                        });
}

auto TentacleDriver::GetNewRadiusEndCentrePosOffset(
    const float radiusScale,
    const Point2dFlt& oldTentacleEndPos,
    const Point2dInt& newCentreEndPosOffset) noexcept -> V3dFlt
{
  const auto oldTentacleEndPosVec = Vec2dInt{.x = static_cast<int32_t>(oldTentacleEndPos.x),
                                             .y = static_cast<int32_t>(oldTentacleEndPos.y)};
  const auto newTentacleEndPos    = Point2dInt{
         .x = static_cast<int32_t>(radiusScale * static_cast<float>(oldTentacleEndPosVec.x)),
         .y = static_cast<int32_t>(radiusScale * static_cast<float>(oldTentacleEndPosVec.y)),
  };
  const auto newRadiusEndPosOffset = newTentacleEndPos - oldTentacleEndPosVec;

  const auto newRadiusCentreEndPosOffset =
      newCentreEndPosOffset + ToVec2dInt(newRadiusEndPosOffset);

  return V3dFlt{.x = static_cast<float>(newRadiusCentreEndPosOffset.x),
                .y = static_cast<float>(newRadiusCentreEndPosOffset.y),
                .z = 0.0F};
}

auto TentacleDriver::Update() -> void
{
  CheckForTimerEvents();

  PreDrawUpdateTentacles();
  DrawTentacles();
  PostDrawUpdateTentacles();
}

inline auto TentacleDriver::PreDrawUpdateTentacles() noexcept -> void
{
  UpdateTentaclesEndCentrePosOffsets();
  UpdateTentaclesEndPos();
}

auto TentacleDriver::DrawTentacles() noexcept -> void
{
  auto colorT = TValue{
      {.stepType = TValue::StepType::CONTINUOUS_REVERSIBLE, .numSteps = m_tentacleGroupSize}
  };

  m_tentaclePlotter.SetNodeTOffset(m_nodeTOffset());

  for (auto i = 0U; i < m_tentacles.size(); ++i)
  {
    auto& tentacleAndAttributes = m_tentacles.at(i);

    tentacleAndAttributes.currentMainColor =
        tentacleAndAttributes.mainColorMapPtr->GetColor(m_currentColorT());
    tentacleAndAttributes.currentLowColor =
        tentacleAndAttributes.lowColorMapPtr->GetColor(m_currentColorT());

    IterateTentacle(tentacleAndAttributes.tentacle3D);

    m_tentaclePlotter.SetEndDotColors({.color1 = m_dominantMainColorMapPtr->GetColor(colorT()),
                                       .color2 = m_dominantLowColorMapPtr->GetColor(colorT())});

    m_tentaclePlotter.SetTentacleLineThickness(GetLineThickness(i));

    static constexpr auto BRIGHTNESS = 10.0F;
    m_tentaclePlotter.SetGetColorsFunc(
        [this, &colorT, &tentacleAndAttributes](const float nodeT)
        { return GetMixedColors(colorT(), nodeT, tentacleAndAttributes, BRIGHTNESS); });

    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks): Not sure why this is flagged???
    m_tentaclePlotter.Plot3D(tentacleAndAttributes.tentacle3D);

    colorT.Increment();
  }
}

inline auto TentacleDriver::PostDrawUpdateTentacles() noexcept -> void
{
  m_tentacleParams.iterZeroYValWave.Increment();
  m_endCentrePosT.Increment();
  m_endRadius.Increment();
  m_currentColorT.Increment();
  m_nodeTOffset.Increment();
}

inline auto TentacleDriver::IterateTentacle(Tentacle3D& tentacle) const noexcept -> void
{
  tentacle.SetIterZeroYVal(m_tentacleParams.iterZeroYValWave.GetNext());
  tentacle.Iterate();
}

auto TentacleDriver::GetMixedColors(const float dominantT,
                                    const float nodeT,
                                    const TentacleAndAttributes& tentacleAndAttributes,
                                    const float brightness) const -> MultiplePixels
{
  const auto mixedColors = MultiplePixels{
      .color1 = ColorMaps::GetColorMix(m_dominantMainColorMapPtr->GetColor(dominantT),
                                       tentacleAndAttributes.mainColorMapPtr->GetColor(nodeT),
                                       m_mainColorSegmentMixT),
      .color2 = ColorMaps::GetColorMix(m_dominantLowColorMapPtr->GetColor(dominantT),
                                       tentacleAndAttributes.lowColorMapPtr->GetColor(nodeT),
                                       m_lowColorSegmentMixT)};

  return {.color1 = m_colorAdjust.GetAdjustment(MAIN_BRIGHTNESS_FACTOR * brightness,
                                                GetMainColor(mixedColors)),
          .color2 = m_colorAdjust.GetAdjustment(LOW_BRIGHTNESS_FACTOR * brightness,
                                                GetLowColor(mixedColors))};
}

inline auto TentacleDriver::ChangeSegmentMixes() noexcept -> void
{
  m_mainColorSegmentMixT = m_goomRand->GetRandInRange<COLOR_SEGMENT_MIX_T_RANGE>();

  m_lowColorSegmentMixT = m_goomRand->ProbabilityOf<PROB_LOW_MIX_SAME>()
                              ? m_mainColorSegmentMixT
                              : m_goomRand->GetRandInRange<COLOR_SEGMENT_MIX_T_RANGE>();
}

inline auto TentacleDriver::GetLineThickness(const uint32_t tentacleNum) const noexcept -> uint8_t
{
  if (not m_useThickLines)
  {
    return 1U;
  }

  static constexpr auto TWICE_MAX_THICKNESS = 2U * MAX_LINE_THICKNESS;

  auto lineThickness = static_cast<uint8_t>(1U + (tentacleNum % TWICE_MAX_THICKNESS));
  if (lineThickness <= MAX_LINE_THICKNESS)
  {
    return lineThickness;
  }

  lineThickness = TWICE_MAX_THICKNESS - lineThickness;
  if (0 == lineThickness)
  {
    lineThickness = 1U;
  }
  return lineThickness;
}

} // namespace GOOM::VISUAL_FX::TENTACLES
