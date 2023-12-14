#include "image_displacement_list.h"

#include "goom/goom_config.h"
#include "goom/point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

#include <cstdint>
#include <string>
#include <vector>

namespace GOOM::FILTER_FX::FILTER_UTILS
{

using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;

// NOLINTNEXTLINE(cert-err58-cpp): Will be fixed with C++20 and 'constexpr'.
const std::vector<std::string> ImageDisplacementList::IMAGE_FILENAMES{
    "checkerboard.jpg",
    "dark-patterns-maze.jpg",
    "concentric.png",
    "pattern1.jpg",
    "pattern2.jpg",
    "pattern3.jpg",
    "pattern4.jpg",
    "pattern5.jpg",
    "pattern6.jpg",
    "pattern7.jpg",
    "pattern8.jpg",
    "chameleon-tail.jpg",
    "mountain_sunset.png",
};

ImageDisplacementList::ImageDisplacementList(const std::string& resourcesDirectory,
                                             const IGoomRand& goomRand)
  : m_resourcesDirectory{resourcesDirectory}, m_goomRand{&goomRand}
{
  if (m_resourcesDirectory.empty())
  {
    return;
  }

  for (const auto& imageFilename : IMAGE_FILENAMES)
  {
    m_imageDisplacements.emplace_back(GetImageFilename(imageFilename), *m_goomRand);
  }
}

auto ImageDisplacementList::SetRandomImageDisplacement() -> void
{
  m_currentImageDisplacementIndex =
      m_goomRand->GetRandInRange(0U, static_cast<uint32_t>(m_imageDisplacements.size()));
}

auto ImageDisplacementList::GetImageFilename(const std::string& imageFilename) const -> std::string
{
  return m_resourcesDirectory + PATH_SEP + IMAGE_DISPLACEMENT_DIR + PATH_SEP + imageFilename;
}

auto ImageDisplacementList::GetParams() const -> Params
{
  return {GetCurrentImageDisplacement().GetAmplitude(),
          GetCurrentImageDisplacement().GetXColorCutoff(),
          GetCurrentImageDisplacement().GetYColorCutoff(),
          GetCurrentImageDisplacement().GetXZoomFactor(),
          GetCurrentImageDisplacement().GetYZoomFactor()};
}

auto ImageDisplacementList::SetParams(const Params& params) -> void
{
  GetCurrentImageDisplacement().SetAmplitude(params.amplitude);
  GetCurrentImageDisplacement().SetXyColorCutoffs(params.xColorCutoff, params.yColorCutoff);
  GetCurrentImageDisplacement().SetZoomFactor(params.xZoomFactor, params.yZoomFactor);
}

auto ImageDisplacementList::GetNameValueParams(const std::string& paramGroup) const
    -> NameValuePairs
{
  return {
      GetPair(paramGroup, "filename", IMAGE_FILENAMES[m_currentImageDisplacementIndex]),
      GetPair(paramGroup,
              "zoom factor",
              Point2dFlt{GetCurrentImageDisplacement().GetXZoomFactor(),
                         GetCurrentImageDisplacement().GetYZoomFactor()}),
      GetPair(paramGroup,
              "amplitude",
              Point2dFlt{GetCurrentImageDisplacement().GetAmplitude().x,
                         GetCurrentImageDisplacement().GetAmplitude().y}),
      GetPair(paramGroup,
              "cutoff",
              Point2dFlt{GetCurrentImageDisplacement().GetXColorCutoff(),
                         GetCurrentImageDisplacement().GetYColorCutoff()}),
  };
}

} // namespace GOOM::FILTER_FX::FILTER_UTILS
