//#undef NO_LOGGING

#include "mobius.h"

#include "complex_utils.h"
#include "filter_fx/normalized_coords.h"
#include "goom/point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/name_value_pairs.h"

#include <array>
#include <cmath>
#include <complex>
#include <cstdint>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;

static constexpr auto DEFAULT_VIEWPORT = Viewport{};

static constexpr auto DEFAULT_AMPLITUDE = 0.1F;
static constexpr auto AMPLITUDE_RANGE   = IGoomRand::NumberRange<float>{0.01F, 0.11F};

static constexpr auto DEFAULT_A = 1.0F;
static constexpr auto A_RANGE   = IGoomRand::NumberRange<float>{0.5F, 1.5F};

static constexpr auto DEFAULT_B = 1.0F;
static constexpr auto B_RANGE   = IGoomRand::NumberRange<float>{0.5F, 1.5F};

static constexpr auto DEFAULT_C = 1.0F;
static constexpr auto C_RANGE   = IGoomRand::NumberRange<float>{0.5F, 1.5F};

static constexpr auto DEFAULT_D = -1.0F;
static constexpr auto D_RANGE   = IGoomRand::NumberRange<float>{-1.5F, -0.5F};

static constexpr auto DEFAULT_MODULATOR_PERIOD = 2.0F;
static constexpr auto MODULATOR_PERIOD_RANGE   = IGoomRand::NumberRange<float>{1.0F, 100.0F};

static constexpr auto PROB_AMPLITUDES_EQUAL       = 0.75F;
static constexpr auto PROB_NO_INVERSE_SQUARE      = 0.90F;
static constexpr auto PROB_USE_MODULATOR_CONTOURS = 0.01F;

static constexpr auto VIEWPORT_RECTANGLES = std::array{
    Viewport::Rectangle{{-1.99F, -1.99F}, {1.99F, 1.99F}},
    Viewport::Rectangle{{-1.00F, -1.00F}, {1.00F, 1.00F}},
    Viewport::Rectangle{    {0.5F, 0.5F},   {2.0F, 2.0F}},
    Viewport::Rectangle{ {0.30F, -0.10F}, {0.70F, 0.10F}},
};

Mobius::Mobius(const IGoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_params{
        DEFAULT_VIEWPORT,
        {DEFAULT_AMPLITUDE, DEFAULT_AMPLITUDE},
        DEFAULT_A,
        DEFAULT_B,
        DEFAULT_C,
        DEFAULT_D,
        true,
        false,
        DEFAULT_MODULATOR_PERIOD,
    }
{
}

auto Mobius::SetRandomParams() noexcept -> void
{
  const auto viewport = Viewport{VIEWPORT_RECTANGLES.at(
      m_goomRand->GetRandInRange(0U, static_cast<uint32_t>(VIEWPORT_RECTANGLES.size())))};

  const auto xAmplitude = m_goomRand->GetRandInRange(AMPLITUDE_RANGE);
  const auto yAmplitude = m_goomRand->ProbabilityOf(PROB_AMPLITUDES_EQUAL)
                              ? xAmplitude
                              : m_goomRand->GetRandInRange(AMPLITUDE_RANGE);

  const auto a = m_goomRand->GetRandInRange(A_RANGE);
  const auto b = m_goomRand->GetRandInRange(B_RANGE);
  const auto c = m_goomRand->GetRandInRange(C_RANGE);
  const auto d = m_goomRand->GetRandInRange(D_RANGE);

  const auto noInverseSquare      = m_goomRand->ProbabilityOf(PROB_NO_INVERSE_SQUARE);
  const auto useModulatorContours = m_goomRand->ProbabilityOf(PROB_USE_MODULATOR_CONTOURS);
  const auto modulatorPeriod =
      not useModulatorContours ? 0.0F : m_goomRand->GetRandInRange(MODULATOR_PERIOD_RANGE);

  SetParams({
      viewport,
      {xAmplitude, yAmplitude},
      a,
      b,
      c,
      d,
      noInverseSquare,
      useModulatorContours,
      modulatorPeriod,
  });
}

auto Mobius::GetZoomAdjustment(const NormalizedCoords& coords,
                               [[maybe_unused]] const float sqDistFromZero) const noexcept
    -> Point2dFlt
{
  Expects(GetZoomAdjustmentViewport().GetViewportWidth() != NormalizedCoords::COORD_WIDTH);

  const auto z = std::complex<FltCalcType>{static_cast<FltCalcType>(coords.GetX()),
                                           static_cast<FltCalcType>(coords.GetY())};

  const auto a = static_cast<FltCalcType>(m_params.a);
  const auto b = static_cast<FltCalcType>(m_params.b);
  const auto c = static_cast<FltCalcType>(m_params.c);
  const auto d = static_cast<FltCalcType>(m_params.d);

  const auto fz      = ((a * z) + b) / ((c * z) + d);
  const auto absSqFz = std::norm(fz);

  if (absSqFz < static_cast<FltCalcType>(UTILS::MATH::SMALL_FLOAT))
  {
    return GetBaseZoomAdjustment();
  }

  const auto phase =
      GetAdjustedPhase(m_params.amplitude, m_params.noInverseSquare, fz, sqDistFromZero);

  if (not m_params.useModulatorContours)
  {
    return {GetBaseZoomAdjustment().x + static_cast<float>(phase.real()),
            GetBaseZoomAdjustment().y + static_cast<float>(phase.imag())};
  }

  const auto modulatedPhase = GetModulatedPhase(absSqFz, phase, m_params.modulatorPeriod);

  return {GetBaseZoomAdjustment().x + static_cast<float>(modulatedPhase.real()),
          GetBaseZoomAdjustment().y + static_cast<float>(modulatedPhase.imag())};
}

auto Mobius::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  const auto fullParamGroup = GetFullParamGroup({PARAM_GROUP, "mobius"});
  return {
      GetPair(fullParamGroup, "amplitude", Point2dFlt{m_params.amplitude.x, m_params.amplitude.y}),
      GetPair(fullParamGroup, "a", m_params.a),
      GetPair(fullParamGroup, "b", m_params.b),
      GetPair(fullParamGroup, "c", m_params.c),
      GetPair(fullParamGroup, "d", m_params.d),
      GetPair(fullParamGroup, "modulatorPeriod", m_params.modulatorPeriod),
      GetPair(PARAM_GROUP,
              "viewport0",
              m_params.viewport
                  .GetViewportCoords({NormalizedCoords::MIN_COORD, NormalizedCoords::MIN_COORD})
                  .GetFltCoords()),
  };
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
