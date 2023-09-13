//#undef NO_LOGGING

#include "color/color_maps.h"
#include "goom/goom_graphic.h"
#include "goom/goom_time.h"
#include "goom/goom_types.h"
#include "goom/point2d.h"
#include "goom/sound_info.h"
#include "goom_visual_fx.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#define REQUIRE_ASSERTS_FOR_ALL_BUILDS // Check for non-null pointers.

#include "color/random_color_maps.h"
#include "draw/goom_draw.h"
#include "fx_helper.h"
#include "goom/goom_config.h"
#include "goom/spimpl.h"
#include "goom_plugin_info.h"
#include "tentacles/circles_tentacle_layout.h"
#include "tentacles/tentacle_driver.h"
#include "tentacles_fx.h"
#include "utils/enum_utils.h"
#include "utils/math/goom_rand_base.h"
#include "utils/timer.h"
#include "visual_fx/fx_utils/random_pixel_blender.h"

#include <array>
#include <vector>

namespace GOOM::VISUAL_FX
{

using COLOR::WeightedRandomColorMaps;
using DRAW::IGoomDraw;
using FX_UTILS::RandomPixelBlender;
using TENTACLES::CirclesTentacleLayout;
using TENTACLES::TentacleDriver;
using UTILS::NUM;
using UTILS::Timer;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::Weights;

class TentaclesFx::TentaclesImpl
{
public:
  explicit TentaclesImpl(const FxHelper& fxHelper);

  auto Start() -> void;
  auto Resume() -> void;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;

  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;
  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>;

  auto ApplyToImageBuffers() -> void;

private:
  const FxHelper* m_fxHelper;

  PixelChannelType m_defaultAlpha = MAX_ALPHA;

  enum class Drivers : UnderlyingEnumType
  {
    NUM0,
    NUM1,
    NUM2,
    NUM3,
    _num // unused, and marks the enum end
  };
  static constexpr size_t NUM_TENTACLE_DRIVERS = NUM<Drivers>;
  Weights<Drivers> m_driverWeights;
  std::vector<TentacleDriver> m_tentacleDrivers;
  [[nodiscard]] static auto GetTentacleDrivers(
      IGoomDraw& draw,
      const IGoomRand& goomRand,
      const GoomTime& goomTime,
      const std::array<CirclesTentacleLayout, NUM_TENTACLE_DRIVERS>& tentacleLayouts,
      PixelChannelType defaultAlpha) -> std::vector<TentacleDriver>;
  TentacleDriver* m_currentTentacleDriver{GetNextDriver()};
  [[nodiscard]] auto GetNextDriver() -> TentacleDriver*;

  WeightedRandomColorMaps m_weightedDominantMainColorMaps{};
  WeightedRandomColorMaps m_weightedDominantLowColorMaps{};
  COLOR::ColorMapSharedPtr m_dominantMainColorMapPtr = nullptr;
  COLOR::ColorMapSharedPtr m_dominantLowColorMapPtr  = nullptr;
  auto ChangeDominantColor() -> void;

  RandomPixelBlender m_pixelBlender;
  auto UpdatePixelBlender() noexcept -> void;

  static constexpr uint32_t MAX_TIME_FOR_DOMINANT_COLOR = 100;
  Timer m_timeWithThisDominantColor{m_fxHelper->goomInfo->GetTime(), MAX_TIME_FOR_DOMINANT_COLOR};

  auto RefreshTentacles() -> void;
  auto DoTentaclesUpdate() -> void;
  auto UpdateTentacleWaveFrequency() -> void;
};

TentaclesFx::TentaclesFx(const FxHelper& fxHelper) noexcept
  : m_pimpl{spimpl::make_unique_impl<TentaclesImpl>(fxHelper)}
{
}

auto TentaclesFx::GetFxName() const noexcept -> std::string
{
  return "Tentacles FX";
}

auto TentaclesFx::Start() noexcept -> void
{
  m_pimpl->Start();
}

auto TentaclesFx::Finish() noexcept -> void
{
  // nothing to do
}

auto TentaclesFx::Resume() noexcept -> void
{
  m_pimpl->Resume();
}

auto TentaclesFx::Suspend() noexcept -> void
{
  // nothing to do
}

auto TentaclesFx::ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pimpl->ChangePixelBlender(pixelBlenderParams);
}

auto TentaclesFx::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  m_pimpl->SetZoomMidpoint(zoomMidpoint);
}

auto TentaclesFx::SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  m_pimpl->SetWeightedColorMaps(weightedColorMaps);
}

auto TentaclesFx::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  return m_pimpl->GetCurrentColorMapsNames();
}

