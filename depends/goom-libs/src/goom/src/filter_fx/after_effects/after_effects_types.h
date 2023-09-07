#pragma once

#include "goom/goom_types.h"

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

enum class AfterEffectsTypes : UnderlyingEnumType
{
  HYPERCOS,
  IMAGE_VELOCITY,
  NOISE,
  PLANES,
  ROTATION,
  TAN_EFFECT,
  XY_LERP_EFFECT,
  _num // unused, and marks the enum end
};

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
