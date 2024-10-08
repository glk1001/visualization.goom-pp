module;

//#undef NO_LOGGING

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

module Goom.VisualFx.TubesFx;

import Goom.Color.ColorUtils;
import Goom.Color.RandomColorMaps;
import Goom.Draw.GoomDrawBase;
import Goom.Draw.GoomDrawToContainer;
import Goom.Draw.GoomDrawToMany;
import Goom.Draw.ShapeDrawers.BitmapDrawer;
import Goom.Draw.ShaperDrawers.CircleDrawer;
import Goom.Draw.ShaperDrawers.LineDrawer;
import Goom.Draw.ShaperDrawers.PixelDrawer;
import Goom.Utils.Timer;
import Goom.Utils.Graphics.ImageBitmaps;
import Goom.Utils.Graphics.SmallImageBitmaps;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Utils.Math.ParametricFunctions2d;
import Goom.Utils.Math.Paths;
import Goom.Utils.Math.TValues;
import Goom.VisualFx.FxHelper;
import Goom.VisualFx.FxUtils;
import Goom.Lib.GoomGraphic;
import Goom.Lib.Point2d;
import Goom.Lib.SPimpl;
import :TubeData;
import :Tubes;

namespace GOOM::VISUAL_FX
{

using COLOR::GetBrighterColor;
using COLOR::GetColorAverage;
using COLOR::WeightedRandomColorMaps;
using DRAW::GoomDrawToContainer;
using DRAW::GoomDrawToMany;
using DRAW::MultiplePixels;
using DRAW::SHAPE_DRAWERS::BitmapDrawer;
using DRAW::SHAPE_DRAWERS::CircleDrawer;
using DRAW::SHAPE_DRAWERS::LineDrawerClippedEndPoints;
using DRAW::SHAPE_DRAWERS::PixelDrawer;
using FX_UTILS::RandomPixelBlender;
using TUBES::BrightnessAttenuation;
using TUBES::Tube;
using TUBES::TubeData;
using TUBES::TubeDrawFuncs;
using UTILS::Timer;
using UTILS::GRAPHICS::ImageBitmap;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::NumberRange;
using UTILS::MATH::OscillatingFunction;
using UTILS::MATH::OscillatingPath;
using UTILS::MATH::SMALL_FLOAT;
using UTILS::MATH::TValue;

namespace
{

constexpr auto NUM_TUBES = 3U;

struct TubeSettings
{
  bool noMoveFromCentre{};
  bool noOscillating{};
  float brightnessFactor{};
  float radiusEdgeOffset{};
  OscillatingFunction::Params circlePathParams;
};
constexpr auto TUBE_SETTINGS = std::array<TubeSettings, NUM_TUBES>{
    {
     {.noMoveFromCentre = true,
         .noOscillating    = false,
         .brightnessFactor = 3.4F,
         .radiusEdgeOffset = 150.0F,
         .circlePathParams = {.oscillatingAmplitude = 10.0F,
                              .xOscillatingFreq     = +0.5F,
                              .yOscillatingFreq     = +0.5F}},
     {.noMoveFromCentre = false,
         .noOscillating    = false,
         .brightnessFactor = 0.19F,
         .radiusEdgeOffset = 130.0F,
         .circlePathParams = {.oscillatingAmplitude = 50.0F,
                              .xOscillatingFreq     = -0.75F,
                              .yOscillatingFreq     = -1.0F}},
     {.noMoveFromCentre = false,
         .noOscillating    = false,
         .brightnessFactor = 0.18F,
         .radiusEdgeOffset = 130.0F,
         .circlePathParams = {.oscillatingAmplitude = 40.0F,
                              .xOscillatingFreq     = +1.0F,
                              .yOscillatingFreq     = +0.75F}},
     }
};
constexpr auto MAIN_TUBE_INDEX             = 0U;
constexpr auto SECONDARY_TUBES_START_INDEX = 1U;
constexpr auto COMMON_CIRCLE_PATH_PARAMS   = OscillatingFunction::Params{
      .oscillatingAmplitude = 10.0F, .xOscillatingFreq = +3.0F, .yOscillatingFreq = +3.0F};

[[nodiscard]] inline auto lerp(const OscillatingFunction::Params& params0,
                               const OscillatingFunction::Params& params1,
                               const float t) -> OscillatingFunction::Params
{
  return {
      .oscillatingAmplitude =
          std::lerp(params0.oscillatingAmplitude, params1.oscillatingAmplitude, t),
      .xOscillatingFreq = std::lerp(params0.xOscillatingFreq, params1.xOscillatingFreq, t),
      .yOscillatingFreq = std::lerp(params0.yOscillatingFreq, params1.yOscillatingFreq, t),
  };
}

constexpr auto COLORMAP_TIME_RANGE = NumberRange{100U, 1000U};

constexpr auto BRIGHTNESS_FACTOR_RANGE = NumberRange{0.01F, 0.20F};

constexpr auto JITTER_TIME_RANGE         = NumberRange{50U, 500U};
constexpr auto SHAPE_JITTER_OFFSET_RANGE = NumberRange{10.0F, 20.0F};

constexpr auto DECREASED_SPEED_TIME_RANGE = NumberRange{100U, 500U};
constexpr auto INCREASED_SPEED_TIME_RANGE = NumberRange{100U, 500U};
constexpr auto NORMAL_SPEED_TIME_RANGE    = NumberRange{20U, 50U};

constexpr auto STAY_IN_CENTRE_TIME_RANGE        = NumberRange{1000U, 1000U};
constexpr auto STAY_AWAY_FROM_CENTRE_TIME_RANGE = NumberRange{100U, 100U};

constexpr auto PROB_RESET_COLOR_MAPS       = 1.0F / 3.0F;
constexpr auto PROB_DECREASE_SPEED         = 1.0F / 5.0F;
constexpr auto PROB_INCREASE_SPEED         = 1.0F / 2.0F;
constexpr auto PROB_RANDOM_INCREASE_SPEED  = 1.0F / 20.0F;
constexpr auto PROB_NORMAL_SPEED           = 1.0F / 20.0F;
constexpr auto PROB_NO_SHAPE_JITTER        = 0.8F;
constexpr auto PROB_PREV_SHAPES_JITTER     = 0.0F;
constexpr auto PROB_OSCILLATING_SHAPE_PATH = 1.0F;
constexpr auto PROB_MOVE_AWAY_FROM_CENTRE  = 0.3F;
constexpr auto PROB_FOLLOW_ZOOM_MID_POINT  = 0.3F;

} // namespace

class TubesFx::TubeFxImpl
{
public:
  TubeFxImpl(FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept;

  auto Start() -> void;
  auto Resume() -> void;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) -> void;

  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>;
  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;

