module;

#include <string>

export module Goom.FilterFx.FilterEffects.AdjustmentEffects.ImageZoomAdjustment;

import Goom.FilterFx.FilterUtils.ImageDisplacementList;
import Goom.FilterFx.NormalizedCoords;
import Goom.FilterFx.ZoomAdjustmentEffect;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.Point2d;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class ImageZoomAdjustment : public IZoomAdjustmentEffect
{
public:
  ImageZoomAdjustment(const std::string& resourcesDirectory,
                      const GOOM::UTILS::MATH::GoomRand& goomRand);

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Vec2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> GOOM::UTILS::NameValuePairs override;

private:
  const GOOM::UTILS::MATH::GoomRand* m_goomRand;
  FILTER_UTILS::ImageDisplacementList m_imageDisplacementList;
  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt;
  auto DoSetRandomParams() noexcept -> void;
};

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

inline auto ImageZoomAdjustment::GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
    -> Vec2dFlt
{
  const auto velocity = GetVelocity(coords);

  return {coords.GetX() * velocity.x, coords.GetY() * velocity.y};
}

inline auto ImageZoomAdjustment::GetVelocity(const NormalizedCoords& coords) const noexcept
    -> Vec2dFlt
{
  return m_imageDisplacementList.GetCurrentImageDisplacement().GetDisplacementVector(coords);
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
