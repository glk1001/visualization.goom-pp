export module Goom.FilterFx.FilterEffects.AdjustmentEffects.Scrunch;

import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.FilterFx.ZoomAdjustmentEffect;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.Point2d;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class Scrunch : public IZoomAdjustmentEffect
{
public:
  explicit Scrunch(const UTILS::MATH::GoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Vec2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> UTILS::NameValuePairs override;

  struct Params
  {
    Amplitude amplitude;
  };
  [[nodiscard]] auto GetParams() const noexcept -> const Params&;

protected:
  auto SetParams(const Params& params) noexcept -> void;

private:
  const UTILS::MATH::GoomRand* m_goomRand;
  Params m_params;
  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt;
};

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

inline auto Scrunch::GetZoomAdjustment(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto velocity = GetVelocity(coords);

  return {.x = coords.GetX() * velocity.x, .y = coords.GetY() * velocity.y};
}

inline auto Scrunch::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline auto Scrunch::SetParams(const Params& params) noexcept -> void
{
  m_params = params;
}

inline auto Scrunch::GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto sqDistFromZero  = SqDistanceFromZero(coords);
  const auto xZoomAdjustment = GetBaseZoomAdjustment().x + (m_params.amplitude.x * sqDistFromZero);
  const auto yZoomAdjustment = m_params.amplitude.y * xZoomAdjustment;

  return {.x = xZoomAdjustment, .y = yZoomAdjustment};
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
