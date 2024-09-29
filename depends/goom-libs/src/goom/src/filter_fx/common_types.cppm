module;

#include <cstdint>

export module Goom.FilterFx.CommonTypes;

import Goom.Utils.Math.GoomRand;

using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::NumberRange;

export namespace GOOM::FILTER_FX
{

template<typename T>
struct XYPair_t // NOLINT(readability-identifier-naming)
{
  T x;
  T y;
};
using Amplitude       = XYPair_t<float>;
using IntAmplitude    = XYPair_t<int32_t>;
using FrequencyFactor = XYPair_t<float>;
using SqDistMult      = XYPair_t<float>;
using SqDistOffset    = XYPair_t<float>;
using FilterBase      = XYPair_t<float>;

template<typename T>
auto GetRandomXYPair(const GoomRand& goomRand,
                     const NumberRange<T>& minMaxRange,
                     float probXYValuesEqual) noexcept -> XYPair_t<T>;

struct XYPairRange
{
  NumberRange<float> xRange;
  NumberRange<float> yRange;
};

using AmplitudeRange       = XYPairRange;
using FrequencyFactorRange = XYPairRange;
using SqDistMultRange      = XYPairRange;
using SqDistOffsetRange    = XYPairRange;
using FilterBaseRange      = XYPairRange;

} // namespace GOOM::FILTER_FX

namespace GOOM::FILTER_FX
{

template<typename T>
auto GetRandomXYPair(const GoomRand& goomRand,
                     const NumberRange<T>& minMaxRange,
                     const float probXYValuesEqual) noexcept -> XYPair_t<T>
{
  const auto xValue = goomRand.GetRandInRange(minMaxRange);
  const auto yValue =
      goomRand.ProbabilityOf(probXYValuesEqual) ? xValue : goomRand.GetRandInRange(minMaxRange);

  return {xValue, yValue};
}

} // namespace GOOM::FILTER_FX
