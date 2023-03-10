#pragma once

#include "goom_visual_fx.h"
#include "spimpl.h"
#include "utils/stopwatch.h"

namespace GOOM
{
struct GoomShaderVariables;

namespace VISUAL_FX
{
struct FxHelper;

class ShaderFx : public IVisualFx
{
public:
  explicit ShaderFx(const FxHelper& fxHelper) noexcept;

  [[nodiscard]] auto GetFxName() const noexcept -> std::string override;

  auto Start() noexcept -> void override;
  auto Finish() noexcept -> void override;

  auto ChangeEffects() noexcept -> void;

  auto ApplyMultiple() noexcept -> void override;
  auto ApplyEndEffect(const UTILS::Stopwatch::TimeValues& timeValues) noexcept -> void;

  [[nodiscard]] auto GetLastShaderVariables() const -> const GoomShaderVariables&;

  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string> override
  {
    return {};
  }

private:
  class ShaderFxImpl;
  spimpl::unique_impl_ptr<ShaderFxImpl> m_pimpl;
};

} // namespace VISUAL_FX
} // namespace GOOM
