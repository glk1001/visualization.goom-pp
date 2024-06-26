module;

//#undef NO_LOGGING

#include <algorithm>
#include <string_view>
#include <utility>
#include <vector>

module Goom.Control.GoomStates;

import Goom.Utils.EnumUtils;
import Goom.Lib.AssertUtils;

namespace GOOM::CONTROL
{

using UTILS::EnumMap;
using UTILS::NUM;

namespace
{

constexpr auto DEFAULT_BUFF_INTENSITY_RANGES = EnumMap<GoomDrawables, BuffIntensityRange>{{{
    {GoomDrawables::CIRCLES, {0.50F, 0.80F}},
    {GoomDrawables::DOTS, {0.40F, 0.70F}},
    {GoomDrawables::IFS, {0.40F, 0.70F}},
    {GoomDrawables::L_SYSTEM, {0.70F, 0.80F}},
    {GoomDrawables::LINES, {0.50F, 0.70F}},
    {GoomDrawables::IMAGE, {0.05F, 0.30F}},
    {GoomDrawables::PARTICLES, {0.50F, 0.80F}},
    {GoomDrawables::RAINDROPS, {0.60F, 0.80F}},
    {GoomDrawables::SHAPES, {0.50F, 0.80F}},
    {GoomDrawables::STARS, {0.50F, 0.60F}},
    {GoomDrawables::TENTACLES, {0.30F, 0.50F}},
    {GoomDrawables::TUBES, {0.70F, 0.80F}},
}}};

constexpr auto STATE_MULTI_THREADED = EnumMap<GoomDrawables, bool>{{{
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

constexpr auto STATE_NAMES = EnumMap<GoomStates, std::string_view>{{{
    {GoomStates::CIRCLES_ONLY, "Circles Only"},
    {GoomStates::CIRCLES_IFS, "Circles and IFS"},
    {GoomStates::CIRCLES_IMAGE, "Circles and Image"},
    {GoomStates::CIRCLES_IMAGE_STARS, "Circles, Image, Stars"},
    {GoomStates::CIRCLES_IMAGE_STARS_L_SYSTEM, "Circles, Img, Strs, L"},
    {GoomStates::CIRCLES_LINES, "Circles and Lines"},
    {GoomStates::CIRCLES_STARS_TUBES, "Circles, Stars, Tubes"},
    {GoomStates::CIRCLES_TENTACLES, "Circles and Tentacles"},

    {GoomStates::DOTS_ONLY, "Dots Only"},
    {GoomStates::DOTS_IFS, "Dots and Ifs"},
    {GoomStates::DOTS_IFS_RAINDROPS, "Dots, IFS, Raindrops"},
    {GoomStates::DOTS_IFS_STARS, "Dots, IFS, Stars"},
    {GoomStates::DOTS_IMAGE_RAINDROPS, "Dots, Image, Raindrops"},
    {GoomStates::DOTS_IMAGE_RAINDROPS_STARS, "Dots, Image, Rain, Strs"},
    {GoomStates::DOTS_IMAGE_STARS, "Dots, Image, Stars"},
    {GoomStates::DOTS_IMAGE_STARS_L_SYSTEM, "Dots, Img, Strs, L"},
    {GoomStates::DOTS_LINES, "Dots and Lines"},
    {GoomStates::DOTS_LINES_RAINDROPS_STARS, "Dots, Lines, Rain, S"},
    {GoomStates::DOTS_LINES_STARS_TENTACLES, "D, Li, S, Te"},
    {GoomStates::DOTS_LINES_TENTACLES_TUBES, "D, Li, Te, Tu"},
    {GoomStates::DOTS_LINES_TUBES, "D, Li, Tu"},
    {GoomStates::DOTS_RAINDROPS_STARS, "Dots, Stars, Rain"},
    {GoomStates::DOTS_STARS, "Dots and Stars"},
    {GoomStates::DOTS_STARS_L_SYSTEM, "Dots, Stars, L"},
    {GoomStates::DOTS_STARS_TENTACLES_TUBES, "D, S, Te, Tu"},
    {GoomStates::DOTS_TENTACLES_TUBES, "Dots, Tentacles, Tubes"},

    {GoomStates::IFS_ONLY, "IFS Only"},
    {GoomStates::IFS_IMAGE, "IFS and Image"},
    {GoomStates::IFS_IMAGE_RAINDROPS_SHAPES, "IFS, Image, Rain, Sh"},
    {GoomStates::IFS_IMAGE_SHAPES, "IFS, Image and Shapes"},
    {GoomStates::IFS_LINES_RAINDROPS_STARS, "IFS, Lines, Rain, S"},
    {GoomStates::IFS_LINES_STARS, "IFS, Lines, Stars"},
    {GoomStates::IFS_PARTICLES, "IFS and Particles"},
    {GoomStates::IFS_RAINDROPS, "IFS and Raindrops"},
    {GoomStates::IFS_RAINDROPS_SHAPES, "IFS, Rain, Sh"},
    {GoomStates::IFS_RAINDROPS_STARS, "IFS, Rain, Stars"},
    {GoomStates::IFS_SHAPES, "IFS and Shapes"},
    {GoomStates::IFS_STARS, "IFS and Stars"},
    {GoomStates::IFS_STARS_TENTACLES, "IFS, Stars, Tentacles"},
    {GoomStates::IFS_TENTACLES, "IFS and Tentacles"},
    {GoomStates::IFS_TENTACLES_TUBES, "IFS, Tentacles, Tubes"},
    {GoomStates::IFS_TUBES, "Ifs and Tubes"},

    {GoomStates::IMAGE_ONLY, "Image Only"},
    {GoomStates::IMAGE_LINES, "Image and Lines"},
    {GoomStates::IMAGE_LINES_RAINDROPS, "Image, Lines and Rain"},
    {GoomStates::IMAGE_LINES_SHAPES, "Image, Lines and Shapes"},
    {GoomStates::IMAGE_LINES_STARS_TENTACLES, "Im, Li, S, Te"},
    {GoomStates::IMAGE_RAINDROPS, "Image and Raindrops"},
    {GoomStates::IMAGE_RAINDROPS_SHAPES, "Image, Rain, Sh"},
    {GoomStates::IMAGE_SHAPES, "Image and Shapes"},
    {GoomStates::IMAGE_SHAPES_L_SYSTEM, "Image, Shapes and L"},
    {GoomStates::IMAGE_SHAPES_STARS, "Image, Shapes and Stars"},
    {GoomStates::IMAGE_SHAPES_TUBES, "Image, Shapes and tubes"},
    {GoomStates::IMAGE_STARS, "Image and Stars"},
    {GoomStates::IMAGE_STARS_L_SYSTEM, "Image, Stars, L"},
    {GoomStates::IMAGE_TENTACLES, "Image and Tentacles"},
    {GoomStates::IMAGE_TUBES, "Image and Tubes"},

    {GoomStates::L_SYSTEM_ONLY, "L-System Only"},

    {GoomStates::LINES_ONLY, "Lines Only"},
    {GoomStates::LINES_PARTICLES, "Lines and Particles"},
    {GoomStates::LINES_RAINDROPS, "Lines and Raindrops"},
    {GoomStates::LINES_SHAPES_STARS, "Lines, Shapes and Stars"},
    {GoomStates::LINES_STARS, "Lines and Stars"},
    {GoomStates::LINES_TENTACLES, "Lines and Tentacles"},

    {GoomStates::PARTICLES_ONLY, "Particles Only"},
    {GoomStates::PARTICLES_TENTACLES, "Particles and Tentacles"},
    {GoomStates::PARTICLES_TUBES, "Particles and Tubes"},

    {GoomStates::RAINDROPS_ONLY, "Raindrops Only"},
    {GoomStates::SHAPES_ONLY, "Shapes Only"},
    {GoomStates::SHAPES_STARS, "Shapes and Stars"},
    {GoomStates::SHAPES_TUBES, "Shapes and Tubes"},
    {GoomStates::STARS_ONLY, "Stars Only"},
    {GoomStates::TENTACLES_ONLY, "Tentacles Only"},
    {GoomStates::TUBES_ONLY, "Tubes Only"},
}}};

[[nodiscard]] inline auto GetStateDrawables() noexcept
    -> const EnumMap<GoomStates, std::vector<GoomDrawables>>&
{
  // clang-format off
  static const auto s_STATE_DRAWABLES = EnumMap<GoomStates, std::vector<GoomDrawables>>{{{
      {GoomStates::CIRCLES_ONLY,                {GoomDrawables::CIRCLES}},
      {GoomStates::CIRCLES_IFS,                 {GoomDrawables::CIRCLES, GoomDrawables::IFS}},
      {GoomStates::CIRCLES_IMAGE,               {GoomDrawables::CIRCLES, GoomDrawables::IMAGE}},
      {GoomStates::CIRCLES_IMAGE_STARS,         {GoomDrawables::CIRCLES, GoomDrawables::IMAGE,
                                                 GoomDrawables::STARS}},
      {GoomStates::CIRCLES_IMAGE_STARS_L_SYSTEM,
                                                {GoomDrawables::CIRCLES, GoomDrawables::IMAGE,
                                                 GoomDrawables::STARS, GoomDrawables::L_SYSTEM}},
      {GoomStates::CIRCLES_LINES,               {GoomDrawables::CIRCLES, GoomDrawables::LINES}},
      {GoomStates::CIRCLES_STARS_TUBES,         {GoomDrawables::CIRCLES, GoomDrawables::STARS,
                                                 GoomDrawables::TUBES}},
      {GoomStates::CIRCLES_TENTACLES,           {GoomDrawables::CIRCLES, GoomDrawables::TENTACLES}},
      {GoomStates::DOTS_IFS,                    {GoomDrawables::DOTS, GoomDrawables::IFS}},
      {GoomStates::DOTS_IFS_RAINDROPS,          {GoomDrawables::DOTS, GoomDrawables::IFS,
                                                 GoomDrawables::RAINDROPS}},
      {GoomStates::DOTS_IFS_STARS,              {GoomDrawables::DOTS, GoomDrawables::IFS,
                                                 GoomDrawables::STARS}},
      {GoomStates::DOTS_IMAGE_RAINDROPS,        {GoomDrawables::DOTS, GoomDrawables::IMAGE,
                                                 GoomDrawables::RAINDROPS}},
      {GoomStates::DOTS_IMAGE_RAINDROPS_STARS,  {GoomDrawables::DOTS, GoomDrawables::IMAGE,
                                                 GoomDrawables::RAINDROPS, GoomDrawables::STARS}},
      {GoomStates::DOTS_IMAGE_STARS,            {GoomDrawables::DOTS, GoomDrawables::IMAGE,
                                                 GoomDrawables::STARS}},
      {GoomStates::DOTS_IMAGE_STARS_L_SYSTEM,   {GoomDrawables::DOTS, GoomDrawables::IMAGE,
                                                 GoomDrawables::STARS, GoomDrawables::L_SYSTEM}},
      {GoomStates::DOTS_LINES,                  {GoomDrawables::DOTS, GoomDrawables::LINES}},
      {GoomStates::DOTS_LINES_RAINDROPS_STARS,  {GoomDrawables::DOTS, GoomDrawables::LINES,
                                                 GoomDrawables::RAINDROPS, GoomDrawables::STARS}},
      {GoomStates::DOTS_LINES_STARS_TENTACLES,  {GoomDrawables::DOTS, GoomDrawables::LINES,
                                                 GoomDrawables::STARS, GoomDrawables::TENTACLES}},
      {GoomStates::DOTS_LINES_TENTACLES_TUBES,  {GoomDrawables::DOTS, GoomDrawables::LINES,
                                                 GoomDrawables::TENTACLES, GoomDrawables::TUBES}},
      {GoomStates::DOTS_LINES_TUBES,            {GoomDrawables::DOTS, GoomDrawables::LINES,
                                                 GoomDrawables::TUBES}},
      {GoomStates::DOTS_RAINDROPS_STARS,        {GoomDrawables::DOTS, GoomDrawables::RAINDROPS,
                                                 GoomDrawables::STARS}},
      {GoomStates::DOTS_ONLY,                   {GoomDrawables::DOTS}},
      {GoomStates::DOTS_STARS,                  {GoomDrawables::DOTS, GoomDrawables::STARS}},
      {GoomStates::DOTS_STARS_L_SYSTEM,         {GoomDrawables::DOTS, GoomDrawables::STARS,
                                                 GoomDrawables::L_SYSTEM}},
      {GoomStates::DOTS_STARS_TENTACLES_TUBES,  {GoomDrawables::DOTS, GoomDrawables::STARS,
                                                 GoomDrawables::TENTACLES, GoomDrawables::TUBES}},
      {GoomStates::DOTS_TENTACLES_TUBES,        {GoomDrawables::DOTS, GoomDrawables::TENTACLES,
                                                 GoomDrawables::TUBES}},
      {GoomStates::IFS_IMAGE,                   {GoomDrawables::IFS, GoomDrawables::IMAGE}},
      {GoomStates::IFS_IMAGE_RAINDROPS_SHAPES,  {GoomDrawables::IFS, GoomDrawables::IMAGE,
                                                 GoomDrawables::RAINDROPS, GoomDrawables::SHAPES}},
      {GoomStates::IFS_IMAGE_SHAPES,            {GoomDrawables::IFS, GoomDrawables::IMAGE,
                                                 GoomDrawables::SHAPES}},
      {GoomStates::IFS_LINES_RAINDROPS_STARS,   {GoomDrawables::IFS, GoomDrawables::LINES,
                                                 GoomDrawables::RAINDROPS, GoomDrawables::STARS}},
      {GoomStates::IFS_LINES_STARS,             {GoomDrawables::IFS, GoomDrawables::LINES,
                                                 GoomDrawables::STARS}},
      {GoomStates::IFS_ONLY,                    {GoomDrawables::IFS}},
      {GoomStates::IFS_PARTICLES,               {GoomDrawables::IFS, GoomDrawables::PARTICLES}},
      {GoomStates::IFS_RAINDROPS,               {GoomDrawables::IFS, GoomDrawables::RAINDROPS}},
      {GoomStates::IFS_RAINDROPS_SHAPES,        {GoomDrawables::IFS, GoomDrawables::RAINDROPS,
                                                 GoomDrawables::SHAPES}},
      {GoomStates::IFS_RAINDROPS_STARS,         {GoomDrawables::IFS, GoomDrawables::RAINDROPS,
                                                 GoomDrawables::STARS}},
      {GoomStates::IFS_SHAPES,                  {GoomDrawables::IFS, GoomDrawables::SHAPES}},
      {GoomStates::IFS_STARS,                   {GoomDrawables::IFS, GoomDrawables::STARS}},
      {GoomStates::IFS_STARS_TENTACLES,         {GoomDrawables::IFS, GoomDrawables::STARS,
                                                 GoomDrawables::TENTACLES}},
      {GoomStates::IFS_TENTACLES,               {GoomDrawables::IFS, GoomDrawables::TENTACLES}},
      {GoomStates::IFS_TENTACLES_TUBES,         {GoomDrawables::IFS, GoomDrawables::TENTACLES,
                                                 GoomDrawables::TUBES}},
      {GoomStates::IFS_TUBES,                   {GoomDrawables::IFS, GoomDrawables::TUBES}},
      {GoomStates::IMAGE_LINES,                 {GoomDrawables::IMAGE, GoomDrawables::LINES}},
      {GoomStates::IMAGE_LINES_RAINDROPS,       {GoomDrawables::IMAGE, GoomDrawables::LINES,
                                                 GoomDrawables::RAINDROPS}},
      {GoomStates::IMAGE_LINES_SHAPES,          {GoomDrawables::IMAGE, GoomDrawables::LINES,
                                                 GoomDrawables::SHAPES}},
      {GoomStates::IMAGE_LINES_STARS_TENTACLES, {GoomDrawables::IMAGE, GoomDrawables::LINES,
                                                 GoomDrawables::STARS, GoomDrawables::TENTACLES}},
      {GoomStates::IMAGE_ONLY,                  {GoomDrawables::IMAGE}},
      {GoomStates::IMAGE_RAINDROPS,             {GoomDrawables::IMAGE, GoomDrawables::RAINDROPS}},
      {GoomStates::IMAGE_RAINDROPS_SHAPES,      {GoomDrawables::IMAGE, GoomDrawables::RAINDROPS,
                                                 GoomDrawables::SHAPES}},
      {GoomStates::IMAGE_SHAPES,                {GoomDrawables::IMAGE, GoomDrawables::SHAPES}},
      {GoomStates::IMAGE_SHAPES_L_SYSTEM,       {GoomDrawables::IMAGE, GoomDrawables::SHAPES,
                                                 GoomDrawables::L_SYSTEM}},
      {GoomStates::IMAGE_SHAPES_STARS,          {GoomDrawables::IMAGE, GoomDrawables::SHAPES,
                                                 GoomDrawables::STARS}},
      {GoomStates::IMAGE_SHAPES_TUBES,          {GoomDrawables::IMAGE, GoomDrawables::SHAPES,
                                                 GoomDrawables::TUBES}},
      {GoomStates::IMAGE_STARS,                 {GoomDrawables::IMAGE, GoomDrawables::STARS}},
      {GoomStates::IMAGE_STARS_L_SYSTEM,        {GoomDrawables::IMAGE, GoomDrawables::STARS,
                                                 GoomDrawables::L_SYSTEM}},
      {GoomStates::IMAGE_TENTACLES,             {GoomDrawables::IMAGE, GoomDrawables::TENTACLES}},
      {GoomStates::IMAGE_TUBES,                 {GoomDrawables::IMAGE, GoomDrawables::TUBES}},
      {GoomStates::L_SYSTEM_ONLY,               {GoomDrawables::L_SYSTEM}},
      {GoomStates::LINES_ONLY,                  {GoomDrawables::LINES}},
      {GoomStates::LINES_PARTICLES,             {GoomDrawables::LINES, GoomDrawables::PARTICLES}},
      {GoomStates::LINES_RAINDROPS,             {GoomDrawables::LINES, GoomDrawables::RAINDROPS}},
      {GoomStates::LINES_SHAPES_STARS,          {GoomDrawables::LINES, GoomDrawables::SHAPES,
                                                 GoomDrawables::STARS}},
      {GoomStates::LINES_STARS,                 {GoomDrawables::LINES, GoomDrawables::STARS}},
      {GoomStates::LINES_TENTACLES,             {GoomDrawables::LINES, GoomDrawables::TENTACLES}},
      {GoomStates::PARTICLES_ONLY,              {GoomDrawables::PARTICLES}},
      {GoomStates::PARTICLES_TENTACLES,         {GoomDrawables::PARTICLES,
                                                 GoomDrawables::TENTACLES}},
      {GoomStates::PARTICLES_TUBES,             {GoomDrawables::PARTICLES, GoomDrawables::TUBES}},
      {GoomStates::RAINDROPS_ONLY,              {GoomDrawables::RAINDROPS}},
      {GoomStates::SHAPES_ONLY,                 {GoomDrawables::SHAPES}},
      {GoomStates::SHAPES_STARS,                {GoomDrawables::SHAPES, GoomDrawables::STARS}},
      {GoomStates::SHAPES_TUBES,                {GoomDrawables::SHAPES, GoomDrawables::TUBES}},
      {GoomStates::STARS_ONLY,                  {GoomDrawables::STARS}},
      {GoomStates::TENTACLES_ONLY,              {GoomDrawables::TENTACLES}},
      {GoomStates::TUBES_ONLY,                  {GoomDrawables::TUBES}},
  }}};
  // clang-format on

  return s_STATE_DRAWABLES;
}

} // namespace

auto GoomStateInfo::GetStateInfoMap() noexcept -> StateInfoMap
{
  static_assert(DEFAULT_BUFF_INTENSITY_RANGES.size() == NUM<GoomDrawables>);
  static_assert(STATE_NAMES.size() == NUM<GoomStates>);
  Expects(GetStateDrawables().size() == NUM<GoomStates>);

  auto statesArray = std::vector<StateInfoMap::KeyValue>{};

  for (auto i = 0U; i < NUM<GoomStates>; ++i)
  {
    const auto goomState = static_cast<GoomStates>(i);
    statesArray.emplace_back(StateInfoMap::KeyValue{
        goomState, {STATE_NAMES[goomState], GetDrawablesInfo(goomState)}
    });
  }

  return StateInfoMap::Make(std::move(statesArray));
}

const GoomStateInfo::StateInfoMap GoomStateInfo::STATE_INFO_MAP = GetStateInfoMap();

auto GoomStateInfo::GetDrawablesInfo(const GoomStates goomState) -> std::vector<DrawableInfo>
{
  auto drawablesInfo = std::vector<DrawableInfo>{};

  for (const auto& drawable : GetStateDrawables()[goomState])
  {
    drawablesInfo.emplace_back(DrawableInfo{drawable, DEFAULT_BUFF_INTENSITY_RANGES[drawable]});
  }

  return drawablesInfo;
}

auto GoomStateInfo::IsMultiThreaded(const GoomStates goomState) -> bool
{
  const auto& goomDrawables = GetStateDrawables()[goomState];

  return std::any_of(cbegin(goomDrawables),
                     cend(goomDrawables),
                     [](const auto& goomDrawable) { return STATE_MULTI_THREADED[goomDrawable]; });
}

auto GoomStateInfo::GetBuffIntensityRange(const GoomStates goomState, const GoomDrawables fx)
    -> BuffIntensityRange
{
  for (const auto& drawableInfo : GetStateInfo(goomState).drawablesInfo)
  {
    if (drawableInfo.fx == fx)
    {
      return drawableInfo.buffIntensityRange;
    }
  }

  FailFast();
}

} // namespace GOOM::CONTROL