  auto ApplyToImageBuffers() -> void;

private:
  FxHelper* m_fxHelper;
  const SmallImageBitmaps* m_smallBitmaps;
  PixelChannelType m_defaultAlpha = DEFAULT_VISUAL_FX_ALPHA;

  GoomDrawToContainer m_drawToContainer{m_fxHelper->GetDimensions()};
  GoomDrawToMany m_drawToMany{
      m_fxHelper->GetDimensions(), {&m_fxHelper->GetDraw(), &m_drawToContainer}
  };
  BitmapDrawer m_bitmapDrawer{m_fxHelper->GetDraw()};
  BitmapDrawer m_bitmapToManyDrawer{m_drawToMany};
  CircleDrawer m_circleDrawer{m_fxHelper->GetDraw()};
  CircleDrawer m_circleToManyDrawer{m_drawToMany};
  LineDrawerClippedEndPoints m_lineDrawer{m_fxHelper->GetDraw()};
  LineDrawerClippedEndPoints m_lineToManyDrawer{m_drawToMany};
  PixelDrawer m_pixelDrawer{m_fxHelper->GetDraw()};

  WeightedRandomColorMaps m_mainColorMaps;
  WeightedRandomColorMaps m_lowColorMaps;
  bool m_allowMovingAwayFromCentre = false;
  bool m_oscillatingShapePath{
      m_fxHelper->GetGoomRand().ProbabilityOf<PROB_OSCILLATING_SHAPE_PATH>()};
  uint32_t m_numCapturedPrevShapesGroups              = 0;
  static constexpr auto PREV_SHAPES_CUTOFF_BRIGHTNESS = 0.005F;
  BrightnessAttenuation m_prevShapesBrightnessAttenuation{
      {.screenWidth      = m_fxHelper->GetDimensions().GetWidth(),
       .screenHeight     = m_fxHelper->GetDimensions().GetHeight(),
       .cutoffBrightness = PREV_SHAPES_CUTOFF_BRIGHTNESS}
  };
  [[nodiscard]] auto GetApproxBrightnessAttenuation() const -> float;

  RandomPixelBlender m_pixelBlender;
  auto UpdatePixelBlender() noexcept -> void;

