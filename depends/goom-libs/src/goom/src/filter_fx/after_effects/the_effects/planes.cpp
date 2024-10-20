module;

#include <cmath>
#include <cstdint>
#include <format>
#include <string>

module Goom.FilterFx.AfterEffects.TheEffects.Planes;

import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.EnumUtils;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Lib.Point2d;

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::NUM;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;
using UTILS::MATH::PI;

static constexpr auto SMALL_EFFECTS_RANGE          = NumberRange{-2, +2 + 1};
static constexpr auto MEDIUM_EFFECTS_RANGE         = NumberRange{-5, +5 + 1};
static constexpr auto LARGE_EFFECTS_RANGE          = NumberRange{-7, +7 + 1};
static constexpr auto VERY_LARGE_EFFECTS_RANGE     = NumberRange{-9, +9 + 1};
static constexpr auto VERY_LARGE_POS_EFFECTS_RANGE = NumberRange{+5, +12 + 1};

static constexpr auto PROB_ZERO_HORIZONTAL_FOR_VERY_LARGE_RANGE = 0.2F;
static constexpr auto PROB_ZERO_VERTICAL_FOR_LARGE_RANGE        = 0.2F;
static constexpr auto PROB_OPPOSITES_FOR_SMALL_EFFECTS          = 0.1F;
static constexpr auto PROB_OPPOSITES_FOR_MEDIUM_EFFECTS         = 0.9F;

// H Plane:
// @since June 2001
static constexpr auto HORIZONTAL_EFFECTS_MULTIPLIER_RANGE = NumberRange{0.0015F, 0.0035F};
static constexpr auto HORIZONTAL_EFFECTS_SPIRALLING_MULTIPLIER_RANGE =
    NumberRange{0.0015F, 0.0351F};

static constexpr auto HORIZONTAL_SWIRL_FREQ_RANGE      = NumberRange{0.1F, 5.01F};
static constexpr auto HORIZONTAL_SWIRL_AMPLITUDE_RANGE = NumberRange{0.1F, 5.01F};

static constexpr auto SWIRL_TYPE_RANGE = NumberRange{1U, NUM<Planes::PlaneSwirlType> - 1};

// V Plane:
static constexpr auto VERTICAL_EFFECTS_AMPLITUDE_RANGE            = NumberRange{0.0015F, 0.0035F};
static constexpr auto VERTICAL_EFFECTS_SPIRALLING_AMPLITUDE_RANGE = NumberRange{0.0015F, 0.0351F};

static constexpr auto VERTICAL_SWIRL_FREQ_RANGE      = NumberRange{0.1F, 30.01F};
static constexpr auto VERTICAL_SWIRL_AMPLITUDE_RANGE = NumberRange{0.1F, 30.01F};

static constexpr auto PROB_PLANE_AMPLITUDES_EQUAL       = 0.75F;
static constexpr auto PROB_ZERO_HORIZONTAL_PLANE_EFFECT = 0.50F;
static constexpr auto PROB_MUCH_SPIRALLING              = 0.20F;
static constexpr auto PROB_NO_SWIRL                     = 0.95F;
static constexpr auto PROB_SWIRL_AMPLITUDES_EQUAL       = 0.70F;
static constexpr auto PROB_SWIRL_FREQ_EQUAL             = 0.70F;

static constexpr auto SMALL_PLANE_EFFECTS_WEIGHT                           = 1.0F;
static constexpr auto MEDIUM_EFFECTS_WEIGHT                                = 4.0F;
static constexpr auto LARGE_EFFECTS_WEIGHT                                 = 1.0F;
static constexpr auto VERY_LARGE_EFFECTS_WEIGHT                            = 1.0F;
static constexpr auto POSITIVE_VERTICAL_NEGATIVE_HORIZONTAL_EFFECTS_WEIGHT = 1.0F;
static constexpr auto POSITIVE_HORIZONTAL_NEGATIVE_VERTICAL_EFFECTS_WEIGHT = 1.0F;
static constexpr auto ZERO_EFFECTS_WEIGHT                                  = 2.0F;

