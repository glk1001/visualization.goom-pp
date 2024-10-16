module;

#include <algorithm>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

module Goom.Control.GoomDrawablesData;

import Goom.Control.GoomDrawables;
import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRand;

namespace GOOM::CONTROL
{

using UTILS::EnumMap;
using UTILS::MATH::GetMidpoint;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;

static constexpr auto DRAWABLE_NAMES = EnumMap<GoomDrawables, std::string_view>{{{
    {GoomDrawables::CIRCLES, "CIRCLS"},
    {GoomDrawables::DOTS, "DOTS"},
    {GoomDrawables::IFS, "IFS"},
    {GoomDrawables::L_SYSTEM, "LSYS"},
    {GoomDrawables::LINES, "LNS"},
    {GoomDrawables::IMAGE, "IMG"},
    {GoomDrawables::PARTICLES, "PART"},
    {GoomDrawables::RAINDROPS, "DROPS"},
    {GoomDrawables::SHAPES, "SHPS"},
    {GoomDrawables::STARS, "STARS"},
    {GoomDrawables::TENTACLES, "TENTCL"},
    {GoomDrawables::TUBES, "TUBES"},
}}};

static constexpr auto BUFF_INTENSITY_RANGES = EnumMap<GoomDrawables, NumberRange<float>>{{{
    {GoomDrawables::CIRCLES, {0.90F, 0.99F}},
    {GoomDrawables::DOTS, {0.90F, 0.99F}},
    {GoomDrawables::IFS, {0.90F, 0.99F}},
    {GoomDrawables::L_SYSTEM, {0.90F, 0.99F}},
    {GoomDrawables::LINES, {0.80F, 0.90F}},
    {GoomDrawables::IMAGE, {0.20F, 0.50F}},
    {GoomDrawables::PARTICLES, {0.90F, 0.99F}},
    {GoomDrawables::RAINDROPS, {0.90F, 0.99F}},
    {GoomDrawables::SHAPES, {0.90F, 0.99F}},
    {GoomDrawables::STARS, {0.90F, 0.99F}},
    {GoomDrawables::TENTACLES, {0.90F, 0.99F}},
    {GoomDrawables::TUBES, {0.90F, 0.99F}},
}}};

static constexpr auto STATE_MULTI_THREADED = EnumMap<GoomDrawables, bool>{{{
    {GoomDrawables::CIRCLES, false},
    {GoomDrawables::DOTS, false},
    {GoomDrawables::IFS, false},
    {GoomDrawables::L_SYSTEM, false},
    {GoomDrawables::LINES, false},
    {GoomDrawables::IMAGE, true},
    {GoomDrawables::PARTICLES, false},
    {GoomDrawables::RAINDROPS, false},
    {GoomDrawables::SHAPES, false},
    {GoomDrawables::STARS, false},
    {GoomDrawables::TENTACLES, false},
    {GoomDrawables::TUBES, false},
}}};

static constexpr auto PROB_SINGLE_DRAWABLE = EnumMap<GoomDrawables, float>{{{
    {GoomDrawables::CIRCLES, 1.0F},
    {GoomDrawables::DOTS, 1.0F},
    {GoomDrawables::IFS, 1.0F},
    {GoomDrawables::L_SYSTEM, 1.0F},
    {GoomDrawables::LINES, 1.0F},
    {GoomDrawables::IMAGE, 0.0F},
    {GoomDrawables::PARTICLES, 1.0F},
    {GoomDrawables::RAINDROPS, 1.0F},
    {GoomDrawables::SHAPES, 1.0F},
    {GoomDrawables::STARS, 1.0F},
    {GoomDrawables::TENTACLES, 1.0F},
    {GoomDrawables::TUBES, 1.0F},
}}};

auto GetDrawablesStateName(const std::vector<GoomDrawables>& drawables) -> std::string
{
  static constexpr auto DELIM = std::string_view{"_"};

  return std::ranges::to<std::string>(
      drawables |
      std::views::transform([](const auto drawable) { return DRAWABLE_NAMES[drawable]; }) |
      std::views::join_with(DELIM));
}

auto GetRandInRangeBuffIntensities(const GoomRand& goomRand,
                                   const std::vector<GoomDrawables>& drawables) noexcept
    -> std::vector<float>
{
  return std::ranges::to<std::vector<float>>(
      drawables |
      std::views::transform([&goomRand](const auto drawable)
                            { return goomRand.GetRandInRange(BUFF_INTENSITY_RANGES[drawable]); }));
}

auto GetMidpointRangeBuffIntensities(const std::vector<GoomDrawables>& drawables) noexcept
    -> std::vector<float>
{
  return std::ranges::to<std::vector<float>>(
      drawables | std::views::transform([](const auto drawable)
                                        { return GetMidpoint(BUFF_INTENSITY_RANGES[drawable]); }));
}

auto AreAnyMultiThreaded(const std::vector<GoomDrawables>& drawables) noexcept -> bool
{
  return std::ranges::any_of(drawables,
                             [](const auto& drawable) { return STATE_MULTI_THREADED[drawable]; });
}

auto GetProbCanBeSingleDrawable(const GoomDrawables drawable) noexcept -> float
{
  return PROB_SINGLE_DRAWABLE[drawable];
}

} // namespace GOOM::CONTROL
