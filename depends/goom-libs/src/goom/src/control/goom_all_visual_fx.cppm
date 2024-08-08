module;

#include "goom/goom_logger.h"

#include <functional>
#include <string>
#include <string_view>
#include <unordered_set>

export module Goom.Control.GoomAllVisualFx;

import Goom.Control.GoomDrawables;
import Goom.Control.GoomStateHandler;
import Goom.Utils.Parallel;
import Goom.Utils.Stopwatch;
import Goom.Utils.Graphics.SmallImageBitmaps;
import Goom.Utils.Math.GoomRand;
import Goom.VisualFx.VisualFxBase;
import Goom.VisualFx.FxHelper;
import Goom.Lib.FrameData;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;
import Goom.Lib.SoundInfo;
import Goom.Lib.SPimpl;
import :AllStandardVisualFx;
import :VisualFxColorMaps;

using GOOM::UTILS::Parallel;
using GOOM::UTILS::Stopwatch;
using GOOM::UTILS::GRAPHICS::SmallImageBitmaps;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::Weights;
using GOOM::VISUAL_FX::FxHelper;
using GOOM::VISUAL_FX::IVisualFx;

export namespace GOOM::CONTROL
{

class GoomAllVisualFx
{
public:
  GoomAllVisualFx() noexcept = delete;
  GoomAllVisualFx(Parallel& parallel,
                  FxHelper& fxHelper,
                  const SmallImageBitmaps& smallBitmaps,
                  const std::string& resourcesDirectory,
                  IGoomStateHandler& goomStateHandler) noexcept;

  auto Start() noexcept -> void;
  auto Finish() noexcept -> void;

  auto SetAllowMultiThreadedStates(bool val) noexcept -> void;

  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;

  auto SetNextState() noexcept -> void;
  [[nodiscard]] auto GetCurrentStateName() const noexcept -> std::string_view;

  using ResetDrawBuffSettingsFunc = std::function<void(const FXBuffSettings& settings)>;
  auto SetResetDrawBuffSettingsFunc(const ResetDrawBuffSettingsFunc& func) noexcept -> void;

  auto ChangeAllFxColorMaps() noexcept -> void;
  auto ChangeAllFxPixelBlenders() noexcept -> void;
  auto RefreshAllFx() noexcept -> void;

  [[nodiscard]] auto GetFrameMiscData() const noexcept -> const MiscData&;
  auto SetFrameMiscData(MiscData& miscData) noexcept -> void;
  auto ApplyCurrentStateToImageBuffers(const AudioSamples& soundData) noexcept -> void;
  auto ApplyEndEffectIfNearEnd(const Stopwatch::TimeValues& timeValues) noexcept -> void;

  [[nodiscard]] static auto GetCurrentColorMapsNames() noexcept -> std::unordered_set<std::string>;

private:
  const GoomRand* m_goomRand;
  [[maybe_unused]] GoomLogger* m_goomLogger;
  spimpl::unique_impl_ptr<AllStandardVisualFx> m_allStandardVisualFx;

  IGoomStateHandler* m_goomStateHandler;
  bool m_allowMultiThreadedStates = true;
  auto ChangeState() noexcept -> void;
  GoomDrawablesState m_currentDrawablesState;

  ResetDrawBuffSettingsFunc m_resetDrawBuffSettings;
  auto ResetCurrentDrawBuffSettings(GoomDrawables fx) noexcept -> void;
  [[nodiscard]] auto GetCurrentBuffSettings(GoomDrawables fx) const noexcept -> FXBuffSettings;

  VisualFxColorMaps m_visualFxColorMaps{*m_goomRand};

  [[nodiscard]] auto GetNextPixelBlenderParams() const noexcept -> IVisualFx::PixelBlenderParams;
  enum class GlobalBlendType : UnderlyingEnumType
  {
    NONRANDOM,
    ASYNC_RANDOM,
    SYNC_RANDOM,
  };
  static constexpr auto NONRANDOM_WEIGHT    = 50.0F;
  static constexpr auto SYNC_RANDOM_WEIGHT  = 50.0F;
  static constexpr auto ASYNC_RANDOM_WEIGHT = 50.0F;
  Weights<GlobalBlendType> m_globalBlendTypeWeight{
      *m_goomRand,
      {{.key = GlobalBlendType::NONRANDOM, .weight = NONRANDOM_WEIGHT},
                  {.key = GlobalBlendType::SYNC_RANDOM, .weight = SYNC_RANDOM_WEIGHT},
                  {.key = GlobalBlendType::ASYNC_RANDOM, .weight = ASYNC_RANDOM_WEIGHT}}
  };
};

} // namespace GOOM::CONTROL

namespace GOOM::CONTROL
{

inline auto GoomAllVisualFx::SetAllowMultiThreadedStates(const bool val) noexcept -> void
{
  m_allowMultiThreadedStates = val;
}

inline auto GoomAllVisualFx::SetNextState() noexcept -> void
{
  ChangeState();
  ChangeAllFxColorMaps();
  ChangeAllFxPixelBlenders();
}

inline auto GoomAllVisualFx::SetResetDrawBuffSettingsFunc(
    const ResetDrawBuffSettingsFunc& func) noexcept -> void
{
  m_resetDrawBuffSettings = func;
}

inline auto GoomAllVisualFx::GetCurrentStateName() const noexcept -> std::string_view
{
  return m_goomStateHandler->GetCurrentState().GetName();
}

} // namespace GOOM::CONTROL
