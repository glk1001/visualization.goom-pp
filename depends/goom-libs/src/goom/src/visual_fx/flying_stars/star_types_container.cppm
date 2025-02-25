module;

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

export module Goom.VisualFx.FlyingStarsFx:StarTypesContainer;

import Goom.Color.ColorData.ColorMapEnums;
import Goom.Color.RandomColorMaps;
import Goom.Color.RandomColorMapsGroups;
import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;
import Goom.PluginInfo;

// TODO(glk): fix this
#if defined(__clang_major__) && __clang_major__ >= 20
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimport-implementation-partition-unit-in-interface-unit"
#endif
import :StarColorsMaker;
#if defined(__clang_major__) && __clang_major__ >= 20
#pragma GCC diagnostic pop
#endif

using GOOM::COLOR::GetUnweightedRandomColorMaps;
using GOOM::COLOR::WeightedRandomColorMaps;
using GOOM::COLOR::COLOR_DATA::ColorMapName;
using GOOM::UTILS::NUM;
using GOOM::UTILS::MATH::FULL_CIRCLE_RANGE;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::I_HALF;
using GOOM::UTILS::MATH::NumberRange;
using GOOM::UTILS::MATH::PI;
using GOOM::UTILS::MATH::Sq;
using GOOM::UTILS::MATH::THIRD_PI;
using GOOM::UTILS::MATH::TWO_PI;

export namespace GOOM::VISUAL_FX::FLYING_STARS
{

class IStarType
{
public:
  IStarType() noexcept                                    = default;
  IStarType(const IStarType&) noexcept                    = delete;
  IStarType(IStarType&&) noexcept                         = delete;
  virtual ~IStarType() noexcept                           = default;
  auto operator=(const IStarType&) noexcept -> IStarType& = delete;
  auto operator=(IStarType&&) noexcept -> IStarType&      = delete;

  struct SetupParams
  {
    Point2dInt startPos;
    float gravity{};
    float sideWind{};
    float nominalPathLength{};
    float starTAgeInc{};
  };
  [[nodiscard]] virtual auto GetRandomizedSetupParams(float defaultPathLength) const noexcept
      -> SetupParams = 0;
  [[nodiscard]] virtual auto GetRandomizedStarPathAngle(const Point2dInt& startPos) const noexcept
      -> float = 0;

  enum class ColorMapMode : UnderlyingEnumType
  {
    ONE_MAP_PER_ANGLE,
    ONE_MAP_FOR_ALL_ANGLES,
    ALL_MAPS_RANDOM,
  };

  [[nodiscard]] virtual auto GetStarColorsMaker() const noexcept -> const StarColorsMaker& = 0;
  virtual auto UpdateFixedColorMapNames() noexcept -> void                                 = 0;
  virtual auto UpdateWindAndGravity() noexcept -> void                                     = 0;
};

class StarType; // NOLINT(readability-identifier-naming): Seems to be a clang-tidy bug?

class StarTypesContainer
{
public:
  StarTypesContainer(const PluginInfo& goomInfo, const GoomRand& goomRand) noexcept;
  StarTypesContainer(const StarTypesContainer&) noexcept                    = delete;
  StarTypesContainer(StarTypesContainer&&) noexcept                         = delete;
  ~StarTypesContainer() noexcept                                            = default;
  auto operator=(const StarTypesContainer&) noexcept -> StarTypesContainer& = delete;
  auto operator=(StarTypesContainer&&) noexcept -> StarTypesContainer&      = delete;

  [[nodiscard]] auto GetRandomStarType() noexcept -> IStarType&;

  auto SetWeightedColorMaps(uint32_t starTypeId,
                            const WeightedRandomColorMaps& weightedMainColorMaps,
                            const WeightedRandomColorMaps& weightedLowColorMaps) noexcept -> void;
  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>;

  auto ChangeColorMode() noexcept -> void;
  auto SetColorMapMode(IStarType::ColorMapMode colorMapMode) noexcept -> void;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;

