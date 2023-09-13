//#undef NO_LOGGING

#include "tubes_fx.h"

#include "color/color_utils.h"
#include "color/random_color_maps.h"
#include "draw/goom_draw.h"
#include "draw/goom_draw_to_container.h"
#include "draw/goom_draw_to_many.h"
#include "draw/shape_drawers/bitmap_drawer.h"
#include "draw/shape_drawers/circle_drawer.h"
#include "draw/shape_drawers/line_drawer.h"
#include "draw/shape_drawers/pixel_drawer.h"
#include "fx_helper.h"
#include "goom/goom_config.h"
#include "goom/goom_graphic.h"
#include "goom/math20.h"
#include "goom/point2d.h"
#include "goom/spimpl.h"
#include "goom_plugin_info.h"
#include "goom_visual_fx.h"
#include "tubes/tube_data.h"
#include "tubes/tubes.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/misc.h"
#include "utils/math/parametric_functions2d.h"
#include "utils/math/paths.h"
#include "utils/t_values.h"
#include "utils/timer.h"
#include "visual_fx/fx_utils/random_pixel_blender.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

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
using UTILS::TValue;
using UTILS::GRAPHICS::ImageBitmap;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::OscillatingFunction;
using UTILS::MATH::OscillatingPath;
using UTILS::MATH::SMALL_FLOAT;

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
     {true, false, 3.4F, 150.0F, {10.0F, +0.5F, +0.5F}},
     {false, false, 0.19F, 130.0F, {50.0F, -0.75F, -1.0F}},
     {false, false, 0.18F, 130.0F, {40.0F, +1.0F, +0.75F}},
     }
};
constexpr auto MAIN_TUBE_INDEX             = 0U;
constexpr auto SECONDARY_TUBES_START_INDEX = 1U;
constexpr auto COMMON_CIRCLE_PATH_PARAMS   = OscillatingFunction::Params{10.0F, +3.0F, +3.0F};

[[nodiscard]] inline auto lerp(const OscillatingFunction::Params& params0,
                               const OscillatingFunction::Params& params1,
                               const float t) -> OscillatingFunction::Params
{
  return {
      STD20::lerp(params0.oscillatingAmplitude, params1.oscillatingAmplitude, t),
      STD20::lerp(params0.xOscillatingFreq, params1.xOscillatingFreq, t),
      STD20::lerp(params0.yOscillatingFreq, params1.yOscillatingFreq, t),
  };
}

constexpr auto MIN_COLORMAP_TIME = 100U;
constexpr auto MAX_COLORMAP_TIME = 1000U;

constexpr auto MIN_BRIGHTNESS_FACTOR = 0.01F;
constexpr auto MAX_BRIGHTNESS_FACTOR = 0.20F;

constexpr auto MIN_JITTER_TIME         = 50U;
constexpr auto MAX_JITTER_TIME         = 500U;
constexpr auto MIN_SHAPE_JITTER_OFFSET = 10.0F;
constexpr auto MAX_SHAPE_JITTER_OFFSET = 20.0F;

constexpr auto MIN_DECREASED_SPEED_TIME = 100U;
constexpr auto MAX_DECREASED_SPEED_TIME = 500U;
constexpr auto MIN_INCREASED_SPEED_TIME = 100U;
constexpr auto MAX_INCREASED_SPEED_TIME = 500U;
constexpr auto MIN_NORMAL_SPEED_TIME    = 20U;
constexpr auto MAX_NORMAL_SPEED_TIME    = 50U;

constexpr auto MIN_STAY_IN_CENTRE_TIME        = 1000U;
constexpr auto MAX_STAY_IN_CENTRE_TIME        = 1000U;
constexpr auto MIN_STAY_AWAY_FROM_CENTRE_TIME = 100U;
constexpr auto MAX_STAY_AWAY_FROM_CENTRE_TIME = 100U;

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
  TubeFxImpl(const FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept;

  auto Start() -> void;
  auto Resume() -> void;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) -> void;

  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>;
  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;

  auto ApplyToImageBuffers() -> void;

