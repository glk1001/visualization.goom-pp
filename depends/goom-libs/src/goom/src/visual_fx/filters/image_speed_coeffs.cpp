#include "image_speed_coeffs.h"

#include "utils/name_value_pairs.h"

#undef NDEBUG
#include <cassert>
#include <string>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
namespace FILTERS
{
#else
namespace GOOM::VISUAL_FX::FILTERS
{
#endif

using UTILS::IGoomRand;
using UTILS::NameValuePairs;

constexpr IGoomRand::NumberRange<float> AMPLITUDE_RANGE = {0.0025F, 0.01000F};
constexpr IGoomRand::NumberRange<float> COLOR_CUTOFF_RANGE = {0.1F, 0.9F};
constexpr IGoomRand::NumberRange<float> ZOOM_FACTOR_RANGE = {0.10F, 1.0F};

constexpr float PROB_XY_COLOR_CUTOFFS_EQUAL = 0.5F;

ImageSpeedCoefficients::ImageSpeedCoefficients(const std::string& resourcesDirectory,
                                               const IGoomRand& goomRand)
  : m_goomRand{goomRand}, m_imageDisplacementList{resourcesDirectory, m_goomRand}
{
  if (!resourcesDirectory.empty())
  {
    SetRandomParams();
  }
}

void ImageSpeedCoefficients::SetRandomParams()
{
  m_imageDisplacementList.SetRandomImageDisplacement();

  const float xColorCutoff = m_goomRand.GetRandInRange(COLOR_CUTOFF_RANGE);

  m_imageDisplacementList.SetParams({
      m_goomRand.GetRandInRange(AMPLITUDE_RANGE),
      xColorCutoff,
      m_goomRand.ProbabilityOf(PROB_XY_COLOR_CUTOFFS_EQUAL)
          ? xColorCutoff
          : m_goomRand.GetRandInRange(COLOR_CUTOFF_RANGE),
      m_goomRand.GetRandInRange(ZOOM_FACTOR_RANGE),
  });
}

auto ImageSpeedCoefficients::GetSpeedCoefficientsEffectNameValueParams() const -> NameValuePairs
{
  constexpr const char* PARAM_GROUP = "ImageSpeedCoeffs";
  return m_imageDisplacementList.GetNameValueParams(PARAM_GROUP);
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::FILTERS
#endif
