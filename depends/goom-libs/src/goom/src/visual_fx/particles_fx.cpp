module;

#undef NO_LOGGING // NOLINT: This maybe be defined on command line.

#include "goom/goom_logger.h"

#include <cmath>
#include <cstdint>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <memory>
#include <string>
#include <vector>

module Goom.VisualFx.ParticlesFx;

import Particles.Effects.Attractor;
import Particles.Effects.Effect;
import Goom.Color.ColorMaps;
import Goom.Color.ColorUtils;
import Goom.Color.RandomColorMaps;
import Goom.Draw.GoomDrawBase;
import Goom.Draw.ShaperDrawers.CircleDrawer;
import Goom.Draw.ShaperDrawers.PixelDrawer;
import Goom.Utils.Graphics.Camera;
import Goom.Utils.Graphics.PixelUtils;
import Goom.Utils.Graphics.PointUtils;
import Goom.Utils.Graphics.SmallImageBitmaps;
import Goom.Utils.Math.IncrementedValues;
import Goom.Utils.Math.TValues;
import Goom.Utils.Math.Misc;
import Goom.Utils.GoomTime;
import Goom.VisualFx.FxHelper;
import Goom.VisualFx.FxUtils;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;
import Goom.Lib.SPimpl;

namespace GOOM::VISUAL_FX
{

using COLOR::ColorMapPtrWrapper;
using COLOR::GetBrighterColor;
using COLOR::WeightedRandomColorMaps;
using DRAW::IGoomDraw;
using DRAW::SHAPE_DRAWERS::CircleDrawer;
using DRAW::SHAPE_DRAWERS::PixelDrawer;
using FX_UTILS::RandomPixelBlender;
using PARTICLES::EFFECTS::AttractorEffect;
using PARTICLES::EFFECTS::IEffect;
using UTILS::GRAPHICS::Camera;
using UTILS::GRAPHICS::GetPointClippedToRectangle;
using UTILS::GRAPHICS::MakePixel;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::IncrementedValue;
using UTILS::MATH::TValue;
using UTILS::MATH::TWO_PI;
using UTILS::MATH::U_HALF;

// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access): glm problem

struct EffectData
{
  std::shared_ptr<IEffect> effect;
  const char* name{};
  uint32_t numParticles             = 1U;
  uint32_t minMaxNumAliveParticles  = 1U;
  uint32_t maxMaxNumAliveParticles  = 1U;
  float minMixAmount                = 1.0F;
  float maxMixAmount                = 1.0F;
  float brightness                  = 1.0F;
  float minDeltaTime                = 1.0;
  float maxDeltaTime                = 1.0;
  uint32_t minNumUpdatesBeforeReset = 1U;
  uint32_t maxNumUpdatesBeforeReset = 1U;
  Camera::Properties cameraProperties{};
};

class Renderer
{
public:
  Renderer(IGoomDraw& draw,
           GoomLogger& goomLogger,
           float brightness,
           const Camera& camera) noexcept;

  auto SetDrawCircleFrequency(uint32_t drawCircleFrequency) noexcept -> void;

  auto UpdateFrame(const IEffect& effect) noexcept -> void;

  [[nodiscard]] auto GetNumSkippedNegativeParticles() const noexcept -> uint64_t
  {
    return m_numSkippedNegativeParticles;
  }
  [[nodiscard]] auto GetNumSkippedTooBigParticles() const noexcept -> uint64_t
  {
    return m_numSkippedTooBigParticles;
  }

private:
  IGoomDraw* m_draw;
  [[maybe_unused]] GoomLogger* m_goomLogger;
  uint64_t m_numSkippedNegativeParticles = 0U;
  uint64_t m_numSkippedTooBigParticles   = 0U;
  uint32_t m_drawCircleFrequency         = 1U;
  float m_circleBrightness;
  static constexpr auto SINGLE_PIXEL_BRIGHTNESS_FACTOR = 5.0F;
  float m_pixelBrightness = SINGLE_PIXEL_BRIGHTNESS_FACTOR * m_circleBrightness;
  const Camera* m_camera;