private:
  const FxHelper* m_fxHelper;
  const SmallImageBitmaps* m_smallBitmaps;
  PixelChannelType m_defaultAlpha = DEFAULT_VISUAL_FX_ALPHA;

  GoomDrawToContainer m_drawToContainer{m_fxHelper->draw->GetDimensions()};
  GoomDrawToMany m_drawToMany{
      m_fxHelper->draw->GetDimensions(), {m_fxHelper->draw, &m_drawToContainer}
  };
  BitmapDrawer m_bitmapDrawer{*m_fxHelper->draw};
  BitmapDrawer m_bitmapToManyDrawer{m_drawToMany};
  CircleDrawer m_circleDrawer{*m_fxHelper->draw};
  CircleDrawer m_circleToManyDrawer{m_drawToMany};
  LineDrawerClippedEndPoints m_lineDrawer{*m_fxHelper->draw};
  LineDrawerClippedEndPoints m_lineToManyDrawer{m_drawToMany};
  PixelDrawer m_pixelDrawer{*m_fxHelper->draw};

  WeightedRandomColorMaps m_mainColorMaps{};
  WeightedRandomColorMaps m_lowColorMaps{};
  bool m_allowMovingAwayFromCentre = false;
  bool m_oscillatingShapePath{m_fxHelper->goomRand->ProbabilityOf(PROB_OSCILLATING_SHAPE_PATH)};
  uint32_t m_numCapturedPrevShapesGroups              = 0;
  static constexpr auto PREV_SHAPES_CUTOFF_BRIGHTNESS = 0.005F;
  BrightnessAttenuation m_prevShapesBrightnessAttenuation{
      {m_fxHelper->draw->GetDimensions().GetWidth(),
       m_fxHelper->draw->GetDimensions().GetHeight(),
       PREV_SHAPES_CUTOFF_BRIGHTNESS}
  };
  [[nodiscard]] auto GetApproxBrightnessAttenuation() const -> float;

  RandomPixelBlender m_pixelBlender;
  auto UpdatePixelBlender() noexcept -> void;

  std::vector<Tube> m_tubes{};
  static constexpr auto ALL_JOIN_CENTRE_STEP = 0.001F;
  TValue m_allJoinCentreT{
      {ALL_JOIN_CENTRE_STEP, TValue::StepType::CONTINUOUS_REVERSIBLE}
  };
  Point2dInt m_screenCentre = m_fxHelper->draw->GetDimensions().GetCentrePoint();
  Point2dInt m_targetMiddlePos{0, 0};
  Point2dInt m_previousMiddlePos{0, 0};
  static constexpr auto MIDDLE_POS_NUM_STEPS = 100U;
  TValue m_middlePosT{
      {TValue::StepType::SINGLE_CYCLE, MIDDLE_POS_NUM_STEPS, TValue::MAX_T_VALUE}
  };
  [[nodiscard]] auto GetMiddlePos() const -> Point2dInt;
  Timer m_allStayInCentreTimer{m_fxHelper->goomInfo->GetTime(), 1};
  Timer m_allStayAwayFromCentreTimer{m_fxHelper->goomInfo->GetTime(),
                                     MAX_STAY_AWAY_FROM_CENTRE_TIME};
  auto IncrementAllJoinCentreT() -> void;
  [[nodiscard]] auto GetTransformedCentreVector(uint32_t tubeId, const Point2dInt& centre) const
      -> Vec2dInt;

  bool m_prevShapesJitter                               = false;
  static constexpr auto PREV_SHAPES_JITTER_AMOUNT       = 2;
  static constexpr auto MIN_CAPTURED_PREV_SHAPES_GROUPS = 4U;
  static constexpr auto JITTER_STEP                     = 0.1F;
  TValue m_shapeJitterT{
      {JITTER_STEP, TValue::StepType::CONTINUOUS_REVERSIBLE}
  };

  Timer m_colorMapTimer{
      m_fxHelper->goomInfo->GetTime(),
      m_fxHelper->goomRand->GetRandInRange(MIN_COLORMAP_TIME, MAX_COLORMAP_TIME + 1)};
  Timer m_changedSpeedTimer{m_fxHelper->goomInfo->GetTime(), 1};
  Timer m_jitterTimer{m_fxHelper->goomInfo->GetTime(), 1};
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

TubesFx::TubesFx(const FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept
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

TubesFx::TubeFxImpl::TubeFxImpl(const FxHelper& fxHelper,
                                const SmallImageBitmaps& smallBitmaps) noexcept
  : m_fxHelper{&fxHelper}, m_smallBitmaps{&smallBitmaps}, m_pixelBlender{*fxHelper.goomRand}
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

  m_oscillatingShapePath      = m_fxHelper->goomRand->ProbabilityOf(PROB_OSCILLATING_SHAPE_PATH);
  m_allowMovingAwayFromCentre = m_fxHelper->goomRand->ProbabilityOf(PROB_MOVE_AWAY_FROM_CENTRE);

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

  if (m_fxHelper->goomRand->ProbabilityOf(PROB_FOLLOW_ZOOM_MID_POINT))
  {
    m_targetMiddlePos = zoomMidpoint - ToVec2dInt(m_screenCentre);
  }
  else
  {
    m_targetMiddlePos = {0, 0};
  }
}

