#pragma once

#include "goom/frame_data.h"
#include "goom/goom_config.h"
#include "goom/goom_graphic.h"
#include "goom/goom_types.h"
#include "goom/spimpl.h"
#include "goom_state_handler.h"
#include "goom_states.h"
#include "utils/math/goom_rand_base.h"
#include "utils/stopwatch.h"
#include "visual_fx/goom_visual_fx.h"
#include "visual_fx_color_maps.h"

#include <functional>
#include <string>
#include <string_view>
#include <unordered_set>

namespace GOOM
{
class AudioSamples;
class GoomLogger;

namespace UTILS
{
class Parallel;

namespace GRAPHICS
{
class SmallImageBitmaps;
}
namespace MATH
{
class IGoomRand;
}
} // namespace UTILS

namespace VISUAL_FX
{
class FxHelper;
}

namespace CONTROL
{

class AllStandardVisualFx;

class GoomAllVisualFx
{
public:
  GoomAllVisualFx() noexcept = delete;
  GoomAllVisualFx(UTILS::Parallel& parallel,
                  VISUAL_FX::FxHelper& fxHelper,
                  const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps,
                  const std::string& resourcesDirectory,
                  IGoomStateHandler& goomStateHandler) noexcept;

  auto Start() noexcept -> void;
  auto Finish() noexcept -> void;

  auto SetAllowMultiThreadedStates(bool val) noexcept -> void;

  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;

  auto SetNextState() noexcept -> void;
  [[nodiscard]] auto GetCurrentState() const noexcept -> GoomStates;
  [[nodiscard]] auto GetCurrentStateName() const noexcept -> std::string_view;

  using ResetDrawBuffSettingsFunc = std::function<void(const FXBuffSettings& settings)>;
  auto SetResetDrawBuffSettingsFunc(const ResetDrawBuffSettingsFunc& func) noexcept -> void;

  auto ChangeAllFxColorMaps() noexcept -> void;
  auto ChangeAllFxPixelBlenders() noexcept -> void;
  auto RefreshAllFx() noexcept -> void;

  [[nodiscard]] auto GetFrameMiscData() const noexcept -> const MiscData&;
  auto SetFrameMiscData(MiscData& miscData) noexcept -> void;
  auto ApplyCurrentStateToImageBuffers(const AudioSamples& soundData) noexcept -> void;
  auto ApplyEndEffectIfNearEnd(const UTILS::Stopwatch::TimeValues& timeValues) noexcept -> void;

  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::unordered_set<std::string>;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  [[maybe_unused]] GoomLogger* m_goomLogger;
  spimpl::unique_impl_ptr<AllStandardVisualFx> m_allStandardVisualFx;

  IGoomStateHandler* m_goomStateHandler;
  bool m_allowMultiThreadedStates = true;
  auto ChangeState() noexcept -> void;
  IGoomStateHandler::DrawablesState m_currentGoomDrawables{};

  ResetDrawBuffSettingsFunc m_resetDrawBuffSettings{};
  auto ResetCurrentDrawBuffSettings(GoomDrawables fx) noexcept -> void;
  [[nodiscard]] auto GetCurrentBuffSettings(GoomDrawables fx) const noexcept -> FXBuffSettings;

  VisualFxColorMaps m_visualFxColorMaps{*m_goomRand};

  [[nodiscard]] auto GetNextPixelBlenderParams() const noexcept
      -> VISUAL_FX::IVisualFx::PixelBlenderParams;
  enum class GlobalBlendType : UnderlyingEnumType
  {
    NONRANDOM,
    ASYNC_RANDOM,
    SYNC_RANDOM,
  };
  static constexpr auto NONRANDOM_WEIGHT    = 50.0F;
  static constexpr auto SYNC_RANDOM_WEIGHT  = 50.0F;
  static constexpr auto ASYNC_RANDOM_WEIGHT = 50.0F;
  UTILS::MATH::Weights<GlobalBlendType> m_globalBlendTypeWeight{
      *m_goomRand,
      {{GlobalBlendType::NONRANDOM, NONRANDOM_WEIGHT},
                  {GlobalBlendType::SYNC_RANDOM, SYNC_RANDOM_WEIGHT},
                  {GlobalBlendType::ASYNC_RANDOM, ASYNC_RANDOM_WEIGHT}}
  };
};

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

inline auto GoomAllVisualFx::GetCurrentState() const noexcept -> GoomStates
{
  return m_goomStateHandler->GetCurrentState();
}

inline auto GoomAllVisualFx::GetCurrentStateName() const noexcept -> std::string_view
{
  return GoomStateInfo::GetStateInfo(m_goomStateHandler->GetCurrentState()).name;
}

} // namespace CONTROL
} // namespace GOOM