  [[nodiscard]] auto GetScreenPos(const glm::vec4& pos) const -> Point2dInt;
};

class EffectFactory
{
public:
  [[nodiscard]] static auto Create(const char* name) -> EffectData;
};

auto EffectFactory::Create(const char* const name) -> EffectData
{
  static constexpr auto DEFAULT_MIN_DELTA_TIME               = 1.0F / 120.0F;
  static constexpr auto DEFAULT_MAX_DELTA_TIME               = 1.0F / 30.0F;
  static constexpr auto DEFAULT_MIN_NUM_UPDATES_BEFORE_RESET = 150U;
  static constexpr auto DEFAULT_MAX_NUM_UPDATES_BEFORE_RESET = 250U;
  static constexpr auto DEFAULT_MIN_MIX_AMOUNT               = 0.75F;
  static constexpr auto DEFAULT_MAX_MIX_AMOUNT               = 1.00F;
  static constexpr auto DEFAULT_FIELD_OF_VIEW_DEGREES =
      Camera::Properties::DEFAULT_FIELD_OF_VIEW_DEGREES;
  static constexpr auto DEFAULT_NEAR_Z_CLIP_PLANE = Camera::Properties::DEFAULT_NEAR_Z_CLIP_PLANE;
  static constexpr auto DEFAULT_FAR_Z_CLIP_PLANE  = Camera::Properties::DEFAULT_FAR_Z_CLIP_PLANE;
  static constexpr auto DEFAULT_EYE_POSITION      = glm::vec3{0.0F, 0.0F, -100.0F};
  static constexpr auto DEFAULT_TARGET_POSITION   = glm::vec3{0.0F, 0.0F, 0.0F};

  static constexpr auto NUM_PARTICLES = 20000U;
  static constexpr auto WORLD_SCALE   = 0.5F;
  static constexpr auto BRIGHTNESS    = 2.50F;
  const auto effect                   = std::make_shared<AttractorEffect>(NUM_PARTICLES);
  return {
      .effect                   = effect,
      .name                     = name,
      .numParticles             = NUM_PARTICLES,
      .minMaxNumAliveParticles  = U_HALF * NUM_PARTICLES,
      .maxMaxNumAliveParticles  = NUM_PARTICLES,
      .minMixAmount             = DEFAULT_MIN_MIX_AMOUNT,
      .maxMixAmount             = DEFAULT_MAX_MIX_AMOUNT,
      .brightness               = BRIGHTNESS,
      .minDeltaTime             = DEFAULT_MIN_DELTA_TIME,
      .maxDeltaTime             = DEFAULT_MAX_DELTA_TIME,
      .minNumUpdatesBeforeReset = DEFAULT_MIN_NUM_UPDATES_BEFORE_RESET,
      .maxNumUpdatesBeforeReset = DEFAULT_MAX_NUM_UPDATES_BEFORE_RESET,
      .cameraProperties         = {
                                   .scale              = WORLD_SCALE,
                                   .fieldOfViewDegrees = DEFAULT_FIELD_OF_VIEW_DEGREES,
                                   .nearZClipPlane     = DEFAULT_NEAR_Z_CLIP_PLANE,
                                   .farZClipPlane      = DEFAULT_FAR_Z_CLIP_PLANE,
                                   .eyePosition        = DEFAULT_EYE_POSITION,
                                   .targetPosition     = DEFAULT_TARGET_POSITION,
                                   }
  };
}

namespace
{

constexpr auto GetPixel(const glm::vec4& color) noexcept -> Pixel
{
  Expects(color.r >= 0.0F);
  Expects(color.g >= 0.0F);
  Expects(color.b >= 0.0F);
  Expects(color.a >= 0.0F);

  return MakePixel(color.r, color.g, color.b, color.a);
}

[[nodiscard]] constexpr auto GetColor(const Pixel& pixel) noexcept -> glm::vec4
{
  return {pixel.RFlt(), pixel.GFlt(), pixel.BFlt(), pixel.AFlt()};
}

} // namespace

Renderer::Renderer(IGoomDraw& draw,
                   GoomLogger& goomLogger,
                   const float brightness,
                   const Camera& camera) noexcept
  : m_draw{&draw}, m_goomLogger{&goomLogger}, m_circleBrightness{brightness}, m_camera{&camera}
{
}

inline auto Renderer::SetDrawCircleFrequency(const uint32_t drawCircleFrequency) noexcept -> void
{
  m_drawCircleFrequency = drawCircleFrequency;
}

auto Renderer::UpdateFrame(const IEffect& effect) noexcept -> void
{
  auto circleDrawer = CircleDrawer{*m_draw};
  auto pixelDrawer  = PixelDrawer{*m_draw};

  for (auto i = 0U; i < effect.GetSystem()->GetNumAliveParticles(); ++i)
  {
    const auto pos       = effect.GetSystem()->GetFinalData()->GetPosition(i);
    const auto color     = effect.GetSystem()->GetFinalData()->GetColor(i);
    const auto screenPos = GetScreenPos(pos);

    if ((screenPos.x < 0) or (screenPos.y < 0))
    {
      ++m_numSkippedTooBigParticles;
      continue;
    }
    if ((screenPos.x >= m_draw->GetDimensions().GetIntWidth()) or
        (screenPos.y >= m_draw->GetDimensions().GetIntHeight()))
    {
      ++m_numSkippedTooBigParticles;
      continue;
    }

    const auto pixelColor = GetPixel(color);

    if (0 == (i % m_drawCircleFrequency))
    {
      const auto brighterPixelColor = GetBrighterColor(m_circleBrightness, pixelColor);
      static constexpr auto RADIUS  = 4;
      circleDrawer.DrawFilledCircle(screenPos, RADIUS, {brighterPixelColor, brighterPixelColor});
    }
    else
    {
      const auto brighterPixelColor = GetBrighterColor(m_pixelBrightness, pixelColor);
      pixelDrawer.DrawPixelsClipped(screenPos, {brighterPixelColor, brighterPixelColor});
    }
  }
}

inline auto Renderer::GetScreenPos(const glm::vec4& pos) const -> Point2dInt
{
  return m_camera->GetScreenPosition(pos);
}

class ParticlesFx::ParticlesFxImpl
{
public:
  ParticlesFxImpl(FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] static auto GetCurrentColorMapsNames() noexcept -> std::vector<std::string>;
  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;

