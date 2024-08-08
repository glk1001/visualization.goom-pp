module;

#include <cmath>

export module Goom.FilterFx.FilterEffects.AdjustmentEffects.Speedway;

import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.FilterFx.ZoomAdjustmentEffect;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;

using GOOM::UTILS::MATH::NumberRange;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class Speedway : public IZoomAdjustmentEffect
{
public:
  enum class Modes : UnderlyingEnumType
  {
    MODE0,
    MODE1,
    MODE2,
  };
  Speedway(Modes mode, const UTILS::MATH::GoomRand& goomRand) noexcept;

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
  Modes m_mode;
  const UTILS::MATH::GoomRand* m_goomRand;
  Params m_params;
  auto SetMode0RandomParams() noexcept -> void;
  auto SetMode1RandomParams() noexcept -> void;
  auto SetMode2RandomParams() noexcept -> void;
  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt;
  [[nodiscard]] auto GetMode0ZoomAdjustment(const NormalizedCoords& coords,
                                            float sqDistFromZero) const noexcept -> Vec2dFlt;
  [[nodiscard]] auto GetMode1ZoomAdjustment(const NormalizedCoords& coords,
                                            float sqDistFromZero) const noexcept -> Vec2dFlt;
  [[nodiscard]] auto GetMode2ZoomAdjustment(const NormalizedCoords& coords,
                                            float sqDistFromZero) const noexcept -> Vec2dFlt;
};

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

inline auto Speedway::GetZoomAdjustment(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto velocity = GetVelocity(coords);

  return {.x = coords.GetX() * velocity.x, .y = coords.GetY() * velocity.y};
}

inline auto Speedway::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline auto Speedway::SetParams(const Params& params) noexcept -> void
{
  m_params = params;
}

inline auto Speedway::GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto sqDistFromZero = SqDistanceFromZero(coords);

  switch (m_mode)
  {
    case Modes::MODE0:
      return GetMode0ZoomAdjustment(coords, sqDistFromZero);
    case Modes::MODE1:
      return GetMode1ZoomAdjustment(coords, sqDistFromZero);
    case Modes::MODE2:
      return GetMode2ZoomAdjustment(coords, sqDistFromZero);
  }
}

inline auto Speedway::GetMode0ZoomAdjustment(const NormalizedCoords& coords,
                                             const float sqDistFromZero) const noexcept -> Vec2dFlt
{
  static constexpr auto SQ_DIST_FACTOR = 0.01F;

  auto xAdd = SQ_DIST_FACTOR * sqDistFromZero;
  if (static constexpr auto PROB_FLIP_X_ADD = 0.5F; m_goomRand->ProbabilityOf<PROB_FLIP_X_ADD>())
  {
    xAdd = -xAdd;
  }

  const auto xZoomAdjustment =
      GetBaseZoomAdjustment().x * (m_params.amplitude.x * (coords.GetY() + xAdd));
  const auto yZoomAdjustment = m_params.amplitude.y * xZoomAdjustment;

  return {.x = xZoomAdjustment, .y = yZoomAdjustment};
}

inline auto Speedway::GetMode1ZoomAdjustment(const NormalizedCoords& coords,
                                             const float sqDistFromZero) const noexcept -> Vec2dFlt
{
  auto xAdd = -1.0F;
  if (static constexpr auto PROB_RANDOM_X_ADD = 0.5F;
      m_goomRand->ProbabilityOf<PROB_RANDOM_X_ADD>())
  {
    static constexpr auto NEGATIVE_X_ADD_RANGE = NumberRange{-1.9F, -0.5F};
    static constexpr auto POSITIVE_X_ADD_RANGE = NumberRange{+0.5F, +1.9F};
    static constexpr auto PROB_NEGATIVE_X_ADD  = 0.5F;

    xAdd = m_goomRand->ProbabilityOf<PROB_NEGATIVE_X_ADD>()
               ? m_goomRand->GetRandInRange<NEGATIVE_X_ADD_RANGE>()
               : m_goomRand->GetRandInRange<POSITIVE_X_ADD_RANGE>();
  }
  else if (static constexpr auto PROB_FLIP_X_ADD = 0.5F;
           m_goomRand->ProbabilityOf<PROB_FLIP_X_ADD>())
  {
    xAdd = -xAdd;
  }

  static constexpr auto X_WARP_MULTIPLIER    = 0.1F;
  static constexpr auto AMPLITUDE_MULTIPLIER = 0.25F;

  const auto xDiff     = coords.GetX() - xAdd;
  const auto sign      = xDiff < 0.0F ? -1.0F : +1.0F;
  const auto xWarp     = X_WARP_MULTIPLIER * (xAdd + ((sign * UTILS::MATH::Sq(xDiff)) / xAdd));
  const auto amplitude = AMPLITUDE_MULTIPLIER * (1.0F - sqDistFromZero);

  const auto xZoomAdjustment =
      amplitude * GetBaseZoomAdjustment().x * (m_params.amplitude.x * xWarp);
  const auto yZoomAdjustment = amplitude * m_params.amplitude.y * xZoomAdjustment;

  return {.x = xZoomAdjustment, .y = yZoomAdjustment};
}

inline auto Speedway::GetMode2ZoomAdjustment(const NormalizedCoords& coords,
                                             const float sqDistFromZero) const noexcept -> Vec2dFlt
{
  static constexpr auto SQ_DIST_FACTOR = 0.01F;

  auto xAdd = SQ_DIST_FACTOR * sqDistFromZero;
  if (static constexpr auto PROB_FLIP_X_ADD = 0.5F; m_goomRand->ProbabilityOf<PROB_FLIP_X_ADD>())
  {
    xAdd = -xAdd;
  }

  const auto xZoomAdjustment =
      GetBaseZoomAdjustment().x * (m_params.amplitude.x * (coords.GetY() + xAdd));
  const auto yZoomAdjustment =
      std::tan(0.01F * sqDistFromZero) * m_params.amplitude.y * xZoomAdjustment;

  return {.x = xZoomAdjustment, .y = yZoomAdjustment};
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
