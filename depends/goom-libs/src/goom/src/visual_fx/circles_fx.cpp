//#undef NO_LOGGING

#include "circles_fx.h"

#include "circles/circle.h"
#include "circles/dot_paths.h"
#include "fx_helper.h"
#include "goom/goom_config.h"
#include "goom/goom_graphic.h"
#include "goom/goom_logger.h"
#include "goom/goom_time.h"
#include "goom/point2d.h"
#include "goom/spimpl.h"
#include "goom_plugin_info.h"
#include "goom_visual_fx.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/parametric_functions2d.h"
#include "utils/timer.h"
#include "visual_fx/circles/circle_params_builder.h"
#include "visual_fx/circles/circles.h"
#include "visual_fx/fx_utils/random_pixel_blender.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace GOOM::VISUAL_FX
{

using CIRCLES::Circle;
using CIRCLES::CircleParamsBuilder;
using CIRCLES::Circles;
using CIRCLES::DotPaths;
using FX_UTILS::RandomPixelBlender;
using UTILS::Timer;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::OscillatingFunction;
using UTILS::MATH::Weights;

class CirclesFx::CirclesFxImpl
{
public:
  CirclesFxImpl(const FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] static auto GetCurrentColorMapsNames() noexcept -> std::vector<std::string>;
  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;

  auto Start() noexcept -> void;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;

  auto ApplyToImageBuffers() noexcept -> void;

private:
  const FxHelper* m_fxHelper;
  const SmallImageBitmaps* m_smallBitmaps;
  Point2dInt m_screenCentre       = m_fxHelper->goomInfo->GetDimensions().GetCentrePoint();
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

  static constexpr uint32_t MIN_BLANK_AT_TARGET_TIME = 1;
  static constexpr uint32_t MAX_BLANK_AT_TARGET_TIME = 5;
  uint32_t m_blankAtTargetTime =
      m_fxHelper->goomRand->GetRandInRange(MIN_BLANK_AT_TARGET_TIME, MAX_BLANK_AT_TARGET_TIME + 1);
  Timer m_blankAtTargetTimer{m_fxHelper->goomInfo->GetTime(), m_blankAtTargetTime, true};

  static constexpr uint32_t MIN_PAUSE_AT_START_TIME = 0;
  static constexpr uint32_t MAX_PAUSE_AT_START_TIME = 0;
  uint32_t m_pauseAtStartTime =
      m_fxHelper->goomRand->GetRandInRange(MIN_PAUSE_AT_START_TIME, MAX_PAUSE_AT_START_TIME + 1);
  Timer m_pauseAtStartTimer{m_fxHelper->goomInfo->GetTime(), m_pauseAtStartTime, true};
};

CirclesFx::CirclesFx(const FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept
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
  return m_pimpl->GetCurrentColorMapsNames();
}

auto CirclesFx::ApplyToImageBuffers() noexcept -> void
{
  m_pimpl->ApplyToImageBuffers();
}

using CircleStartModes  = CircleParamsBuilder::CircleStartModes;
using CircleTargetModes = CircleParamsBuilder::CircleTargetModes;

static constexpr auto CIRCLE_START_SAME_RADIUS_WEIGHT      = 10.0F;
static constexpr auto CIRCLE_START_FOUR_CORNERED_WEIGHT    = 10.0F;
static constexpr auto CIRCLE_START_REDUCING_RADIUS_WEIGHT  = 10.0F;
static constexpr auto CIRCLE_TARGET_SIMILAR_TARGETS_WEIGHT = 10.0F;
static constexpr auto CIRCLE_TARGET_FOUR_CORNERS_WEIGHT    = 10.0F;

CirclesFx::CirclesFxImpl::CirclesFxImpl(const FxHelper& fxHelper,
                                        const SmallImageBitmaps& smallBitmaps) noexcept
  : m_fxHelper{&fxHelper}, m_smallBitmaps{&smallBitmaps},
    m_weightedCircleStartModes{
        *m_fxHelper->goomRand,
        {
            {CircleStartModes::SAME_RADIUS, CIRCLE_START_SAME_RADIUS_WEIGHT},
            {CircleStartModes::FOUR_CORNERED_IN_MAIN, CIRCLE_START_FOUR_CORNERED_WEIGHT},
            {CircleStartModes::REDUCING_RADIUS, CIRCLE_START_REDUCING_RADIUS_WEIGHT},
        }
    },
    m_weightedCircleTargetModes{
        *m_fxHelper->goomRand,
        {
            {CircleTargetModes::SIMILAR_TARGETS, CIRCLE_TARGET_SIMILAR_TARGETS_WEIGHT},
            {CircleTargetModes::FOUR_CORNERS, CIRCLE_TARGET_FOUR_CORNERS_WEIGHT},
        }
    },
    m_pixelBlender{*fxHelper.goomRand}
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
  static constexpr auto MIN_LERP = 0.0F;
  static constexpr auto MAX_LERP = 1.0F;
  const auto midLerp             = m_fxHelper->goomRand->GetRandInRange(MIN_LERP, MAX_LERP);
  const auto newCircleCentre     = lerp(m_screenCentre, zoomMidpoint, midLerp);

  const auto minPoint = Point2dInt{m_fxHelper->goomInfo->GetDimensions().GetIntWidth() / 10,
                                   m_fxHelper->goomInfo->GetDimensions().GetIntHeight() / 10};
  const auto maxPoint =
      Point2dInt{m_fxHelper->goomInfo->GetDimensions().GetIntWidth() - minPoint.x,
                 m_fxHelper->goomInfo->GetDimensions().GetIntHeight() - minPoint.y};

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
    m_blankAtTargetTimer.SetTimeLimit(m_blankAtTargetTime);
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
    m_pauseAtStartTimer.SetTimeLimit(m_pauseAtStartTime);
  }
}