  auto Start() noexcept -> void;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;

  auto ApplyToImageBuffers() noexcept -> void;

private:
  FxHelper* m_fxHelper;
  //const SmallImageBitmaps* m_smallBitmaps;
  static constexpr auto DEFAULT_PARTICLES_FX_ALPHA = MAX_ALPHA / 20U;
  PixelChannelType m_defaultAlpha                  = DEFAULT_PARTICLES_FX_ALPHA;
  auto UpdateCounter() noexcept -> void;

  static constexpr auto ADD_WEIGHT          = 100.0F;
  static constexpr auto DARKEN_ONLY_WEIGHT  = 0.0F;
  static constexpr auto LIGHTEN_ONLY_WEIGHT = 5.0F;
  static constexpr auto LUMA_MIX_WEIGHT     = 5.0F;
  static constexpr auto MULTIPLY_WEIGHT     = 5.0F;
  static constexpr auto ALPHA_WEIGHT        = 100.0F;
  // clang-format off
  RandomPixelBlender m_pixelBlender{
      m_fxHelper->GetGoomRand(),
      {
          {RandomPixelBlender::PixelBlendType::ADD,          ADD_WEIGHT},
          {RandomPixelBlender::PixelBlendType::DARKEN_ONLY,  DARKEN_ONLY_WEIGHT},
          {RandomPixelBlender::PixelBlendType::LIGHTEN_ONLY, LIGHTEN_ONLY_WEIGHT},
          {RandomPixelBlender::PixelBlendType::LUMA_MIX,     LUMA_MIX_WEIGHT},
          {RandomPixelBlender::PixelBlendType::MULTIPLY,     MULTIPLY_WEIGHT},
          {RandomPixelBlender::PixelBlendType::ALPHA,        ALPHA_WEIGHT},
      }
  };
  // clang-format on
  auto UpdatePixelBlender() noexcept -> void;

