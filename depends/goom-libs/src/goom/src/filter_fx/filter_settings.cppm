module;

#include <cstdint>
#include <functional>
#include <memory>

export module Goom.FilterFx.FilterSettings;

import Goom.FilterFx.AfterEffects.AfterEffectsStates;
import Goom.FilterFx.FilterUtils.GoomLerpData;
import Goom.FilterFx.FilterSpeed;
import Goom.FilterFx.FilterEffects.ZoomAdjustmentEffect;
import Goom.FilterFx.GpuFilterEffects.GpuLerpFactor;
import Goom.FilterFx.GpuFilterEffects.GpuZoomFilterEffect;
import Goom.Utils.Math.Lerper;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;

using GOOM::FILTER_FX::AFTER_EFFECTS::AfterEffectsStates;
using GOOM::FILTER_FX::FILTER_EFFECTS::IZoomAdjustmentEffect;
using GOOM::FILTER_FX::FILTER_UTILS::GoomLerpData;
using GOOM::FILTER_FX::GPU_FILTER_EFFECTS::GpuLerpFactor;
using GOOM::FILTER_FX::GPU_FILTER_EFFECTS::IGpuZoomFilterEffect;
using GOOM::UTILS::MATH::Lerper;

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
  std::function<bool()> okToChangeFilterSettings;

  bool filterZoomMidpointHasChanged = false;
  Point2dInt zoomMidpoint;

  FilterMultiplierEffectsSettings filterMultiplierEffectsSettings;
  AfterEffectsStates::AfterEffectsSettings afterEffectsSettings;
};

struct GpuFilterEffectsSettings
{
  std::shared_ptr<IGpuZoomFilterEffect> gpuZoomFilterEffect;
  int32_t maxTimeToNextFilterModeChange;
  std::function<bool()> okToChangeGpuFilterSettings;
  GpuLerpFactor gpuLerpFactor{};
  Lerper<float> srceDestLerpFactor{};
  Lerper<Point2dFlt> midpoint{};
};

enum class TextureWrapType : UnderlyingEnumType
{
  REPEAT,
  MIRRORED_REPEAT,
  CLAMP_TO_EDGE,
  MIRRORED_CLAMP_TO_EDGE,
};

struct FilterSettings
{
  bool filterEffectsSettingsHaveChanged = false;
  FilterEffectsSettings filterEffectsSettings{};
  bool gpuFilterEffectsSettingsHaveChanged = false;
  GpuFilterEffectsSettings gpuFilterEffectsSettings{};
  TextureWrapType textureWrapType{};
  GoomLerpData transformBufferLerpData;
};

} // namespace GOOM::FILTER_FX
