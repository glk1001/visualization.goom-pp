module;

#include <PerlinNoise.hpp>
#include <cmath>
#include <mdspan.hpp>

module Goom.FilterFx.FilterEffects.AdjustmentEffects.FlowField;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Lib.AssertUtils;
import Goom.Lib.Point2d;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{
using UTILS::NameValuePairs;
using UTILS::MATH::GetRandSeed;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;
using UTILS::MATH::Sq;
using UTILS::MATH::TWO_PI;

namespace
{

using PerlinSeedType = siv::BasicPerlinNoise<float>::seed_type;

auto GetRandSeedForPerlinNoise() -> PerlinSeedType
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

constexpr auto DEFAULT_AMPLITUDE = Amplitude{0.1F, 0.1F};
constexpr auto AMPLITUDE_RANGE   = NumberRange{0.05F, 0.251F};

constexpr auto DEFAULT_LERP_TO_ONE_T_S = LerpToOneTs{.xLerpT = 0.5F, .yLerpT = 0.5F};
constexpr auto LERP_TO_ONE_T_RANGE     = NumberRange{0.0F, 1.0F};

constexpr auto DEFAULT_NOISE_FREQUENCY_FACTOR = 0.01F;
constexpr auto NOISE_FREQUENCY_FACTOR_RANGE   = NumberRange{0.005F, 0.05F};

constexpr auto DEFAULT_ANGLE_FREQUENCY_FACTOR = 5.0F;
constexpr auto ANGLE_FREQUENCY_FACTOR_RANGE   = NumberRange{0.1F, 20.0F};

constexpr auto DEFAULT_OCTAVES = 1;
constexpr auto OCTAVES_RANGE   = NumberRange{1, 5};

constexpr auto DEFAULT_PERSISTENCE = 0.5F;
constexpr auto PERSISTENCE_RANGE   = NumberRange{0.1F, 1.0F};

constexpr auto DEFAULT_NOISE_FACTOR = 0.5F;
constexpr auto NOISE_FACTOR_RANGE   = NumberRange{0.0F, 1.0F};

constexpr auto PROB_XY_AMPLITUDES_EQUAL        = 0.98F;
constexpr auto PROB_LERP_TO_ONE_T_S_EQUAL      = 0.95F;
constexpr auto PROB_XY_NOISE_FREQUENCIES_EQUAL = 0.5F;
constexpr auto PROB_XY_ANGLE_FREQUENCIES_EQUAL = 0.5F;
constexpr auto PROB_OCTAVES_EQUAL              = 0.5F;
constexpr auto PROB_PERSISTENCE_EQUAL          = 0.5F;

} // namespace

FlowField::FlowField(const GoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_perlinNoise{GetRandSeedForPerlinNoise()},
    m_perlinNoise2{GetRandSeedForPerlinNoise()},
    m_params{
      .amplitude   = DEFAULT_AMPLITUDE,
      .lerpToOneTs = DEFAULT_LERP_TO_ONE_T_S,
      .noiseFrequencyFactor ={.x=DEFAULT_NOISE_FREQUENCY_FACTOR, .y=DEFAULT_NOISE_FREQUENCY_FACTOR},
      .angleFrequencyFactor ={.x=DEFAULT_ANGLE_FREQUENCY_FACTOR, .y=DEFAULT_ANGLE_FREQUENCY_FACTOR},
      .octaves1     = DEFAULT_OCTAVES,
      .persistence1 = DEFAULT_PERSISTENCE,
      .octaves2     = DEFAULT_OCTAVES,
      .persistence2 = DEFAULT_PERSISTENCE,
      .noiseFactor  = DEFAULT_NOISE_FACTOR}
{
  SetupAngles();
}

auto FlowField::SetupAngles() noexcept -> void
{
  auto gridAngles = std::mdspan{m_gridArray.data(), GRID_HEIGHT, GRID_WIDTH};

  Expects(gridAngles.extent(0) == GRID_HEIGHT);
  Expects(gridAngles.extent(1) == GRID_WIDTH);

  for (auto col = 0U; col < GRID_WIDTH; ++col)
  {
    const auto x = 2.0F * (-0.5F + (static_cast<float>(col) / static_cast<float>(GRID_WIDTH)));

    for (auto row = 0U; row < GRID_HEIGHT; ++row)
    {
      const auto y = 2.0F * (-0.5F + (static_cast<float>(row) / static_cast<float>(GRID_HEIGHT)));

      const auto distFromCentre = std::sqrt(Sq(x) + Sq(y));

      const auto xFreq = (0.1F + distFromCentre) * m_params.noiseFrequencyFactor.x;
      const auto yFreq = (0.1F + distFromCentre) * m_params.noiseFrequencyFactor.y;
      //const auto xFreq = m_params.noiseFrequencyFactor.x;
      //const auto yFreq = m_params.noiseFrequencyFactor.y;
      // const auto xFreq = 0.01F;
      // const auto yFreq  = 0.01F;

      const auto xNoise = m_perlinNoise.octave2D_11(xFreq * static_cast<float>(col),
                                                    xFreq * static_cast<float>(row),
                                                    m_params.octaves1,
                                                    m_params.persistence1);
      const auto yNoise = m_perlinNoise2.octave2D_11(yFreq * static_cast<float>(col),
                                                     yFreq * static_cast<float>(row),
                                                     m_params.octaves2,
                                                     m_params.persistence2);

      // const auto xNoise = m_perlinNoise.octave2D_11(
      //     xFreq * static_cast<float>(col), xFreq * static_cast<float>(row), 5, 0.5F);
      // const auto yNoise = m_perlinNoise2.octave2D_11(
      //     yFreq * static_cast<float>(col), yFreq * static_cast<float>(row), 2, 1.0F);

      const auto noise = m_params.noiseFactor * (0.5F * (xNoise + yNoise));
      auto angle       = (1.0F - noise) * (distFromCentre * TWO_PI);
      //angle = std::floor(5.0F * angle) / 5.0F;

      gridAngles[row, col] = angle;
    }
  }
}

auto FlowField::GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto gridCoords =
      ToPoint2dInt(m_normalizedCoordsToGridConverter.NormalizedToOtherCoordsFlt(coords));

  const auto gridAngles = std::mdspan{m_gridArray.data(), GRID_HEIGHT, GRID_WIDTH};
  const auto gridAngle  = gridAngles[std::clamp(gridCoords.y, 0, static_cast<int>(GRID_HEIGHT) - 1),
                                    std::clamp(gridCoords.x, 0, static_cast<int>(GRID_WIDTH) - 1)];

  const auto sqDistFromZero = std::sqrt(SqDistanceFromZero(coords));

  return {.x = GetBaseZoomAdjustment().x + (m_params.amplitude.x * sqDistFromZero *
                                            std::cos(m_params.angleFrequencyFactor.x * gridAngle)),
          .y = GetBaseZoomAdjustment().y + (m_params.amplitude.y * sqDistFromZero *
                                            std::sin(m_params.angleFrequencyFactor.y * gridAngle))};

  // return {coords.GetX() * (m_params.amplitude.x * sqDistFromZero*std::cos(15.0F * gridAngle)),
  //         coords.GetY() * + (m_params.amplitude.y * sqDistFromZero*std::sin(25.0F * gridAngle))};
}

auto FlowField::SetRandomParams() noexcept -> void
{
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

  const auto octaves1 = m_goomRand->GetRandInRange<OCTAVES_RANGE>();
  const auto octaves2 = m_goomRand->ProbabilityOf<PROB_OCTAVES_EQUAL>()
                            ? octaves1
                            : m_goomRand->GetRandInRange<OCTAVES_RANGE>();

  const auto persistence1 = m_goomRand->GetRandInRange<PERSISTENCE_RANGE>();
  const auto persistence2 = m_goomRand->ProbabilityOf<PROB_PERSISTENCE_EQUAL>()
                                ? persistence1
                                : m_goomRand->GetRandInRange<PERSISTENCE_RANGE>();

  const auto noiseFactor = m_goomRand->GetRandInRange<NOISE_FACTOR_RANGE>();

  SetParams({
      .amplitude            = {                xAmplitude,                 yAmplitude},
      .lerpToOneTs          = {     .xLerpT = xLerpToOneT,      .yLerpT = yLerpToOneT},
      .noiseFrequencyFactor = {.x = xNoiseFrequencyFactor, .y = yNoiseFrequencyFactor},
      .angleFrequencyFactor = {.x = xAngleFrequencyFactor, .y = yAngleFrequencyFactor},
      .octaves1             = octaves1,
      .persistence1         = persistence1,
      .octaves2             = octaves2,
      .persistence2         = persistence2,
      .noiseFactor          = noiseFactor,
  });
}

auto FlowField::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  return {};
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
