#pragma once

#include "image_displacement_list.h"
#include "normalized_coords.h"
#include "speed_coefficients_effect.h"
#include "utils/goom_rand_base.h"
#include "utils/name_value_pairs.h"
#include "v2d.h"

#include <string>

namespace GOOM::VISUAL_FX::FILTERS
{

class ImageSpeedCoefficients : public ISpeedCoefficientsEffect
{
public:
  ImageSpeedCoefficients(const std::string& resourcesDirectory, const UTILS::IGoomRand& goomRand);

  void SetRandomParams() override;

  [[nodiscard]] auto GetSpeedCoefficients(const V2dFlt& baseSpeedCoeffs,
                                          float sqDistFromZero,
                                          const NormalizedCoords& coords) const -> V2dFlt override;

  [[nodiscard]] auto GetSpeedCoefficientsEffectNameValueParams() const
      -> UTILS::NameValuePairs override;

private:
  const UTILS::IGoomRand& m_goomRand;
  ImageDisplacementList m_imageDisplacementList;
  void DoSetRandomParams();
};

inline auto ImageSpeedCoefficients::GetSpeedCoefficients(
    [[maybe_unused]] const V2dFlt& baseSpeedCoeffs,
    [[maybe_unused]] const float sqDistFromZero,
    const NormalizedCoords& coords) const -> V2dFlt
{
  return m_imageDisplacementList.GetCurrentImageDisplacement().GetDisplacementVector(
      coords.ToFlt());
}

} // namespace GOOM::VISUAL_FX::FILTERS
