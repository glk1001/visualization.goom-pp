#include "star_types_container.h"

#include "color/random_color_maps_groups.h"
#include "goom_config.h"
#include "goom_plugin_info.h"
#include "point2d.h"
#include "star_colors_maker.h"
#include "stars.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"

#include <memory>

namespace GOOM::VISUAL_FX::FLYING_STARS
{

using COLOR::RandomColorMaps;
using COLOR::RandomColorMapsGroups;
using COLOR::COLOR_DATA::ColorMapName;
using UTILS::NUM;
using UTILS::MATH::I_HALF;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::PI;
using UTILS::MATH::Sq;
using UTILS::MATH::THIRD_PI;
using UTILS::MATH::TWO_PI;

class StarType : public IStarType
{
public:
  StarType(const PluginInfo& goomInfo, const IGoomRand& goomRand) noexcept;

  [[nodiscard]] auto GetStarColorsMaker() const noexcept -> const StarColorsMaker& override;
  auto UpdateFixedColorMapNames() noexcept -> void override;
  auto UpdateWindAndGravity() noexcept -> void override;

protected:
  [[nodiscard]] auto GetGoomInfo() const noexcept -> const PluginInfo& { return *m_goomInfo; }
  [[nodiscard]] auto GetGoomRand() const noexcept -> const IGoomRand& { return *m_goomRand; }
  [[nodiscard]] auto GetZoomMidpoint() const noexcept -> Point2dInt { return m_zoomMidpoint; }
  [[nodiscard]] auto GetHalfWidth() const noexcept -> int32_t { return m_halfWidth; }
  [[nodiscard]] auto GetHalfHeight() const noexcept -> int32_t { return m_halfHeight; }
  [[nodiscard]] auto GetXMax() const noexcept -> float { return m_xMax; }
  [[nodiscard]] auto GetMinSideWind() const noexcept -> float { return m_minSideWind; }
  [[nodiscard]] auto GetMaxSideWind() const noexcept -> float { return m_maxSideWind; }
  [[nodiscard]] auto GetMinGravity() const noexcept -> float { return m_minGravity; }
  [[nodiscard]] auto GetMaxGravity() const noexcept -> float { return m_maxGravity; }

  [[nodiscard]] auto GetTAgeInc() const noexcept -> float;

private:
  const PluginInfo* m_goomInfo;
  const IGoomRand* m_goomRand;
  int32_t m_halfWidth;
  int32_t m_halfHeight;
  float m_xMax;
  Point2dInt m_zoomMidpoint;

  ColorMapMode m_currentColorMapMode{};
  std::shared_ptr<const RandomColorMaps> m_weightedMainColorMaps{
      RandomColorMapsGroups::MakeSharedAllMapsUnweighted(*m_goomRand)};
  std::shared_ptr<const RandomColorMaps> m_weightedLowColorMaps{
      RandomColorMapsGroups::MakeSharedAllMapsUnweighted(*m_goomRand)};
  ColorMapName m_fixedMainColorMapName = ColorMapName::_NULL;
  ColorMapName m_fixedLowColorMapName  = ColorMapName::_NULL;
  [[nodiscard]] auto GetMainColorMapName() const noexcept -> ColorMapName;
  [[nodiscard]] auto GetLowColorMapName() const noexcept -> ColorMapName;

  StarColorsMaker m_starColorsMaker;
  [[nodiscard]] auto GetColorMapsSet() const noexcept -> StarColors::ColorMapsSet;
  [[nodiscard]] auto GetFixedMainColorMapName() const noexcept -> ColorMapName;
  [[nodiscard]] auto GetFixedLowColorMapName() const noexcept -> ColorMapName;

  static constexpr auto MIN_NUM_STEPS_TO_REACH_MAX_AGE = 100.0F;
  static constexpr auto MAX_NUM_STEPS_TO_REACH_MAX_AGE = 1000.0F;

