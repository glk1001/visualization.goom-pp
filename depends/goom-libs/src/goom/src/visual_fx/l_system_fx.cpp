//#undef NO_LOGGING

#define REQUIRE_ASSERTS_FOR_ALL_BUILDS // Check for non-null pointers.
#include "l_system_fx.h"

#include "draw/goom_draw.h"
#include "fx_helper.h"
#include "goom/goom_graphic.h"
#include "goom/goom_logger.h"
#include "goom/point2d.h"
#include "goom/spimpl.h"
#include "goom_plugin_info.h"
#include "goom_visual_fx.h"
#include "l_systems/l_system.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/goom_rand_base.h"
#include "utils/timer.h"
#include "visual_fx/fx_utils/random_pixel_blender.h"

#include <algorithm>
#include <cstdint>
#include <lsys/rand.h>
#include <memory>
#include <string>
#include <vector>

namespace GOOM::VISUAL_FX
{

using DRAW::IGoomDraw;
using FX_UTILS::RandomPixelBlender;
using L_SYSTEM::LSystem;
using UTILS::Timer;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::IGoomRand;

using ::LSYS::SetRandFunc;

class LSystemFx::LSystemFxImpl
{
public:
  LSystemFxImpl(const FxHelper& fxHelper, const std::string& resourcesDirectory);

  auto Start() -> void;
  auto Resume() -> void;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;

  [[nodiscard]] static auto GetCurrentColorMapsNames() noexcept -> std::vector<std::string>;

  auto ApplyToImageBuffers() -> void;

private:
  const FxHelper* m_fxHelper;
  Point2dInt m_screenCentre       = m_fxHelper->goomInfo->GetDimensions().GetCentrePoint();
  PixelChannelType m_defaultAlpha = DEFAULT_VISUAL_FX_ALPHA;

  static constexpr auto MAX_DOT_SIZE = 17U;
  static_assert(MAX_DOT_SIZE <= SmallImageBitmaps::MAX_IMAGE_SIZE, "Max dot size mismatch.");

  auto Update() noexcept -> void;
  auto ChangeColors() noexcept -> void;

  RandomPixelBlender m_pixelBlender;
  auto UpdatePixelBlender() noexcept -> void;

  [[nodiscard]] static auto GetLSystemFileList() noexcept -> std::vector<LSystem::LSystemFile>;
  static inline const std::vector<LSystem::LSystemFile> L_SYS_FILE_LIST = GetLSystemFileList();
  static inline const auto NUM_L_SYSTEMS = static_cast<uint32_t>(L_SYS_FILE_LIST.size());
  std::vector<std::unique_ptr<LSystem>> m_lSystems;
  [[nodiscard]] static auto GetLSystems(IGoomDraw& draw,
                                        const PluginInfo& goomInfo,
                                        const IGoomRand& goomRand,
                                        const std::string& resourcesDirectory,
                                        PixelChannelType defaultAlpha) noexcept
      -> std::vector<std::unique_ptr<LSystem>>;
  static constexpr auto DEFAULT_BOUNDS_EXPAND_FACTOR = 2.0F;
  [[nodiscard]] static auto GetLSystemDirectory(const std::string& resourcesDirectory) noexcept
      -> std::string;

  std::vector<LSystem*> m_activeLSystems{m_lSystems.at(0).get()};
  static constexpr auto MIN_TIME_TO_KEEP_ACTIVE_LSYS = 200U;
  static constexpr auto MAX_TIME_TO_KEEP_ACTIVE_LSYS = 1000U;
  Timer m_timeForTheseActiveLSys{
      m_fxHelper->goomInfo->GetTime(),
      m_fxHelper->goomRand->GetRandInRange(MIN_TIME_TO_KEEP_ACTIVE_LSYS,
                                           MAX_TIME_TO_KEEP_ACTIVE_LSYS + 1U)};

  static constexpr auto MIN_NUM_ROTATE_DEGREES_STEPS = 50U;
  static constexpr auto MAX_NUM_ROTATE_DEGREES_STEPS = 500U;
  static constexpr auto MIN_NUM_SPIN_DEGREES_STEPS   = 50U;
  static constexpr auto MAX_NUM_SPIN_DEGREES_STEPS   = 200U;
  static constexpr auto MAX_VERTICAL_MOVE            = +100.0F;
  static constexpr auto MIN_VERTICAL_MOVE            = -100.0F;
  auto InitNextActiveLSystems() noexcept -> void;
  auto DrawLSystem() noexcept -> void;
};

LSystemFx::LSystemFx(const FxHelper& fxHelper, const std::string& resourcesDirectory) noexcept
  : m_pimpl{spimpl::make_unique_impl<LSystemFxImpl>(fxHelper, resourcesDirectory)}
{
}

auto LSystemFx::GetFxName() const noexcept -> std::string
{
  return "L-System";
}

auto LSystemFx::Start() noexcept -> void
{
  m_pimpl->Start();
}

auto LSystemFx::Finish() noexcept -> void
{
  // nothing to do
}

auto LSystemFx::Resume() noexcept -> void
{
  m_pimpl->Resume();
}

auto LSystemFx::Suspend() noexcept -> void
{
  // nothing to do
}

auto LSystemFx::ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pimpl->ChangePixelBlender(pixelBlenderParams);
}

