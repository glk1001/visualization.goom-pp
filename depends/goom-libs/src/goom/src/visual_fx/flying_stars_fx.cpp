module;

//#undef NO_LOGGING

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

module Goom.VisualFx.FlyingStarsFx;

import Goom.Control.GoomSoundEvents;
import Goom.Draw.GoomDrawBase;
import Goom.Utils.Graphics.SmallImageBitmaps;
import Goom.Utils.Math.GoomRand;
import Goom.VisualFx.FxHelper;
import Goom.VisualFx.FxUtils;
import Goom.Lib.GoomGraphic;
import Goom.Lib.Point2d;
import Goom.Lib.SPimpl;
import :StarDrawer;
import :StarMaker;
import :Stars;
import :StarTypesContainer;

namespace GOOM::VISUAL_FX
{

using FLYING_STARS::IStarType;
using FLYING_STARS::Star;
using FLYING_STARS::StarDrawer;
using FLYING_STARS::StarMaker;
using FLYING_STARS::StarTypesContainer;
using FX_UTILS::RandomPixelBlender;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::NumberRange;
using UTILS::MATH::Weights;

using enum IStarType::ColorMapMode;

static constexpr auto COLOR_MAP_MODE_ONE_MAP_PER_ANGLE_WGT      = 30.0F;
static constexpr auto COLOR_MAP_MODE_ONE_MAP_FOR_ALL_ANGLES_WGT = 10.0F;
static constexpr auto COLOR_MAP_MODE_MEGA_RANDOM_WGT            = 01.0F;

class FlyingStarsFx::FlyingStarsImpl
{
public:
  FlyingStarsImpl(FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps);

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;

  [[nodiscard]] auto GetCurrentStarTypeColorMapsNames() const noexcept -> std::vector<std::string>;
  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;

  auto ApplyToImageBuffers() noexcept -> void;

private:
  FxHelper* m_fxHelper;
  PixelChannelType m_defaultAlpha = DEFAULT_VISUAL_FX_ALPHA;
  StarMaker m_starMaker;
  StarDrawer m_starDrawer;
  StarTypesContainer m_starTypesContainer;

  uint32_t m_counter                  = 0;
  static constexpr uint32_t MAX_COUNT = 100;

  using ColorMapMode = IStarType::ColorMapMode;
  Weights<ColorMapMode> m_colorMapModeWeights;
  auto ChangeMapsAndModes() -> void;

  RandomPixelBlender m_pixelBlender;
  auto UpdatePixelBlender() noexcept -> void;

  static constexpr auto TOTAL_NUM_ACTIVE_STARS_RANGE = NumberRange{100U, 1024U};
  std::vector<Star> m_activeStars;
  auto CheckForStarEvents() noexcept -> void;
  auto SoundEventOccurred() noexcept -> void;
  auto ChangeColorMapMode() noexcept -> void;
  auto DrawStars() noexcept -> void;
  [[nodiscard]] auto IsStarDead(const Star& star) const noexcept -> bool;
  auto RemoveDeadStars() noexcept -> void;

  static constexpr auto NUM_STAR_CLUSTERS_RANGE = NumberRange{0U, 2U};
  static constexpr auto MAX_STAR_CLUSTER_WIDTH  = 320.0F;
  static constexpr auto MAX_STAR_CLUSTER_HEIGHT = 200.0F;
  // Why 320,200 ? Because the FX was developed on 320x200.
  static constexpr auto STAR_CLUSTER_HEIGHT_RANGE = NumberRange{50.0F, MAX_STAR_CLUSTER_HEIGHT};
  float m_heightRatio = m_fxHelper->GetDimensions().GetFltHeight() / MAX_STAR_CLUSTER_HEIGHT;
  auto AddStarClusters() -> void;
  auto AddStarCluster(const IStarType& starType, uint32_t totalNumActiveStars) noexcept -> void;
  [[nodiscard]] auto GetNumStarsToAdd(uint32_t totalNumActiveStars) const noexcept -> uint32_t;
  [[nodiscard]] auto GetMaxStarsInACluster() const noexcept -> uint32_t;
  [[nodiscard]] auto GetStarProperties() const noexcept -> StarMaker::StarProperties;
};

FlyingStarsFx::FlyingStarsFx(FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept
  : m_pimpl{spimpl::make_unique_impl<FlyingStarsImpl>(fxHelper, smallBitmaps)}
{
}

auto FlyingStarsFx::GetFxName() const noexcept -> std::string
{
  return "Flying Stars FX";
}

auto FlyingStarsFx::Start() noexcept -> void
{
  // nothing to be done
}

auto FlyingStarsFx::Finish() noexcept -> void
{
  // nothing to be done
}

auto FlyingStarsFx::Resume() noexcept -> void
{
  // nothing to be done
}

auto FlyingStarsFx::Suspend() noexcept -> void
{
  // nothing to be done
}

auto FlyingStarsFx::ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept
    -> void
{
  m_pimpl->ChangePixelBlender(pixelBlenderParams);
}

auto FlyingStarsFx::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  m_pimpl->SetZoomMidpoint(zoomMidpoint);
}

auto FlyingStarsFx::SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept
    -> void
{
  m_pimpl->SetWeightedColorMaps(weightedColorMaps);
}

auto FlyingStarsFx::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  return m_pimpl->GetCurrentStarTypeColorMapsNames();
}

