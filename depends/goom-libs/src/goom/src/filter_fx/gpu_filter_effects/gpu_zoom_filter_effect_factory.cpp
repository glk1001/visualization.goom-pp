module;

#include <memory>

module Goom.FilterFx.GpuFilterEffects.GpuZoomFilterEffectFactory;

import Goom.FilterFx.GpuFilterEffects.Amulet;
import Goom.FilterFx.GpuFilterEffects.GpuZoomFilterEffect;
import Goom.FilterFx.GpuFilterEffects.None;
import Goom.FilterFx.GpuFilterEffects.Vortex;
import Goom.FilterFx.GpuFilterEffects.Wave;
import Goom.FilterFx.FilterModes;
import Goom.Utils.Math.GoomRand;

namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
{

using UTILS::MATH::GoomRand;

using enum GpuZoomFilterMode;

auto CreateGpuZoomFilterEffect(const GpuZoomFilterMode gpuFilterMode, const GoomRand& goomRand)
    -> std::unique_ptr<IGpuZoomFilterEffect>
{
  switch (gpuFilterMode)
  {
    case GPU_NONE_MODE:
      return std::make_unique<None>();
    case GPU_AMULET_MODE:
      return std::make_unique<Amulet>(goomRand);
    case GPU_WAVE_MODE:
      return std::make_unique<Wave>(goomRand);
    case GPU_VORTEX_MODE:
      return std::make_unique<Vortex>(goomRand);
    case GPU_REFLECTING_POOL_MODE:
      return std::make_unique<Amulet>(goomRand);
    case GPU_BEAUTIFUL_FIELD_MODE:
      return std::make_unique<Amulet>(goomRand);
  }
}

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
