module;

#include <string>

export module Goom.FilterFx.AfterEffects.TheEffects.Noise;

import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRandBase;

export namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

class Noise
{
public:
  explicit Noise(const UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto SetRandomParams() -> void;

  [[nodiscard]] auto GetVelocity(const NormalizedCoords& velocity) const -> NormalizedCoords;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs;

  struct Params
  {
    float noiseFactor;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  auto SetParams(const Params& params) -> void;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  // For noise amplitude, take the reciprocal of these.
  static constexpr float NOISE_MIN = 40.0F;
  static constexpr float NOISE_MAX = 120.0F;
  Params m_params;
};

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

inline auto Noise::GetVelocity(const NormalizedCoords& velocity) const -> NormalizedCoords
{
  const auto amp = m_params.noiseFactor / m_goomRand->GetRandInRange(NOISE_MIN, NOISE_MAX);
  return velocity + NormalizedCoords{m_goomRand->GetRandInRange(-amp, +amp),
                                     m_goomRand->GetRandInRange(-amp, +amp)};
}

inline auto Noise::GetParams() const -> const Params&
{
  return m_params;
}

inline auto Noise::SetParams(const Params& params) -> void
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