  static const auto NUM_STAR_TYPES = 3U;

private:
  enum class AvailableStarTypes : UnderlyingEnumType
  {
    FIREWORKS,
    RAIN,
    FOUNTAIN,
  };
  static_assert(NUM_STAR_TYPES == NUM<AvailableStarTypes>);
  std::array<std::unique_ptr<StarType>, NUM_STAR_TYPES> m_starTypesList;
  static constexpr float STAR_TYPES_FIREWORKS_WEIGHT = 10.0F;
  static constexpr float STAR_TYPES_FOUNTAIN_WEIGHT  = 07.0F;
  static constexpr float STAR_TYPES_RAIN_WEIGHT      = 07.0F;
  UTILS::MATH::Weights<AvailableStarTypes> m_weightedStarTypes;
};

} //namespace GOOM::VISUAL_FX::FLYING_STARS

namespace GOOM::VISUAL_FX::FLYING_STARS
{

class StarType : public IStarType
{
public:
  StarType(const PluginInfo& goomInfo, const GoomRand& goomRand) noexcept;

  [[nodiscard]] auto GetStarColorsMaker() const noexcept -> const StarColorsMaker& override;
  auto UpdateFixedColorMapNames() noexcept -> void override;
  auto UpdateWindAndGravity() noexcept -> void override;

protected:
  [[nodiscard]] auto GetGoomInfo() const noexcept -> const PluginInfo& { return *m_goomInfo; }
  [[nodiscard]] auto GetGoomRand() const noexcept -> const GoomRand& { return *m_goomRand; }
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
  const GoomRand* m_goomRand;
  int32_t m_halfWidth;
  int32_t m_halfHeight;
  float m_xMax;
  Point2dInt m_zoomMidpoint;

  ColorMapMode m_currentColorMapMode{};
  WeightedRandomColorMaps m_weightedMainColorMaps{
      GetUnweightedRandomColorMaps(*m_goomRand, MAX_ALPHA)};
  WeightedRandomColorMaps m_weightedLowColorMaps{
      GetUnweightedRandomColorMaps(*m_goomRand, MAX_ALPHA)};
  ColorMapName m_fixedMainColorMapName = ColorMapName::_NULL;
  ColorMapName m_fixedLowColorMapName  = ColorMapName::_NULL;
  [[nodiscard]] auto GetMainColorMapName() const noexcept -> ColorMapName;
  [[nodiscard]] auto GetLowColorMapName() const noexcept -> ColorMapName;

  StarColorsMaker m_starColorsMaker;
  [[nodiscard]] auto GetColorMapsSet() const noexcept -> StarColors::ColorMapsSet;
  [[nodiscard]] auto GetFixedMainColorMapName() const noexcept -> ColorMapName;
  [[nodiscard]] auto GetFixedLowColorMapName() const noexcept -> ColorMapName;

  static constexpr auto NUM_STEPS_TO_REACH_MAX_AGE_RANGE = NumberRange{100.0F, 1000.0F};

  static constexpr auto MIN_SIDE_WIND_RANGE   = NumberRange{-0.10F, -0.01F};
  static constexpr auto MAX_SIDE_WIND_RANGE   = NumberRange{+0.01F, +0.10F};
  static constexpr auto DEFAULT_MIN_SIDE_WIND = 0.0F;
  static constexpr auto DEFAULT_MAX_SIDE_WIND = 0.00001F;
  float m_minSideWind                         = DEFAULT_MIN_SIDE_WIND;
  float m_maxSideWind                         = DEFAULT_MAX_SIDE_WIND;

  static constexpr auto MIN_GRAVITY_RANGE = NumberRange{+0.005F, +0.010F};
  static constexpr auto MAX_GRAVITY_RANGE = NumberRange{+0.050F, +0.090F};
  float m_minGravity                      = MIN_GRAVITY_RANGE.max;
  float m_maxGravity                      = MAX_GRAVITY_RANGE.max;

  friend class StarTypesContainer;
  auto ChangeColorMode() noexcept -> void;
  auto SetColorMapMode(ColorMapMode colorMapMode) noexcept -> void;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;

