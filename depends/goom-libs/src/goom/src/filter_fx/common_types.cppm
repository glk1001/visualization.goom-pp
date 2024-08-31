module;

#include <cstdint>

export module Goom.FilterFx.CommonTypes;

import Goom.Utils.Math.GoomRand;

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

struct XYPairRange
{
  UTILS::MATH::NumberRange<float> xRange;
  UTILS::MATH::NumberRange<float> yRange;
};

using AmplitudeRange       = XYPairRange;
using FrequencyFactorRange = XYPairRange;
using SqDistMultRange      = XYPairRange;
using SqDistOffsetRange    = XYPairRange;
using FilterBaseRange      = XYPairRange;

} // namespace GOOM::FILTER_FX
