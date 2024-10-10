module;

//#undef NO_LOGGING

#include <algorithm>
#include <cstdint>
#include <string>

module Goom.VisualFx.ShaderFx;

import Goom.Utils.Math.Misc;
import Goom.VisualFx.FxHelper;
import Goom.Lib.AssertUtils;
import Goom.Lib.FrameData;
import Goom.Lib.SPimpl;
import :HighContrast;
import :HueShifterLerper;
import :ShaderObjectLerper;

namespace GOOM::VISUAL_FX
{

using SHADERS::HighContrast;
using SHADERS::HueShiftLerper;
using SHADERS::ShaderObjectLerper;
using UTILS::Stopwatch;
using UTILS::MATH::Sq;

class ShaderFx::ShaderFxImpl
{
public:
  explicit ShaderFxImpl(const FxHelper& fxHelper) noexcept;

  auto Start() noexcept -> void;

  auto ChangeEffects() -> void;
  auto SetFrameMiscData(MiscData& miscData) noexcept -> void;
  auto ApplyToImageBuffers() -> void;
  auto ApplyEndEffect(const Stopwatch::TimeValues& timeValues) -> void;

private:
  MiscData* m_frameMiscData = nullptr;

  HighContrast m_highContrast;

  static constexpr auto HUE_SHIFT_LERPER_PARAMS = HueShiftLerper::Params{
      .numLerpStepsRange  = { 25U,  100U},
      .lerpConstTimeRange = {500U, 5000U},
  };
  HueShiftLerper m_hueShiftLerper;

  static constexpr auto CHROMA_FACTOR_LERPER_PARAMS = ShaderObjectLerper::Params{
      .valueRange           = {0.5F, 5.0F},
      .minValueRangeDist    = 0.1F,
      .numLerpStepsRange    = { 50U, 500U},
      .lerpConstTimeRange   = { 50U, 100U},
      .initialNumLerpSteps  = 50U,
      .initialLerpConstTime = 50U,
  };
  ShaderObjectLerper m_chromaFactorLerper;

  static constexpr auto BASE_COLOR_MULTIPLIER_LERPER_PARAMS = ShaderObjectLerper::Params{
      .valueRange           = {0.92F, 0.99F},
      .minValueRangeDist    = 0.025F,
      .numLerpStepsRange    = {  50U,  500U},
      .lerpConstTimeRange   = {  10U,   50U},
      .initialNumLerpSteps  = 50U,
      .initialLerpConstTime = 10U,
  };
  ShaderObjectLerper m_baseColorMultiplierLerper;

  static constexpr auto PREV_FRAME_T_MIX_LERPER_PARAMS = ShaderObjectLerper::Params{
      .valueRange           = {0.1F, 0.9F},
      .minValueRangeDist    = 0.2F,
      .numLerpStepsRange    = { 50U, 500U},
      .lerpConstTimeRange   = { 10U,  50U},
      .initialNumLerpSteps  = 50U,
      .initialLerpConstTime = 10U,
  };
  ShaderObjectLerper m_prevFrameTMixLerper;