  Point2dInt m_screenPositionOffset{};
  Point2dInt m_previousScreenPositionOffset{};
  static constexpr auto SCREEN_POSITION_OFFSET_NUM_STEPS = 100U;
  TValue m_screenPositionOffsetT{
      {TValue::StepType::SINGLE_CYCLE, SCREEN_POSITION_OFFSET_NUM_STEPS}
  };
  [[nodiscard]] auto GetScreenPositionOffset() const noexcept -> Point2dInt;
  auto GetAdjustedZoomMidpoint(const Point2dInt& zoomMidpoint) const noexcept -> Point2dInt;

  ColorMapPtrWrapper m_tintMainColorMap{nullptr};
  ColorMapPtrWrapper m_tintLowColorMap{nullptr};
  static constexpr auto TINT_COLORS_NUM_STEPS = 100U;
  TValue m_tintColorT{
      {TValue::StepType::CONTINUOUS_REVERSIBLE, TINT_COLORS_NUM_STEPS}
  };

  static constexpr auto MIN_DRAW_CIRCLE_FREQUENCY = 5U;
  static constexpr auto MAX_DRAW_CIRCLE_FREQUENCY = 100U;

  EffectData m_effectData;
  uint32_t m_numUpdatesBeforeReset;
  float m_deltaTime;
  auto ResetEffect() noexcept -> void;
  auto UpdateEffect() noexcept -> void;

  static constexpr auto PROB_CHANGE_SPEED   = 0.3F;
  static constexpr auto PROB_INCREASE_SPEED = 0.5F;
  auto ChangeEffectSpeed() noexcept -> void;

  Camera m_camera;
  static constexpr auto CAMERA_EYE_ROTATE_NUM_STEPS = 200U;
  IncrementedValue<float> m_cameraEyeRotateRadians{
      0.0F, TWO_PI, TValue::StepType::CONTINUOUS_REPEATABLE, CAMERA_EYE_ROTATE_NUM_STEPS};
  static constexpr auto CAMERA_ROLL_NUM_STEPS = 500U;
  IncrementedValue<float> m_cameraRollRadians{
      0.0F, TWO_PI, TValue::StepType::CONTINUOUS_REPEATABLE, CAMERA_ROLL_NUM_STEPS};
  Renderer m_renderer;
  auto UpdateCamera() noexcept -> void;

  auto IncrementTs() noexcept -> void;
};

ParticlesFx::ParticlesFx(FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept
  : m_pimpl{spimpl::make_unique_impl<ParticlesFxImpl>(fxHelper, smallBitmaps)}
{
}

auto ParticlesFx::GetFxName() const noexcept -> std::string
{
  return "particles";
}

auto ParticlesFx::Start() noexcept -> void
{
  m_pimpl->Start();
}

auto ParticlesFx::Finish() noexcept -> void
{
  // nothing to do
}

auto ParticlesFx::ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pimpl->ChangePixelBlender(pixelBlenderParams);
}

auto ParticlesFx::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  m_pimpl->SetZoomMidpoint(zoomMidpoint);
}

auto ParticlesFx::SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  m_pimpl->SetWeightedColorMaps(weightedColorMaps);
}

