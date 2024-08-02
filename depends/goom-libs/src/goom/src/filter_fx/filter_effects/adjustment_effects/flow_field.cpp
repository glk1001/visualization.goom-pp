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
using UTILS::MATH::PI;

using PerlinSeedType = siv::BasicPerlinNoise<float>::seed_type;
static auto GetRandSeedForPerlinNoise() -> PerlinSeedType
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

static constexpr auto DEFAULT_AMPLITUDE = 1.0F;
static constexpr auto AMPLITUDE_RANGE   = NumberRange{0.05F, 0.251F};

static constexpr auto PROB_XY_AMPLITUDES_EQUAL = 0.98F;

FlowField::FlowField(const GoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_perlinNoise{GetRandSeedForPerlinNoise()},
    m_perlinNoise2{GetRandSeedForPerlinNoise()},
    m_params{{DEFAULT_AMPLITUDE, DEFAULT_AMPLITUDE}}
{
  SetupAngles();
}

auto FlowField::SetupAngles() noexcept -> void
{
  auto gridAngles = std::mdspan{m_gridArray.data(), GRID_HEIGHT, GRID_WIDTH};

  Expects(gridAngles.extent(0) == GRID_HEIGHT);
  Expects(gridAngles.extent(1) == GRID_WIDTH);

  const auto xFreq = 0.01F;
  const auto yFreq = 0.01F;

  // const auto xNoise = m_perlinNoise.octave2D_11(
  //     xFreq * coords.GetX(), yFreq * coords.GetY(), 1, 1.0F);
  //const auto yNoise = m_perlinNoise2.octave2D_11(
  //    xFreq * coords.GetX(), yFreq * coords.GetY(), 1, 1.0F);

  for (auto col = 0U; col < GRID_WIDTH; ++col)
  {
    const auto x = -0.5F + (static_cast<float>(col) / static_cast<float>(GRID_WIDTH));
    for (auto row = 0U; row < GRID_HEIGHT; ++row)
    {
      const auto xNoise = m_perlinNoise.octave2D_11(
          xFreq * static_cast<float>(col), yFreq * static_cast<float>(row), 2, 0.5F);
      // const auto angle = xNoise * 2.0F * PI;
      const auto y         = -0.5F + (static_cast<float>(row) / static_cast<float>(GRID_HEIGHT));
      const auto angle     = (1.0F - 0.5F * xNoise) * std::sqrt(x * x + y * y) * 2.0F * PI;
      gridAngles[row, col] = angle;
    }
  }
}

auto FlowField::GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto gridCoords =
      ToPoint2dInt(m_normalizedCoordsToGridConverter.NormalizedToOtherCoordsFlt(coords));
  const auto gridAngles = std::mdspan{m_gridArray.data(), GRID_HEIGHT, GRID_WIDTH};
  const auto gridAngle  = gridAngles[gridCoords.x, gridCoords.y];

  return {GetBaseZoomAdjustment().x + (m_params.amplitude.x * std::cos(15.0F * gridAngle)),
          GetBaseZoomAdjustment().y + (m_params.amplitude.y * std::sin(25.0F * gridAngle))};
}

auto FlowField::SetRandomParams() noexcept -> void
{
  const auto xAmplitude = m_goomRand->GetRandInRange<AMPLITUDE_RANGE>();
  const auto yAmplitude = m_goomRand->ProbabilityOf<PROB_XY_AMPLITUDES_EQUAL>()
                              ? xAmplitude
                              : m_goomRand->GetRandInRange<AMPLITUDE_RANGE>();

  SetParams({
      {xAmplitude, yAmplitude}
  });
}

auto FlowField::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  return NameValuePairs();
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