  auto FadeToBlack(const Stopwatch::TimeValues& timeValues) -> void;
};

ShaderFx::ShaderFx(const FxHelper& fxHelper) noexcept
  : m_pimpl{spimpl::make_unique_impl<ShaderFxImpl>(fxHelper)}
{
}

auto ShaderFx::GetFxName() const noexcept -> std::string
{
  return "shader";
}

auto ShaderFx::Start() noexcept -> void
{
  m_pimpl->Start();
}

auto ShaderFx::Finish() noexcept -> void
{
  // nothing to do
}

auto ShaderFx::ChangePixelBlender(
    [[maybe_unused]] const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  // nothing to do.
}

auto ShaderFx::ChangeEffects() noexcept -> void
{
  m_pimpl->ChangeEffects();
}

auto ShaderFx::SetFrameMiscData(MiscData& miscData) noexcept -> void
{
  m_pimpl->SetFrameMiscData(miscData);
}

auto ShaderFx::ApplyToImageBuffers() noexcept -> void
{
  m_pimpl->ApplyToImageBuffers();
}

auto ShaderFx::ApplyEndEffect(const Stopwatch::TimeValues& timeValues) noexcept -> void
{
  m_pimpl->ApplyEndEffect(timeValues);
}

ShaderFx::ShaderFxImpl::ShaderFxImpl(const FxHelper& fxHelper) noexcept
  : m_highContrast{fxHelper.GetGoomInfo(), fxHelper.GetGoomRand()},
    m_hueShiftLerper{fxHelper.GetGoomInfo(), fxHelper.GetGoomRand(), HUE_SHIFT_LERPER_PARAMS},
    m_chromaFactorLerper{
        fxHelper.GetGoomInfo(), fxHelper.GetGoomRand(), CHROMA_FACTOR_LERPER_PARAMS},
    m_baseColorMultiplierLerper{
        fxHelper.GetGoomInfo(), fxHelper.GetGoomRand(), BASE_COLOR_MULTIPLIER_LERPER_PARAMS},
    m_prevFrameTMixLerper{
        fxHelper.GetGoomInfo(), fxHelper.GetGoomRand(), PREV_FRAME_T_MIX_LERPER_PARAMS}
{
}

inline auto ShaderFx::ShaderFxImpl::Start() noexcept -> void
{
  m_frameMiscData = nullptr;
}

inline auto ShaderFx::ShaderFxImpl::ChangeEffects() -> void
{
  m_highContrast.ChangeHighContrast();
  m_hueShiftLerper.ChangeValueRange();
  m_chromaFactorLerper.ChangeValueRange();
  m_baseColorMultiplierLerper.ChangeValueRange();
  m_prevFrameTMixLerper.ChangeValueRange();
}

inline auto ShaderFx::ShaderFxImpl::SetFrameMiscData(MiscData& miscData) noexcept -> void
{
  m_frameMiscData = &miscData;
}

inline auto ShaderFx::ShaderFxImpl::ApplyToImageBuffers() -> void
{
  // NOLINTBEGIN(clang-analyzer-core.NullDereference)
  Expects(m_frameMiscData != nullptr);

  m_highContrast.UpdateHighContrast();
  m_hueShiftLerper.Update();
  m_chromaFactorLerper.Update();
  m_baseColorMultiplierLerper.Update();
  m_prevFrameTMixLerper.Update();

  m_frameMiscData->brightness          = m_highContrast.GetCurrentBrightness();
  m_frameMiscData->hueShift            = m_hueShiftLerper.GetLerpedValue();
  m_frameMiscData->chromaFactor        = m_chromaFactorLerper.GetLerpedValue();
  m_frameMiscData->baseColorMultiplier = m_baseColorMultiplierLerper.GetLerpedValue();
  m_frameMiscData->prevFrameTMix       = m_prevFrameTMixLerper.GetLerpedValue();
  // NOLINTEND(clang-analyzer-core.NullDereference)
}

inline auto ShaderFx::ShaderFxImpl::ApplyEndEffect(const Stopwatch::TimeValues& timeValues) -> void
{
  FadeToBlack(timeValues);
}

inline auto ShaderFx::ShaderFxImpl::FadeToBlack(const Stopwatch::TimeValues& timeValues) -> void
{
  Expects(m_frameMiscData != nullptr);

  static constexpr auto TIME_REMAINING_CUTOFF_IN_MS = 20000.0F;

  if (timeValues.timeRemainingInMs > TIME_REMAINING_CUTOFF_IN_MS)
  {
    return;
  }

  static constexpr auto BRING_FINAL_BLACK_FORWARD_MS = 1000.0F;
  const auto timeLeftAsFraction =
      std::max(0.0F, timeValues.timeRemainingInMs - BRING_FINAL_BLACK_FORWARD_MS) /
      TIME_REMAINING_CUTOFF_IN_MS;

  m_frameMiscData->brightness = Sq(timeLeftAsFraction);
}

} // namespace GOOM::VISUAL_FX
