module;

#include <memory>

export module Goom.FilterFx.GpuFilterEffects.GpuZoomFilterEffectFactory;

import Goom.FilterFx.GpuFilterEffects.GpuZoomFilterEffect;
import Goom.FilterFx.FilterModes;
import Goom.Utils.Math.GoomRand;

using GOOM::UTILS::MATH::GoomRand;

export namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
{

[[nodiscard]] auto CreateGpuZoomFilterEffect(GpuZoomFilterMode gpuFilterMode,
                                             const GoomRand& goomRand)
    -> std::unique_ptr<IGpuZoomFilterEffect>;

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
