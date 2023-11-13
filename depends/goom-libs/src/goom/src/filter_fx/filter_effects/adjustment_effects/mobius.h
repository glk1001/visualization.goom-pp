#pragma once

#include "filter_fx/common_types.h"
#include "filter_fx/normalized_coords.h"
#include "filter_fx/zoom_adjustment_effect.h"
#include "goom/point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class Mobius : public IZoomAdjustmentEffect
{
public:
  explicit Mobius(const UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void override;
  [[nodiscard]] auto GetZoomAdjustmentViewport() const noexcept -> Viewport override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords,
                                       float sqDistFromZero) const noexcept -> Point2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> UTILS::NameValuePairs override;

  struct Params
  {
    Viewport viewport;
    Amplitude amplitude;
    float a;
    float b;
    float c;
    float d;
    bool noInverseSquare;
    bool useModulatorContours;
    float modulatorPeriod;
  };
  [[nodiscard]] auto GetParams() const noexcept -> const Params&;

protected:
  auto SetParams(const Params& params) noexcept -> void;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  Params m_params;
};

inline auto Mobius::GetZoomAdjustmentViewport() const noexcept -> Viewport
{
  return m_params.viewport;
}

inline auto Mobius::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline void Mobius::SetParams(const Params& params) noexcept
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
