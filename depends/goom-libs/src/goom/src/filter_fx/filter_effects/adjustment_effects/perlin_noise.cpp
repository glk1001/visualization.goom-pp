module;

#include <PerlinNoise.hpp>
#include <cstdint>
#include <limits>
#include <type_traits>

module Goom.FilterFx.FilterEffects.AdjustmentEffects.PerlinNoise;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.Point2d;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using FILTER_UTILS::GetVelocityByZoomLerpedToOne;
using FILTER_UTILS::LerpToOneTs;
using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::MATH::GetRandSeed;
using UTILS::MATH::GOOM_RAND_MAX;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;

static constexpr auto DEFAULT_AMPLITUDE = 0.1F;
static constexpr auto AMPLITUDE_RANGE   = NumberRange{0.05F, 1.5F};

static constexpr auto DEFAULT_LERP_TO_ONE_T_S = LerpToOneTs{.xLerpT = 0.0F, .yLerpT = 0.0F};
static constexpr auto LERP_TO_ONE_T_RANGE     = NumberRange{0.0F, 1.0F};

static constexpr auto DEFAULT_NOISE_FREQUENCY_FACTOR = 1.0F;
static constexpr auto NOISE_FREQUENCY_FACTOR_RANGE   = NumberRange{0.1F, 2.0F};

static constexpr auto DEFAULT_ANGLE_FREQUENCY_FACTOR = 1.0F;
static constexpr auto ANGLE_FREQUENCY_FACTOR_RANGE   = NumberRange{0.1F, 2.0F};

static constexpr auto DEFAULT_OCTAVES = 1;
static constexpr auto OCTAVES_RANGE   = NumberRange{1, 5};

static constexpr auto DEFAULT_PERSISTENCE = 1.0F;
static constexpr auto PERSISTENCE_RANGE   = NumberRange{0.1F, 1.0F};

static constexpr auto PROB_XY_AMPLITUDES_EQUAL        = 0.98F;
static constexpr auto PROB_LERP_TO_ONE_T_S_EQUAL      = 0.95F;
static constexpr auto PROB_XY_NOISE_FREQUENCIES_EQUAL = 0.5F;
static constexpr auto PROB_XY_ANGLE_FREQUENCIES_EQUAL = 0.5F;