  [[nodiscard]] auto GetWeightedMainColorMaps() const noexcept -> const WeightedRandomColorMaps&;
  [[nodiscard]] auto GetWeightedLowColorMaps() const noexcept -> const WeightedRandomColorMaps&;
  auto SetWeightedColorMaps(const WeightedRandomColorMaps& weightedMainColorMaps,
                            const WeightedRandomColorMaps& weightedLowColorMaps) noexcept -> void;
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
                                       const GoomRand& goomRand) noexcept
  : m_starTypesList{
        std::make_unique<FireworksStarType>(goomInfo, goomRand),
        std::make_unique<RainStarType>(goomInfo, goomRand),
        std::make_unique<FountainStarType>(goomInfo, goomRand),
    },
    m_weightedStarTypes{
        goomRand,
        {
            {.key=AvailableStarTypes::FIREWORKS, .weight=STAR_TYPES_FIREWORKS_WEIGHT},
            {.key=AvailableStarTypes::FOUNTAIN, .weight=STAR_TYPES_FOUNTAIN_WEIGHT},
            {.key=AvailableStarTypes::RAIN, .weight=STAR_TYPES_RAIN_WEIGHT},
        }
    }
{
}

auto StarTypesContainer::GetRandomStarType() noexcept -> IStarType&
{
  return *m_starTypesList.at(static_cast<size_t>(m_weightedStarTypes.GetRandomWeighted())).get();
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
    const WeightedRandomColorMaps& weightedMainColorMaps,
    const WeightedRandomColorMaps& weightedLowColorMaps) noexcept -> void
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
static const auto DEFAULT_COLOR_MAP_TYPES = WeightedRandomColorMaps::GetAllColorMapsTypes();

static constexpr auto Y_DISTANCE_OUT_OF_SCREEN_RANGE = NumberRange{10, 50};

StarType::StarType(const PluginInfo& goomInfo, const GoomRand& goomRand) noexcept
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
  m_fixedMainColorMapName = m_weightedMainColorMaps.GetRandomColorMapName();
  m_fixedLowColorMapName  = m_weightedLowColorMaps.GetRandomColorMapName();
}

inline auto StarType::GetWeightedMainColorMaps() const noexcept -> const WeightedRandomColorMaps&
{
  return m_weightedMainColorMaps;
}

inline auto StarType::GetWeightedLowColorMaps() const noexcept -> const WeightedRandomColorMaps&
{
  return m_weightedLowColorMaps;
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
    const WeightedRandomColorMaps& weightedMainColorMaps,
    const WeightedRandomColorMaps& weightedLowColorMaps) noexcept -> void
{
  m_weightedMainColorMaps = weightedMainColorMaps;
  m_weightedLowColorMaps  = weightedLowColorMaps;
  m_starColorsMaker.SetColorMapSet(GetColorMapsSet());
}

auto StarType::UpdateWindAndGravity() noexcept -> void
{
  if (static constexpr auto PROB_NEW_WIND_AND_GRAVITY = 0.10F;
      not m_goomRand->ProbabilityOf<PROB_NEW_WIND_AND_GRAVITY>())
  {
    return;
  }

  m_minSideWind = m_goomRand->GetRandInRange<MIN_SIDE_WIND_RANGE>();
  m_maxSideWind = m_goomRand->GetRandInRange<MAX_SIDE_WIND_RANGE>();
  m_minGravity  = m_goomRand->GetRandInRange<MIN_GRAVITY_RANGE>();
  m_maxGravity  = m_goomRand->GetRandInRange<MAX_GRAVITY_RANGE>();
}