  std::vector<Tube> m_tubes;
  static constexpr auto ALL_JOIN_CENTRE_STEP = 0.001F;
  TValue m_allJoinCentreT{
      {.stepSize = ALL_JOIN_CENTRE_STEP, .stepType = TValue::StepType::CONTINUOUS_REVERSIBLE}
  };
  Point2dInt m_screenCentre = m_fxHelper->GetDimensions().GetCentrePoint();
  Point2dInt m_targetMiddlePos{.x = 0, .y = 0};
  Point2dInt m_previousMiddlePos{.x = 0, .y = 0};
  static constexpr auto MIDDLE_POS_NUM_STEPS = 100U;
  TValue m_middlePosT{
      {.stepType  = TValue::StepType::SINGLE_CYCLE,
       .numSteps  = MIDDLE_POS_NUM_STEPS,
       .startingT = TValue::MAX_T_VALUE}
  };
  [[nodiscard]] auto GetMiddlePos() const -> Point2dInt;
  Timer m_allStayInCentreTimer{m_fxHelper->GetGoomTime(), 1};
  Timer m_allStayAwayFromCentreTimer{m_fxHelper->GetGoomTime(),
                                     STAY_AWAY_FROM_CENTRE_TIME_RANGE.max};
  auto IncrementAllJoinCentreT() -> void;
  [[nodiscard]] auto GetTransformedCentreVector(uint32_t tubeId, const Point2dInt& centre) const
      -> Vec2dInt;

  bool m_prevShapesJitter                               = false;
  static constexpr auto PREV_SHAPES_JITTER_AMOUNT       = 2;
  static constexpr auto MIN_CAPTURED_PREV_SHAPES_GROUPS = 4U;
  static constexpr auto JITTER_STEP                     = 0.1F;
  TValue m_shapeJitterT{
      {.stepSize = JITTER_STEP, .stepType = TValue::StepType::CONTINUOUS_REVERSIBLE}
  };

  Timer m_colorMapTimer{m_fxHelper->GetGoomTime(),
                        m_fxHelper->GetGoomRand().GetRandInRange<COLORMAP_TIME_RANGE>()};
  Timer m_changedSpeedTimer{m_fxHelper->GetGoomTime(), 1};
  Timer m_jitterTimer{m_fxHelper->GetGoomTime(), 1};
  auto InitTubes() -> void;
  auto InitPaths() -> void;
  auto ResetTubes() -> void;
  auto DoUpdates() -> void;
  auto DrawShapes() -> void;
  auto AdjustTubePaths() -> void;
  auto DrawTubeCircles() -> void;
  auto DrawPreviousShapes() -> void;
  auto DrawCapturedPreviousShapesGroups() -> void;
  [[nodiscard]] static auto GetAverageColor(const GoomDrawToContainer::ColorsList& colorsList)
      -> Pixel;
  [[nodiscard]] static auto GetClipped(int32_t val, int32_t maxVal) -> int32_t;
  auto UpdatePreviousShapesSettings() -> void;
  auto UpdateColorMaps() -> void;
  auto UpdateSpeeds() -> void;
  auto ChangeSpeedForLowerVolumes(Tube& tube) -> void;
  auto ChangeSpeedForHigherVolumes(Tube& tube) -> void;
  auto ChangeJitterOffsets(Tube& tube) -> void;

  auto DrawLineToOne(const Point2dInt& point1,
                     const Point2dInt& point2,
                     const MultiplePixels& colors,
                     uint8_t thickness) -> void;
  auto DrawCircleToOne(const Point2dInt& point,
                       int radius,
                       const MultiplePixels& colors,
                       uint8_t thickness) -> void;
  auto DrawImageToOne(const Point2dInt& point,
                      SmallImageBitmaps::ImageNames imageName,
                      uint32_t size,
                      const MultiplePixels& colors) -> void;
  auto DrawLineToMany(const Point2dInt& point1,
                      const Point2dInt& point2,
                      const MultiplePixels& colors,
                      uint8_t thickness) -> void;
  auto DrawCircleToMany(const Point2dInt& point,
                        int radius,
                        const MultiplePixels& colors,
                        uint8_t thickness) -> void;
  auto DrawImageToMany(const Point2dInt& point,
                       SmallImageBitmaps::ImageNames imageName,
                       uint32_t size,
                       const MultiplePixels& colors) -> void;
  [[nodiscard]] auto GetImageBitmap(SmallImageBitmaps::ImageNames imageName, size_t size) const
      -> const ImageBitmap&;
  static auto GetSimpleColorFuncs(const MultiplePixels& colors)
      -> std::vector<BitmapDrawer::GetBitmapColorFunc>;
};

TubesFx::TubesFx(FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept
  : m_pimpl{spimpl::make_unique_impl<TubeFxImpl>(fxHelper, smallBitmaps)}
{
}

auto TubesFx::GetFxName() const noexcept -> std::string
{
  return "tube";
}

auto TubesFx::Start() noexcept -> void
{
  m_pimpl->Start();
}

auto TubesFx::Finish() noexcept -> void
{
  // Not needed.
}

auto TubesFx::Resume() noexcept -> void
{
  m_pimpl->Resume();
}

auto TubesFx::Suspend() noexcept -> void
{
  // Not needed.
}

auto TubesFx::ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pimpl->ChangePixelBlender(pixelBlenderParams);
}

auto TubesFx::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  m_pimpl->SetZoomMidpoint(zoomMidpoint);
}

