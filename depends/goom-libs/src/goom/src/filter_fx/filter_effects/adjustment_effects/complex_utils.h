#pragma once

#include "filter_fx/common_types.h"

#include <complex>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using FltCalcType         = double;
inline constexpr auto ONE = static_cast<FltCalcType>(1.0F);

[[nodiscard]] auto GetAdjustedPhase(const Amplitude& amplitude,
                                    bool noInverseSquare,
                                    const std::complex<FltCalcType>& fz,
                                    float sqDistFromZero) noexcept -> std::complex<FltCalcType>;
[[nodiscard]] auto GetModulatedPhase(FltCalcType absSqFz,
                                     const std::complex<FltCalcType>& phase,
                                     float modulatorPeriod) noexcept -> std::complex<FltCalcType>;

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
