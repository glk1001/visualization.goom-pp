#pragma once

#include "filter_fx/normalized_coords.h"
#include "filter_fx/zoom_adjustment_effect.h"
#include "goom/goom_types.h"
#include "goom/point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

#include <cmath>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class Wave : public IZoomAdjustmentEffect
{
public:
  enum class Modes : UnderlyingEnumType
  {
    SQ_DIST_ANGLE_EFFECT_MODE0,
    SQ_DIST_ANGLE_EFFECT_MODE1,
    ATAN_ANGLE_EFFECT_MODE0,
    ATAN_ANGLE_EFFECT_MODE1,
  };
  Wave(Modes mode, const UTILS::MATH::IGoomRand& goomRand);

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Point2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> UTILS::NameValuePairs override;

  enum class AngleEffect : UnderlyingEnumType
  {
    ATAN,
    SQ_DIST,
    SQ_DIST_AND_SPIRAL
  };
  enum class WaveEffect : UnderlyingEnumType
  {
    WAVE_SIN_EFFECT,
    WAVE_COS_EFFECT,
    WAVE_SIN_COS_EFFECT,
    WAVE_TAN_EFFECT,
    WAVE_TAN_SIN_EFFECT,
    WAVE_TAN_COS_EFFECT,
    WAVE_COT_EFFECT,
    WAVE_COT_SIN_EFFECT,
    WAVE_COT_COS_EFFECT,
    _num // unused, and marks the enum end
  };
  struct Params
  {
    WaveEffect xWaveEffect;
    WaveEffect yWaveEffect;
    AngleEffect angleEffect;
    float sqDistPower;
    float freqFactor;
    float amplitude;
    float periodicFactor;
    float reducerCoeff;
    float spiralRotateBaseAngle;
    bool useModifiedATanAngle;
    float modifiedATanAngleFactor;
  };
  [[nodiscard]] auto GetParams() const noexcept -> const Params&;

protected:
  auto SetParams(const Params& params) noexcept -> void;

private:
  Modes m_mode;
  const UTILS::MATH::IGoomRand* m_goomRand;
  UTILS::MATH::Weights<WaveEffect> m_weightedEffects;
  Params m_params;
  auto SetSqDistAngleEffectMode0RandomParams() noexcept -> void;
  auto SetSqDistAngleEffectMode1RandomParams() noexcept -> void;
  auto SetAtanAngleEffectMode0RandomParams() noexcept -> void;
  auto SetAtanAngleEffectMode1RandomParams() noexcept -> void;

  struct WaveModeSettings
  {
    AngleEffect angleEffect{};
    UTILS::MATH::IGoomRand::NumberRange<float> freqFactorRange{};
    UTILS::MATH::IGoomRand::NumberRange<float> amplitudeRange{};
    UTILS::MATH::IGoomRand::NumberRange<float> periodicFactorRange{};
    UTILS::MATH::IGoomRand::NumberRange<float> sinCosPeriodicFactorRange{};
  };
  auto SetWaveModeSettings(const WaveModeSettings& waveModeSettings) noexcept -> void;

  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const noexcept -> Point2dFlt;
  [[nodiscard]] auto GetZoomAdjustmentAdd(WaveEffect waveEffect,
                                          float angle,
                                          float reducer) const noexcept -> float;
  [[nodiscard]] auto GetAngle(float sqDistFromZero, const NormalizedCoords& coords) const noexcept
      -> float;
  [[nodiscard]] static auto GetPeriodicPart(WaveEffect waveEffect,
                                            float angle,
                                            float periodicFactor) noexcept -> float;
  [[nodiscard]] auto GetPeriodicFactor(
      WaveEffect xWaveEffect,
      WaveEffect yWaveEffect,
      const UTILS::MATH::IGoomRand::NumberRange<float>& periodicFactorRange,
      const UTILS::MATH::IGoomRand::NumberRange<float>& sinCosPeriodicFactorRange) const noexcept
      -> float;
  [[nodiscard]] auto GetReducerCoeff(WaveEffect xWaveEffect,
                                     WaveEffect yWaveEffect,
                                     float periodicFactor) const noexcept -> float;

  [[nodiscard]] auto GetSqDistEffect() const noexcept -> Wave::AngleEffect;
  [[nodiscard]] auto GetATanAngle(const NormalizedCoords& coords) const noexcept -> float;
  [[nodiscard]] auto GetSqDistSpiralRotateAngle(float sqDistFromZero,
                                                const NormalizedCoords& coords) const noexcept
      -> float;
};

inline auto Wave::GetZoomAdjustment(const NormalizedCoords& coords) const noexcept -> Point2dFlt
{
  const auto velocity = GetVelocity(coords);

  return {coords.GetX() * velocity.x, coords.GetY() * velocity.y};
}

inline auto Wave::GetVelocity(const NormalizedCoords& coords) const noexcept -> Point2dFlt
{
  const auto sqDistFromZero = SqDistanceFromZero(coords);
  const auto angle          = GetAngle(sqDistFromZero, coords);
  const auto reducer        = std::exp(-m_params.reducerCoeff * sqDistFromZero);

  const auto xZoomAdjustment =
      GetBaseZoomAdjustment().x + GetZoomAdjustmentAdd(m_params.xWaveEffect, angle, reducer);
  const auto yZoomAdjustment =
      GetBaseZoomAdjustment().y + GetZoomAdjustmentAdd(m_params.yWaveEffect, angle, reducer);

  return {xZoomAdjustment, yZoomAdjustment};
}

inline auto Wave::GetZoomAdjustmentAdd(const WaveEffect waveEffect,
                                       const float angle,
                                       const float reducer) const noexcept -> float
{
  return reducer * m_params.amplitude *
         GetPeriodicPart(waveEffect, m_params.freqFactor * angle, m_params.periodicFactor);
}

inline auto Wave::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline auto Wave::SetParams(const Params& params) noexcept -> void
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
