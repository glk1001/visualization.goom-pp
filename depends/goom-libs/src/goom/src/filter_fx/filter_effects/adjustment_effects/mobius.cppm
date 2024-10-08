export module Goom.FilterFx.FilterEffects.AdjustmentEffects.Mobius;

import Goom.FilterFx.FilterEffects.ZoomAdjustmentEffect;
import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.Point2d;

using GOOM::UTILS::NameValuePairs;
using GOOM::UTILS::MATH::GoomRand;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class Mobius : public IZoomAdjustmentEffect
{
public:
  explicit Mobius(const GoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Vec2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> NameValuePairs override;

  struct Params
  {
    Viewport viewport;
    Amplitude amplitude;
    FILTER_UTILS::LerpToOneTs lerpToOneTs;
    float a;
    float b;
    float c;
    float d;
    bool noInverseSquare;
    bool useNormalizedAmplitude;
    bool useModulatorContours;
    float modulatorPeriod;
  };
  [[nodiscard]] auto GetParams() const noexcept -> const Params&;

protected:
  auto SetParams(const Params& params) noexcept -> void;

private:
  const GoomRand* m_goomRand;
  FILTER_UTILS::RandomViewport m_randomViewport;
  Params m_params;
  [[nodiscard]] auto GetRandomParams() const noexcept -> Params;
  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt;
};

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

inline auto Mobius::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline void Mobius::SetParams(const Params& params) noexcept
{
  m_params = params;
}

inline auto Mobius::SetRandomParams() noexcept -> void
{
  m_params = GetRandomParams();
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
