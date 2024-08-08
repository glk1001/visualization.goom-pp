module;

//#undef NO_LOGGING

#include "goom/goom_logger.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

module Goom.VisualFx.CirclesFx;

import Goom.Utils.GoomTime;
import Goom.Utils.Timer;
import Goom.Utils.Graphics.SmallImageBitmaps;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.ParametricFunctions2d;
import Goom.VisualFx.CirclesFx.Circle;
import Goom.VisualFx.CirclesFx.CircleParamsBuilder;
import Goom.VisualFx.CirclesFx.Circles;
import Goom.VisualFx.CirclesFx.DotPaths;
import Goom.VisualFx.FxHelper;
import Goom.VisualFx.FxUtils;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomGraphic;
import Goom.Lib.Point2d;
import Goom.Lib.SPimpl;

namespace GOOM::VISUAL_FX
{

using CIRCLES::Circle;
using CIRCLES::CircleParamsBuilder;
using CIRCLES::Circles;
using CIRCLES::DotPaths;
using FX_UTILS::RandomPixelBlender;
using UTILS::Timer;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::NumberRange;
using UTILS::MATH::OscillatingFunction;
using UTILS::MATH::Weights;

class CirclesFx::CirclesFxImpl
{
public:
  CirclesFxImpl(FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] static auto GetCurrentColorMapsNames() noexcept -> std::vector<std::string>;
  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;

  auto Start() noexcept -> void;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;

  auto ApplyToImageBuffers() noexcept -> void;

private:
  FxHelper* m_fxHelper;
  const SmallImageBitmaps* m_smallBitmaps;
  Point2dInt m_screenCentre       = m_fxHelper->GetDimensions().GetCentrePoint();
  PixelChannelType m_defaultAlpha = DEFAULT_VISUAL_FX_ALPHA;

  static constexpr uint32_t NUM_CIRCLES = 5;
  Weights<CircleParamsBuilder::CircleStartModes> m_weightedCircleStartModes;
  Weights<CircleParamsBuilder::CircleTargetModes> m_weightedCircleTargetModes;
  CircleParamsBuilder m_circleParamsBuilder{NUM_CIRCLES, *m_fxHelper};
  std::vector<Circle::Params> m_circleParams{m_circleParamsBuilder.GetCircleParams()};
  std::unique_ptr<Circles> m_circles{MakeCircles()};
  [[nodiscard]] auto MakeCircles() const noexcept -> std::unique_ptr<Circles>;
  [[nodiscard]] auto GetBrightnessFactors() const noexcept -> std::vector<float>;

  auto DrawCircles() noexcept -> void;
  auto IncrementTs() noexcept -> void;
  auto UpdateStates() noexcept -> void;
  auto UpdateCirclePathParams() noexcept -> void;
  [[nodiscard]] auto GetPathParams() const noexcept -> std::vector<OscillatingFunction::Params>;
  [[nodiscard]] auto GetNextCircleCentre(const Point2dInt& zoomMidpoint) const noexcept
      -> Point2dInt;

  bool m_circlesFullReset = false;
  WeightedColorMaps m_lastWeightedColorMaps{};
  auto CheckCirclesFullReset() noexcept -> void;

  RandomPixelBlender m_pixelBlender;
  auto UpdatePixelBlender() noexcept -> void;

  static constexpr auto BLANK_AT_TARGET_TIME_RANGE = NumberRange{1U, 5U};
  uint32_t m_blankAtTargetTime =
      m_fxHelper->GetGoomRand().GetRandInRange<BLANK_AT_TARGET_TIME_RANGE>();
  Timer m_blankAtTargetTimer{m_fxHelper->GetGoomTime(), m_blankAtTargetTime, true};

