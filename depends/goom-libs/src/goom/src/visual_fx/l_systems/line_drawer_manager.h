#pragma once

#include "draw/goom_draw.h"
#include "draw/shape_drawers/line_drawer_with_effects.h"
#include "utils/math/goom_rand_base.h"

namespace GOOM::VISUAL_FX::L_SYSTEM
{

class LineDrawerManager
{
  using ILineDrawerWithEffects = DRAW::SHAPE_DRAWERS::ILineDrawerWithEffects;

public:
  LineDrawerManager(DRAW::IGoomDraw& draw, const UTILS::MATH::IGoomRand& goomRand) noexcept;

  [[nodiscard]] auto GetLineDrawer() const noexcept -> const ILineDrawerWithEffects&;
  [[nodiscard]] auto GetLineDrawer() noexcept -> ILineDrawerWithEffects&;

  auto SwitchLineDrawers() noexcept -> void;

  enum class SwitchLineDrawerType
  {
    CONST,
    MOVING,
    NONE,
    _num // must be last - gives number of enums
  };
  auto SwitchLineDrawers(SwitchLineDrawerType forceType) noexcept -> void;

  auto Update() noexcept -> void;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;

  using LineDrawerWithMovingNoiseEffect = DRAW::SHAPE_DRAWERS::LineDrawerWithMovingNoiseEffect;

  static constexpr MinMaxValues<uint8_t> MIN_MAX_MOVING_NOISE_RADIUS{3U, 6U};
  static constexpr auto NUM_MOVING_NOISE_RADIUS_STEPS = 10000U;
  static constexpr MinMaxValues<uint8_t> MIN_MAX_NUM_MOVING_NOISE_PIXELS{3U, 5U};
  static constexpr auto NUM_NUM_MOVING_NOISE_PIXEL_STEPS = 10000U;
  LineDrawerWithMovingNoiseEffect m_lineDrawerWithMovingNoiseEffect;

  UTILS::MATH::Weights<SwitchLineDrawerType> m_switchLineDrawerWeights;

  ILineDrawerWithEffects* m_lineDrawer = &m_lineDrawerWithMovingNoiseEffect;
};

inline auto LineDrawerManager::GetLineDrawer() const noexcept -> const ILineDrawerWithEffects&
{
  Expects(m_lineDrawer != nullptr);
  return *m_lineDrawer;
}

inline auto LineDrawerManager::GetLineDrawer() noexcept -> ILineDrawerWithEffects&
{
  Expects(m_lineDrawer != nullptr);
  return *m_lineDrawer;
}

} // namespace GOOM::VISUAL_FX::L_SYSTEM
