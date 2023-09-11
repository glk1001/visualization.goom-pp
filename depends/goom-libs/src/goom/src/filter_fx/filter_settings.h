#pragma once

#include "after_effects/after_effects_states.h"
#include "filter_speed.h"
#include "goom/goom_lerp_data.h"
#include "goom/point2d.h"
#include "normalized_coords.h"

#include <memory>

namespace GOOM::FILTER_FX
{

class IZoomAdjustmentEffect;

struct FilterMultiplierEffectsSettings
{
  bool isActive                    = false;
  float xFreq                      = 1.0F;
  float yFreq                      = 1.0F;
  float xAmplitude                 = 1.0F;
  float yAmplitude                 = 1.0F;
  float lerpZoomAdjustmentToCoords = 1.0F;
};

struct FilterEffectsSettings
{
  Vitesse vitesse;

  float maxZoomAdjustment;
  float baseZoomAdjustmentFactorMultiplier;
  float afterEffectsVelocityMultiplier;
  std::shared_ptr<IZoomAdjustmentEffect> zoomAdjustmentEffect;

  Point2dInt zoomMidpoint;
  Viewport filterViewport{};

  FilterMultiplierEffectsSettings filterMultiplierEffectsSettings;
  AFTER_EFFECTS::AfterEffectsStates::AfterEffectsSettings afterEffectsSettings;
};

struct FilterSettings
{
  bool filterEffectsSettingsHaveChanged = false;
  FilterEffectsSettings filterEffectsSettings{};
  GoomLerpData transformBufferLerpData{};
};

} // namespace GOOM::FILTER_FX