  static constexpr auto PAUSE_AT_START_TIME_RANGE = NumberRange{0U, 0U};
  uint32_t m_pauseAtStartTime =
      m_fxHelper->GetGoomRand().GetRandInRange<PAUSE_AT_START_TIME_RANGE>();
  Timer m_pauseAtStartTimer{m_fxHelper->GetGoomTime(), m_pauseAtStartTime, true};
};

CirclesFx::CirclesFx(FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept
  : m_pimpl{spimpl::make_unique_impl<CirclesFxImpl>(fxHelper, smallBitmaps)}
{
}

auto CirclesFx::GetFxName() const noexcept -> std::string
{
  return "circles";
}

auto CirclesFx::Start() noexcept -> void
{
  m_pimpl->Start();
}

auto CirclesFx::Finish() noexcept -> void
{
  // nothing to do
}

auto CirclesFx::ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pimpl->ChangePixelBlender(pixelBlenderParams);
}

auto CirclesFx::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  m_pimpl->SetZoomMidpoint(zoomMidpoint);
}

auto CirclesFx::SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  m_pimpl->SetWeightedColorMaps(weightedColorMaps);
}

auto CirclesFx::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  return CirclesFxImpl::GetCurrentColorMapsNames();
}

auto CirclesFx::ApplyToImageBuffers() noexcept -> void
{
  m_pimpl->ApplyToImageBuffers();
}

using CircleStartModes  = CircleParamsBuilder::CircleStartModes;
using CircleTargetModes = CircleParamsBuilder::CircleTargetModes;

using enum CircleStartModes;
using enum CircleTargetModes;

static constexpr auto CIRCLE_START_SAME_RADIUS_WEIGHT      = 10.0F;
static constexpr auto CIRCLE_START_FOUR_CORNERED_WEIGHT    = 10.0F;
static constexpr auto CIRCLE_START_REDUCING_RADIUS_WEIGHT  = 10.0F;
static constexpr auto CIRCLE_TARGET_SIMILAR_TARGETS_WEIGHT = 10.0F;
static constexpr auto CIRCLE_TARGET_FOUR_CORNERS_WEIGHT    = 10.0F;

CirclesFx::CirclesFxImpl::CirclesFxImpl(FxHelper& fxHelper,
                                        const SmallImageBitmaps& smallBitmaps) noexcept
  : m_fxHelper{&fxHelper}, m_smallBitmaps{&smallBitmaps},
    m_weightedCircleStartModes{
        m_fxHelper->GetGoomRand(),
        {
            {.key= SAME_RADIUS, .weight=CIRCLE_START_SAME_RADIUS_WEIGHT},
            {.key= FOUR_CORNERED_IN_MAIN, .weight=CIRCLE_START_FOUR_CORNERED_WEIGHT},
            {.key= REDUCING_RADIUS, .weight=CIRCLE_START_REDUCING_RADIUS_WEIGHT},
        }
    },
    m_weightedCircleTargetModes{
        m_fxHelper->GetGoomRand(),
        {
            {.key= SIMILAR_TARGETS, .weight=CIRCLE_TARGET_SIMILAR_TARGETS_WEIGHT},
            {.key= FOUR_CORNERS, .weight=CIRCLE_TARGET_FOUR_CORNERS_WEIGHT},
        }
    },
    m_pixelBlender{fxHelper.GetGoomRand()}
{
}

inline auto CirclesFx::CirclesFxImpl::MakeCircles() const noexcept -> std::unique_ptr<Circles>
{
  auto circles = std::make_unique<Circles>(
      *m_fxHelper, *m_smallBitmaps, NUM_CIRCLES, GetPathParams(), m_circleParams);

  circles->SetGlobalBrightnessFactors(GetBrightnessFactors());

  return circles;
}

