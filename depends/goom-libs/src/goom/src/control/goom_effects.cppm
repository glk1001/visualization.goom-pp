module;

export module Goom.Control.GoomEffects;

import Goom.Lib.GoomTypes;

export namespace GOOM::CONTROL
{

enum class GoomEffect : UnderlyingEnumType
{
  CIRCLES_MAIN = 0,
  CIRCLES_LOW,
  DOTS0,
  DOTS1,
  DOTS2,
  DOTS3,
  DOTS4,
  IFS,
  IMAGE,
  L_SYSTEM_MAIN,
  L_SYSTEM_LOW,
  LINES1,
  LINES2,
  PARTICLES_MAIN,
  PARTICLES_LOW,
  RAINDROPS_MAIN,
  RAINDROPS_LOW,
  SHAPES_MAIN,
  SHAPES_LOW,
  SHAPES_INNER,
  STARS_MAIN_FIREWORKS,
  STARS_LOW_FIREWORKS,
  STARS_MAIN_RAIN,
  STARS_LOW_RAIN,
  STARS_MAIN_FOUNTAIN,
  STARS_LOW_FOUNTAIN,
  TENTACLES_DOMINANT_MAIN,
  TENTACLES_DOMINANT_LOW,
  TENTACLES_MAIN,
  TENTACLES_LOW,
  TUBE_MAIN,
  TUBE_LOW,
};

} // namespace GOOM::CONTROL