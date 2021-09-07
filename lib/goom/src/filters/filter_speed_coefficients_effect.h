#ifndef VISUALIZATION_GOOM_FILTER_SPEED_COEFFICIENTS_EFFECT_H
#define VISUALIZATION_GOOM_FILTER_SPEED_COEFFICIENTS_EFFECT_H

#include "filter_normalized_coords.h"
#include "v2d.h"

#include <string>
#include <tuple>
#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

class SpeedCoefficientsEffect
{
public:
  virtual ~SpeedCoefficientsEffect() noexcept = default;

  virtual void SetRandomParams() = 0;

  [[nodiscard]] virtual auto GetSpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                                  float sqDistFromZero,
                                                  const NormalizedCoords& coords) const
      -> V2dFlt = 0;

  [[nodiscard]] virtual auto GetSpeedCoefficientsEffectNameValueParams() const
      -> std::vector<std::pair<std::string, std::string>> = 0;
};

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif

#endif //VISUALIZATION_GOOM_FILTER_SPEED_COEFFICIENTS_EFFECT_H
