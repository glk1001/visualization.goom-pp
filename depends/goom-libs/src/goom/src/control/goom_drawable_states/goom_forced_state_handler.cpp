module;

#include <array>
#include <vector>

module Goom.Control.GoomForcedStateHandler;

import Goom.Control.GoomDrawables;
import Goom.Control.GoomDrawablesData;
import Goom.Control.GoomStateHandler;

namespace GOOM::CONTROL
{

//static constexpr auto FORCED_STATE = std::array{GoomDrawables::CIRCLES};
//static constexpr auto FORCED_STATE = std::array{GoomDrawables::DOTS};
//static constexpr auto FORCED_STATE = std::array{GoomDrawables::L_SYSTEM};
static constexpr auto FORCED_STATE = std::array{GoomDrawables::LINES};
//static constexpr auto FORCED_STATE = std::array{GoomDrawables::PARTICLES};
//static constexpr auto FORCED_STATE = std::array{GoomDrawables::SHAPES};
//static constexpr auto FORCED_STATE = std::array{GoomDrawables::STARS};
//static constexpr auto FORCED_STATE = std::array{GoomDrawables::TUBES};

GoomForcedStateHandler::GoomForcedStateHandler() noexcept
  : m_constDrawablesState{
        GetGoomDrawablesState(std::vector<GoomDrawables>{cbegin(FORCED_STATE), cend(FORCED_STATE)})}
{
}

auto GoomForcedStateHandler::GetGoomDrawablesState(
    const std::vector<GoomDrawables>& drawables) noexcept -> GoomDrawablesState
{
  return GoomDrawablesState{drawables, GetMidpointRangeBuffIntensities(drawables)};
}

} // namespace GOOM::CONTROL
