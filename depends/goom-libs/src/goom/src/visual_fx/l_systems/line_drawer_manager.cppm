module;

#include <cstdint>

module Goom.VisualFx.LSystemFx:LineDrawerManager;

import Goom.Draw.GoomDrawBase;
import Goom.Draw.ShaperDrawers.LineDrawerMovingNoise;
import Goom.Draw.ShaperDrawers.LineDrawerWithEffects;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomTypes;

namespace GOOM::VISUAL_FX::L_SYSTEM
{

class LineDrawerManager
{
  using ILineDrawerWithEffects = DRAW::SHAPE_DRAWERS::ILineDrawerWithEffects;

public:
  LineDrawerManager(DRAW::IGoomDraw& draw, const UTILS::MATH::GoomRand& goomRand) noexcept;

  [[nodiscard]] auto GetLineDrawer() const noexcept -> const ILineDrawerWithEffects&;
  [[nodiscard]] auto GetLineDrawer() noexcept -> ILineDrawerWithEffects&;

  enum class SwitchLineDrawerType : UnderlyingEnumType
  {
    CONST,
    MOVING,
    NONE,
  };
  auto SwitchLineDrawers() noexcept -> void;
  auto SwitchLineDrawers(SwitchLineDrawerType forceType) noexcept -> void;

  auto Update() noexcept -> void;

private:
  const UTILS::MATH::GoomRand* m_goomRand;

  using LineDrawerWithMovingNoiseEffect = DRAW::SHAPE_DRAWERS::LineDrawerWithMovingNoiseEffect;

  static constexpr MinMaxValues<uint8_t> MIN_MAX_MOVING_NOISE_RADIUS{.minValue = 3U,
                                                                     .maxValue = 6U};
  static constexpr auto NUM_MOVING_NOISE_RADIUS_STEPS = 10000U;
  static constexpr MinMaxValues<uint8_t> MIN_MAX_NUM_MOVING_NOISE_PIXELS{.minValue = 3U,
                                                                         .maxValue = 5U};
  static constexpr auto NUM_NUM_MOVING_NOISE_PIXEL_STEPS = 10000U;
  LineDrawerWithMovingNoiseEffect m_lineDrawerWithMovingNoiseEffect;

  UTILS::MATH::Weights<SwitchLineDrawerType> m_switchLineDrawerWeights;

  ILineDrawerWithEffects* m_lineDrawer = &m_lineDrawerWithMovingNoiseEffect;
};

} // namespace GOOM::VISUAL_FX::L_SYSTEM

namespace GOOM::VISUAL_FX::L_SYSTEM
{

inline auto LineDrawerManager::GetLineDrawer() const noexcept -> const ILineDrawerWithEffects&
{
  Expects(m_lineDrawer != nullptr);
  // NOLINTNEXTLINE(clang-analyzer-core.uninitialized.UndefReturn)
  return *m_lineDrawer;
}

inline auto LineDrawerManager::GetLineDrawer() noexcept -> ILineDrawerWithEffects&
{
  Expects(m_lineDrawer != nullptr);
  // NOLINTNEXTLINE(clang-analyzer-core.uninitialized.UndefReturn)
  return *m_lineDrawer;
}

using DRAW::IGoomDraw;
using DRAW::SHAPE_DRAWERS::LineDrawerMovingNoise;
using UTILS::MATH::GoomRand;

static constexpr auto LINE_BRIGHTNESS_FACTOR = 2.0F;

static constexpr auto SWITCH_LINE_DRAWER_TYPE_CONST_WEIGHT  = 0.4F;
static constexpr auto SWITCH_LINE_DRAWER_TYPE_MOVING_WEIGHT = 0.5F;
static constexpr auto SWITCH_LINE_DRAWER_TYPE_NONE_WEIGHT   = 0.1F;

// clang-format off
LineDrawerManager::LineDrawerManager(IGoomDraw& draw, const GoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_lineDrawerWithMovingNoiseEffect{
        LineDrawerMovingNoise{draw,
                              *m_goomRand,
                              MIN_MAX_MOVING_NOISE_RADIUS,
                              NUM_MOVING_NOISE_RADIUS_STEPS,
                              MIN_MAX_NUM_MOVING_NOISE_PIXELS,
                              NUM_NUM_MOVING_NOISE_PIXEL_STEPS}
    },
    m_switchLineDrawerWeights{
        *m_goomRand,
        {
            { .key=SwitchLineDrawerType::CONST, .weight=SWITCH_LINE_DRAWER_TYPE_CONST_WEIGHT },
            { .key=SwitchLineDrawerType::MOVING, .weight=SWITCH_LINE_DRAWER_TYPE_MOVING_WEIGHT },
            { .key=SwitchLineDrawerType::NONE, .weight=SWITCH_LINE_DRAWER_TYPE_NONE_WEIGHT },
        }
    }
{
  m_lineDrawerWithMovingNoiseEffect.GetLineDrawer().SetUseMainPointWithoutNoise(false);
  m_lineDrawerWithMovingNoiseEffect.GetLineDrawer().SetBrightnessFactor(LINE_BRIGHTNESS_FACTOR);
}
// clang-format on

auto LineDrawerManager::SwitchLineDrawers() noexcept -> void
{
  SwitchLineDrawers(m_switchLineDrawerWeights.GetRandomWeighted());
}

auto LineDrawerManager::SwitchLineDrawers(const SwitchLineDrawerType forceType) noexcept -> void
{
  switch (forceType)
  {
    case SwitchLineDrawerType::CONST:
    {
      const auto currentNoiseRadius =
          m_lineDrawerWithMovingNoiseEffect.GetLineDrawer().GetCurrentNoiseRadius();
      const auto currentNumNoisePixelsPerPixel =
          m_lineDrawerWithMovingNoiseEffect.GetLineDrawer().GetCurrentNumNoisePixelsPerPixel();
      m_lineDrawerWithMovingNoiseEffect.GetLineDrawer().SetMinMaxNoiseValues(
          {.minValue = currentNoiseRadius, .maxValue = currentNoiseRadius},
          {.minValue = currentNumNoisePixelsPerPixel, .maxValue = currentNumNoisePixelsPerPixel});
      break;
    }
    case SwitchLineDrawerType::NONE:
    {
      m_lineDrawerWithMovingNoiseEffect.GetLineDrawer().SetMinMaxNoiseValues(
          {.minValue = 0U, .maxValue = 0U}, {.minValue = 1U, .maxValue = 1U});
      break;
    }
    case SwitchLineDrawerType::MOVING:
    {
      m_lineDrawerWithMovingNoiseEffect.GetLineDrawer().SetMinMaxNoiseValues(
          MIN_MAX_MOVING_NOISE_RADIUS, MIN_MAX_NUM_MOVING_NOISE_PIXELS);
      break;
    }
  }
}

auto LineDrawerManager::Update() noexcept -> void
{
  m_lineDrawerWithMovingNoiseEffect.GetLineDrawer().IncrementNoise();
}

} // namespace GOOM::VISUAL_FX::L_SYSTEM
