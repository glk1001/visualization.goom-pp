#pragma once

#include "filter_fx/normalized_coords.h"
#include "filter_fx/zoom_in_coefficients_effect.h"
#include "point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class CrystalBall : public IZoomInCoefficientsEffect
{
public:
  enum class Modes
  {
    MODE0,
    MODE1
  };
  explicit CrystalBall(Modes mode, const UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto SetRandomParams() -> void override;

  [[nodiscard]] auto GetZoomInCoefficients(const NormalizedCoords& coords,
                                           float sqDistFromZero) const -> Point2dFlt override;

  [[nodiscard]] auto GetZoomInCoefficientsEffectNameValueParams() const
      -> UTILS::NameValuePairs override;

  struct Params
  {
    float xAmplitude;
    float yAmplitude;
    float xSqDistMult;
    float ySqDistMult;
    float xSqDistOffset;
    float ySqDistOffset;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  auto SetParams(const Params& params) -> void;

private:
  const Modes m_mode;
  const UTILS::MATH::IGoomRand& m_goomRand;
  Params m_params;
  auto SetMode0RandomParams() -> void;
  auto SetMode1RandomParams() -> void;
  auto SetRandomParams(const UTILS::MATH::IGoomRand::NumberRange<float>& xAmplitudeRange,
                       const UTILS::MATH::IGoomRand::NumberRange<float>& yAmplitudeRange,
                       const UTILS::MATH::IGoomRand::NumberRange<float>& xSqDistMultRange,
                       const UTILS::MATH::IGoomRand::NumberRange<float>& ySqDistMultRange,
                       const UTILS::MATH::IGoomRand::NumberRange<float>& xSqDistOffsetRange,
                       const UTILS::MATH::IGoomRand::NumberRange<float>& ySqDistOffsetRange)
      -> void;
  [[nodiscard]] static auto GetZoomInCoefficient(float baseZoomInCoeff,
                                                 float sqDistFromZero,
                                                 float amplitude,
                                                 float sqDistMult,
                                                 float sqDistOffset) -> float;
};

inline auto CrystalBall::GetZoomInCoefficients([[maybe_unused]] const NormalizedCoords& coords,
                                               const float sqDistFromZero) const -> Point2dFlt
{
  return {GetZoomInCoefficient(GetBaseZoomInCoeffs().x,
                               sqDistFromZero,
                               m_params.xAmplitude,
                               m_params.xSqDistMult,
                               m_params.xSqDistOffset),
          GetZoomInCoefficient(GetBaseZoomInCoeffs().y,
                               sqDistFromZero,
                               m_params.yAmplitude,
                               m_params.ySqDistMult,
                               m_params.ySqDistOffset)};
}

inline auto CrystalBall::GetZoomInCoefficient(const float baseZoomInCoeff,
                                              const float sqDistFromZero,
                                              const float amplitude,
                                              const float sqDistMult,
                                              const float sqDistOffset) -> float
{
  return baseZoomInCoeff - (amplitude * ((sqDistMult * sqDistFromZero) - sqDistOffset));
}

inline auto CrystalBall::GetParams() const -> const Params&
{
  return m_params;
}

inline auto CrystalBall::SetParams(const Params& params) -> void
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