auto LSystemFx::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  m_pimpl->SetZoomMidpoint(zoomMidpoint);
}

auto LSystemFx::SetWeightedColorMaps(
    [[maybe_unused]] const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  // Nothing to do
}

auto LSystemFx::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  return m_pimpl->GetCurrentColorMapsNames();
}

auto LSystemFx::ApplyToImageBuffers() noexcept -> void
{
  m_pimpl->ApplyToImageBuffers();
}

LSystemFx::LSystemFxImpl::LSystemFxImpl(const FxHelper& fxHelper,
                                        const std::string& resourcesDirectory)
  : m_fxHelper{&fxHelper},
    m_pixelBlender{*fxHelper.goomRand},
    m_lSystems{GetLSystems(*fxHelper.draw,
                           *m_fxHelper->goomInfo,
                           *m_fxHelper->goomRand,
                           resourcesDirectory,
                           m_defaultAlpha)}
{
}

auto LSystemFx::LSystemFxImpl::GetLSystemFileList() noexcept -> std::vector<LSystem::LSystemFile>
{
  // TODO(glk) With C++20, can use constexpr here.
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
  auto lSysFileList = std::vector{
      LSystem::LSystemFile{        "bourke_bush",
                           {
                           /*.minNumLSysCopies =*/2U,
                           /*.maxNumLSysCopies =*/5U,
                           /*.minMaxGen =*/3U,
                           /*.maxMaxGen =*/3U,
                           /*.lineWidthFactor =*/1.0F,
                           /*.expandBounds =*/DEFAULT_BOUNDS_EXPAND_FACTOR,
                           /*.minNumRotateDegreeSteps =*/MIN_NUM_ROTATE_DEGREES_STEPS,
                           /*.maxNumRotateDegreeSteps =*/MAX_NUM_ROTATE_DEGREES_STEPS,
                           /*.minNumSpinDegreeSteps =*/MIN_NUM_SPIN_DEGREES_STEPS,
                           /*.maxNumSpinDegreeSteps =*/MAX_NUM_SPIN_DEGREES_STEPS,
                           /*.xScale =*/1.0F,
                           /*.yScale =*/0.5F,
                           /*.verticalMoveMin =*/MIN_VERTICAL_MOVE,
                           /*.verticalMoveMax =*/2.5F * MAX_VERTICAL_MOVE,
                           /*.minDefaultLineWidthFactor =*/1.0F,
                           /*.maxDefaultLineWidthFactor =*/2.01F,
                           /*.minDefaultDistanceFactor =*/1.0F,
                           /*.maxDefaultDistanceFactor =*/3.01F,
                           /*.minDefaultTurnAngleInDegreesFactor =*/0.1F,
                           /*.maxDefaultTurnAngleInDegreesFactor =*/1.0F,
                           /*.probabilityOfSimpleColors =*/0.75F,
                           /*.probabilityOfNoise =*/0.75F,
                           /*.namedArgs = */ {},
                           }},
      LSystem::LSystemFile{"bourke_pentaplexity",
                           {
                           /*.minNumLSysCopies =*/1U,
                           /*.maxNumLSysCopies =*/5U,
                           /*.minMaxGen =*/2U,
                           /*.maxMaxGen =*/3U,
                           /*.lineWidthFactor =*/1.0F,
                           /*.expandBounds =*/DEFAULT_BOUNDS_EXPAND_FACTOR,
                           /*.minNumRotateDegreeSteps =*/MIN_NUM_ROTATE_DEGREES_STEPS,
                           /*.maxNumRotateDegreeSteps =*/MAX_NUM_ROTATE_DEGREES_STEPS,
                           /*.minNumSpinDegreeSteps =*/MIN_NUM_SPIN_DEGREES_STEPS,
                           /*.maxNumSpinDegreeSteps =*/MAX_NUM_SPIN_DEGREES_STEPS,
                           /*.xScale =*/1.0F,
                           /*.yScale =*/1.0F,
                           /*.verticalMoveMin =*/MIN_VERTICAL_MOVE,
                           /*.verticalMoveMax =*/2.5F * MAX_VERTICAL_MOVE,
                           /*.minDefaultLineWidthFactor =*/1.0F,
                           /*.maxDefaultLineWidthFactor =*/2.01F,
                           /*.minDefaultDistanceFactor =*/0.5F,
                           /*.maxDefaultDistanceFactor =*/1.1F,
                           /*.minDefaultTurnAngleInDegreesFactor =*/1.0F,
                           /*.maxDefaultTurnAngleInDegreesFactor =*/1.00001F,
                           /*.probabilityOfSimpleColors =*/0.8F,
                           /*.probabilityOfNoise =*/0.5F,
                           /*.namedArgs = */ {},
                           }},
      LSystem::LSystemFile{       "honda_tree_b",
                           {
                           /*.minNumLSysCopies =*/2U,
                           /*.maxNumLSysCopies =*/5U,
                           /*.minMaxGen =*/5U,
                           /*.maxMaxGen =*/10U,
                           /*.lineWidthFactor =*/1.0F,
                           /*.expandBounds =*/DEFAULT_BOUNDS_EXPAND_FACTOR,
                           /*.minNumRotateDegreeSteps =*/MIN_NUM_ROTATE_DEGREES_STEPS,
                           /*.maxNumRotateDegreeSteps =*/MAX_NUM_ROTATE_DEGREES_STEPS,
                           /*.minNumSpinDegreeSteps =*/MIN_NUM_SPIN_DEGREES_STEPS,
                           /*.maxNumSpinDegreeSteps =*/MAX_NUM_SPIN_DEGREES_STEPS,
                           /*.xScale =*/2.0F,
                           /*.yScale =*/1.0F,
                           /*.verticalMoveMin =*/MIN_VERTICAL_MOVE,
                           /*.verticalMoveMax =*/2.5F * MAX_VERTICAL_MOVE,
                           /*.minDefaultLineWidthFactor =*/1.0F,
                           /*.maxDefaultLineWidthFactor =*/2.01F,
                           /*.minDefaultDistanceFactor =*/1.0F,
                           /*.maxDefaultDistanceFactor =*/2.01F,
                           /*.minDefaultTurnAngleInDegreesFactor =*/1.0F,
                           /*.maxDefaultTurnAngleInDegreesFactor =*/1.01F,
                           /*.probabilityOfSimpleColors =*/0.8F,
                           /*.probabilityOfNoise =*/0.75F,
                           /*.namedArgs = */
                           {
                           {"r2", 0.7F, 0.9F},
                           {"a0", 30.0F, 30.01F},
                           {"a2", 30.0F, 30.01F},
                           {"d", 137.5F, 137.51F},
                           },
                           }},
      LSystem::LSystemFile{     "ternary_tree_a",
                           {
                           /*.minNumLSysCopies =*/2U,
                           /*.maxNumLSysCopies =*/5U,
                           /*.minMaxGen =*/5U,
                           /*.maxMaxGen =*/6U,
                           /*.lineWidthFactor =*/1.1F,
                           /*.expandBounds =*/DEFAULT_BOUNDS_EXPAND_FACTOR,
                           /*.minNumRotateDegreeSteps =*/MIN_NUM_ROTATE_DEGREES_STEPS,
                           /*.maxNumRotateDegreeSteps =*/MAX_NUM_ROTATE_DEGREES_STEPS,
                           /*.minNumSpinDegreeSteps =*/MIN_NUM_SPIN_DEGREES_STEPS,
                           /*.maxNumSpinDegreeSteps =*/MAX_NUM_SPIN_DEGREES_STEPS,
                           /*.xScale =*/0.8F,
                           /*.yScale =*/1.0F,
                           /*.verticalMoveMin =*/2.0F * MIN_VERTICAL_MOVE,
                           /*.verticalMoveMax =*/MAX_VERTICAL_MOVE,
                           /*.minDefaultLineWidthFactor =*/1.0F,
                           /*.maxDefaultLineWidthFactor =*/1.01F,
                           /*.minDefaultDistanceFactor =*/1.0F,
                           /*.maxDefaultDistanceFactor =*/1.01F,
                           /*.minDefaultTurnAngleInDegreesFactor =*/1.0F,
                           /*.maxDefaultTurnAngleInDegreesFactor =*/1.01F,
                           /*.probabilityOfSimpleColors =*/0.8F,
                           /*.probabilityOfNoise =*/0.0F, // too slow with noise
                           /*.namedArgs = */
                           {
                           {"lr", 1.109F, 1.209F},
                           },
                           }},
  };
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

  return lSysFileList;
}

