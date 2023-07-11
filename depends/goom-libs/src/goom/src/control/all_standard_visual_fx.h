#pragma once

#include "goom_graphic.h"
#include "goom_state_handler.h"
#include "goom_states.h"
#include "point2d.h"
#include "utils/enum_utils.h"
#include "utils/stopwatch.h"
#include "visual_fx/shader_fx.h"
#include "visual_fx_color_maps.h"

#include <functional>
#include <memory>
#include <unordered_set>
#include <vector>

namespace GOOM
{
class AudioSamples;

namespace UTILS
{
class Parallel;

namespace GRAPHICS
{
class SmallImageBitmaps;
}
}

namespace VISUAL_FX
{
class IVisualFx;
class LinesFx;
class ShaderFx;
}

namespace CONTROL
{

class AllStandardVisualFx
{
public:
  AllStandardVisualFx(UTILS::Parallel& parallel,
                      const VISUAL_FX::FxHelper& fxHelper,
                      const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps,
                      const std::string& resourcesDirectory) noexcept;

  [[nodiscard]] auto GetCurrentGoomDrawables() const -> const IGoomStateHandler::DrawablesState&;
  auto SetCurrentGoomDrawables(const IGoomStateHandler::DrawablesState& goomDrawablesSet) -> void;

  auto ChangeColorMaps() -> void;
  [[nodiscard]] static auto GetActiveColorMapsNames() -> std::unordered_set<std::string>;

  using ResetCurrentDrawBuffSettingsFunc = std::function<void(GoomDrawables fx)>;
  auto SetResetDrawBuffSettingsFunc(const ResetCurrentDrawBuffSettingsFunc& func) -> void;

  auto Start() -> void;
  auto Finish() -> void;

  auto RefreshAllFx() -> void;
  auto SuspendFx() -> void;
  auto ResumeFx() -> void;
  auto ChangeAllFxPixelBlenders(
      const VISUAL_FX::IVisualFx::PixelBlenderParams& pixelBlenderParams) noexcept -> void;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) -> void;

  auto ApplyCurrentStateToMultipleBuffers(const AudioSamples& soundData) -> void;
  auto ApplyEndEffectIfNearEnd(const UTILS::Stopwatch::TimeValues& timeValues) -> void;

  [[nodiscard]] auto GetLinesFx() noexcept -> VISUAL_FX::LinesFx&;

  auto ChangeShaderVariables() -> void;
  [[nodiscard]] auto GetLastShaderVariables() const -> const GoomShaderVariables&;

private:
  std::unique_ptr<VISUAL_FX::ShaderFx> m_shaderFx;
  UTILS::EnumMap<GoomDrawables, std::unique_ptr<VISUAL_FX::IVisualFx>> m_drawablesMap;
  [[nodiscard]] static auto GetDrawablesMap(UTILS::Parallel& parallel,
                                            const VISUAL_FX::FxHelper& fxHelper,
                                            const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps,
                                            const std::string& resourcesDirectory)
      -> UTILS::EnumMap<GoomDrawables, std::unique_ptr<VISUAL_FX::IVisualFx>>;
  VisualFxColorMaps m_visualFxColorMaps;
  auto ChangeDotsColorMaps() noexcept -> void;
  auto ChangeLinesColorMaps() noexcept -> void;
  auto ChangeShapesColorMaps() noexcept -> void;
  auto ChangeStarsColorMaps() noexcept -> void;
  auto ChangeTentaclesColorMaps() noexcept -> void;

  IGoomStateHandler::DrawablesState m_currentGoomDrawables{};
  ResetCurrentDrawBuffSettingsFunc m_resetCurrentDrawBuffSettingsFunc{};
  auto ResetDrawBuffSettings(GoomDrawables fx) -> void;

  auto ApplyStandardFxToMultipleBuffers(const AudioSamples& soundData) -> void;
  auto ApplyShaderToBothBuffersIfRequired() -> void;
};

inline auto AllStandardVisualFx::GetCurrentGoomDrawables() const
    -> const IGoomStateHandler::DrawablesState&
{
  return m_currentGoomDrawables;
}

inline void AllStandardVisualFx::SetCurrentGoomDrawables(
    const IGoomStateHandler::DrawablesState& goomDrawablesSet)
{
  m_currentGoomDrawables = goomDrawablesSet;
}

inline void AllStandardVisualFx::SetResetDrawBuffSettingsFunc(
    const ResetCurrentDrawBuffSettingsFunc& func)
{
  m_resetCurrentDrawBuffSettingsFunc = func;
}

inline void AllStandardVisualFx::ResetDrawBuffSettings(const GoomDrawables fx)
{
  m_resetCurrentDrawBuffSettingsFunc(fx);
}

inline auto AllStandardVisualFx::ApplyCurrentStateToMultipleBuffers(const AudioSamples& soundData)
    -> void
{
  ApplyStandardFxToMultipleBuffers(soundData);
  ApplyShaderToBothBuffersIfRequired();
}

} // namespace CONTROL
} // namespace GOOM
