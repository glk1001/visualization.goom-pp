module;

#include <algorithm>
#include <cmath>
#include <string>

export module Goom.FilterFx.AfterEffects.TheEffects.TanEffect;

import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Lib.GoomTypes;

export namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

class TanEffect
{
public:
  explicit TanEffect(const UTILS::MATH::GoomRand& goomRand);
  TanEffect(const TanEffect&) noexcept           = delete;
  TanEffect(TanEffect&&) noexcept                = delete;
  virtual ~TanEffect() noexcept                  = default;
  auto operator=(const TanEffect&) -> TanEffect& = delete;
  auto operator=(TanEffect&&) -> TanEffect&      = delete;

  virtual auto SetRandomParams() -> void;

  [[nodiscard]] auto GetVelocity(float sqDistFromZero,
                                 const NormalizedCoords& velocity) const -> NormalizedCoords;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs;

  enum class TanType : UnderlyingEnumType
  {
    TAN_ONLY,
    COT_ONLY,
    COT_MIX,
  };
  struct Params
  {
    TanType tanType;
    float cotMix;
    Amplitude amplitude;
    float limitingFactor;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  auto SetParams(const Params& params) -> void;

private:
  const UTILS::MATH::GoomRand* m_goomRand;
  Params m_params;
  UTILS::MATH::Weights<TanType> m_tanEffectWeights;
  static constexpr auto HALF_PI = UTILS::MATH::HALF_PI;
  [[nodiscard]] auto GetTanSqDist(float tanArg) const -> float;
};

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

inline auto TanEffect::GetVelocity(const float sqDistFromZero,
                                   const NormalizedCoords& velocity) const -> NormalizedCoords
{
  const auto limit     = m_params.limitingFactor * HALF_PI;
  const auto tanArg    = std::clamp(std::fmod(sqDistFromZero, HALF_PI), -limit, +limit);
  const auto tanSqDist = GetTanSqDist(tanArg);
  return {m_params.amplitude.x * tanSqDist * velocity.GetX(),
          m_params.amplitude.y * tanSqDist * velocity.GetY()};
}

inline auto TanEffect::GetParams() const -> const Params&
{
  return m_params;
}

inline auto TanEffect::SetParams(const Params& params) -> void
{
  m_params = params;
}

inline auto TanEffect::GetTanSqDist(const float tanArg) const -> float
{
  switch (m_params.tanType)
  {
    case TanType::TAN_ONLY:
      return std::tan(tanArg);
    case TanType::COT_ONLY:
      return std::tan(HALF_PI - tanArg);
    case TanType::COT_MIX:
      return std::tan((m_params.cotMix * HALF_PI) - tanArg);
  }
}

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