  static constexpr float MIN_MIN_SIDE_WIND     = -0.10F;
  static constexpr float MAX_MIN_SIDE_WIND     = -0.01F;
  static constexpr float MIN_MAX_SIDE_WIND     = +0.01F;
  static constexpr float MAX_MAX_SIDE_WIND     = +0.10F;
  static constexpr float DEFAULT_MIN_SIDE_WIND = 0.0F;
  static constexpr float DEFAULT_MAX_SIDE_WIND = 0.00001F;
  float m_minSideWind                          = DEFAULT_MIN_SIDE_WIND;
  float m_maxSideWind                          = DEFAULT_MAX_SIDE_WIND;

  static constexpr float MIN_MIN_GRAVITY = +0.005F;
  static constexpr float MAX_MIN_GRAVITY = +0.010F;
  static constexpr float MIN_MAX_GRAVITY = +0.050F;
  static constexpr float MAX_MAX_GRAVITY = +0.090F;
  float m_minGravity                     = MAX_MIN_GRAVITY;
  float m_maxGravity                     = MAX_MAX_GRAVITY;

  friend class StarTypesContainer;
  auto ChangeColorMode() noexcept -> void;
  auto SetColorMapMode(ColorMapMode colorMapMode) noexcept -> void;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;

  [[nodiscard]] auto GetWeightedMainColorMaps() const noexcept -> const RandomColorMaps&;
  [[nodiscard]] auto GetWeightedLowColorMaps() const noexcept -> const RandomColorMaps&;
  auto SetWeightedColorMaps(
      const std::shared_ptr<const RandomColorMaps>& weightedMainColorMaps,
      const std::shared_ptr<const RandomColorMaps>& weightedLowColorMaps) noexcept -> void;
};

namespace
{

class FireworksStarType : public StarType
{
public:
  using StarType::StarType;

  [[nodiscard]] auto GetRandomizedSetupParams(float defaultPathLength) const noexcept
      -> SetupParams override;
  [[nodiscard]] auto GetRandomizedStarPathAngle(const Point2dInt& startPos) const noexcept
      -> float override;
};

class RainStarType : public StarType
{
public:
  using StarType::StarType;

  [[nodiscard]] auto GetRandomizedSetupParams(float defaultPathLength) const noexcept
      -> SetupParams override;
  [[nodiscard]] auto GetRandomizedStarPathAngle(const Point2dInt& startPos) const noexcept
      -> float override;
};

class FountainStarType : public StarType
{
public:
  using StarType::StarType;

  [[nodiscard]] auto GetRandomizedSetupParams(float defaultPathLength) const noexcept
      -> SetupParams override;
  [[nodiscard]] auto GetRandomizedStarPathAngle(const Point2dInt& startPos) const noexcept
      -> float override;
};

} // namespace

StarTypesContainer::StarTypesContainer(const PluginInfo& goomInfo,
                                       const IGoomRand& goomRand) noexcept
  : m_starTypesList{
        std::make_unique<FireworksStarType>(goomInfo, goomRand),
        std::make_unique<RainStarType>(goomInfo, goomRand),
        std::make_unique<FountainStarType>(goomInfo, goomRand),
    },
    m_weightedStarTypes{
        goomRand,
        {
            {AvailableStarTypes::FIREWORKS, STAR_TYPES_FIREWORKS_WEIGHT},
            {AvailableStarTypes::FOUNTAIN, STAR_TYPES_FOUNTAIN_WEIGHT},
            {AvailableStarTypes::RAIN, STAR_TYPES_RAIN_WEIGHT},
        }
    }
{
}

StarTypesContainer::~StarTypesContainer() noexcept = default;

auto StarTypesContainer::GetRandomStarType() noexcept -> IStarType&
{
  return *m_starTypesList.at(static_cast<uint32_t>(m_weightedStarTypes.GetRandomWeighted())).get();
}

auto StarTypesContainer::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  auto colorMapsNames = std::vector<std::string>{};

