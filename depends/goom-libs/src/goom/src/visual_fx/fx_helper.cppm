module;

#include "goom/goom_logger.h"

export module Goom.VisualFx.FxHelper;

import Goom.Control.GoomSoundEvents;
import Goom.Draw.GoomDrawBase;
import Goom.Utils.Graphics.Blend2dToGoom;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.GoomTime;
import Goom.Lib.GoomTypes;
import Goom.PluginInfo;

export namespace GOOM::VISUAL_FX
{

class FxHelper
{
public:
  FxHelper(DRAW::IGoomDraw& draw,
           const PluginInfo& goomInfo,
           const UTILS::MATH::GoomRand& goomRand,
           GoomLogger& goomLogger,
           UTILS::GRAPHICS::Blend2dContexts& blend2dContexts) noexcept;

  [[nodiscard]] auto GetDraw() const noexcept -> const DRAW::IGoomDraw&;
  [[nodiscard]] auto GetDraw() noexcept -> DRAW::IGoomDraw&;
  [[nodiscard]] auto GetGoomInfo() const noexcept -> const PluginInfo&;
  [[nodiscard]] auto GetGoomRand() const noexcept -> const UTILS::MATH::GoomRand&;
  [[nodiscard]] auto GetGoomLogger() const noexcept -> const GoomLogger&;
  [[nodiscard]] auto GetGoomLogger() noexcept -> GoomLogger&;
  [[nodiscard]] auto GetBlend2dContexts() const noexcept -> const UTILS::GRAPHICS::Blend2dContexts&;
  [[nodiscard]] auto GetBlend2dContexts() noexcept -> UTILS::GRAPHICS::Blend2dContexts&;

  [[nodiscard]] auto GetDimensions() const noexcept -> const Dimensions&;
  [[nodiscard]] auto GetSoundEvents() const -> const CONTROL::GoomSoundEvents&;
  [[nodiscard]] auto GetGoomTime() const noexcept -> const UTILS::GoomTime&;

private:
  DRAW::IGoomDraw* m_draw;
  const PluginInfo* m_goomInfo;
  const UTILS::MATH::GoomRand* m_goomRand;
  GoomLogger* m_goomLogger;
  UTILS::GRAPHICS::Blend2dContexts* m_blend2dContexts;
};

inline FxHelper::FxHelper(DRAW::IGoomDraw& draw,
                          const PluginInfo& goomInfo,
                          const UTILS::MATH::GoomRand& goomRand,
                          GoomLogger& goomLogger,
                          UTILS::GRAPHICS::Blend2dContexts& blend2dContexts) noexcept
  : m_draw{&draw},
    m_goomInfo{&goomInfo},
    m_goomRand{&goomRand},
    m_goomLogger{&goomLogger},
    m_blend2dContexts{&blend2dContexts}
{
}

inline auto FxHelper::GetDraw() const noexcept -> const DRAW::IGoomDraw&
{
  return *m_draw;
}

inline auto FxHelper::GetDraw() noexcept -> DRAW::IGoomDraw&
{
  return *m_draw;
}

inline auto FxHelper::GetGoomInfo() const noexcept -> const PluginInfo&
{
  return *m_goomInfo;
}

inline auto FxHelper::GetGoomRand() const noexcept -> const UTILS::MATH::GoomRand&
{
  return *m_goomRand;
}

inline auto FxHelper::GetGoomLogger() const noexcept -> const GoomLogger&
{
  return *m_goomLogger;
}

inline auto FxHelper::GetGoomLogger() noexcept -> GoomLogger&
{
  return *m_goomLogger;
}

inline auto FxHelper::GetBlend2dContexts() const noexcept -> const UTILS::GRAPHICS::Blend2dContexts&
{
  return *m_blend2dContexts;
}

inline auto FxHelper::GetBlend2dContexts() noexcept -> UTILS::GRAPHICS::Blend2dContexts&
{
  return *m_blend2dContexts;
}

inline auto FxHelper::GetDimensions() const noexcept -> const Dimensions&
{
  return m_goomInfo->GetDimensions();
}

inline auto FxHelper::GetSoundEvents() const -> const CONTROL::GoomSoundEvents&
{
  return m_goomInfo->GetSoundEvents();
}

inline auto FxHelper::GetGoomTime() const noexcept -> const UTILS::GoomTime&
{
  return m_goomInfo->GetTime();
}

} // namespace GOOM::VISUAL_FX