auto LSystemFx::LSystemFxImpl::GetLSystems(IGoomDraw& draw,
                                           const PluginInfo& goomInfo,
                                           const IGoomRand& goomRand,
                                           const std::string& resourcesDirectory,
                                           const PixelChannelType defaultAlpha) noexcept
    -> std::vector<std::unique_ptr<LSystem>>
{
  auto lSystem = std::vector<std::unique_ptr<LSystem>>{};

  for (const auto& lSysFile : L_SYS_FILE_LIST)
  {
    lSystem.emplace_back(std::make_unique<LSystem>(
        draw, goomInfo, goomRand, GetLSystemDirectory(resourcesDirectory), lSysFile, defaultAlpha));
  }

  return lSystem;
}

auto LSystemFx::LSystemFxImpl::InitNextActiveLSystems() noexcept -> void
{
  //LogInfo("Setting new active l-systems.");

  m_activeLSystems.clear();
  const auto lSystemIndex = m_fxHelper->goomRand->GetRandInRange(0U, NUM_L_SYSTEMS);
  //const auto lSystemIndex = 1U;
  // m_activeLSystems.push_back(m_lSystems.at(lSystemIndex).get());
  m_activeLSystems.push_back(m_lSystems.at(lSystemIndex).get());
  m_timeForTheseActiveLSys.SetTimeLimit(m_fxHelper->goomRand->GetRandInRange(
      MIN_TIME_TO_KEEP_ACTIVE_LSYS, MAX_TIME_TO_KEEP_ACTIVE_LSYS + 1U));
}

