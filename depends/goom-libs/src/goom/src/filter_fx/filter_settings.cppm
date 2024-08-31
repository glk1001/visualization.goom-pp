module;

#include <memory>

export module Goom.FilterFx.FilterSettings;

import Goom.FilterFx.AfterEffects.AfterEffectsStates;
import Goom.FilterFx.FilterUtils.GoomLerpData;
import Goom.FilterFx.FilterSpeed;
import Goom.FilterFx.FilterEffects.ZoomAdjustmentEffect;
import Goom.FilterFx.GpuFilterEffects.GpuZoomFilterEffect;
import Goom.Lib.Point2d;

using GOOM::FILTER_FX::AFTER_EFFECTS::AfterEffectsStates;
using GOOM::FILTER_FX::FILTER_EFFECTS::IZoomAdjustmentEffect;
using GOOM::FILTER_FX::FILTER_UTILS::GoomLerpData;
using GOOM::FILTER_FX::GPU_FILTER_EFFECTS::IGpuZoomFilterEffect;

export namespace GOOM::FILTER_FX
{

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

  FilterMultiplierEffectsSettings filterMultiplierEffectsSettings;
  AfterEffectsStates::AfterEffectsSettings afterEffectsSettings;
};

struct GpuFilterEffectsSettings
{
  std::shared_ptr<IGpuZoomFilterEffect> gpuZoomFilterEffect;
};

struct FilterSettings
{
  bool filterEffectsSettingsHaveChanged = false;
  FilterEffectsSettings filterEffectsSettings{};
  bool gpuFilterEffectsSettingsHaveChanged = false;
  GpuFilterEffectsSettings gpuFilterEffectsSettings{};
  GoomLerpData transformBufferLerpData;
};

} // namespace GOOM::FILTER_FX