  for (const auto& starType : m_starTypesList)
  {
    colorMapsNames.emplace_back(starType->GetWeightedMainColorMaps().GetColorMapsName());
    colorMapsNames.emplace_back(starType->GetWeightedLowColorMaps().GetColorMapsName());
  }

  return colorMapsNames;
}

auto StarTypesContainer::SetWeightedColorMaps(
    const uint32_t starTypeId,
    const std::shared_ptr<const RandomColorMaps>& weightedMainColorMaps,
    const std::shared_ptr<const RandomColorMaps>& weightedLowColorMaps) noexcept -> void
{
  Expects(starTypeId < NUM<StarTypesContainer::AvailableStarTypes>);
  m_starTypesList.at(starTypeId)->SetWeightedColorMaps(weightedMainColorMaps, weightedLowColorMaps);
}

auto StarTypesContainer::SetColorMapMode(const IStarType::ColorMapMode colorMapMode) noexcept
    -> void
{
  for (auto& starType : m_starTypesList)
  {
    starType->SetColorMapMode(colorMapMode);
  }
}

auto StarTypesContainer::ChangeColorMode() noexcept -> void
{
  for (auto& starType : m_starTypesList)
  {
    starType->ChangeColorMode();
  }
}

auto StarTypesContainer::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  for (auto& starType : m_starTypesList)
  {
    starType->SetZoomMidpoint(zoomMidpoint);
  }
}

// NOLINTNEXTLINE(cert-err58-cpp)
static const auto DEFAULT_COLOR_MAP_TYPES = RandomColorMaps::GetAllColorMapsTypes();

static constexpr auto MIN_Y_DISTANCE_OUT_OF_SCREEN = 10;
static constexpr auto MAX_Y_DISTANCE_OUT_OF_SCREEN = 50;

StarType::StarType(const PluginInfo& goomInfo, const IGoomRand& goomRand) noexcept
  : m_goomInfo{&goomInfo},
    m_goomRand{&goomRand},
    m_halfWidth{I_HALF * goomInfo.GetDimensions().GetIntWidth()},
    m_halfHeight{I_HALF * goomInfo.GetDimensions().GetIntHeight()},
    m_xMax{goomInfo.GetDimensions().GetFltWidth() - 1.0F},
    m_starColorsMaker{goomRand, GetColorMapsSet()}
{
}

inline auto StarType::GetStarColorsMaker() const noexcept -> const StarColorsMaker&
{
  return m_starColorsMaker;
}

inline auto StarType::UpdateFixedColorMapNames() noexcept -> void
{
  m_fixedMainColorMapName = m_weightedMainColorMaps->GetRandomColorMapName();
  m_fixedLowColorMapName  = m_weightedLowColorMaps->GetRandomColorMapName();
}

inline auto StarType::GetWeightedMainColorMaps() const noexcept -> const RandomColorMaps&
{
  Expects(m_weightedMainColorMaps != nullptr);
  return *m_weightedMainColorMaps;
}

inline auto StarType::GetWeightedLowColorMaps() const noexcept -> const RandomColorMaps&
{
  Expects(m_weightedLowColorMaps != nullptr);
  return *m_weightedLowColorMaps;
}

inline auto StarType::SetColorMapMode(const ColorMapMode colorMapMode) noexcept -> void
{
  m_currentColorMapMode = colorMapMode;
}

inline auto StarType::ChangeColorMode() noexcept -> void
{
  m_starColorsMaker.ChangeColorMode();
}

inline auto StarType::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  m_zoomMidpoint = zoomMidpoint;
}

inline auto StarType::SetWeightedColorMaps(
    const std::shared_ptr<const RandomColorMaps>& weightedMainColorMaps,
    const std::shared_ptr<const RandomColorMaps>& weightedLowColorMaps) noexcept -> void
{
  m_weightedMainColorMaps = weightedMainColorMaps;
  m_weightedLowColorMaps  = weightedLowColorMaps;
  m_starColorsMaker.SetColorMapSet(GetColorMapsSet());
}