auto FlyingStarsFx::ApplyToImageBuffers() noexcept -> void
{
  m_pimpl->ApplyToImageBuffers();
}

FlyingStarsFx::FlyingStarsImpl::FlyingStarsImpl(FxHelper& fxHelper,
                                                const SmallImageBitmaps& smallBitmaps)
  : m_fxHelper{&fxHelper},
    m_starMaker{fxHelper.GetGoomRand()},
    m_starDrawer{fxHelper.GetDraw(), m_fxHelper->GetGoomRand(), smallBitmaps},
    m_starTypesContainer{fxHelper.GetGoomInfo(), fxHelper.GetGoomRand()},
    m_colorMapModeWeights{
        m_fxHelper->GetGoomRand(),
        {
            { .key = ONE_MAP_PER_ANGLE,      .weight = COLOR_MAP_MODE_ONE_MAP_PER_ANGLE_WGT },
            { .key = ONE_MAP_FOR_ALL_ANGLES, .weight = COLOR_MAP_MODE_ONE_MAP_FOR_ALL_ANGLES_WGT },
            { .key = ALL_MAPS_RANDOM,        .weight = COLOR_MAP_MODE_MEGA_RANDOM_WGT },
        }
    },
    m_pixelBlender{m_fxHelper->GetGoomRand()}
{
  m_activeStars.reserve(TOTAL_NUM_ACTIVE_STARS_RANGE.max);
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetCurrentStarTypeColorMapsNames() const noexcept
    -> std::vector<std::string>
{
  return m_starTypesContainer.GetCurrentColorMapsNames();
}

auto FlyingStarsFx::FlyingStarsImpl::SetWeightedColorMaps(
    const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  const auto newWeightedColorMaps =
      GetWeightedColorMapsWithNewAlpha(weightedColorMaps, m_defaultAlpha);

  m_starTypesContainer.SetWeightedColorMaps(newWeightedColorMaps.id,
                                            newWeightedColorMaps.mainColorMaps,
                                            newWeightedColorMaps.lowColorMaps);

  ChangeColorMapMode();
}

inline auto FlyingStarsFx::FlyingStarsImpl::ChangeColorMapMode() noexcept -> void
{
  m_starTypesContainer.SetColorMapMode(m_colorMapModeWeights.GetRandomWeighted());
}

inline auto FlyingStarsFx::FlyingStarsImpl::ChangeMapsAndModes() -> void
{
  ChangeColorMapMode();
  m_starTypesContainer.ChangeColorMode();
  m_starDrawer.ChangeDrawMode();
}

inline auto FlyingStarsFx::FlyingStarsImpl::ChangePixelBlender(
    const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pixelBlender.SetPixelBlendType(pixelBlenderParams);
}

inline auto FlyingStarsFx::FlyingStarsImpl::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept
    -> void
{
  m_starTypesContainer.SetZoomMidpoint(zoomMidpoint);
}

/**
 * Ajoute de nouvelles particules au moment d'un evenement sonore.
 */
inline auto FlyingStarsFx::FlyingStarsImpl::SoundEventOccurred() noexcept -> void
{
  ChangeColorMapMode();
  AddStarClusters();
}

inline auto FlyingStarsFx::FlyingStarsImpl::ApplyToImageBuffers() noexcept -> void
{
  ++m_counter;

  UpdatePixelBlender();
  CheckForStarEvents();
  DrawStars();
  RemoveDeadStars();
}

inline auto FlyingStarsFx::FlyingStarsImpl::UpdatePixelBlender() noexcept -> void
{
  m_fxHelper->GetDraw().SetPixelBlendFunc(m_pixelBlender.GetCurrentPixelBlendFunc());
  m_pixelBlender.Update();
}

auto FlyingStarsFx::FlyingStarsImpl::CheckForStarEvents() noexcept -> void
{
  if ((not m_activeStars.empty()) and (m_fxHelper->GetSoundEvents().GetTimeSinceLastGoom() >= 1))
  {
    return;
  }

  SoundEventOccurred();

  if (static constexpr auto PROB_CHANGE_MAPS_AND_MODES = 1.0F / 20.0F;
      m_fxHelper->GetGoomRand().ProbabilityOf<PROB_CHANGE_MAPS_AND_MODES>())
  {
    ChangeMapsAndModes();
  }
  else if (m_counter > MAX_COUNT)
  {
    m_counter = 0;
    ChangeMapsAndModes();
  }
}

auto FlyingStarsFx::FlyingStarsImpl::DrawStars() noexcept -> void
{
  static constexpr auto SPEED_FACTOR_RANGE = NumberRange{0.1F, 10.0F};
  const auto speedFactor = m_fxHelper->GetGoomRand().GetRandInRange<SPEED_FACTOR_RANGE>();

  for (auto& star : m_activeStars)
  {
    star.Update();

    if (star.IsTooOld())
    {
      continue;
    }

    m_starDrawer.DrawStar(star, speedFactor);
  }
}

auto FlyingStarsFx::FlyingStarsImpl::RemoveDeadStars() noexcept -> void
{
  const auto isDead = [this](const Star& star) { return IsStarDead(star); };
  std::erase_if(m_activeStars, isDead);
}

auto FlyingStarsFx::FlyingStarsImpl::IsStarDead(const Star& star) const noexcept -> bool
{
  static constexpr auto DEAD_MARGIN = 64;

  if ((star.GetStartPos().x < -DEAD_MARGIN) ||
      (star.GetStartPos().x >
       static_cast<float>(m_fxHelper->GetDimensions().GetWidth() + DEAD_MARGIN)))
  {
    return true;
  }
  if ((star.GetStartPos().y < -DEAD_MARGIN) ||
      (star.GetStartPos().y >
       static_cast<float>(m_fxHelper->GetDimensions().GetHeight() + DEAD_MARGIN)))
  {
    return true;
  }

  return star.IsTooOld();
}

auto FlyingStarsFx::FlyingStarsImpl::AddStarClusters() -> void
{
  const auto numStarClusters = m_fxHelper->GetGoomRand().GetRandInRange<NUM_STAR_CLUSTERS_RANGE>();
  const auto totalNumActiveStars =
      m_fxHelper->GetGoomRand().GetRandInRange<TOTAL_NUM_ACTIVE_STARS_RANGE>();

  for (auto i = 0U; i < numStarClusters; ++i)
  {
    auto& starType = m_starTypesContainer.GetRandomStarType();

    starType.UpdateWindAndGravity();
    starType.UpdateFixedColorMapNames();

    AddStarCluster(starType, totalNumActiveStars);
  }
}

auto FlyingStarsFx::FlyingStarsImpl::AddStarCluster(
    const IStarType& starType, const uint32_t totalNumActiveStars) noexcept -> void
{
  if (m_activeStars.size() >= totalNumActiveStars)
  {
    return;
  }

  m_starMaker.StartNewCluster(starType, GetNumStarsToAdd(totalNumActiveStars), GetStarProperties());
  while (m_starMaker.MoreStarsToMake())
  {
    m_activeStars.emplace_back(m_starMaker.MakeNewStar());
  }
}

auto FlyingStarsFx::FlyingStarsImpl::GetStarProperties() const noexcept -> StarMaker::StarProperties
{
  static constexpr auto NOMINAL_PATH_LENGTH_FACTOR = 1.5F;

  return StarMaker::StarProperties{
      /* .heightRatio = */ m_heightRatio,
      /* .defaultPathLength = */ (1.0F + m_fxHelper->GetSoundEvents().GetGoomPower()) *
          (m_fxHelper->GetGoomRand().GetRandInRange<STAR_CLUSTER_HEIGHT_RANGE>() /
           MAX_STAR_CLUSTER_WIDTH),
      /* .nominalPathLengthFactor = */
      (m_fxHelper->GetSoundEvents().GetTimeSinceLastBigGoom() >= 1) ? 1.0F
                                                                    : NOMINAL_PATH_LENGTH_FACTOR};
}

auto FlyingStarsFx::FlyingStarsImpl::GetNumStarsToAdd(
    const uint32_t totalNumActiveStars) const noexcept -> uint32_t
{
  const auto numStarsThatCanBeAdded =
      static_cast<uint32_t>(totalNumActiveStars - m_activeStars.size());
  const auto maxStarsInACluster = GetMaxStarsInACluster();

  return std::min(maxStarsInACluster, numStarsThatCanBeAdded);
}

auto FlyingStarsFx::FlyingStarsImpl::GetMaxStarsInACluster() const noexcept -> uint32_t
{
  static constexpr auto BASE_STARS_IN_CLUSTER        = 100.0F;
  static constexpr auto EXTRA_STARS_IN_CLUSTER_RANGE = NumberRange{0.0F, 150.0F};
  const auto maxStarsInACluster                      = static_cast<uint32_t>(
      m_heightRatio * (BASE_STARS_IN_CLUSTER +
                       ((m_fxHelper->GetSoundEvents().GetGoomPower() + 1.0F) *
                        m_fxHelper->GetGoomRand().GetRandInRange<EXTRA_STARS_IN_CLUSTER_RANGE>())));

  if (m_fxHelper->GetSoundEvents().GetTimeSinceLastBigGoom() < 1)
  {
    return 2 * maxStarsInACluster;
  }

  return maxStarsInACluster;
}

} // namespace GOOM::VISUAL_FX