auto StarType::GetColorMapsSet() const noexcept -> StarColors::ColorMapsSet
{
  if (static constexpr auto PROB_RANDOM_COLOR_MAPS = 0.5F;
      m_goomRand->ProbabilityOf<PROB_RANDOM_COLOR_MAPS>())
  {
    return {
        .currentMainColorMapPtr =
            GetWeightedMainColorMaps().GetRandomColorMapSharedPtr(DEFAULT_COLOR_MAP_TYPES),
        .currentLowColorMapPtr =
            GetWeightedLowColorMaps().GetRandomColorMapSharedPtr(DEFAULT_COLOR_MAP_TYPES),
        .dominantMainColorMapPtr =
            GetWeightedMainColorMaps().GetRandomColorMapSharedPtr(DEFAULT_COLOR_MAP_TYPES),
        .dominantLowColorMapPtr =
            GetWeightedLowColorMaps().GetRandomColorMapSharedPtr(DEFAULT_COLOR_MAP_TYPES),
    };
  }

  return {
      .currentMainColorMapPtr = GetWeightedMainColorMaps().GetRandomColorMapSharedPtr(
          GetMainColorMapName(), DEFAULT_COLOR_MAP_TYPES),
      .currentLowColorMapPtr = GetWeightedLowColorMaps().GetRandomColorMapSharedPtr(
          GetLowColorMapName(), DEFAULT_COLOR_MAP_TYPES),
      .dominantMainColorMapPtr = GetWeightedMainColorMaps().GetRandomColorMapSharedPtr(
          GetMainColorMapName(), DEFAULT_COLOR_MAP_TYPES),
      .dominantLowColorMapPtr = GetWeightedLowColorMaps().GetRandomColorMapSharedPtr(
          GetLowColorMapName(), DEFAULT_COLOR_MAP_TYPES),
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
      GetGoomRand().GetRandInRange<NUM_STEPS_TO_REACH_MAX_AGE_RANGE>() *
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
        .x = static_cast<int32_t>(GetGoomRand().GetNRand(GetGoomInfo().GetDimensions().GetWidth())),
        .y =
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

  setupParams.sideWind = INITIAL_WIND_FACTOR * GetGoomRand().GetRandInRange(
                                                   NumberRange{GetMinSideWind(), GetMaxSideWind()});

  static constexpr auto INITIAL_GRAVITY_FACTOR = 0.4F;
  setupParams.gravity                          = INITIAL_GRAVITY_FACTOR *
                        GetGoomRand().GetRandInRange(NumberRange{GetMinGravity(), GetMaxGravity()});

  static constexpr auto T_AGE_INC_FACTOR = 1.5F;
  setupParams.starTAgeInc                = T_AGE_INC_FACTOR * GetTAgeInc();

  return setupParams;
}

auto RainStarType::GetRandomizedSetupParams(const float defaultPathLength) const noexcept
    -> SetupParams
{
  SetupParams setupParams;

  static constexpr auto SPREAD_FRACTION_OF_WIDTH = 0.5F;
  static constexpr auto SPREAD_FRACTION_OF_WIDTH_RANGE =
      NumberRange{-SPREAD_FRACTION_OF_WIDTH, SPREAD_FRACTION_OF_WIDTH};

  const auto xFracOfHalfWidth =
      1.0F + GetGoomRand().GetRandInRange<SPREAD_FRACTION_OF_WIDTH_RANGE>();
  setupParams.startPos.x =
      static_cast<int32_t>(xFracOfHalfWidth * static_cast<float_t>(GetHalfWidth()));

  setupParams.startPos.y = -GetGoomRand().GetRandInRange<Y_DISTANCE_OUT_OF_SCREEN_RANGE>();

  static constexpr auto LENGTH_FACTOR = 1.5F;
  setupParams.nominalPathLength       = LENGTH_FACTOR * defaultPathLength;

  static constexpr auto INITIAL_WIND_FACTOR = 1.0F;
  setupParams.sideWind                      = INITIAL_WIND_FACTOR * GetGoomRand().GetRandInRange(
                                                   NumberRange{GetMinSideWind(), GetMaxSideWind()});

  static constexpr auto INITIAL_GRAVITY_FACTOR = 0.4F;
  setupParams.gravity                          = INITIAL_GRAVITY_FACTOR *
                        GetGoomRand().GetRandInRange(NumberRange{GetMinGravity(), GetMaxGravity()});

  static constexpr auto T_AGE_INC_FACTOR = 2.0F;
  setupParams.starTAgeInc                = T_AGE_INC_FACTOR * GetTAgeInc();

  return setupParams;
}

auto FountainStarType::GetRandomizedSetupParams(const float defaultPathLength) const noexcept
    -> SetupParams
{
  SetupParams setupParams;

  static constexpr auto SPREAD_FRACTION_OF_WIDTH = 0.25F;
  static constexpr auto SPREAD_FRACTION_OF_WIDTH_RANGE =
      NumberRange{-SPREAD_FRACTION_OF_WIDTH, SPREAD_FRACTION_OF_WIDTH};

  const auto xFracOfHalfWidth =
      1.0F + GetGoomRand().GetRandInRange<SPREAD_FRACTION_OF_WIDTH_RANGE>();
  setupParams.startPos.x =
      static_cast<int32_t>(xFracOfHalfWidth * static_cast<float_t>(GetHalfWidth()));

  setupParams.startPos.y = GetGoomInfo().GetDimensions().GetIntHeight() +
                           GetGoomRand().GetRandInRange<Y_DISTANCE_OUT_OF_SCREEN_RANGE>();

  setupParams.nominalPathLength = 1.0F + defaultPathLength;

  static constexpr auto INITIAL_WIND_FACTOR = 1.0F;

  setupParams.sideWind = INITIAL_WIND_FACTOR * GetGoomRand().GetRandInRange(
                                                   NumberRange{GetMinSideWind(), GetMaxSideWind()});

  static constexpr auto INITIAL_GRAVITY_FACTOR = 1.0F;
  setupParams.gravity                          = INITIAL_GRAVITY_FACTOR *
                        GetGoomRand().GetRandInRange(NumberRange{GetMinGravity(), GetMaxGravity()});

  static constexpr auto T_AGE_INC_FACTOR = 1.0F;
  setupParams.starTAgeInc                = T_AGE_INC_FACTOR * GetTAgeInc();

  return setupParams;
}

auto FireworksStarType::GetRandomizedStarPathAngle(
    [[maybe_unused]] const Point2dInt& startPos) const noexcept -> float
{
  static constexpr auto ANGLE_RANGE = FULL_CIRCLE_RANGE;

  return GetGoomRand().GetRandInRange<ANGLE_RANGE>();
}

auto RainStarType::GetRandomizedStarPathAngle(const Point2dInt& startPos) const noexcept -> float
{
  static constexpr auto MIN_ANGLE     = 0.05F * PI;
  static constexpr auto MAX_MIN_ANGLE = THIRD_PI;
  static constexpr auto MIN_MAX_ANGLE = MAX_MIN_ANGLE + (0.1F * PI);
  static constexpr auto MAX_ANGLE     = PI - MIN_ANGLE;

  const auto tLerp    = std::fabs((0.5F * GetXMax()) - static_cast<float>(startPos.x)) / GetXMax();
  const auto minAngle = std::lerp(MIN_ANGLE, MAX_MIN_ANGLE, tLerp);
  const auto maxAngle = std::lerp(MIN_MAX_ANGLE, MAX_ANGLE, 1.0F - tLerp);

  return GetGoomRand().GetRandInRange(NumberRange{minAngle, maxAngle});
}

auto FountainStarType::GetRandomizedStarPathAngle(const Point2dInt& startPos) const noexcept
    -> float
{
  static constexpr auto MIN_ANGLE     = 1.05F * PI;
  static constexpr auto MAX_MIN_ANGLE = PI + THIRD_PI;
  static constexpr auto MIN_MAX_ANGLE = MAX_MIN_ANGLE + (0.1F * PI);
  static constexpr auto MAX_ANGLE     = TWO_PI - (MIN_ANGLE - PI);

  const auto tLerp    = std::fabs((0.5F * GetXMax()) - static_cast<float>(startPos.x)) / GetXMax();
  const auto minAngle = std::lerp(MIN_ANGLE, MAX_MIN_ANGLE, tLerp);
  const auto maxAngle = std::lerp(MIN_MAX_ANGLE, MAX_ANGLE, 1.0F - tLerp);

  return GetGoomRand().GetRandInRange(NumberRange{minAngle, maxAngle});
}

} // namespace
} //namespace GOOM::VISUAL_FX::FLYING_STARS