auto StarType::UpdateWindAndGravity() noexcept -> void
{
  if (static constexpr auto PROB_NEW_WIND_AND_GRAVITY = 0.10F;
      not m_goomRand->ProbabilityOf(PROB_NEW_WIND_AND_GRAVITY))
  {
    return;
  }

  m_minSideWind = m_goomRand->GetRandInRange(MIN_MIN_SIDE_WIND, MAX_MIN_SIDE_WIND);
  m_maxSideWind = m_goomRand->GetRandInRange(MIN_MAX_SIDE_WIND, MAX_MAX_SIDE_WIND);
  m_minGravity  = m_goomRand->GetRandInRange(MIN_MIN_GRAVITY, MAX_MIN_GRAVITY);
  m_maxGravity  = m_goomRand->GetRandInRange(MIN_MAX_GRAVITY, MAX_MAX_GRAVITY);
}

auto StarType::GetColorMapsSet() const noexcept -> StarColors::ColorMapsSet
{
  if (static constexpr auto PROB_RANDOM_COLOR_MAPS = 0.5F;
      m_goomRand->ProbabilityOf(PROB_RANDOM_COLOR_MAPS))
  {
    return {
        GetWeightedMainColorMaps().GetRandomColorMapPtr(DEFAULT_COLOR_MAP_TYPES),
        GetWeightedLowColorMaps().GetRandomColorMapPtr(DEFAULT_COLOR_MAP_TYPES),
        GetWeightedMainColorMaps().GetRandomColorMapPtr(DEFAULT_COLOR_MAP_TYPES),
        GetWeightedLowColorMaps().GetRandomColorMapPtr(DEFAULT_COLOR_MAP_TYPES),
    };
  }

  return {
      GetWeightedMainColorMaps().GetRandomColorMapPtr(GetMainColorMapName(),
                                                      DEFAULT_COLOR_MAP_TYPES),
      GetWeightedLowColorMaps().GetRandomColorMapPtr(GetLowColorMapName(), DEFAULT_COLOR_MAP_TYPES),
      GetWeightedMainColorMaps().GetRandomColorMapPtr(GetMainColorMapName(),
                                                      DEFAULT_COLOR_MAP_TYPES),
      GetWeightedLowColorMaps().GetRandomColorMapPtr(GetLowColorMapName(), DEFAULT_COLOR_MAP_TYPES),
  };
}

inline auto StarType::GetMainColorMapName() const noexcept -> ColorMapName
{
  switch (m_currentColorMapMode)
  {
    case ColorMapMode::ONE_MAP_PER_ANGLE:
      return GetWeightedMainColorMaps().GetRandomColorMapName();
    case ColorMapMode::ONE_MAP_FOR_ALL_ANGLES:
      return GetFixedMainColorMapName();
    case ColorMapMode::ALL_MAPS_RANDOM:
      return ColorMapName::_NULL;
    default:
      FailFast();
  }
}

inline auto StarType::GetLowColorMapName() const noexcept -> ColorMapName
{
  switch (m_currentColorMapMode)
  {
    case ColorMapMode::ONE_MAP_PER_ANGLE:
      return GetWeightedLowColorMaps().GetRandomColorMapName();
    case ColorMapMode::ONE_MAP_FOR_ALL_ANGLES:
      return GetFixedLowColorMapName();
    case ColorMapMode::ALL_MAPS_RANDOM:
      return ColorMapName::_NULL;
    default:
      FailFast();
  }
}

inline auto StarType::GetFixedMainColorMapName() const noexcept -> ColorMapName
{
  return m_fixedMainColorMapName;
}

inline auto StarType::GetFixedLowColorMapName() const noexcept -> ColorMapName
{
  return m_fixedLowColorMapName;
}

