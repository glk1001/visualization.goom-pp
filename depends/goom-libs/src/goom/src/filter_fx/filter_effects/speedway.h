#pragma once

#include "filter_fx/normalized_coords.h"
#include "filter_fx/speed_coefficients_effect.h"
#include "point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class Speedway : public ISpeedCoefficientsEffect
{
public:
  enum class Modes
  {
    MODE0,
    MODE1,
    MODE2,
  };
  Speedway(Modes mode, const GOOM::UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto SetRandomParams() -> void override;

  [[nodiscard]] auto GetSpeedCoefficients(const NormalizedCoords& coords,
                                          float sqDistFromZero,
                                          const Point2dFlt& baseSpeedCoeffs) const
      -> Point2dFlt override;

  [[nodiscard]] auto GetSpeedCoefficientsEffectNameValueParams() const
      -> GOOM::UTILS::NameValuePairs override;

  struct Params
  {
    float xAmplitude;
    float yAmplitude;
    float tFreq;
    bool flipY;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  auto SetParams(const Params& params) -> void;

private:
  const Modes m_mode;
  const GOOM::UTILS::MATH::IGoomRand& m_goomRand;
  Params m_params;
  auto SetMode0RandomParams() -> void;
  auto SetMode1RandomParams() -> void;
  auto SetMode2RandomParams() -> void;
  [[nodiscard]] auto GetMode0SpeedCoefficients(const NormalizedCoords& coords,
                                               float sqDistFromZero,
                                               const Point2dFlt& baseSpeedCoeffs) const
      -> Point2dFlt;
  [[nodiscard]] auto GetMode1SpeedCoefficients(const NormalizedCoords& coords,
                                               float sqDistFromZero,
                                               const Point2dFlt& baseSpeedCoeffs) const
      -> Point2dFlt;
  [[nodiscard]] auto GetMode2SpeedCoefficients(const NormalizedCoords& coords,
                                               float sqDistFromZero,
                                               const Point2dFlt& baseSpeedCoeffs) const
      -> Point2dFlt;
};

inline auto Speedway::GetSpeedCoefficients(const NormalizedCoords& coords,
                                           const float sqDistFromZero,
                                           const Point2dFlt& baseSpeedCoeffs) const -> Point2dFlt
{
  switch (m_mode)
  {
    case Modes::MODE0:
      return GetMode0SpeedCoefficients(coords, sqDistFromZero, baseSpeedCoeffs);
    case Modes::MODE1:
      return GetMode1SpeedCoefficients(coords, sqDistFromZero, baseSpeedCoeffs);
    case Modes::MODE2:
      return GetMode2SpeedCoefficients(coords, sqDistFromZero, baseSpeedCoeffs);
    default:
      throw std::logic_error("Unexpected Modes enum.");
  }
}

inline auto Speedway::GetMode0SpeedCoefficients(const NormalizedCoords& coords,
                                                const float sqDistFromZero,
                                                const Point2dFlt& baseSpeedCoeffs) const
    -> Point2dFlt
{
  static constexpr auto SQ_DIST_FACTOR = 0.01F;

  auto xAdd = SQ_DIST_FACTOR * sqDistFromZero;
  if (static constexpr auto PROB_FLIP_X_ADD = 0.5F; m_goomRand.ProbabilityOf(PROB_FLIP_X_ADD))
  {
    xAdd = -xAdd;
  }

  const auto xSpeedCoeff = baseSpeedCoeffs.x * (m_params.xAmplitude * (coords.GetY() + xAdd));
  const auto ySpeedCoeff = m_params.yAmplitude * xSpeedCoeff;

  return {xSpeedCoeff, ySpeedCoeff};
}

inline auto Speedway::GetMode1SpeedCoefficients(const NormalizedCoords& coords,
                                                const float sqDistFromZero,
                                                const Point2dFlt& baseSpeedCoeffs) const
    -> Point2dFlt
{
  auto xAdd = -1.0F;
  if (static constexpr auto PROB_RANDOM_X_ADD = 0.5F; m_goomRand.ProbabilityOf(PROB_RANDOM_X_ADD))
  {
    static constexpr auto MIN_NEGATIVE_X_ADD  = -1.9F;
    static constexpr auto MAX_NEGATIVE_X_ADD  = -0.5F;
    static constexpr auto MIN_POSITIVE_X_ADD  = +0.5F;
    static constexpr auto MAX_POSITIVE_X_ADD  = +1.9F;
    static constexpr auto PROB_NEGATIVE_X_ADD = 0.5F;

    xAdd = m_goomRand.ProbabilityOf(PROB_NEGATIVE_X_ADD)
               ? m_goomRand.GetRandInRange(MIN_NEGATIVE_X_ADD, MAX_NEGATIVE_X_ADD)
               : m_goomRand.GetRandInRange(MIN_POSITIVE_X_ADD, MAX_POSITIVE_X_ADD);
  }
  else if (static constexpr auto PROB_FLIP_X_ADD = 0.5F; m_goomRand.ProbabilityOf(PROB_FLIP_X_ADD))
  {
    xAdd = -xAdd;
  }

  static constexpr auto X_WARP_MULTIPLIER    = 0.1F;
  static constexpr auto AMPLITUDE_MULTIPLIER = 0.25F;

  const auto xDiff = coords.GetX() - xAdd;
  const auto sign  = xDiff < 0.0F ? -1.0F : +1.0F;
  const auto xWarp = X_WARP_MULTIPLIER * (xAdd + ((sign * GOOM::UTILS::MATH::Sq(xDiff)) / xAdd));
  const auto amplitude = AMPLITUDE_MULTIPLIER * (1.0F - sqDistFromZero);

  const auto xSpeedCoeff = amplitude * baseSpeedCoeffs.x * (m_params.xAmplitude * xWarp);
  const auto ySpeedCoeff = amplitude * m_params.yAmplitude * xSpeedCoeff;

  return {xSpeedCoeff, ySpeedCoeff};
}

inline auto Speedway::GetMode2SpeedCoefficients(const NormalizedCoords& coords,
                                                const float sqDistFromZero,
                                                const Point2dFlt& baseSpeedCoeffs) const
    -> Point2dFlt
{
  static constexpr auto SQ_DIST_FACTOR = 0.01F;

  auto xAdd = SQ_DIST_FACTOR * sqDistFromZero;
  if (static constexpr auto PROB_FLIP_X_ADD = 0.5F; m_goomRand.ProbabilityOf(PROB_FLIP_X_ADD))
  {
    xAdd = -xAdd;
  }

  const auto xSpeedCoeff = baseSpeedCoeffs.x * (m_params.xAmplitude * (coords.GetY() + xAdd));
  const auto ySpeedCoeff = std::tan(0.01F * sqDistFromZero) * m_params.yAmplitude * xSpeedCoeff;

  const auto t = std::sin(m_params.tFreq * sqDistFromZero);
  if (m_params.flipY)
  {
    return {
        STD20::lerp(ySpeedCoeff, xSpeedCoeff, t),
        -STD20::lerp(xSpeedCoeff, ySpeedCoeff, t),
    };
  }

  return {
      STD20::lerp(ySpeedCoeff, xSpeedCoeff, t),
      STD20::lerp(xSpeedCoeff, ySpeedCoeff, t),
  };
}

inline auto Speedway::GetParams() const -> const Params&
{
  return m_params;
}

inline auto Speedway::SetParams(const Params& params) -> void
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
