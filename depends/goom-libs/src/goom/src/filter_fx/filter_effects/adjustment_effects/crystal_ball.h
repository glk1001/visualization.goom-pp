#pragma once

#include "filter_fx/common_types.h"
#include "filter_fx/normalized_coords.h"
#include "filter_fx/zoom_adjustment_effect.h"
#include "goom/goom_types.h"
#include "goom/point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class CrystalBall : public IZoomAdjustmentEffect
{
public:
  enum class Modes : UnderlyingEnumType
  {
    MODE0,
    MODE1
  };
  explicit CrystalBall(Modes mode, const UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Point2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> UTILS::NameValuePairs override;

  struct Params
  {
    Amplitude amplitude;
    SqDistMult sqDistMult;
    SqDistOffset sqDistOffset;
  };
  [[nodiscard]] auto GetParams() const noexcept -> const Params&;

protected:
  auto SetParams(const Params& params) noexcept -> void;

private:
  Modes m_mode;
  const UTILS::MATH::IGoomRand* m_goomRand;
  Params m_params;
  auto SetMode0RandomParams() noexcept -> void;
  auto SetMode1RandomParams() noexcept -> void;
  auto SetRandomParams(const AmplitudeRange& amplitudeRange,
                       const SqDistMultRange& sqDistMultRange,
                       const SqDistOffsetRange& sqDistOffsetRange) noexcept -> void;
  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const noexcept -> Point2dFlt;
  [[nodiscard]] static auto GetZoomAdjustment(float baseZoomAdjustment,
                                              float sqDistFromZero,
                                              float amplitude,
                                              float sqDistMult,
                                              float sqDistOffset) noexcept -> float;
};

inline auto CrystalBall::GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
    -> Point2dFlt
{
  const auto velocity = GetVelocity(coords);

  return {coords.GetX() * velocity.x, coords.GetY() * velocity.y};
}

inline auto CrystalBall::GetVelocity(const NormalizedCoords& coords) const noexcept -> Point2dFlt
{
  const auto sqDistFromZero = SqDistanceFromZero(coords);

  return {GetZoomAdjustment(GetBaseZoomAdjustment().x,
                            sqDistFromZero,
                            m_params.amplitude.x,
                            m_params.sqDistMult.x,
                            m_params.sqDistOffset.x),
          GetZoomAdjustment(GetBaseZoomAdjustment().y,
                            sqDistFromZero,
                            m_params.amplitude.y,
                            m_params.sqDistMult.y,
                            m_params.sqDistOffset.y)};
}

inline auto CrystalBall::GetZoomAdjustment(const float baseZoomAdjustment,
                                           const float sqDistFromZero,
                                           const float amplitude,
                                           const float sqDistMult,
                                           const float sqDistOffset) noexcept -> float
{
  return baseZoomAdjustment - (amplitude * ((sqDistMult * sqDistFromZero) - sqDistOffset));
}

inline auto CrystalBall::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline auto CrystalBall::SetParams(const Params& params) noexcept -> void
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