inline auto StarType::GetTAgeInc() const noexcept -> float
{
  const auto numStepsToReachMaxAge =
      GetGoomRand().GetRandInRange(MIN_NUM_STEPS_TO_REACH_MAX_AGE, MAX_NUM_STEPS_TO_REACH_MAX_AGE) *
      (1.0F - GetGoomInfo().GetSoundEvents().GetGoomPower());

  return 1.0F / numStepsToReachMaxAge;
}

namespace
{

auto FireworksStarType::GetRandomizedSetupParams(const float defaultPathLength) const noexcept
    -> SetupParams
{
  SetupParams setupParams;

  const auto rSq = Sq(GetHalfHeight() / 2);
  while (true)
  {
    setupParams.startPos = {
        static_cast<int32_t>(GetGoomRand().GetNRand(GetGoomInfo().GetDimensions().GetWidth())),
        static_cast<int32_t>(GetGoomRand().GetNRand(GetGoomInfo().GetDimensions().GetHeight())),
    };
    const auto sqDist = SqDistance(setupParams.startPos, GetZoomMidpoint());
    if (sqDist < rSq)
    {
      break;
    }
  }

  static constexpr auto LENGTH_FACTOR = 1.0F;
  setupParams.nominalPathLength       = LENGTH_FACTOR * defaultPathLength;

  static constexpr auto INITIAL_WIND_FACTOR = 0.1F;
  setupParams.sideWind =
      INITIAL_WIND_FACTOR * GetGoomRand().GetRandInRange(GetMinSideWind(), GetMaxSideWind());

  static constexpr auto INITIAL_GRAVITY_FACTOR = 0.4F;
  setupParams.gravity =
      INITIAL_GRAVITY_FACTOR * GetGoomRand().GetRandInRange(GetMinGravity(), GetMaxGravity());

  static constexpr auto T_AGE_INC_FACTOR = 1.5F;
  setupParams.starTAgeInc                = T_AGE_INC_FACTOR * GetTAgeInc();

  return setupParams;
}

auto RainStarType::GetRandomizedSetupParams(const float defaultPathLength) const noexcept
    -> SetupParams
{
  SetupParams setupParams;

  static constexpr auto SPREAD_FRACTION_OF_WIDTH = 0.5F;
  const auto xFracOfHalfWidth =
      1.0F + GetGoomRand().GetRandInRange(-SPREAD_FRACTION_OF_WIDTH, SPREAD_FRACTION_OF_WIDTH);
  setupParams.startPos.x =
      static_cast<int32_t>(xFracOfHalfWidth * static_cast<float_t>(GetHalfWidth()));

  setupParams.startPos.y =
      -GetGoomRand().GetRandInRange(MIN_Y_DISTANCE_OUT_OF_SCREEN, MAX_Y_DISTANCE_OUT_OF_SCREEN + 1);

  static constexpr auto LENGTH_FACTOR = 1.5F;
  setupParams.nominalPathLength       = LENGTH_FACTOR * defaultPathLength;

  static constexpr auto INITIAL_WIND_FACTOR = 1.0F;
  setupParams.sideWind =
      INITIAL_WIND_FACTOR * GetGoomRand().GetRandInRange(GetMinSideWind(), GetMaxSideWind());

  static constexpr auto INITIAL_GRAVITY_FACTOR = 0.4F;
  setupParams.gravity =
      INITIAL_GRAVITY_FACTOR * GetGoomRand().GetRandInRange(GetMinGravity(), GetMaxGravity());

  static constexpr auto T_AGE_INC_FACTOR = 2.0F;
  setupParams.starTAgeInc                = T_AGE_INC_FACTOR * GetTAgeInc();

  return setupParams;
}

auto FountainStarType::GetRandomizedSetupParams(const float defaultPathLength) const noexcept
    -> SetupParams
{
  SetupParams setupParams;

  static constexpr auto SPREAD_FRACTION_OF_WIDTH = 0.25F;
  const auto xFracOfHalfWidth =
      1.0F + GetGoomRand().GetRandInRange(-SPREAD_FRACTION_OF_WIDTH, SPREAD_FRACTION_OF_WIDTH);
  setupParams.startPos.x =
      static_cast<int32_t>(xFracOfHalfWidth * static_cast<float_t>(GetHalfWidth()));

  setupParams.startPos.y =
      GetGoomInfo().GetDimensions().GetIntHeight() +
      GetGoomRand().GetRandInRange(MIN_Y_DISTANCE_OUT_OF_SCREEN, MAX_Y_DISTANCE_OUT_OF_SCREEN + 1);

  setupParams.nominalPathLength = 1.0F + defaultPathLength;

  static constexpr auto INITIAL_WIND_FACTOR = 1.0F;
  setupParams.sideWind =
      INITIAL_WIND_FACTOR * GetGoomRand().GetRandInRange(GetMinSideWind(), GetMaxSideWind());

  static constexpr auto INITIAL_GRAVITY_FACTOR = 1.0F;
  setupParams.gravity =
      INITIAL_GRAVITY_FACTOR * GetGoomRand().GetRandInRange(GetMinGravity(), GetMaxGravity());

  static constexpr auto T_AGE_INC_FACTOR = 1.0F;
  setupParams.starTAgeInc                = T_AGE_INC_FACTOR * GetTAgeInc();

  return setupParams;
}

auto FireworksStarType::GetRandomizedStarPathAngle(
    [[maybe_unused]] const Point2dInt& startPos) const noexcept -> float
{
  static constexpr auto MIN_ANGLE = 0.0F;
  static constexpr auto MAX_ANGLE = TWO_PI;

  return GetGoomRand().GetRandInRange(MIN_ANGLE, MAX_ANGLE);
}

auto RainStarType::GetRandomizedStarPathAngle(const Point2dInt& startPos) const noexcept -> float
{
  static constexpr auto MIN_ANGLE     = 0.05F * PI;
  static constexpr auto MAX_MIN_ANGLE = THIRD_PI;
  static constexpr auto MIN_MAX_ANGLE = MAX_MIN_ANGLE + (0.1F * PI);
  static constexpr auto MAX_ANGLE     = PI - MIN_ANGLE;

  const auto tLerp    = std::fabs((0.5F * GetXMax()) - static_cast<float>(startPos.x)) / GetXMax();
  const auto minAngle = STD20::lerp(MIN_ANGLE, MAX_MIN_ANGLE, tLerp);
  const auto maxAngle = STD20::lerp(MIN_MAX_ANGLE, MAX_ANGLE, 1.0F - tLerp);

  return GetGoomRand().GetRandInRange(minAngle, maxAngle);
}

auto FountainStarType::GetRandomizedStarPathAngle(const Point2dInt& startPos) const noexcept
    -> float
{
  static constexpr auto MIN_ANGLE     = 1.05F * PI;
  static constexpr auto MAX_MIN_ANGLE = PI + THIRD_PI;
  static constexpr auto MIN_MAX_ANGLE = MAX_MIN_ANGLE + (0.1F * PI);
  static constexpr auto MAX_ANGLE     = TWO_PI - (MIN_ANGLE - PI);

  const auto tLerp    = std::fabs((0.5F * GetXMax()) - static_cast<float>(startPos.x)) / GetXMax();
  const auto minAngle = STD20::lerp(MIN_ANGLE, MAX_MIN_ANGLE, tLerp);
  const auto maxAngle = STD20::lerp(MIN_MAX_ANGLE, MAX_ANGLE, 1.0F - tLerp);

  return GetGoomRand().GetRandInRange(minAngle, maxAngle);
}

} // namespace
} //namespace GOOM::VISUAL_FX::FLYING_STARS