auto TubesFx::SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  m_pimpl->SetWeightedColorMaps(weightedColorMaps);
}

auto TubesFx::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  return m_pimpl->GetCurrentColorMapsNames();
}

auto TubesFx::ApplyToImageBuffers() noexcept -> void
{
  m_pimpl->ApplyToImageBuffers();
}

TubesFx::TubeFxImpl::TubeFxImpl(FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept
  : m_fxHelper{&fxHelper}, m_smallBitmaps{&smallBitmaps}, m_pixelBlender{m_fxHelper->GetGoomRand()}
{
}

auto TubesFx::TubeFxImpl::Start() -> void
{
  m_numCapturedPrevShapesGroups = 0;

  InitTubes();
}

auto TubesFx::TubeFxImpl::Resume() -> void
{
  m_numCapturedPrevShapesGroups = 0;

  m_oscillatingShapePath = m_fxHelper->GetGoomRand().ProbabilityOf<PROB_OSCILLATING_SHAPE_PATH>();
  m_allowMovingAwayFromCentre =
      m_fxHelper->GetGoomRand().ProbabilityOf<PROB_MOVE_AWAY_FROM_CENTRE>();

  ResetTubes();
}

inline auto TubesFx::TubeFxImpl::GetImageBitmap(const SmallImageBitmaps::ImageNames imageName,
                                                const size_t size) const -> const ImageBitmap&
{
  return m_smallBitmaps->GetImageBitmap(
      imageName,
      std::clamp(size, SmallImageBitmaps::MIN_IMAGE_SIZE, SmallImageBitmaps::MAX_IMAGE_SIZE));
}

auto TubesFx::TubeFxImpl::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  return {m_mainColorMaps.GetColorMapsName(), m_lowColorMaps.GetColorMapsName()};
}

auto TubesFx::TubeFxImpl::SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept
    -> void
{
  m_mainColorMaps = WeightedRandomColorMaps{weightedColorMaps.mainColorMaps, m_defaultAlpha};
  for (auto& tube : m_tubes)
  {
    tube.SetWeightedMainColorMaps(m_mainColorMaps);
  }

  m_lowColorMaps = WeightedRandomColorMaps{weightedColorMaps.lowColorMaps, m_defaultAlpha};
  for (auto& tube : m_tubes)
  {
    tube.SetWeightedLowColorMaps(m_lowColorMaps);
  }
}

inline auto TubesFx::TubeFxImpl::ChangePixelBlender(
    const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pixelBlender.SetPixelBlendType(pixelBlenderParams);
}

inline auto TubesFx::TubeFxImpl::SetZoomMidpoint(const Point2dInt& zoomMidpoint) -> void
{
  m_previousMiddlePos = GetMiddlePos();
  m_middlePosT.Reset();

  if (m_fxHelper->GetGoomRand().ProbabilityOf<PROB_FOLLOW_ZOOM_MID_POINT>())
  {
    m_targetMiddlePos = zoomMidpoint - ToVec2dInt(m_screenCentre);
  }
  else
  {
    m_targetMiddlePos = {.x = 0, .y = 0};
  }
}

inline auto TubesFx::TubeFxImpl::GetMiddlePos() const -> Point2dInt
{
  return lerp(m_previousMiddlePos, m_targetMiddlePos, m_middlePosT());
}

