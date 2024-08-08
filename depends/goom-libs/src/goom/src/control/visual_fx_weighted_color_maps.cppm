module Goom.Control.GoomAllVisualFx:VisualFxWeightedColorMaps;

import Goom.Color.RandomColorMapsGroups;
import Goom.Control.GoomEffects;
import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRand;

using GOOM::UTILS::RuntimeEnumMap;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::Weights;

using Groups = GOOM::COLOR::RandomColorMapsGroups::Groups;

namespace GOOM::CONTROL
{

class VisualFxWeightedColorMaps
{
public:
  explicit VisualFxWeightedColorMaps(const GoomRand& goomRand) noexcept;

  [[nodiscard]] auto GetCurrentRandomColorMapsGroup(GoomEffect goomEffect) const noexcept -> Groups;

private:
  using WeightedGroups = Weights<Groups>;
  RuntimeEnumMap<GoomEffect, WeightedGroups> m_goomEffectsWeightedColorMaps;

  [[nodiscard]] static auto GetCirclesMainGroups(const GoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetCirclesLowGroups(const GoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetDots0Groups(const GoomRand& goomRand) noexcept -> WeightedGroups;
  [[nodiscard]] static auto GetDots1Groups(const GoomRand& goomRand) noexcept -> WeightedGroups;
  [[nodiscard]] static auto GetDots2Groups(const GoomRand& goomRand) noexcept -> WeightedGroups;
  [[nodiscard]] static auto GetDots3Groups(const GoomRand& goomRand) noexcept -> WeightedGroups;
  [[nodiscard]] static auto GetDots4Groups(const GoomRand& goomRand) noexcept -> WeightedGroups;
  [[nodiscard]] static auto GetIfsGroups(const GoomRand& goomRand) noexcept -> WeightedGroups;
  [[nodiscard]] static auto GetImageGroups(const GoomRand& goomRand) noexcept -> WeightedGroups;
  [[nodiscard]] static auto GetLines1Groups(const GoomRand& goomRand) noexcept -> WeightedGroups;
  [[nodiscard]] static auto GetLines2Groups(const GoomRand& goomRand) noexcept -> WeightedGroups;
  [[nodiscard]] static auto GetLSystemMainGroups(const GoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetLSystemLowGroups(const GoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetParticlesMainGroups(const GoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetParticlesLowGroups(const GoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetRaindropsMainGroups(const GoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetRaindropsLowGroups(const GoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetShapesMainGroups(const GoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetShapesLowGroups(const GoomRand& goomRand) noexcept -> WeightedGroups;
  [[nodiscard]] static auto GetShapesInnerGroups(const GoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetStarsMainFireworksGroups(const GoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetStarsLowFireworksGroups(const GoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetStarsMainRainGroups(const GoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetStarsLowRainGroups(const GoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetStarsMainFountainGroups(const GoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetStarsLowFountainGroups(const GoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetTentaclesDominantMainGroups(const GoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetTentaclesDominantLowGroups(const GoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetTentaclesMainGroups(const GoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetTentaclesLowGroups(const GoomRand& goomRand) noexcept
      -> WeightedGroups;
  [[nodiscard]] static auto GetTubesMainGroups(const GoomRand& goomRand) noexcept -> WeightedGroups;
  [[nodiscard]] static auto GetTubesLowGroups(const GoomRand& goomRand) noexcept -> WeightedGroups;
};

} // namespace GOOM::CONTROL

namespace GOOM::CONTROL
{

inline auto VisualFxWeightedColorMaps::GetCurrentRandomColorMapsGroup(
    const GoomEffect goomEffect) const noexcept -> Groups
{
  return m_goomEffectsWeightedColorMaps[goomEffect].GetRandomWeighted();
}

} // namespace GOOM::CONTROL

namespace GOOM::CONTROL
{

using enum Groups;

VisualFxWeightedColorMaps::VisualFxWeightedColorMaps(const GoomRand& goomRand) noexcept
  : m_goomEffectsWeightedColorMaps{{{
        {GoomEffect::CIRCLES_MAIN, GetCirclesMainGroups(goomRand)},
        {GoomEffect::CIRCLES_LOW, GetCirclesLowGroups(goomRand)},
        {GoomEffect::DOTS0, GetDots0Groups(goomRand)},
        {GoomEffect::DOTS1, GetDots1Groups(goomRand)},
        {GoomEffect::DOTS2, GetDots2Groups(goomRand)},
        {GoomEffect::DOTS3, GetDots3Groups(goomRand)},
        {GoomEffect::DOTS4, GetDots4Groups(goomRand)},
        {GoomEffect::IFS, GetIfsGroups(goomRand)},
        {GoomEffect::IMAGE, GetImageGroups(goomRand)},
        {GoomEffect::L_SYSTEM_MAIN, GetLSystemMainGroups(goomRand)},
        {GoomEffect::L_SYSTEM_LOW, GetLSystemLowGroups(goomRand)},
        {GoomEffect::LINES1, GetLines1Groups(goomRand)},
        {GoomEffect::LINES2, GetLines2Groups(goomRand)},
        {GoomEffect::PARTICLES_MAIN, GetParticlesMainGroups(goomRand)},
        {GoomEffect::PARTICLES_LOW, GetParticlesLowGroups(goomRand)},
        {GoomEffect::RAINDROPS_MAIN, GetRaindropsMainGroups(goomRand)},
        {GoomEffect::RAINDROPS_LOW, GetRaindropsLowGroups(goomRand)},
        {GoomEffect::SHAPES_MAIN, GetShapesMainGroups(goomRand)},
        {GoomEffect::SHAPES_LOW, GetShapesLowGroups(goomRand)},
        {GoomEffect::SHAPES_INNER, GetShapesInnerGroups(goomRand)},
        {GoomEffect::STARS_MAIN_FIREWORKS, GetStarsMainFireworksGroups(goomRand)},
        {GoomEffect::STARS_LOW_FIREWORKS, GetStarsLowFireworksGroups(goomRand)},
        {GoomEffect::STARS_MAIN_RAIN, GetStarsMainRainGroups(goomRand)},
        {GoomEffect::STARS_LOW_RAIN, GetStarsLowRainGroups(goomRand)},
        {GoomEffect::STARS_MAIN_FOUNTAIN, GetStarsMainFountainGroups(goomRand)},
        {GoomEffect::STARS_LOW_FOUNTAIN, GetStarsLowFountainGroups(goomRand)},
        {GoomEffect::TENTACLES_DOMINANT_MAIN, GetTentaclesDominantMainGroups(goomRand)},
        {GoomEffect::TENTACLES_DOMINANT_LOW, GetTentaclesDominantLowGroups(goomRand)},
        {GoomEffect::TENTACLES_MAIN, GetTentaclesMainGroups(goomRand)},
        {GoomEffect::TENTACLES_LOW, GetTentaclesLowGroups(goomRand)},
        {GoomEffect::TUBE_MAIN, GetTubesMainGroups(goomRand)},
        {GoomEffect::TUBE_LOW, GetTubesLowGroups(goomRand)},
    }}}
{
}

auto VisualFxWeightedColorMaps::GetCirclesMainGroups(const GoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetCirclesLowGroups(const GoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetDots0Groups(const GoomRand& goomRand) noexcept -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetDots1Groups(const GoomRand& goomRand) noexcept -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetDots2Groups(const GoomRand& goomRand) noexcept -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetDots3Groups(const GoomRand& goomRand) noexcept -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetDots4Groups(const GoomRand& goomRand) noexcept -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetIfsGroups(const GoomRand& goomRand) noexcept -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetImageGroups(const GoomRand& goomRand) noexcept -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 00.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 00.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 00.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetLines1Groups(const GoomRand& goomRand) noexcept -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetLines2Groups(const GoomRand& goomRand) noexcept -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetLSystemMainGroups(const GoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 00.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 00.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 00.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetLSystemLowGroups(const GoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 00.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 00.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 00.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetParticlesMainGroups(const GoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetRaindropsMainGroups(const GoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetParticlesLowGroups(const GoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetRaindropsLowGroups(const GoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetShapesMainGroups(const GoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetShapesLowGroups(const GoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetShapesInnerGroups(const GoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetStarsMainFireworksGroups(const GoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 00.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 00.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 00.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetStarsLowFireworksGroups(const GoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 00.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 00.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 00.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetStarsMainRainGroups(const GoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 00.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 00.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 00.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetStarsLowRainGroups(const GoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 00.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 00.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 00.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetStarsMainFountainGroups(const GoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 00.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 00.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 00.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetStarsLowFountainGroups(const GoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 00.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 00.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 00.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 00.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetTentaclesDominantMainGroups(const GoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetTentaclesDominantLowGroups(const GoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetTentaclesMainGroups(const GoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetTentaclesLowGroups(const GoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetTubesMainGroups(const GoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

auto VisualFxWeightedColorMaps::GetTubesLowGroups(const GoomRand& goomRand) noexcept
    -> WeightedGroups
{
  static constexpr auto ALL_MAPS_UNWEIGHTED_WGT              = 05.0F;
  static constexpr auto ALL_STANDARD_MAPS_WGT                = 05.0F;
  static constexpr auto ALL_SLIM_MAPS_WGT                    = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT  = 05.0F;
  static constexpr auto MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT      = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT = 05.0F;
  static constexpr auto SLIGHTLY_DIVERGING_SLIM_MAPS_WGT     = 20.0F;
  static constexpr auto DIVERGING_BLACK_STANDARD_MAPS_WGT    = 35.0F;
  static constexpr auto RED_STANDARD_MAPS_WGT                = 35.0F;
  static constexpr auto GREEN_STANDARD_MAPS_WGT              = 40.0F;
  static constexpr auto BLUE_STANDARD_MAPS_WGT               = 40.0F;
  static constexpr auto YELLOW_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto ORANGE_STANDARD_MAPS_WGT             = 40.0F;
  static constexpr auto PURPLE_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto CITIES_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto SEASONS_STANDARD_MAPS_WGT            = 90.0F;
  static constexpr auto HEAT_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto COLD_STANDARD_MAPS_WGT               = 90.0F;
  static constexpr auto PASTEL_STANDARD_MAPS_WGT             = 90.0F;
  static constexpr auto WES_ANDERSON_MAPS_WGT                = 90.0F;

  return {
      goomRand,
      {
        {.key = ALL_MAPS_UNWEIGHTED, .weight = ALL_MAPS_UNWEIGHTED_WGT},
        {.key = ALL_STANDARD_MAPS, .weight = ALL_STANDARD_MAPS_WGT},
        {.key = ALL_SLIM_MAPS, .weight = ALL_SLIM_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_STANDARD_MAPS, .weight = MOSTLY_SEQUENTIAL_STANDARD_MAPS_WGT},
        {.key = MOSTLY_SEQUENTIAL_SLIM_MAPS, .weight = MOSTLY_SEQUENTIAL_SLIM_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_STANDARD_MAPS, .weight = SLIGHTLY_DIVERGING_STANDARD_MAPS_WGT},
        {.key = SLIGHTLY_DIVERGING_SLIM_MAPS, .weight = SLIGHTLY_DIVERGING_SLIM_MAPS_WGT},
        {.key = DIVERGING_BLACK_STANDARD_MAPS, .weight = DIVERGING_BLACK_STANDARD_MAPS_WGT},
        {.key = RED_STANDARD_MAPS, .weight = RED_STANDARD_MAPS_WGT},
        {.key = GREEN_STANDARD_MAPS, .weight = GREEN_STANDARD_MAPS_WGT},
        {.key = BLUE_STANDARD_MAPS, .weight = BLUE_STANDARD_MAPS_WGT},
        {.key = YELLOW_STANDARD_MAPS, .weight = YELLOW_STANDARD_MAPS_WGT},
        {.key = ORANGE_STANDARD_MAPS, .weight = ORANGE_STANDARD_MAPS_WGT},
        {.key = PURPLE_STANDARD_MAPS, .weight = PURPLE_STANDARD_MAPS_WGT},
        {.key = CITIES_STANDARD_MAPS, .weight = CITIES_STANDARD_MAPS_WGT},
        {.key = SEASONS_STANDARD_MAPS, .weight = SEASONS_STANDARD_MAPS_WGT},
        {.key = HEAT_STANDARD_MAPS, .weight = HEAT_STANDARD_MAPS_WGT},
        {.key = COLD_STANDARD_MAPS, .weight = COLD_STANDARD_MAPS_WGT},
        {.key = PASTEL_STANDARD_MAPS, .weight = PASTEL_STANDARD_MAPS_WGT},
        {.key = WES_ANDERSON_MAPS, .weight = WES_ANDERSON_MAPS_WGT},
        }
  };
}

} // namespace GOOM::CONTROL