auto CirclesFx::CirclesFxImpl::GetBrightnessFactors() const noexcept -> std::vector<float>
{
  static constexpr auto LARGE_ENOUGH_RADIUS_RATIO     = 0.15F;
  static constexpr auto LARGE_RADIUS_RATIO_BRIGHTNESS = 1.0F;
  static constexpr auto SMALL_RADIUS_RATIO_BRIGHTNESS = 0.1F;

  const auto mainCircleRadius = m_circleParams.at(0).toTargetParams.circleRadius;

  LogInfo("mainCircleRadius = {}", mainCircleRadius);

  auto brightnessFactors = std::vector<float>(NUM_CIRCLES);
  for (auto i = 0U; i < NUM_CIRCLES; ++i)
  {
    LogInfo("circleRadius[{}] = {}", i, m_circleParams.at(i).toTargetParams.circleRadius);
    const auto radiusFactor = m_circleParams.at(i).toTargetParams.circleRadius / mainCircleRadius;
    LogInfo("radiusFactor[{}] = {}", i, radiusFactor);
    brightnessFactors.at(i) = radiusFactor > LARGE_ENOUGH_RADIUS_RATIO
                                  ? LARGE_RADIUS_RATIO_BRIGHTNESS
                                  : SMALL_RADIUS_RATIO_BRIGHTNESS;
  }

  return brightnessFactors;
}

inline auto CirclesFx::CirclesFxImpl::GetCurrentColorMapsNames() noexcept
    -> std::vector<std::string>
{
  return {};
}

inline auto CirclesFx::CirclesFxImpl::SetWeightedColorMaps(
    const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  const auto newWeightedColorMaps =
      GetWeightedColorMapsWithNewAlpha(weightedColorMaps, m_defaultAlpha);

  m_circles->SetWeightedColorMaps(newWeightedColorMaps.mainColorMaps,
                                  newWeightedColorMaps.lowColorMaps);

  m_lastWeightedColorMaps = newWeightedColorMaps;
}

inline auto CirclesFx::CirclesFxImpl::ChangePixelBlender(
    const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pixelBlender.SetPixelBlendType(pixelBlenderParams);
}

inline auto CirclesFx::CirclesFxImpl::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept
    -> void
{
  if (zoomMidpoint == m_circleParamsBuilder.GetMainCircleCentreTarget())
  {
    return;
  }

  m_circleParamsBuilder.SetMainCircleStartCentre(GetNextCircleCentre(zoomMidpoint));
  m_circleParamsBuilder.SetMainCircleCentreTarget(zoomMidpoint);

  m_circlesFullReset = true;
}

inline auto CirclesFx::CirclesFxImpl::GetNextCircleCentre(
    const Point2dInt& zoomMidpoint) const noexcept -> Point2dInt
{
  static constexpr auto LERP_RANGE = NumberRange{0.0F, 1.0F};
  const auto midLerp               = m_fxHelper->GetGoomRand().GetRandInRange<LERP_RANGE>();
  const auto newCircleCentre       = lerp(m_screenCentre, zoomMidpoint, midLerp);

  const auto minPoint = Point2dInt{.x = m_fxHelper->GetDimensions().GetIntWidth() / 10,
                                   .y = m_fxHelper->GetDimensions().GetIntHeight() / 10};
  const auto maxPoint = Point2dInt{.x = m_fxHelper->GetDimensions().GetIntWidth() - minPoint.x,
                                   .y = m_fxHelper->GetDimensions().GetIntHeight() - minPoint.y};

  // NOLINTNEXTLINE(readability-suspicious-call-argument)
  return clamp(newCircleCentre, minPoint, maxPoint);
}

inline auto CirclesFx::CirclesFxImpl::Start() noexcept -> void
{
  m_blankAtTargetTimer.SetToFinished();
  m_pauseAtStartTimer.SetToFinished();

  m_circles->Start();
}

inline auto CirclesFx::CirclesFxImpl::ApplyToImageBuffers() noexcept -> void
{
  UpdatePixelBlender();
  UpdateStates();
  DrawCircles();
  CheckCirclesFullReset();
}

inline auto CirclesFx::CirclesFxImpl::DrawCircles() noexcept -> void
{
  if (not m_blankAtTargetTimer.Finished())
  {
    return;
  }

  m_circles->UpdateAndDraw();
  IncrementTs();

  if (m_circles->HasPositionTJustHitEndBoundary())
  {
    m_blankAtTargetTimer.SetTimeLimitAndResetToZero(m_blankAtTargetTime);
  }
}