auto ParticlesFx::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  return ParticlesFxImpl::GetCurrentColorMapsNames();
}

auto ParticlesFx::ApplyToImageBuffers() noexcept -> void
{
  m_pimpl->ApplyToImageBuffers();
}

ParticlesFx::ParticlesFxImpl::ParticlesFxImpl(
    FxHelper& fxHelper, [[maybe_unused]] const SmallImageBitmaps& smallBitmaps) noexcept
  : m_fxHelper{&fxHelper},
    //m_smallBitmaps{&smallBitmaps},
    m_effectData{EffectFactory::Create("attractor")},
    m_numUpdatesBeforeReset{fxHelper.GetGoomRand().GetRandInRange(
        m_effectData.minNumUpdatesBeforeReset, m_effectData.maxNumUpdatesBeforeReset + 1)},
    m_deltaTime{m_fxHelper->GetGoomRand().GetRandInRange(m_effectData.minDeltaTime,
                                                         m_effectData.maxDeltaTime)},
    m_camera{m_effectData.cameraProperties, fxHelper.GetDimensions()},
    m_renderer{fxHelper.GetDraw(), fxHelper.GetGoomLogger(), m_effectData.brightness, m_camera}
{
}

inline auto ParticlesFx::ParticlesFxImpl::ResetEffect() noexcept -> void
{
  m_effectData.effect->Reset();

  m_effectData.effect->SetMaxNumAliveParticles(m_fxHelper->GetGoomRand().GetRandInRange(
      m_effectData.minMaxNumAliveParticles, m_effectData.maxMaxNumAliveParticles + 1U));
  m_effectData.effect->SetTintMixAmount(m_fxHelper->GetGoomRand().GetRandInRange(
      m_effectData.minMixAmount, m_effectData.maxMixAmount));

  m_numUpdatesBeforeReset = m_fxHelper->GetGoomRand().GetRandInRange(
      m_effectData.minNumUpdatesBeforeReset, m_effectData.maxNumUpdatesBeforeReset + 1U);

  ChangeEffectSpeed();
}

inline auto ParticlesFx::ParticlesFxImpl::ChangeEffectSpeed() noexcept -> void
{
  if (not m_fxHelper->GetGoomRand().ProbabilityOf(PROB_CHANGE_SPEED))
  {
    return;
  }

  if (not m_fxHelper->GetGoomRand().ProbabilityOf(PROB_INCREASE_SPEED))
  {
    m_deltaTime = m_fxHelper->GetGoomRand().GetRandInRange(m_deltaTime, m_effectData.maxDeltaTime);
  }
  else
  {
    m_deltaTime = m_fxHelper->GetGoomRand().GetRandInRange(m_effectData.minDeltaTime, m_deltaTime);
  }
}

inline auto ParticlesFx::ParticlesFxImpl::GetCurrentColorMapsNames() noexcept
    -> std::vector<std::string>
{
  return {};
}

inline auto ParticlesFx::ParticlesFxImpl::SetWeightedColorMaps(
    const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  m_tintMainColorMap =
      WeightedRandomColorMaps{weightedColorMaps.mainColorMaps, m_defaultAlpha}.GetRandomColorMap();
  m_tintLowColorMap =
      WeightedRandomColorMaps{weightedColorMaps.lowColorMaps, m_defaultAlpha}.GetRandomColorMap();

  m_renderer.SetDrawCircleFrequency(m_fxHelper->GetGoomRand().GetRandInRange(
      MIN_DRAW_CIRCLE_FREQUENCY, MAX_DRAW_CIRCLE_FREQUENCY + 1));
}

inline auto ParticlesFx::ParticlesFxImpl::ChangePixelBlender(
    const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pixelBlender.SetPixelBlendType(pixelBlenderParams);
}