auto TentaclesFx::ApplyToImageBuffers() noexcept -> void
{
  m_pimpl->ApplyToImageBuffers();
}

static constexpr auto LAYOUT0_START_RADIUS = 380.0F;
static constexpr auto LAYOUT1_START_RADIUS = 380.0F;
static constexpr auto LAYOUT2_START_RADIUS = 400.0F;
static constexpr auto LAYOUT3_START_RADIUS = 410.0F;

static constexpr auto LAYOUT0_END_RADIUS = 120.0F;
static constexpr auto LAYOUT1_END_RADIUS = 120.0F;
static constexpr auto LAYOUT2_END_RADIUS = 120.0F;
static constexpr auto LAYOUT3_END_RADIUS = 120.0F;

static constexpr auto LAYOUT0_NUM_TENTACLES = 40U;
static constexpr auto LAYOUT1_NUM_TENTACLES = 50U;
static constexpr auto LAYOUT2_NUM_TENTACLES = 70U;
static constexpr auto LAYOUT3_NUM_TENTACLES = 150U;

static constexpr auto DRIVERS_NUM0_WEIGHT = 50.0F;
static constexpr auto DRIVERS_NUM1_WEIGHT = 30.0F;
static constexpr auto DRIVERS_NUM2_WEIGHT = 10.0F;
static constexpr auto DRIVERS_NUM3_WEIGHT = 05.0F;

TentaclesFx::TentaclesImpl::TentaclesImpl(const FxHelper& fxHelper)
  : m_fxHelper{&fxHelper},
    m_driverWeights{
      *m_fxHelper->goomRand,
      {
          {Drivers::NUM0, DRIVERS_NUM0_WEIGHT},
          {Drivers::NUM1, DRIVERS_NUM1_WEIGHT},
          {Drivers::NUM2, DRIVERS_NUM2_WEIGHT},
          {Drivers::NUM3, DRIVERS_NUM3_WEIGHT},
      }},
    // clang-format off
    m_tentacleDrivers{GetTentacleDrivers(
        *fxHelper.draw,
        *m_fxHelper->goomRand,
        m_fxHelper->goomInfo->GetTime(),
        {{
           CirclesTentacleLayout{{LAYOUT0_START_RADIUS, LAYOUT0_END_RADIUS, LAYOUT0_NUM_TENTACLES}},
           CirclesTentacleLayout{{LAYOUT1_START_RADIUS, LAYOUT1_END_RADIUS, LAYOUT1_NUM_TENTACLES}},
           CirclesTentacleLayout{{LAYOUT2_START_RADIUS, LAYOUT2_END_RADIUS, LAYOUT2_NUM_TENTACLES}},
           CirclesTentacleLayout{{LAYOUT3_START_RADIUS, LAYOUT3_END_RADIUS, LAYOUT3_NUM_TENTACLES}},
        }},
        m_defaultAlpha
    )},
    // clang-format on
    m_pixelBlender{*fxHelper.goomRand}
{
  Expects(NUM_TENTACLE_DRIVERS == m_driverWeights.GetNumElements());
  Ensures(m_currentTentacleDriver != nullptr);
}

inline auto TentaclesFx::TentaclesImpl::Start() -> void
{
  m_timeWithThisDominantColor.SetToFinished();

  RefreshTentacles();
}

inline auto TentaclesFx::TentaclesImpl::Resume() -> void
{
  if (static constexpr auto PROB_NEW_DRIVER = 0.5F;
      m_fxHelper->goomRand->ProbabilityOf(PROB_NEW_DRIVER))
  {
    m_currentTentacleDriver = GetNextDriver();
  }

  m_timeWithThisDominantColor.SetToFinished();

  RefreshTentacles();
}

auto TentaclesFx::TentaclesImpl::GetTentacleDrivers(
    IGoomDraw& draw,
    const IGoomRand& goomRand,
    const GoomTime& goomTime,
    const std::array<CirclesTentacleLayout, NUM_TENTACLE_DRIVERS>& tentacleLayouts,
    const PixelChannelType defaultAlpha) -> std::vector<TentacleDriver>
{
  auto tentacleDrivers = std::vector<TentacleDriver>{};
  for (auto i = 0U; i < NUM_TENTACLE_DRIVERS; ++i)
  {
    tentacleDrivers.emplace_back(draw, goomRand, goomTime, tentacleLayouts.at(i), defaultAlpha);
  }

  for (auto i = 0U; i < NUM_TENTACLE_DRIVERS; ++i)
  {
    tentacleDrivers[i].StartIterating();
  }

  return tentacleDrivers;
}

