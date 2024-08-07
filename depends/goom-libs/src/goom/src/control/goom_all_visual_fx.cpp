module;

#include <algorithm>
#include <string>
#include <unordered_set>

module Goom.Control.GoomAllVisualFx;

import Goom.Control.GoomDrawables;
import Goom.Control.GoomStateHandler;
import Goom.VisualFx.FxHelper;
import Goom.VisualFx.FxUtils;
import Goom.Lib.FrameData;
import Goom.Lib.GoomGraphic;
import Goom.Lib.Point2d;
import Goom.Lib.SoundInfo;
import Goom.Lib.SPimpl;
import :AllStandardVisualFx;

namespace GOOM::CONTROL
{

using CONTROL::GoomDrawables;
using UTILS::Parallel;
using UTILS::Stopwatch;
using UTILS::GRAPHICS::SmallImageBitmaps;
using VISUAL_FX::FxHelper;
using VISUAL_FX::IVisualFx;
using VISUAL_FX::FX_UTILS::RandomPixelBlender;

GoomAllVisualFx::GoomAllVisualFx(Parallel& parallel,
                                 FxHelper& fxHelper,
                                 const SmallImageBitmaps& smallBitmaps,
                                 const std::string& resourcesDirectory,
                                 IGoomStateHandler& goomStateHandler) noexcept
  : m_goomRand{&fxHelper.GetGoomRand()},
    m_goomLogger{&fxHelper.GetGoomLogger()},
    m_allStandardVisualFx{spimpl::make_unique_impl<AllStandardVisualFx>(
        parallel, fxHelper, smallBitmaps, resourcesDirectory)},
    m_goomStateHandler{&goomStateHandler}
{
  m_allStandardVisualFx->SetResetDrawBuffSettingsFunc([this](const GoomDrawables fx)
                                                      { ResetCurrentDrawBuffSettings(fx); });
}

auto GoomAllVisualFx::Start() noexcept -> void
{
  ChangeAllFxPixelBlenders();

  m_allStandardVisualFx->Start();
}

auto GoomAllVisualFx::Finish() noexcept -> void
{
  m_allStandardVisualFx->Finish();
}

auto GoomAllVisualFx::ChangeState() noexcept -> void
{
  m_allStandardVisualFx->SuspendFx();

  static constexpr auto MAX_TRIES = 10U;
  const auto oldStateId           = m_goomStateHandler->GetCurrentState().GetId();

  for (auto numTry = 0U; numTry < MAX_TRIES; ++numTry)
  {
    m_goomStateHandler->ChangeToNextState();

    if ((not m_allowMultiThreadedStates) and
        m_goomStateHandler->GetCurrentState().IsMultiThreaded())
    {
      continue;
    }

    // Pick a different state if possible.
    if (not m_goomStateHandler->GetCurrentState().HasSameId(oldStateId))
    {
      break;
    }
  }

  m_currentDrawablesState = m_goomStateHandler->GetCurrentState();
  m_allStandardVisualFx->SetCurrentDrawablesState(m_currentDrawablesState);
  m_allStandardVisualFx->ChangeShaderVariables();

  m_allStandardVisualFx->GetLinesFx().ResetLineModes();

  m_allStandardVisualFx->ResumeFx();
}

inline auto GoomAllVisualFx::ResetCurrentDrawBuffSettings(const GoomDrawables fx) noexcept -> void
{
  m_resetDrawBuffSettings(GetCurrentBuffSettings(fx));
}

inline auto GoomAllVisualFx::GetCurrentBuffSettings(const GoomDrawables fx) const noexcept
    -> FXBuffSettings
{
  const auto buffIntensity = m_goomStateHandler->GetCurrentState().GetBuffIntensity(fx);
  // Careful here. > 1 reduces smearing.
  static constexpr auto INTENSITY_FACTOR = 1.0F;
  return {INTENSITY_FACTOR * buffIntensity};
}

auto GoomAllVisualFx::RefreshAllFx() noexcept -> void
{
  m_allStandardVisualFx->RefreshAllFx();
}

auto GoomAllVisualFx::ChangeAllFxColorMaps() noexcept -> void
{
  m_allStandardVisualFx->ChangeColorMaps();
  m_allStandardVisualFx->ChangeShaderVariables();
}

auto GoomAllVisualFx::ChangeAllFxPixelBlenders() noexcept -> void
{
  m_allStandardVisualFx->ChangeAllFxPixelBlenders(GetNextPixelBlenderParams());
}

auto GoomAllVisualFx::GetNextPixelBlenderParams() const noexcept -> IVisualFx::PixelBlenderParams
{
  switch (m_globalBlendTypeWeight.GetRandomWeighted())
  {
    case GlobalBlendType::NONRANDOM:
      return {false, RandomPixelBlender::PixelBlendType::ALPHA};
    case GlobalBlendType::ASYNC_RANDOM:
      return {true};
    case GlobalBlendType::SYNC_RANDOM:
      return {false, RandomPixelBlender::GetRandomPixelBlendType(*m_goomRand)};
  }
}

auto GoomAllVisualFx::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  m_allStandardVisualFx->SetZoomMidpoint(zoomMidpoint);
}

auto GoomAllVisualFx::GetFrameMiscData() const noexcept -> const MiscData&
{
  return m_allStandardVisualFx->GetFrameMiscData();
}

auto GoomAllVisualFx::SetFrameMiscData(MiscData& miscData) noexcept -> void
{
  m_allStandardVisualFx->SetFrameMiscData(miscData);
}

auto GoomAllVisualFx::ApplyCurrentStateToImageBuffers(const AudioSamples& soundData) noexcept
    -> void
{
  m_allStandardVisualFx->ApplyCurrentStateToImageBuffers(soundData);
}

auto GoomAllVisualFx::ApplyEndEffectIfNearEnd(const Stopwatch::TimeValues& timeValues) noexcept
    -> void
{
  m_allStandardVisualFx->ApplyEndEffectIfNearEnd(timeValues);
}

auto GoomAllVisualFx::GetCurrentColorMapsNames() noexcept -> std::unordered_set<std::string>
{
  return AllStandardVisualFx::GetActiveColorMapsNames();
}

} // namespace GOOM::CONTROL