auto TubesFx::TubeFxImpl::InitTubes() -> void
{
  const auto drawToOneFuncs = TubeDrawFuncs{
      .drawLine = [this](const Point2dInt& point1,
                         const Point2dInt& point2,
                         const MultiplePixels& colors,
                         const uint8_t thickness)
      { DrawLineToOne(point1, point2, colors, thickness); },
      .drawCircle = [this](const Point2dInt& point,
                           const int radius,
                           const MultiplePixels& colors,
                           const uint8_t thickness)
      { DrawCircleToOne(point, radius, colors, thickness); },
      .drawSmallImage = [this](const Point2dInt& point,
                               const SmallImageBitmaps::ImageNames imageName,
                               const uint32_t size,
                               const MultiplePixels& colors)
      { DrawImageToOne(point, imageName, size, colors); },
  };
  const auto drawToManyFuncs = TubeDrawFuncs{
      .drawLine = [this](const Point2dInt& point1,
                         const Point2dInt& point2,
                         const MultiplePixels& colors,
                         const uint8_t thickness)
      { DrawLineToMany(point1, point2, colors, thickness); },
      .drawCircle = [this](const Point2dInt& point,
                           const int radius,
                           const MultiplePixels& colors,
                           const uint8_t thickness)
      { DrawCircleToMany(point, radius, colors, thickness); },
      .drawSmallImage = [this](const Point2dInt& point,
                               const SmallImageBitmaps::ImageNames imageName,
                               const uint32_t size,
                               const MultiplePixels& colors)
      { DrawImageToMany(point, imageName, size, colors); },
  };

  const auto mainTubeData =
      TubeData{.tubeId           = MAIN_TUBE_INDEX,
               .drawFuncs        = drawToManyFuncs,
               .screenWidth      = m_fxHelper->GetDimensions().GetWidth(),
               .screenHeight     = m_fxHelper->GetDimensions().GetHeight(),
               .goomTime         = &m_fxHelper->GetGoomTime(),
               .goomRand         = &m_fxHelper->GetGoomRand(),
               .mainColorMaps    = m_mainColorMaps,
               .lowColorMaps     = m_lowColorMaps,
               .radiusEdgeOffset = TUBE_SETTINGS.at(MAIN_TUBE_INDEX).radiusEdgeOffset,
               .brightnessFactor = TUBE_SETTINGS.at(MAIN_TUBE_INDEX).brightnessFactor};
  m_tubes.emplace_back(mainTubeData, TUBE_SETTINGS.at(MAIN_TUBE_INDEX).circlePathParams);

  for (auto i = SECONDARY_TUBES_START_INDEX; i < NUM_TUBES; ++i)
  {
    const auto tubeData = TubeData{.tubeId           = i,
                                   .drawFuncs        = drawToOneFuncs,
                                   .screenWidth      = m_fxHelper->GetDimensions().GetWidth(),
                                   .screenHeight     = m_fxHelper->GetDimensions().GetHeight(),
                                   .goomTime         = &m_fxHelper->GetGoomTime(),
                                   .goomRand         = &m_fxHelper->GetGoomRand(),
                                   .mainColorMaps    = m_mainColorMaps,
                                   .lowColorMaps     = m_lowColorMaps,
                                   .radiusEdgeOffset = TUBE_SETTINGS.at(i).radiusEdgeOffset,
                                   .brightnessFactor = TUBE_SETTINGS.at(i).brightnessFactor};
    m_tubes.emplace_back(tubeData, TUBE_SETTINGS.at(i).circlePathParams);
  }

  for (auto& tube : m_tubes)
  {
    tube.ResetColorMaps();
    tube.SetCircleSpeed(Tube::NORMAL_CIRCLE_SPEED);
    tube.SetMaxJitterOffset(0);
  }

  InitPaths();
}

inline auto TubesFx::TubeFxImpl::DrawLineToOne(const Point2dInt& point1,
                                               const Point2dInt& point2,
                                               const MultiplePixels& colors,
                                               const uint8_t thickness) -> void
{
  m_lineDrawer.SetLineThickness(thickness);
  m_lineDrawer.DrawLine(point1, point2, colors);
}

inline auto TubesFx::TubeFxImpl::DrawLineToMany(const Point2dInt& point1,
                                                const Point2dInt& point2,
                                                const MultiplePixels& colors,
                                                const uint8_t thickness) -> void
{
  m_lineToManyDrawer.SetLineThickness(thickness);
  m_lineToManyDrawer.DrawLine(point1, point2, colors);
}

inline auto TubesFx::TubeFxImpl::DrawCircleToOne(const Point2dInt& point,
                                                 const int radius,
                                                 const MultiplePixels& colors,
                                                 [[maybe_unused]] const uint8_t thickness) -> void
{
  m_circleDrawer.DrawCircle(point, radius, colors);
}

inline auto TubesFx::TubeFxImpl::DrawCircleToMany(const Point2dInt& point,
                                                  const int radius,
                                                  const MultiplePixels& colors,
                                                  [[maybe_unused]] const uint8_t thickness) -> void
{
  m_circleToManyDrawer.DrawCircle(point, radius, colors);
}

