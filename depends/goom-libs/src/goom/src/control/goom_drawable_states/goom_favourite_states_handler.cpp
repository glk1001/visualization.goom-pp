module;

#include <vector>

module Goom.Control.GoomFavouriteStatesHandler;

import Goom.Control.GoomDrawables;
import Goom.Control.GoomDrawablesData;
import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRand;

namespace GOOM::CONTROL
{

using UTILS::EnumMap;
using UTILS::MATH::GoomRand;

using enum GoomDrawables;

const EnumMap<GoomFavouriteStatesHandler::Favourites, std::vector<GoomDrawables>>
    // NOLINTNEXTLINE(cert-err58-cpp)
    GoomFavouriteStatesHandler::FAVOURITE_DRAWABLES{{{
        {Favourites::DOTS_ONLY, {DOTS}},
        {Favourites::DOTS_PARTICLES, {DOTS, PARTICLES}},
        {Favourites::DOTS_IFS, {DOTS, IFS}},
        {Favourites::PARTICLES_ONLY, {PARTICLES}},
        {Favourites::PARTICLES_IFS, {PARTICLES, IFS}},
    }}};

static constexpr auto DOTS_ONLY_WEIGHT      = 1.0F;
static constexpr auto DOTS_PARTICLES_WEIGHT = 1.0F;
static constexpr auto DOTS_IFS_WEIGHT       = 1.0F;
static constexpr auto PARTICLES_ONLY_WEIGHT = 1.0F;
static constexpr auto PARTICLES_IFS_WEIGHT  = 1.0F;

GoomFavouriteStatesHandler::GoomFavouriteStatesHandler(const GoomRand& goomRand)
:  m_goomRand{&goomRand},
   m_weightedFavourites{
        goomRand,
        {
          {.key = Favourites::DOTS_ONLY,      .weight = DOTS_ONLY_WEIGHT},
          {.key = Favourites::DOTS_PARTICLES, .weight = DOTS_PARTICLES_WEIGHT},
          {.key = Favourites::DOTS_IFS,       .weight = DOTS_IFS_WEIGHT},
          {.key = Favourites::PARTICLES_ONLY, .weight = PARTICLES_ONLY_WEIGHT},
          {.key = Favourites::PARTICLES_IFS,  .weight = PARTICLES_IFS_WEIGHT},
        }
  },
  m_currentDrawablesState{GetGoomDrawablesState(m_weightedFavourites.GetRandomWeighted())}
{
}

auto GoomFavouriteStatesHandler::ChangeToNextState() -> void
{
  auto favourite                  = m_weightedFavourites.GetRandomWeighted();
  static constexpr auto MAX_TRIES = 10U;
  for (auto i = 0U; i < MAX_TRIES; ++i)
  {
    if (m_currentFavourite != favourite)
    {
      break;
    }
    favourite = m_weightedFavourites.GetRandomWeighted();
  }

  ChangeToFavourite(favourite);
}

auto GoomFavouriteStatesHandler::GetGoomDrawablesState(const Favourites favourite) const noexcept
    -> GoomDrawablesState
{
  const auto& drawables = FAVOURITE_DRAWABLES[favourite];
  return GoomDrawablesState{drawables, GetRandInRangeBuffIntensities(*m_goomRand, drawables)};
}

auto GoomFavouriteStatesHandler::ChangeToFavourite(const Favourites favourite) -> void
{
  m_currentFavourite      = favourite;
  m_currentDrawablesState = GetGoomDrawablesState(favourite);
}

} // namespace GOOM::CONTROL
