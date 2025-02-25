export module Goom.Control.GoomAllVisualFx:VisualFxColorMaps;

import Goom.Color.RandomColorMaps;
import Goom.Color.RandomColorMapsGroups;
import Goom.Control.GoomEffects;
import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRand;

// TODO(glk): fix this
#if defined(__clang_major__) && __clang_major__ >= 20
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimport-implementation-partition-unit-in-interface-unit"
#endif
import :VisualFxColorMatchedSets;
import :VisualFxWeightedColorMaps;
#if defined(__clang_major__) && __clang_major__ >= 20
#pragma GCC diagnostic pop
#endif

using GOOM::COLOR::RandomColorMapsGroups;
using GOOM::COLOR::WeightedRandomColorMaps;
using GOOM::UTILS::NUM;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::NumberRange;

export namespace GOOM::CONTROL
{

class VisualFxColorMaps
{
public:
  explicit VisualFxColorMaps(const GoomRand& goomRand);

  auto ChangeRandomColorMaps() -> void;

  [[nodiscard]] auto GetCurrentRandomColorMaps(GoomEffect goomEffect) const
      -> WeightedRandomColorMaps;

private:
  const GoomRand* m_goomRand;
  RandomColorMapsGroups m_randomColorMapsGroups{*m_goomRand};
  VisualFxColorMatchedSets m_visualFxColorMatchedSets{*m_goomRand};
  VisualFxWeightedColorMaps m_visualFxWeightedColorMaps{*m_goomRand};
  [[nodiscard]] auto GetNextRandomColorMapsGroup(GoomEffect goomEffect) const
      -> RandomColorMapsGroups::Groups;
  [[nodiscard]] auto GetNextCompletelyRandomColorMapsGroup() const -> RandomColorMapsGroups::Groups;
};

} // namespace GOOM::CONTROL

namespace GOOM::CONTROL
{

inline VisualFxColorMaps::VisualFxColorMaps(const GoomRand& goomRand) : m_goomRand{&goomRand}
{
}

inline auto VisualFxColorMaps::ChangeRandomColorMaps() -> void
{
  m_visualFxColorMatchedSets.SetNextRandomColorMatchedSet();
}

inline auto VisualFxColorMaps::GetCurrentRandomColorMaps(const GoomEffect goomEffect) const
    -> WeightedRandomColorMaps
{
  return m_randomColorMapsGroups.GetWeightedRandomColorMapsForGroup(
      GetNextRandomColorMapsGroup(goomEffect));
}

} // namespace GOOM::CONTROL

namespace GOOM::CONTROL
{

inline auto VisualFxColorMaps::GetNextRandomColorMapsGroup(const GoomEffect goomEffect) const
    -> RandomColorMapsGroups::Groups
{
  //  COLOR::RandomColorMapsGroups randomColorMapsGroups{m_goomRand};
  //  const auto group = COLOR::RandomColorMapsGroups::Groups::PURPLE_STANDARD_MAPS;
  //  return randomColorMapsGroups.MakeRandomColorMapsGroup(group);

  if (static constexpr auto PROB_COMPLETELY_RANDOM = 0.05F;
      m_goomRand->ProbabilityOf<PROB_COMPLETELY_RANDOM>())
  {
    return GetNextCompletelyRandomColorMapsGroup();
  }
  if (static constexpr auto PROB_WEIGHTED_COLOR_MAPS = 0.25F;
      m_goomRand->ProbabilityOf<PROB_WEIGHTED_COLOR_MAPS>())
  {
    return m_visualFxWeightedColorMaps.GetCurrentRandomColorMapsGroup(goomEffect);
  }

  return m_visualFxColorMatchedSets.GetCurrentRandomColorMapsGroup(goomEffect);
}

inline auto VisualFxColorMaps::GetNextCompletelyRandomColorMapsGroup() const
    -> RandomColorMapsGroups::Groups
{
  return static_cast<RandomColorMapsGroups::Groups>(
      m_goomRand->GetRandInRange<NumberRange{0U, NUM<RandomColorMapsGroups::Groups> - 1}>());
}

} // namespace GOOM::CONTROL