inline auto CirclesFx::CirclesFxImpl::UpdateStates() noexcept -> void
{
  if (static constexpr auto NUM_UPDATE_SKIPS = 10U;
      (m_fxHelper->goomInfo->GetTime().GetCurrentTime() % NUM_UPDATE_SKIPS) != 0)
  {
    return;
  }

  m_blankAtTargetTime =
      m_fxHelper->goomRand->GetRandInRange(MIN_BLANK_AT_TARGET_TIME, MAX_BLANK_AT_TARGET_TIME + 1);
  m_pauseAtStartTime =
      m_fxHelper->goomRand->GetRandInRange(MIN_PAUSE_AT_START_TIME, MAX_PAUSE_AT_START_TIME + 1);

  m_circleParamsBuilder.SetCircleStartMode(m_weightedCircleStartModes.GetRandomWeighted());
  m_circleParamsBuilder.SetCircleTargetMode(m_weightedCircleTargetModes.GetRandomWeighted());
  m_circlesFullReset = true;

  UpdateCirclePathParams();
}

inline auto CirclesFx::CirclesFxImpl::UpdatePixelBlender() noexcept -> void
{
  m_fxHelper->draw->SetPixelBlendFunc(m_pixelBlender.GetCurrentPixelBlendFunc());
  m_pixelBlender.Update();
}

inline auto CirclesFx::CirclesFxImpl::UpdateCirclePathParams() noexcept -> void
{
  if (m_fxHelper->goomInfo->GetSoundEvents().GetTimeSinceLastGoom() > 0)
  {
    return;
  }

  m_circles->SetPathParams(GetPathParams());
}

inline auto CirclesFx::CirclesFxImpl::GetPathParams() const noexcept
    -> std::vector<OscillatingFunction::Params>
{
  static constexpr auto MIN_PATH_AMPLITUDE = 90.0F;
  static constexpr auto MAX_PATH_AMPLITUDE = 110.0F;
  static constexpr auto MIN_PATH_X_FREQ    = 0.9F;
  static constexpr auto MAX_PATH_X_FREQ    = 2.0F;
  static constexpr auto MIN_PATH_Y_FREQ    = 0.9F;
  static constexpr auto MAX_PATH_Y_FREQ    = 2.0F;

  const auto params = OscillatingFunction::Params{
      m_fxHelper->goomRand->GetRandInRange(MIN_PATH_AMPLITUDE, MAX_PATH_AMPLITUDE),
      m_fxHelper->goomRand->GetRandInRange(MIN_PATH_X_FREQ, MAX_PATH_X_FREQ),
      m_fxHelper->goomRand->GetRandInRange(MIN_PATH_Y_FREQ, MAX_PATH_Y_FREQ),
  };

  auto pathParams = std::vector<OscillatingFunction::Params>(NUM_CIRCLES);
  std::fill(begin(pathParams), end(pathParams), params);

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