inline auto TubesFx::TubeFxImpl::DrawImageToOne(const Point2dInt& point,
                                                const SmallImageBitmaps::ImageNames imageName,
                                                const uint32_t size,
                                                const MultiplePixels& colors) -> void
{
  m_bitmapDrawer.Bitmap(
      point, GetImageBitmap(imageName, static_cast<size_t>(size)), GetSimpleColorFuncs(colors));
}

inline auto TubesFx::TubeFxImpl::DrawImageToMany(const Point2dInt& point,
                                                 const SmallImageBitmaps::ImageNames imageName,
                                                 const uint32_t size,
                                                 const MultiplePixels& colors) -> void
{
  //m_drawToContainer.Bitmap(x, y, GetImageBitmap(imageName, size), GetSimpleColorFuncs(colors));
  m_bitmapToManyDrawer.Bitmap(
      point, GetImageBitmap(imageName, static_cast<size_t>(size)), GetSimpleColorFuncs(colors));
}

inline auto TubesFx::TubeFxImpl::GetSimpleColorFuncs(const MultiplePixels& colors)
    -> std::vector<BitmapDrawer::GetBitmapColorFunc>
{
  const auto getColor1 = [&colors]([[maybe_unused]] const Point2dInt& bitmapPoint,
                                   [[maybe_unused]] const Pixel& bgnd) { return colors.color1; };
  const auto getColor2 = [&colors]([[maybe_unused]] const Point2dInt& bitmapPoint,
                                   [[maybe_unused]] const Pixel& bgnd) { return colors.color2; };
  return {getColor1, getColor2};
}

auto TubesFx::TubeFxImpl::InitPaths() -> void
{
  const auto transformCentre = [this](const uint32_t tubeId, const Point2dInt& centre)
  { return this->GetTransformedCentreVector(tubeId, centre); };
  const auto centreStep = 1.0F / static_cast<float>(m_tubes.size());
  auto centreT          = 0.0F;
  for (auto& tube : m_tubes)
  {
    tube.SetCentrePathT(centreT);
    tube.SetTransformCentreFunc(transformCentre);
    centreT += centreStep;
  }

  for (auto i = 0U; i < NUM_TUBES; ++i)
  {
    m_tubes[i].SetCirclePathParams(TUBE_SETTINGS.at(i).circlePathParams);
  }
}

auto TubesFx::TubeFxImpl::ResetTubes() -> void
{
  //  m_drawToContainer.ClearAll();

  for (auto i = 0U; i < m_tubes.size(); ++i)
  {
    if (!TUBE_SETTINGS.at(i).noOscillating)
    {
      m_tubes[i].SetAllowOscillatingCirclePaths(m_oscillatingShapePath);
    }
  }
}

auto TubesFx::TubeFxImpl::ApplyToImageBuffers() -> void
{
  DoUpdates();

  DrawPreviousShapes();
  DrawShapes();
}

inline auto TubesFx::TubeFxImpl::UpdatePixelBlender() noexcept -> void
{
  m_fxHelper->GetDraw().SetPixelBlendFunc(m_pixelBlender.GetCurrentPixelBlendFunc());
  m_pixelBlender.Update();
}

auto TubesFx::TubeFxImpl::DoUpdates() -> void
{
  m_middlePosT.Increment();

  UpdatePixelBlender();
  UpdatePreviousShapesSettings();
  UpdateColorMaps();
  UpdateSpeeds();
}

auto TubesFx::TubeFxImpl::UpdateColorMaps() -> void
{
  if (m_fxHelper->GetSoundEvents().GetTimeSinceLastGoom() >= 1)
  {
    return;
  }

  for (auto& tube : m_tubes)
  {
    if (m_colorMapTimer.Finished() and
        m_fxHelper->GetGoomRand().ProbabilityOf<PROB_RESET_COLOR_MAPS>())
    {
      m_colorMapTimer.SetTimeLimitAndResetToZero(
          m_fxHelper->GetGoomRand().GetRandInRange<COLORMAP_TIME_RANGE>());
      tube.ResetColorMaps();
      tube.SetBrightnessFactor(m_fxHelper->GetGoomRand().GetRandInRange<BRIGHTNESS_FACTOR_RANGE>());
    }
  }
}

auto TubesFx::TubeFxImpl::UpdateSpeeds() -> void
{
  for (auto& tube : m_tubes)
  {
    if (!tube.IsActive())
    {
      continue;
    }

    if (m_jitterTimer.Finished())
    {
      tube.SetMaxJitterOffset(0);
    }

    if (m_changedSpeedTimer.Finished())
    {
      ChangeJitterOffsets(tube);

      if (m_fxHelper->GetSoundEvents().GetTimeSinceLastGoom() >= 1)
      {
        ChangeSpeedForLowerVolumes(tube);
      }
      else
      {
        ChangeSpeedForHigherVolumes(tube);
      }
    }
  }
}

