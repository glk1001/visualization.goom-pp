module;

#include <array>
//#include <mdspan.hpp>
#include <PerlinNoise.hpp>

export module Goom.FilterFx.FilterEffects.AdjustmentEffects.FlowField;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.FilterFx.ZoomAdjustmentEffect;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.Point2d;

using GOOM::FILTER_FX::FILTER_UTILS::LerpToOneTs;
using GOOM::UTILS::MATH::GoomRand;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class FlowField : public IZoomAdjustmentEffect
{
public:
  explicit FlowField(const GoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Vec2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> UTILS::NameValuePairs override;

  struct Params
  {
    Amplitude amplitude{};
    LerpToOneTs lerpToOneTs{};
    FrequencyFactor noiseFrequencyFactor;
    FrequencyFactor angleFrequencyFactor;
    int32_t octaves;
    float persistence;
    float noiseFactor = 0.0F;
  };
  [[nodiscard]] auto GetParams() const noexcept -> const Params&;

protected:
  auto SetParams(const Params& params) noexcept -> void;

private:
  const GoomRand* m_goomRand;
  static constexpr auto GRID_WIDTH  = 500UL;
  static constexpr auto GRID_HEIGHT = 500UL;
  std::array<float, GRID_WIDTH * GRID_HEIGHT> m_gridArray{};
  //std::mdspan<float, std::extents<unsigned long, GRID_HEIGHT, GRID_WIDTH>> gridAngles{m_gridArray.data()};
  NormalizedCoordsConverter m_normalizedCoordsToGridConverter{
      {GRID_WIDTH, GRID_HEIGHT}
  };
  siv::BasicPerlinNoise<float> m_perlinNoise;
  siv::BasicPerlinNoise<float> m_perlinNoise2;
  auto SetupAngles() noexcept -> void;
  Params m_params;
  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt;
};

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

inline auto FlowField::GetZoomAdjustment(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto velocity = GetVelocity(coords);

  return GetVelocityByZoomLerpedToOne(coords, m_params.lerpToOneTs, velocity);
}

inline auto FlowField::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline void FlowField::SetParams(const Params& params) noexcept
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