Planes::Planes(const GoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_planeEffectWeights{
      *m_goomRand,
      {
          { .key=PlaneEffectEvents::ZERO_EFFECTS,       .weight=ZERO_EFFECTS_WEIGHT },
          { .key=PlaneEffectEvents::SMALL_EFFECTS,      .weight=SMALL_PLANE_EFFECTS_WEIGHT },
          { .key=PlaneEffectEvents::MEDIUM_EFFECTS,     .weight=MEDIUM_EFFECTS_WEIGHT },
          { .key=PlaneEffectEvents::LARGE_EFFECTS,      .weight=LARGE_EFFECTS_WEIGHT },
          { .key=PlaneEffectEvents::VERY_LARGE_EFFECTS, .weight=VERY_LARGE_EFFECTS_WEIGHT },
          { .key=PlaneEffectEvents::POS_VERTICAL_NEG_HORIZONTAL_VERY_LARGE_EFFECTS,
                                     .weight=POSITIVE_VERTICAL_NEGATIVE_HORIZONTAL_EFFECTS_WEIGHT },
          { .key=PlaneEffectEvents::POS_HORIZONTAL_NEG_VERTICAL_VERY_LARGE_EFFECTS,
                                     .weight=POSITIVE_HORIZONTAL_NEGATIVE_VERTICAL_EFFECTS_WEIGHT },
      }
    },
    m_params{GetRandomParams(Point2dInt{}, 0U)}
{
}

auto Planes::GetRandomParams(const Point2dInt& zoomMidpoint,
                             const uint32_t screenWidth) const noexcept -> Params
{
  return GetRandomParams(
      *m_goomRand, m_planeEffectWeights.GetRandomWeighted(), zoomMidpoint, screenWidth);
}

auto Planes::GetRandomParams(const GoomRand& goomRand,
                             const PlaneEffectEvents planeEffectsEvent,
                             const Point2dInt& zoomMidpoint,
                             const uint32_t screenWidth) -> Params
{
  const auto muchSpiralling = goomRand.ProbabilityOf<PROB_MUCH_SPIRALLING>();

  return {.planeEffects = GetRandomPlaneEffects(
              goomRand, planeEffectsEvent, muchSpiralling, zoomMidpoint, screenWidth),
          .swirlEffects = GetRandomSwirlEffects(goomRand, muchSpiralling)};
}

auto Planes::GetRandomPlaneEffects(const GoomRand& goomRand,
                                   const PlaneEffectEvents planeEffectsEvent,
                                   const bool muchSpiralling,
                                   const Point2dInt& zoomMidpoint,
                                   const uint32_t screenWidth) -> PlaneEffects
{
  const auto intAmplitudes = GetRandomIntAmplitude(goomRand, planeEffectsEvent);
  const auto adjustedIntAmplitudes =
      GetAdjustedIntAmplitude(goomRand, intAmplitudes, zoomMidpoint, screenWidth);

  const auto effectMultipliers = GetRandomEffectMultiplier(goomRand, muchSpiralling);

  return GetRandomPlaneEffects(adjustedIntAmplitudes, effectMultipliers);
}

inline auto Planes::GetRandomPlaneEffects(const IntAmplitude& adjustedIntAmplitude,
                                          const Amplitude& effectMultiplier) -> PlaneEffects
{
  auto planeEffects = PlaneEffects{};

  if (0 == adjustedIntAmplitude.x)
  {
    planeEffects.horizontalEffectActive = false;
    planeEffects.amplitude.x            = 0.0F;
  }
  else
  {
    planeEffects.horizontalEffectActive = true;
    planeEffects.amplitude.x = effectMultiplier.x * static_cast<float>(adjustedIntAmplitude.x);
  }
  if (0 == adjustedIntAmplitude.y)
  {
    planeEffects.verticalEffectActive = false;
    planeEffects.amplitude.y          = 0.0F;
  }
  else
  {
    planeEffects.verticalEffectActive = true;
    planeEffects.amplitude.y = effectMultiplier.y * static_cast<float>(adjustedIntAmplitude.y);
  }

  return planeEffects;
}

auto Planes::GetRandomIntAmplitude(const GoomRand& goomRand,
                                   const PlaneEffectEvents planeEffectsEvent) -> IntAmplitude
{
  auto intAmplitude = IntAmplitude{};

  switch (planeEffectsEvent)
  {
    case PlaneEffectEvents::ZERO_EFFECTS:
      intAmplitude.x = 0;
      intAmplitude.y = 0;
      break;
    case PlaneEffectEvents::SMALL_EFFECTS:
      intAmplitude.x = goomRand.GetRandInRange<SMALL_EFFECTS_RANGE>();
      intAmplitude.y = goomRand.ProbabilityOf<PROB_OPPOSITES_FOR_SMALL_EFFECTS>()
                           ? (-intAmplitude.x + 1)
                           : goomRand.GetRandInRange<SMALL_EFFECTS_RANGE>();
      break;
    case PlaneEffectEvents::MEDIUM_EFFECTS:
      intAmplitude.x = goomRand.GetRandInRange<MEDIUM_EFFECTS_RANGE>();
      intAmplitude.y = goomRand.ProbabilityOf<PROB_OPPOSITES_FOR_MEDIUM_EFFECTS>()
                           ? (-intAmplitude.x + 1)
                           : goomRand.GetRandInRange<MEDIUM_EFFECTS_RANGE>();
      break;
    case PlaneEffectEvents::LARGE_EFFECTS:
      intAmplitude.x = goomRand.GetRandInRange<LARGE_EFFECTS_RANGE>();
      intAmplitude.y = goomRand.ProbabilityOf<PROB_ZERO_VERTICAL_FOR_LARGE_RANGE>()
                           ? 0
                           : goomRand.GetRandInRange<LARGE_EFFECTS_RANGE>();
      break;
    case PlaneEffectEvents::VERY_LARGE_EFFECTS:
      intAmplitude.x = goomRand.ProbabilityOf<PROB_ZERO_HORIZONTAL_FOR_VERY_LARGE_RANGE>()
                           ? 0
                           : goomRand.GetRandInRange<VERY_LARGE_EFFECTS_RANGE>();
      intAmplitude.y = goomRand.GetRandInRange<VERY_LARGE_EFFECTS_RANGE>();
      break;
    case PlaneEffectEvents::POS_VERTICAL_NEG_HORIZONTAL_VERY_LARGE_EFFECTS:
      intAmplitude.y = goomRand.GetRandInRange<VERY_LARGE_POS_EFFECTS_RANGE>();
      intAmplitude.x = -intAmplitude.y + 1;
      break;
    case PlaneEffectEvents::POS_HORIZONTAL_NEG_VERTICAL_VERY_LARGE_EFFECTS:
      intAmplitude.x = goomRand.GetRandInRange<VERY_LARGE_POS_EFFECTS_RANGE>();
      intAmplitude.y = -intAmplitude.x + 1;
      break;
  }

  return intAmplitude;
}

auto Planes::GetAdjustedIntAmplitude(const GoomRand& goomRand,
                                     const IntAmplitude& intAmplitude,
                                     const Point2dInt& zoomMidpoint,
                                     const uint32_t screenWidth) -> IntAmplitude
{
  auto adjustedIntAmplitude = intAmplitude;

  if ((1 == zoomMidpoint.x) or (zoomMidpoint.x == static_cast<int32_t>(screenWidth) - 1))
  {
    adjustedIntAmplitude.y = 0;
    if (goomRand.ProbabilityOf<PROB_ZERO_HORIZONTAL_PLANE_EFFECT>())
    {
      adjustedIntAmplitude.x = 0;
    }
  }

  return adjustedIntAmplitude;
}

auto Planes::GetRandomEffectMultiplier(const GoomRand& goomRand, const bool muchSpiralling)
    -> Amplitude
{
  auto effectMultiplier = Amplitude{};

  effectMultiplier.x =
      muchSpiralling ? goomRand.GetRandInRange<HORIZONTAL_EFFECTS_SPIRALLING_MULTIPLIER_RANGE>()
                     : goomRand.GetRandInRange<HORIZONTAL_EFFECTS_MULTIPLIER_RANGE>();

  if (goomRand.ProbabilityOf<PROB_PLANE_AMPLITUDES_EQUAL>())
  {
    effectMultiplier.y = effectMultiplier.x;
  }
  else
  {
    effectMultiplier.y =
        muchSpiralling ? goomRand.GetRandInRange<VERTICAL_EFFECTS_SPIRALLING_AMPLITUDE_RANGE>()
                       : goomRand.GetRandInRange<VERTICAL_EFFECTS_AMPLITUDE_RANGE>();
  }

  return effectMultiplier;
}

auto Planes::GetRandomSwirlEffects(const UTILS::MATH::GoomRand& goomRand, const bool muchSpiralling)
    -> PlaneSwirlEffects
{
  if (muchSpiralling || goomRand.ProbabilityOf<PROB_NO_SWIRL>())
  {
    return GetZeroSwirlEffects();
  }

  return GetNonzeroRandomSwirlEffects(goomRand);
}

inline auto Planes::GetZeroSwirlEffects() -> PlaneSwirlEffects
{
  return {
      .swirlType       = PlaneSwirlType::NONE,
      .amplitude       = {     0.0F,      0.0F},
      .frequencyFactor = {.x = 0.0F, .y = 0.0F},
  };
}

inline auto Planes::GetNonzeroRandomSwirlEffects(const UTILS::MATH::GoomRand& goomRand)
    -> PlaneSwirlEffects
{
  auto swirlEffects = PlaneSwirlEffects{};

  swirlEffects.swirlType = static_cast<PlaneSwirlType>(goomRand.GetRandInRange<SWIRL_TYPE_RANGE>());

  swirlEffects.amplitude.x = goomRand.GetRandInRange<HORIZONTAL_SWIRL_AMPLITUDE_RANGE>();
  swirlEffects.amplitude.y = goomRand.ProbabilityOf<PROB_SWIRL_AMPLITUDES_EQUAL>()
                                 ? swirlEffects.amplitude.x
                                 : goomRand.GetRandInRange<VERTICAL_SWIRL_AMPLITUDE_RANGE>();

  swirlEffects.frequencyFactor.x = goomRand.GetRandInRange<HORIZONTAL_SWIRL_FREQ_RANGE>();
  swirlEffects.frequencyFactor.y = goomRand.ProbabilityOf<PROB_SWIRL_FREQ_EQUAL>()
                                       ? swirlEffects.frequencyFactor.x
                                       : goomRand.GetRandInRange<VERTICAL_SWIRL_FREQ_RANGE>();

  return swirlEffects;
}

auto Planes::GetHorizontalPlaneVelocity(const CoordsAndVelocity& coordsAndVelocity) const -> float
{
  const auto yCoordValue = coordsAndVelocity.coords.GetY();
  const auto horizontalSwirlOffset =
      m_params.swirlEffects.amplitude.x * GetHorizontalSwirlOffsetFactor(yCoordValue);

  return coordsAndVelocity.velocity.GetX() +
         (m_params.planeEffects.amplitude.x * (yCoordValue + horizontalSwirlOffset));
}

auto Planes::GetVerticalPlaneVelocity(const CoordsAndVelocity& coordsAndVelocity) const -> float
{
  const auto xCoordValue = coordsAndVelocity.coords.GetX();
  const auto verticalSwirlOffset =
      m_params.swirlEffects.amplitude.y * GetVerticalSwirlOffsetFactor(xCoordValue);

  return coordsAndVelocity.velocity.GetY() +
         (m_params.planeEffects.amplitude.y * (xCoordValue + verticalSwirlOffset));
}

auto Planes::GetHorizontalSwirlOffsetFactor(const float coordValue) const -> float
{
  const auto swirlFreq = m_params.swirlEffects.frequencyFactor.x;

  switch (m_params.swirlEffects.swirlType)
  {
    case PlaneSwirlType::NONE:
      return 0.0F;
    case PlaneSwirlType::SIN_CURL_SWIRL:
    case PlaneSwirlType::SIN_COS_CURL_SWIRL:
      // 'sin' is for horizontal
      return std::sin(swirlFreq * coordValue);
    case PlaneSwirlType::COS_CURL_SWIRL:
    case PlaneSwirlType::COS_SIN_CURL_SWIRL:
      // 'cos' is for horizontal
      return std::cos(swirlFreq * coordValue);
    case PlaneSwirlType::SIN_OF_COS_SWIRL:
      // 'sin' is for horizontal
      return std::sin(PI * std::cos(swirlFreq * coordValue));
    case PlaneSwirlType::COS_OF_SIN_SWIRL:
      // 'cos' is for horizontal
      return std::cos(PI * std::sin(swirlFreq * coordValue));
  }
}

auto Planes::GetVerticalSwirlOffsetFactor(const float coordValue) const -> float
{
  const auto swirlFreq = m_params.swirlEffects.frequencyFactor.y;

  switch (m_params.swirlEffects.swirlType)
  {
    case PlaneSwirlType::NONE:
      return 0.0F;
    case PlaneSwirlType::SIN_CURL_SWIRL:
    case PlaneSwirlType::SIN_COS_CURL_SWIRL:
      // 'cos' is for vertical
      return std::cos(swirlFreq * coordValue);
    case PlaneSwirlType::COS_CURL_SWIRL:
    case PlaneSwirlType::COS_SIN_CURL_SWIRL:
      // 'sin' is for vertical
      return std::sin(swirlFreq * coordValue);
    case PlaneSwirlType::SIN_OF_COS_SWIRL:
      // 'cos' is for vertical
      return std::cos(PI * std::cos(swirlFreq * coordValue));
    case PlaneSwirlType::COS_OF_SIN_SWIRL:
      // 'sin' is for vertical
      return std::sin(PI * std::sin(swirlFreq * coordValue));
  }
}

auto Planes::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  const auto fullParamGroup = GetFullParamGroup({paramGroup, "planes"});
  return {GetPair(fullParamGroup,
                  "planeEffects",
                  std::format("({}, {}, {:.3f},{:.3f}),",
                              m_params.planeEffects.horizontalEffectActive,
                              m_params.planeEffects.verticalEffectActive,
                              m_params.planeEffects.amplitude.x,
                              m_params.planeEffects.amplitude.y)),
          GetPair(fullParamGroup,
                  "swirlEffects",
                  std::format("({}, {:.3f},{:.3f}), {:.1f},{:.1f})",
                              static_cast<int32_t>(m_params.swirlEffects.swirlType),
                              m_params.swirlEffects.amplitude.x,
                              m_params.swirlEffects.amplitude.y,
                              m_params.swirlEffects.frequencyFactor.x,
                              m_params.swirlEffects.frequencyFactor.y))};
}

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