auto TubesFx::TubeFxImpl::DrawShapes() -> void
{
  const auto prevShapesSize = m_drawToContainer.GetNumChangedCoords();

  DrawTubeCircles();
  AdjustTubePaths();

  ++m_numCapturedPrevShapesGroups;
  if (m_numCapturedPrevShapesGroups >= MIN_CAPTURED_PREV_SHAPES_GROUPS)
  {
    m_drawToContainer.ResizeChangedCoordsKeepingNewest(prevShapesSize);
  }

  IncrementAllJoinCentreT();
}

auto TubesFx::TubeFxImpl::DrawPreviousShapes() -> void
{
  if (0 == m_drawToContainer.GetNumChangedCoords())
  {
    return;
  }

  DrawCapturedPreviousShapesGroups();
}

auto TubesFx::TubeFxImpl::DrawTubeCircles() -> void
{
  const auto drawTubeCircles = [this](const size_t i)
  {
    if (!m_tubes[i].IsActive())
    {
      return;
    }
    m_tubes[i].DrawCircleOfShapes();
    //    tube.RotateShapeColorMaps();
  };

  for (auto i = 0U; i < m_tubes.size(); ++i)
  {
    drawTubeCircles(i);
  }
}

auto TubesFx::TubeFxImpl::AdjustTubePaths() -> void
{
  if (!m_allowMovingAwayFromCentre)
  {
    return;
  }

  for (auto i = 0U; i < NUM_TUBES; ++i)
  {
    m_tubes[i].SetCirclePathParams(
        lerp(TUBE_SETTINGS.at(i).circlePathParams, COMMON_CIRCLE_PATH_PARAMS, m_allJoinCentreT()));
  }
}

auto TubesFx::TubeFxImpl::DrawCapturedPreviousShapesGroups() -> void
{
  const auto brightnessAttenuation = GetApproxBrightnessAttenuation();
  using ColorsList                 = GoomDrawToContainer::ColorsList;

  m_drawToContainer.IterateChangedCoordsNewToOld(
      [this, &brightnessAttenuation](const Point2dInt& point, const ColorsList& colorsList)
      {
        static constexpr auto PREV_SHAPES_JITTER_AMOUNT_RANGE =
            NumberRange{-PREV_SHAPES_JITTER_AMOUNT, PREV_SHAPES_JITTER_AMOUNT};
        const auto jitterAmount =
            not m_prevShapesJitter
                ? 0
                : m_fxHelper->GetGoomRand().GetRandInRange<PREV_SHAPES_JITTER_AMOUNT_RANGE>();
        const auto newPoint = Point2dInt{
            .x = GetClipped(point.x + jitterAmount, m_fxHelper->GetDimensions().GetIntWidth() - 1),
            .y =
                GetClipped(point.y + jitterAmount, m_fxHelper->GetDimensions().GetIntHeight() - 1)};

        const auto avColor                      = GetAverageColor(colorsList);
        static constexpr auto BRIGHTNESS_FACTOR = 0.1F;
        const auto brightness                   = BRIGHTNESS_FACTOR * brightnessAttenuation;
        const auto newColor0                    = GetBrighterColor(brightness, avColor);

        // IMPORTANT - Best results come from putting color in second buffer.
        m_pixelDrawer.DrawPixels(newPoint, {.color1 = BLACK_PIXEL, .color2 = newColor0});
      });
}

inline auto TubesFx::TubeFxImpl::GetAverageColor(const GoomDrawToContainer::ColorsList& colorsList)
    -> Pixel
{
  if (1 == colorsList.count)
  {
    return colorsList.colorsArray[0];
  }
  if (0 == colorsList.count)
  {
    return BLACK_PIXEL;
  }

  return GetColorAverage(colorsList.count, colorsList.colorsArray);
}

inline auto TubesFx::TubeFxImpl::GetClipped(const int32_t val, const int32_t maxVal) -> int32_t
{
  if (val < 0)
  {
    return 0;
  }
  if (val > maxVal)
  {
    return maxVal;
  }
  return val;
}

auto TubesFx::TubeFxImpl::GetApproxBrightnessAttenuation() const -> float
{
  static constexpr auto MIN_BRIGHTNESS = 0.1F;
  const auto& firstCoords              = m_drawToContainer.GetChangedCoordsList().front();
  return m_prevShapesBrightnessAttenuation.GetPositionBrightness(
      {.x = firstCoords.x, .y = firstCoords.y}, MIN_BRIGHTNESS);
}