inline auto ParticlesFx::ParticlesFxImpl::UpdatePixelBlender() noexcept -> void
{
  m_fxHelper->GetDraw().SetPixelBlendFunc(m_pixelBlender.GetCurrentPixelBlendFunc());
  m_pixelBlender.Update();
}

inline auto ParticlesFx::ParticlesFxImpl::SetZoomMidpoint(
    [[maybe_unused]] const Point2dInt& zoomMidpoint) noexcept -> void
{
  const auto clippedZoomMidpoint = GetAdjustedZoomMidpoint(zoomMidpoint);
  m_previousScreenPositionOffset = GetScreenPositionOffset();
  m_screenPositionOffset =
      clippedZoomMidpoint - ToVec2dInt(m_fxHelper->GetDimensions().GetCentrePoint());
  m_screenPositionOffsetT.Reset();
}

auto ParticlesFx::ParticlesFxImpl::GetAdjustedZoomMidpoint(
    const Point2dInt& zoomMidpoint) const noexcept -> Point2dInt
{
  if (m_fxHelper->GetDimensions().GetCentrePoint() == zoomMidpoint)
  {
    return zoomMidpoint;
  }

  const auto xMax = m_fxHelper->GetDimensions().GetIntWidth() - 1;
  const auto yMax = m_fxHelper->GetDimensions().GetIntHeight() - 1;

  const auto minZoomMidpoint   = Point2dInt{xMax / 10, yMax / 10};
  const auto maxZoomMidpoint   = Point2dInt{xMax - minZoomMidpoint.x, yMax - minZoomMidpoint.y};
  const auto zoomClipRectangle = Rectangle2dInt{minZoomMidpoint, maxZoomMidpoint};

  return GetPointClippedToRectangle(
      zoomMidpoint, zoomClipRectangle, m_fxHelper->GetDimensions().GetCentrePoint());
}

inline auto ParticlesFx::ParticlesFxImpl::GetScreenPositionOffset() const noexcept -> Point2dInt
{
  return lerp(m_previousScreenPositionOffset, m_screenPositionOffset, m_screenPositionOffsetT());
}

inline auto ParticlesFx::ParticlesFxImpl::Start() noexcept -> void
{
  ResetEffect();
}

inline auto ParticlesFx::ParticlesFxImpl::ApplyToImageBuffers() noexcept -> void
{
  UpdatePixelBlender();

  UpdateEffect();

  m_renderer.UpdateFrame(*m_effectData.effect);

  UpdateCounter();

  UpdateCamera();

  IncrementTs();
}

inline auto ParticlesFx::ParticlesFxImpl::UpdateEffect() noexcept -> void
{
  m_effectData.effect->SetTintColor(GetColor(m_tintMainColorMap.GetColor(m_tintColorT())));
  m_effectData.effect->Update(static_cast<double>(m_deltaTime));
}

inline auto ParticlesFx::ParticlesFxImpl::UpdateCounter() noexcept -> void
{
  if (0 == (m_fxHelper->GetGoomTime().GetCurrentTime() % m_numUpdatesBeforeReset))
  {
    ResetEffect();
  }
}

inline auto ParticlesFx::ParticlesFxImpl::UpdateCamera() noexcept -> void
{
  const auto zRadius = 100.0F;
  m_camera.SetEyePosition({zRadius * std::sin(m_cameraEyeRotateRadians()),
                           0.0F,
                           zRadius * std::cos(m_cameraEyeRotateRadians())});

  m_camera.SetRoll(m_cameraRollRadians());

  m_camera.SetScreenPositionOffset(GetScreenPositionOffset());
}

inline auto ParticlesFx::ParticlesFxImpl::IncrementTs() noexcept -> void
{
  m_cameraEyeRotateRadians.Increment();
  m_cameraRollRadians.Increment();
  m_screenPositionOffsetT.Increment();
  m_tintColorT.Increment();
}

// NOLINTEND(cppcoreguidelines-pro-type-union-access): glm problem

} // namespace GOOM::VISUAL_FX