inline auto CirclesFx::CirclesFxImpl::IncrementTs() noexcept -> void
{
  if (not m_pauseAtStartTimer.Finished())
  {
    return;
  }

  m_circles->IncrementTs();

  if (m_circles->HasPositionTJustHitStartBoundary())
  {
    m_pauseAtStartTimer.SetTimeLimitAndResetToZero(m_pauseAtStartTime);
  }
}

inline auto CirclesFx::CirclesFxImpl::UpdateStates() noexcept -> void
{
  if (static constexpr auto NUM_UPDATE_SKIPS = 10U;
      (m_fxHelper->GetGoomTime().GetCurrentTime() % NUM_UPDATE_SKIPS) != 0)
  {
    return;
  }

  m_blankAtTargetTime = m_fxHelper->GetGoomRand().GetRandInRange<BLANK_AT_TARGET_TIME_RANGE>();
  m_pauseAtStartTime  = m_fxHelper->GetGoomRand().GetRandInRange<PAUSE_AT_START_TIME_RANGE>();

  m_circleParamsBuilder.SetCircleStartMode(m_weightedCircleStartModes.GetRandomWeighted());
  m_circleParamsBuilder.SetCircleTargetMode(m_weightedCircleTargetModes.GetRandomWeighted());
  m_circlesFullReset = true;

  UpdateCirclePathParams();
}

inline auto CirclesFx::CirclesFxImpl::UpdatePixelBlender() noexcept -> void
{
  m_fxHelper->GetDraw().SetPixelBlendFunc(m_pixelBlender.GetCurrentPixelBlendFunc());
  m_pixelBlender.Update();
}

inline auto CirclesFx::CirclesFxImpl::UpdateCirclePathParams() noexcept -> void
{
  if (m_fxHelper->GetSoundEvents().GetTimeSinceLastGoom() > 0)
  {
    return;
  }

  m_circles->SetPathParams(GetPathParams());
}

inline auto CirclesFx::CirclesFxImpl::GetPathParams() const noexcept
    -> std::vector<OscillatingFunction::Params>
{
  static constexpr auto PATH_AMPLITUDE_RANGE = NumberRange{90.0F, 110.0F};
  static constexpr auto PATH_X_FREQ_RANGE    = NumberRange{0.9F, 2.0F};
  static constexpr auto PATH_Y_FREQ_RANGE    = NumberRange{0.9F, 2.0F};

  const auto params = OscillatingFunction::Params{
      .oscillatingAmplitude = m_fxHelper->GetGoomRand().GetRandInRange<PATH_AMPLITUDE_RANGE>(),
      .xOscillatingFreq     = m_fxHelper->GetGoomRand().GetRandInRange<PATH_X_FREQ_RANGE>(),
      .yOscillatingFreq     = m_fxHelper->GetGoomRand().GetRandInRange<PATH_Y_FREQ_RANGE>(),
  };

  auto pathParams = std::vector<OscillatingFunction::Params>(NUM_CIRCLES);
  std::ranges::fill(pathParams, params);

  return pathParams;
}

inline auto CirclesFx::CirclesFxImpl::CheckCirclesFullReset() noexcept -> void
{
  if (not m_circlesFullReset)
  {
    return;
  }
  if (not m_circles->HasPositionTJustHitEndBoundary())
  {
    return;
  }

  Expects(not m_blankAtTargetTimer.Finished());
  Expects(m_circles->HasPositionTJustHitEndBoundary());
  Expects(m_circles->GetCurrentDirection() == DotPaths::Direction::TO_TARGET);

  m_circleParams = m_circleParamsBuilder.GetCircleParams();
  m_circles      = MakeCircles();
  m_circles->ChangeDirection(DotPaths::Direction::FROM_TARGET);
  m_circles->SetWeightedColorMaps(m_lastWeightedColorMaps.mainColorMaps,
                                  m_lastWeightedColorMaps.lowColorMaps);
  m_circlesFullReset = false;

  Ensures(m_circles->GetCurrentDirection() == DotPaths::Direction::FROM_TARGET);
}

} // namespace GOOM::VISUAL_FX