inline auto TubesFx::TubeFxImpl::GetMiddlePos() const -> Point2dInt
{
  return lerp(m_previousMiddlePos, m_targetMiddlePos, m_middlePosT());
}

auto TubesFx::TubeFxImpl::InitTubes() -> void
{
  const auto drawToOneFuncs = TubeDrawFuncs{
      [this](const Point2dInt& point1,
             const Point2dInt& point2,
             const MultiplePixels& colors,
             const uint8_t thickness) { DrawLineToOne(point1, point2, colors, thickness); },
      [this](const Point2dInt& point,
             const int radius,
             const MultiplePixels& colors,
             const uint8_t thickness) { DrawCircleToOne(point, radius, colors, thickness); },
      [this](const Point2dInt& point,
             const SmallImageBitmaps::ImageNames imageName,
             const uint32_t size,
             const MultiplePixels& colors) { DrawImageToOne(point, imageName, size, colors); },
  };
  const auto drawToManyFuncs = TubeDrawFuncs{
      [this](const Point2dInt& point1,
             const Point2dInt& point2,
             const MultiplePixels& colors,
             const uint8_t thickness) { DrawLineToMany(point1, point2, colors, thickness); },
      [this](const Point2dInt& point,
             const int radius,
             const MultiplePixels& colors,
             const uint8_t thickness) { DrawCircleToMany(point, radius, colors, thickness); },
      [this](const Point2dInt& point,
             const SmallImageBitmaps::ImageNames imageName,
             const uint32_t size,
             const MultiplePixels& colors) { DrawImageToMany(point, imageName, size, colors); },
  };

  const auto mainTubeData = TubeData{MAIN_TUBE_INDEX,
                                     drawToManyFuncs,
                                     m_fxHelper->draw->GetDimensions().GetWidth(),
                                     m_fxHelper->draw->GetDimensions().GetHeight(),
                                     &m_fxHelper->goomInfo->GetTime(),
                                     m_fxHelper->goomRand,
                                     m_mainColorMaps,
                                     m_lowColorMaps,
                                     TUBE_SETTINGS.at(MAIN_TUBE_INDEX).radiusEdgeOffset,
                                     TUBE_SETTINGS.at(MAIN_TUBE_INDEX).brightnessFactor};
  m_tubes.emplace_back(mainTubeData, TUBE_SETTINGS.at(MAIN_TUBE_INDEX).circlePathParams);

  for (auto i = SECONDARY_TUBES_START_INDEX; i < NUM_TUBES; ++i)
  {
    const auto tubeData = TubeData{i,
                                   drawToOneFuncs,
                                   m_fxHelper->draw->GetDimensions().GetWidth(),
                                   m_fxHelper->draw->GetDimensions().GetHeight(),
                                   &m_fxHelper->goomInfo->GetTime(),
                                   m_fxHelper->goomRand,
                                   m_mainColorMaps,
                                   m_lowColorMaps,
                                   TUBE_SETTINGS.at(i).radiusEdgeOffset,
                                   TUBE_SETTINGS.at(i).brightnessFactor};
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
  m_fxHelper->draw->SetPixelBlendFunc(m_pixelBlender.GetCurrentPixelBlendFunc());
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
  if (m_fxHelper->goomInfo->GetSoundEvents().GetTimeSinceLastGoom() >= 1)
  {
    return;
  }

  for (auto& tube : m_tubes)
  {
    if (m_colorMapTimer.Finished() and m_fxHelper->goomRand->ProbabilityOf(PROB_RESET_COLOR_MAPS))
    {
      m_colorMapTimer.SetTimeLimit(
          m_fxHelper->goomRand->GetRandInRange(MIN_COLORMAP_TIME, MAX_COLORMAP_TIME + 1));
      tube.ResetColorMaps();
      tube.SetBrightnessFactor(
          m_fxHelper->goomRand->GetRandInRange(MIN_BRIGHTNESS_FACTOR, MAX_BRIGHTNESS_FACTOR));
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

      if (m_fxHelper->goomInfo->GetSoundEvents().GetTimeSinceLastGoom() >= 1)
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
        const auto jitterAmount =
            !m_prevShapesJitter ? 0
                                : m_fxHelper->goomRand->GetRandInRange(
                                      -PREV_SHAPES_JITTER_AMOUNT, PREV_SHAPES_JITTER_AMOUNT + 1);
        const auto newPoint = Point2dInt{
            GetClipped(point.x + jitterAmount, m_fxHelper->draw->GetDimensions().GetIntWidth() - 1),
            GetClipped(point.y + jitterAmount,
                       m_fxHelper->draw->GetDimensions().GetIntHeight() - 1)};

        const auto avColor                      = GetAverageColor(colorsList);
        static constexpr auto BRIGHTNESS_FACTOR = 0.1F;
        const auto brightness                   = BRIGHTNESS_FACTOR * brightnessAttenuation;
        const auto newColor0                    = GetBrighterColor(brightness, avColor);

        // IMPORTANT - Best results come from putting color in second buffer.
        m_pixelDrawer.DrawPixels(newPoint, {BLACK_PIXEL, newColor0});
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
  return m_prevShapesBrightnessAttenuation.GetPositionBrightness({firstCoords.x, firstCoords.y},
                                                                 MIN_BRIGHTNESS);
}

auto TubesFx::TubeFxImpl::UpdatePreviousShapesSettings() -> void
{
  m_prevShapesJitter = m_fxHelper->goomRand->ProbabilityOf(PROB_PREV_SHAPES_JITTER);
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
    m_allStayInCentreTimer.SetTimeLimit(
        m_fxHelper->goomRand->GetRandInRange(MIN_STAY_IN_CENTRE_TIME, MAX_STAY_IN_CENTRE_TIME + 1));
  }
  else if (m_allJoinCentreT() <= (0.0F + SMALL_FLOAT))
  {
    m_allStayAwayFromCentreTimer.SetTimeLimit(m_fxHelper->goomRand->GetRandInRange(
        MIN_STAY_AWAY_FROM_CENTRE_TIME, MAX_STAY_AWAY_FROM_CENTRE_TIME + 1));
  }

  m_allJoinCentreT.Increment();
}

auto TubesFx::TubeFxImpl::ChangeSpeedForLowerVolumes(Tube& tube) -> void
{
  if (m_fxHelper->goomRand->ProbabilityOf(PROB_DECREASE_SPEED))
  {
    tube.DecreaseCentreSpeed();
    tube.DecreaseCircleSpeed();

    m_changedSpeedTimer.SetTimeLimit(m_fxHelper->goomRand->GetRandInRange(
        MIN_DECREASED_SPEED_TIME, MAX_DECREASED_SPEED_TIME + 1));
  }
  else if (m_fxHelper->goomRand->ProbabilityOf(PROB_NORMAL_SPEED))
  {
    tube.SetCentreSpeed(Tube::NORMAL_CENTRE_SPEED);
    tube.SetCircleSpeed(Tube::NORMAL_CIRCLE_SPEED);

    m_changedSpeedTimer.SetTimeLimit(
        m_fxHelper->goomRand->GetRandInRange(MIN_NORMAL_SPEED_TIME, MAX_NORMAL_SPEED_TIME + 1));
  }
  else if (m_fxHelper->goomRand->ProbabilityOf(PROB_RANDOM_INCREASE_SPEED))
  {
    tube.IncreaseCentreSpeed();
    tube.IncreaseCircleSpeed();

    m_changedSpeedTimer.SetTimeLimit(m_fxHelper->goomRand->GetRandInRange(
        MIN_INCREASED_SPEED_TIME, MAX_INCREASED_SPEED_TIME + 1));
  }
}

auto TubesFx::TubeFxImpl::ChangeSpeedForHigherVolumes(Tube& tube) -> void
{
  if (m_fxHelper->goomRand->ProbabilityOf(PROB_INCREASE_SPEED))
  {
    tube.IncreaseCentreSpeed();
    tube.IncreaseCircleSpeed();

    m_changedSpeedTimer.SetTimeLimit(m_fxHelper->goomRand->GetRandInRange(
        MIN_INCREASED_SPEED_TIME, MAX_INCREASED_SPEED_TIME + 1));
  }
}

auto TubesFx::TubeFxImpl::ChangeJitterOffsets(Tube& tube) -> void
{
  if (m_fxHelper->goomRand->ProbabilityOf(PROB_NO_SHAPE_JITTER))
  {
    tube.SetMaxJitterOffset(0);
  }
  else
  {
    const auto maxJitter = static_cast<int32_t>(std::round(
        STD20::lerp(MIN_SHAPE_JITTER_OFFSET, MAX_SHAPE_JITTER_OFFSET, m_shapeJitterT())));
    tube.SetMaxJitterOffset(maxJitter);
    m_shapeJitterT.Increment();
    m_jitterTimer.SetTimeLimit(
        m_fxHelper->goomRand->GetRandInRange(MIN_JITTER_TIME, MAX_JITTER_TIME + 1));
  }
}

} // namespace GOOM::VISUAL_FX