PerlinNoise::PerlinNoise(const GoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_params{.amplitude={DEFAULT_AMPLITUDE, DEFAULT_AMPLITUDE},
             .lerpToOneTs=DEFAULT_LERP_TO_ONE_T_S,
             .noiseFrequencyFactor={.x=DEFAULT_NOISE_FREQUENCY_FACTOR, .y=DEFAULT_NOISE_FREQUENCY_FACTOR},
             .angleFrequencyFactor={.x=DEFAULT_ANGLE_FREQUENCY_FACTOR, .y=DEFAULT_ANGLE_FREQUENCY_FACTOR},
             .octaves=DEFAULT_OCTAVES,
             .persistence=DEFAULT_PERSISTENCE},
    m_perlinNoise{GetRandSeedForPerlinNoise()},
    m_perlinNoise2{GetRandSeedForPerlinNoise()}
{
}

auto PerlinNoise::GetRandSeedForPerlinNoise() -> PerlinSeedType
{
  if constexpr (std::is_same_v<PerlinSeedType, decltype(GetRandSeed())>)
  {
    return GetRandSeed();
  }

#if defined(__GNUC__) and not defined(__clang_major__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
  return static_cast<PerlinSeedType>(GetRandSeed() % std::numeric_limits<PerlinSeedType>::max());
#if defined(__GNUC__) and not defined(__clang_major__)
#pragma GCC diagnostic pop
#endif
}

auto PerlinNoise::SetRandomParams() noexcept -> void
{
  m_perlinNoise.reseed(m_goomRand->GetNRand(GOOM_RAND_MAX));
  m_perlinNoise2.reseed(m_goomRand->GetNRand(GOOM_RAND_MAX));

  const auto xAmplitude = m_goomRand->GetRandInRange<AMPLITUDE_RANGE>();
  const auto yAmplitude = m_goomRand->ProbabilityOf<PROB_XY_AMPLITUDES_EQUAL>()
                              ? xAmplitude
                              : m_goomRand->GetRandInRange<AMPLITUDE_RANGE>();

  const auto xLerpToOneT = m_goomRand->GetRandInRange<LERP_TO_ONE_T_RANGE>();
  const auto yLerpToOneT = m_goomRand->ProbabilityOf<PROB_LERP_TO_ONE_T_S_EQUAL>()
                               ? xLerpToOneT
                               : m_goomRand->GetRandInRange<LERP_TO_ONE_T_RANGE>();

  const auto xNoiseFrequencyFactor = m_goomRand->GetRandInRange<NOISE_FREQUENCY_FACTOR_RANGE>();
  const auto yNoiseFrequencyFactor =
      m_goomRand->ProbabilityOf<PROB_XY_NOISE_FREQUENCIES_EQUAL>()
          ? xNoiseFrequencyFactor
          : m_goomRand->GetRandInRange<NOISE_FREQUENCY_FACTOR_RANGE>();

  const auto xAngleFrequencyFactor = m_goomRand->GetRandInRange<ANGLE_FREQUENCY_FACTOR_RANGE>();
  const auto yAngleFrequencyFactor =
      m_goomRand->ProbabilityOf<PROB_XY_ANGLE_FREQUENCIES_EQUAL>()
          ? xAngleFrequencyFactor
          : m_goomRand->GetRandInRange<ANGLE_FREQUENCY_FACTOR_RANGE>();

  const auto octaves = m_goomRand->GetRandInRange<OCTAVES_RANGE>();

  const auto persistence = m_goomRand->GetRandInRange<PERSISTENCE_RANGE>();

  SetParams({
      .amplitude            = {                xAmplitude,                 yAmplitude},
      .lerpToOneTs          = {     .xLerpT = xLerpToOneT,      .yLerpT = yLerpToOneT},
      .noiseFrequencyFactor = {.x = xNoiseFrequencyFactor, .y = yNoiseFrequencyFactor},
      .angleFrequencyFactor = {.x = xAngleFrequencyFactor, .y = yAngleFrequencyFactor},
      .octaves              = octaves,
      .persistence          = persistence
  });
}

auto PerlinNoise::GetZoomAdjustment(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto velocity = GetVelocity(coords);

  //return velocity;

  return GetVelocityByZoomLerpedToOne(coords, m_params.lerpToOneTs, velocity);

  //const auto absCoord = std::sqrt(SqDistanceFromZero(coords));
  //if (absCoord < SMALL_FLOAT)
  //{
  //  return {0.0F, 0.0F};
  //}
  //return {coords.GetX()/absCoord * velocity.x, coords.GetY()/absCoord * velocity.y};
}

auto PerlinNoise::GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  static constexpr auto MAX_SQ_DIST_FROM_ZERO =
      SqDistanceFromZero({NormalizedCoords::MAX_COORD, NormalizedCoords::MAX_COORD});
  const auto sqDistFromZeroFrac = SqDistanceFromZero(coords) / MAX_SQ_DIST_FROM_ZERO;

  const auto xFreq = (0.1F + sqDistFromZeroFrac) * m_params.noiseFrequencyFactor.x;
  const auto yFreq = (0.1F + sqDistFromZeroFrac) * m_params.noiseFrequencyFactor.y;

  const auto xNoise = m_perlinNoise.octave2D_11(
      xFreq * coords.GetX(), yFreq * coords.GetY(), m_params.octaves, m_params.persistence);
  const auto yNoise = m_perlinNoise2.octave2D_11(
      xFreq * coords.GetX(), yFreq * coords.GetY(), m_params.octaves, m_params.persistence);
  //  const auto xNoise = std::cos(m_params.angleFrequencyFactor.x * angle);
  //  const auto yNoise = std::cos(m_params.angleFrequencyFactor.y * angle);

  return {.x = (m_params.amplitude.x * xNoise), .y = (m_params.amplitude.y * yNoise)};
}

auto PerlinNoise::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  const auto fullParamGroup = GetFullParamGroup({PARAM_GROUP, "perlin noise"});
  return {
      GetPair(fullParamGroup,
              "amplitude",
              Point2dFlt{.x = m_params.amplitude.x, .y = m_params.amplitude.y}),
      GetPair(fullParamGroup,
              "lerpToOneTs",
              Point2dFlt{.x = m_params.lerpToOneTs.xLerpT, .y = m_params.lerpToOneTs.yLerpT}),
      GetPair(
          fullParamGroup,
          "angle freq fac",
          Point2dFlt{.x = m_params.angleFrequencyFactor.x, .y = m_params.angleFrequencyFactor.y}),
      GetPair(
          fullParamGroup,
          "noise freq fac",
          Point2dFlt{.x = m_params.noiseFrequencyFactor.x, .y = m_params.noiseFrequencyFactor.y}),
      GetPair(fullParamGroup, "octaves", m_params.octaves),
      GetPair(fullParamGroup, "persistence", m_params.persistence),
  };
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