auto TubesFx::TubeFxImpl::UpdatePreviousShapesSettings() -> void
{
  m_prevShapesJitter = m_fxHelper->GetGoomRand().ProbabilityOf<PROB_PREV_SHAPES_JITTER>();
}

auto TubesFx::TubeFxImpl::GetTransformedCentreVector(const uint32_t tubeId,
                                                     const Point2dInt& centre) const -> Vec2dInt
{
  if ((!m_allowMovingAwayFromCentre) or TUBE_SETTINGS.at(tubeId).noMoveFromCentre)
  {
    return ToVec2dInt(GetMiddlePos());
  }
  return ToVec2dInt(lerp(centre, GetMiddlePos(), m_allJoinCentreT()));
}

auto TubesFx::TubeFxImpl::IncrementAllJoinCentreT() -> void
{
  if (not m_allStayInCentreTimer.Finished())
  {
    return;
  }
  if (not m_allStayAwayFromCentreTimer.Finished())
  {
    return;
  }

  if (m_allJoinCentreT() >= (1.0F - SMALL_FLOAT))
  {
    m_allStayInCentreTimer.SetTimeLimitAndResetToZero(
        m_fxHelper->GetGoomRand().GetRandInRange<STAY_IN_CENTRE_TIME_RANGE>());
  }
  else if (m_allJoinCentreT() <= (0.0F + SMALL_FLOAT))
  {
    m_allStayAwayFromCentreTimer.SetTimeLimitAndResetToZero(
        m_fxHelper->GetGoomRand().GetRandInRange<STAY_AWAY_FROM_CENTRE_TIME_RANGE>());
  }

  m_allJoinCentreT.Increment();
}

auto TubesFx::TubeFxImpl::ChangeSpeedForLowerVolumes(Tube& tube) -> void
{
  if (m_fxHelper->GetGoomRand().ProbabilityOf<PROB_DECREASE_SPEED>())
  {
    tube.DecreaseCentreSpeed();
    tube.DecreaseCircleSpeed();

    m_changedSpeedTimer.SetTimeLimitAndResetToZero(
        m_fxHelper->GetGoomRand().GetRandInRange<DECREASED_SPEED_TIME_RANGE>());
  }
  else if (m_fxHelper->GetGoomRand().ProbabilityOf<PROB_NORMAL_SPEED>())
  {
    tube.SetCentreSpeed(Tube::NORMAL_CENTRE_SPEED);
    tube.SetCircleSpeed(Tube::NORMAL_CIRCLE_SPEED);

    m_changedSpeedTimer.SetTimeLimitAndResetToZero(
        m_fxHelper->GetGoomRand().GetRandInRange<NORMAL_SPEED_TIME_RANGE>());
  }
  else if (m_fxHelper->GetGoomRand().ProbabilityOf<PROB_RANDOM_INCREASE_SPEED>())
  {
    tube.IncreaseCentreSpeed();
    tube.IncreaseCircleSpeed();

    m_changedSpeedTimer.SetTimeLimitAndResetToZero(
        m_fxHelper->GetGoomRand().GetRandInRange<INCREASED_SPEED_TIME_RANGE>());
  }
}

auto TubesFx::TubeFxImpl::ChangeSpeedForHigherVolumes(Tube& tube) -> void
{
  if (m_fxHelper->GetGoomRand().ProbabilityOf<PROB_INCREASE_SPEED>())
  {
    tube.IncreaseCentreSpeed();
    tube.IncreaseCircleSpeed();

    m_changedSpeedTimer.SetTimeLimitAndResetToZero(
        m_fxHelper->GetGoomRand().GetRandInRange<INCREASED_SPEED_TIME_RANGE>());
  }
}

auto TubesFx::TubeFxImpl::ChangeJitterOffsets(Tube& tube) -> void
{
  if (m_fxHelper->GetGoomRand().ProbabilityOf<PROB_NO_SHAPE_JITTER>())
  {
    tube.SetMaxJitterOffset(0);
  }
  else
  {
    const auto maxJitter = static_cast<int32_t>(std::round(
        std::lerp(SHAPE_JITTER_OFFSET_RANGE.min, SHAPE_JITTER_OFFSET_RANGE.max, m_shapeJitterT())));
    tube.SetMaxJitterOffset(maxJitter);
    m_shapeJitterT.Increment();
    m_jitterTimer.SetTimeLimitAndResetToZero(
        m_fxHelper->GetGoomRand().GetRandInRange<JITTER_TIME_RANGE>());
  }
}

} // namespace GOOM::VISUAL_FX