auto LSystemFx::LSystemFxImpl::Start() -> void
{
  SetRandFunc([this]() { return m_fxHelper->goomRand->GetRandInRange(0.0, 1.0); });

  std::for_each(begin(m_lSystems),
                end(m_lSystems),
                [this](auto& lSystem)
                {
                  lSystem->SetPathStart(m_screenCentre);
                  lSystem->Start();
                });

  InitNextActiveLSystems();
}

inline auto LSystemFx::LSystemFxImpl::ChangeColors() noexcept -> void
{
  //LogInfo("Changing colors.");
  std::for_each(begin(m_activeLSystems),
                end(m_activeLSystems),
                [](auto* lSystem) { lSystem->ChangeColors(); });
}

inline auto LSystemFx::LSystemFxImpl::GetCurrentColorMapsNames() noexcept
    -> std::vector<std::string>
{
  return {};
}

inline auto LSystemFx::LSystemFxImpl::ChangePixelBlender(
    const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pixelBlender.SetPixelBlendType(pixelBlenderParams);
}

inline auto LSystemFx::LSystemFxImpl::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept
    -> void
{
  LogInfo("Setting new zoom midpoint ({}, {}).", zoomMidpoint.x, zoomMidpoint.y);
  std::for_each(begin(m_activeLSystems),
                end(m_activeLSystems),
                [&zoomMidpoint](auto* lSystem) { lSystem->SetPathTarget(zoomMidpoint); });
}

inline auto LSystemFx::LSystemFxImpl::Resume() -> void
{
  InitNextActiveLSystems();
}

inline auto LSystemFx::LSystemFxImpl::ApplyToImageBuffers() -> void
{
  Update();
  DrawLSystem();
}

inline auto LSystemFx::LSystemFxImpl::Update() noexcept -> void
{
  //LogInfo("Doing update.");

  UpdatePixelBlender();

  if (static constexpr auto PROB_CHANGE_COLORS = 0.01F;
      (0 == m_fxHelper->goomInfo->GetSoundEvents().GetTimeSinceLastGoom()) or
      m_fxHelper->goomRand->ProbabilityOf(PROB_CHANGE_COLORS))
  {
    ChangeColors();
  }

  if (m_timeForTheseActiveLSys.Finished())
  {
    //LogInfo("Active l-system time finished.");
    InitNextActiveLSystems();
  }

  std::for_each(
      begin(m_activeLSystems), end(m_activeLSystems), [](auto* lSystem) { lSystem->Update(); });
}

inline auto LSystemFx::LSystemFxImpl::UpdatePixelBlender() noexcept -> void
{
  m_fxHelper->draw->SetPixelBlendFunc(m_pixelBlender.GetCurrentPixelBlendFunc());
  m_pixelBlender.Update();
}

inline auto LSystemFx::LSystemFxImpl::DrawLSystem() noexcept -> void
{
  //LogInfo("Start L-System draw.");

  std::for_each(begin(m_activeLSystems),
                end(m_activeLSystems),
                [](auto* lSystem) { lSystem->DrawLSystem(); });
}

inline auto LSystemFx::LSystemFxImpl::GetLSystemDirectory(
    const std::string& resourcesDirectory) noexcept -> std::string
{
  return resourcesDirectory + PATH_SEP + L_SYSTEMS_DIR;
}

} // namespace GOOM::VISUAL_FX