inline auto TentaclesFx::TentaclesImpl::GetNextDriver() -> TentacleDriver*
{
  const auto driverIndex = static_cast<size_t>(m_driverWeights.GetRandomWeighted());
  return &m_tentacleDrivers[driverIndex];
}

inline auto TentaclesFx::TentaclesImpl::RefreshTentacles() -> void
{
  Expects(m_currentTentacleDriver != nullptr);

  ChangeDominantColor();

  m_currentTentacleDriver->ChangeTentacleColorMaps();
}

auto TentaclesFx::TentaclesImpl::GetCurrentColorMapsNames() const noexcept
    -> std::vector<std::string>
{
  return {m_weightedDominantMainColorMaps.GetColorMapsName(),
          m_weightedDominantLowColorMaps.GetColorMapsName()};
}

auto TentaclesFx::TentaclesImpl::SetWeightedColorMaps(
    const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  const auto newWeightedColorMaps =
      GetWeightedColorMapsWithNewAlpha(weightedColorMaps, m_defaultAlpha);

  if (weightedColorMaps.id == NORMAL_COLOR_TYPE)
  {
    std::for_each(begin(m_tentacleDrivers),
                  end(m_tentacleDrivers),
                  [&newWeightedColorMaps](auto& driver)
                  { driver.SetWeightedColorMaps(newWeightedColorMaps); });
  }
  else if (newWeightedColorMaps.id == DOMINANT_COLOR_TYPE)
  {
    m_weightedDominantMainColorMaps = newWeightedColorMaps.mainColorMaps;
    m_weightedDominantLowColorMaps  = newWeightedColorMaps.lowColorMaps;

    m_dominantMainColorMapPtr = m_weightedDominantMainColorMaps.GetRandomColorMapSharedPtr(
        WeightedRandomColorMaps::GetAllColorMapsTypes());
    m_dominantLowColorMapPtr = m_weightedDominantLowColorMaps.GetRandomColorMapSharedPtr(
        WeightedRandomColorMaps::GetAllColorMapsTypes());
  }
  else
  {
    FailFast();
  }
}

inline auto TentaclesFx::TentaclesImpl::ChangePixelBlender(
    const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pixelBlender.SetPixelBlendType(pixelBlenderParams);
}

inline auto TentaclesFx::TentaclesImpl::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept
    -> void
{
  std::for_each(begin(m_tentacleDrivers),
                end(m_tentacleDrivers),
                [&zoomMidpoint](auto& driver) { driver.SetTentaclesEndCentrePos(zoomMidpoint); });
}

inline auto TentaclesFx::TentaclesImpl::ChangeDominantColor() -> void
{
  Expects(m_currentTentacleDriver != nullptr);

  if (!m_timeWithThisDominantColor.Finished())
  {
    return;
  }

  m_timeWithThisDominantColor.ResetToZero();

  m_currentTentacleDriver->SetDominantColorMaps(m_dominantMainColorMapPtr,
                                                m_dominantLowColorMapPtr);
}

inline auto TentaclesFx::TentaclesImpl::ApplyToImageBuffers() -> void
{
  UpdatePixelBlender();
  DoTentaclesUpdate();
}

inline auto TentaclesFx::TentaclesImpl::UpdatePixelBlender() noexcept -> void
{
  m_fxHelper->draw->SetPixelBlendFunc(m_pixelBlender.GetCurrentPixelBlendFunc());
  m_pixelBlender.Update();
}

inline auto TentaclesFx::TentaclesImpl::DoTentaclesUpdate() -> void
{
  Expects(m_currentTentacleDriver != nullptr);

  if (0 == m_fxHelper->goomInfo->GetSoundEvents().GetTimeSinceLastGoom())
  {
    ChangeDominantColor();
  }

  UpdateTentacleWaveFrequency();

  m_currentTentacleDriver->Update();
}

inline auto TentaclesFx::TentaclesImpl::UpdateTentacleWaveFrequency() -> void
{
  Expects(m_currentTentacleDriver != nullptr);

  // Higher sound acceleration increases tentacle wave frequency.
  const auto tentacleWaveFreqMultiplier =
      m_fxHelper->goomInfo->GetSoundEvents().GetSoundInfo().GetAcceleration() <
              SoundInfo::ACCELERATION_MIDPOINT
          ? 0.95F
          : (1.0F /
             (1.1F - m_fxHelper->goomInfo->GetSoundEvents().GetSoundInfo().GetAcceleration()));
  m_currentTentacleDriver->MultiplyIterZeroYValWaveFreq(tentacleWaveFreqMultiplier);
}

} // namespace GOOM::VISUAL_FX
