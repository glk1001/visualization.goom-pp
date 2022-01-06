#include "image_displacement.h"

#include "goom_graphic.h"
#include "normalized_coords.h"
#include "utils/graphics/image_bitmaps.h"
#include "v2d.h"

#include <cmath>
#include <cstdint>
#include <memory>

namespace GOOM::VISUAL_FX::FILTERS
{

using UTILS::IGoomRand;
using UTILS::ImageBitmap;

ImageDisplacement::ImageDisplacement(const std::string& imageFilename, const IGoomRand& goomRand)
  : m_imageBuffer(std::make_unique<ImageBitmap>(imageFilename)),
    m_imageFilename{imageFilename},
    m_goomRand{goomRand},
    m_xMax{static_cast<int32_t>(m_imageBuffer->GetWidth() - 1)},
    m_yMax{static_cast<int32_t>(m_imageBuffer->GetHeight() - 1)},
    m_ratioNormalizedCoordToImageCoord{
        static_cast<float>(std::min(m_xMax, m_yMax)) /
        (NormalizedCoords::MAX_NORMALIZED_COORD - NormalizedCoords::MIN_NORMALIZED_COORD)}
{
}

auto ImageDisplacement::GetDisplacementVector(const V2dFlt& normalizedPoint) const -> V2dFlt
{
  const V2dInt imagePoint = NormalizedToImagePoint(normalizedPoint);
  if ((imagePoint.x < 0) || (imagePoint.x > m_xMax))
  {
    return {0.0F, 0.0F};
  }
  if ((imagePoint.y < 0) || (imagePoint.y > m_yMax))
  {
    return {0.0F, 0.0F};
  }

  const Pixel color =
      (*m_imageBuffer)(static_cast<size_t>(imagePoint.x), static_cast<size_t>(imagePoint.y));

  const float x =
      (NormalizedCoords::MAX_NORMALIZED_COORD - NormalizedCoords::MIN_NORMALIZED_COORD) *
      m_amplitude * (color.RFlt() - m_xColorCutoff);
  const float y =
      (NormalizedCoords::MAX_NORMALIZED_COORD - NormalizedCoords::MIN_NORMALIZED_COORD) *
      m_amplitude * (color.GFlt() - m_yColorCutoff);
  //const float y = (ProbabilityOfMInN(1, 2) ? color.GFlt() : color.BFlt()) - 0.5F;

  return {x, y};
}

inline auto ImageDisplacement::NormalizedToImagePoint(const V2dFlt& normalizedPoint) const -> V2dInt
{
  const float xZoom = m_zoomFactor * normalizedPoint.x;
  const float yZoom = m_zoomFactor * normalizedPoint.y;

  const auto x = static_cast<int32_t>(std::lround(
      m_ratioNormalizedCoordToImageCoord * (xZoom - NormalizedCoords::MIN_NORMALIZED_COORD)));
  const auto y = static_cast<int32_t>(std::lround(
      m_ratioNormalizedCoordToImageCoord * (yZoom - NormalizedCoords::MIN_NORMALIZED_COORD)));

  constexpr int32_t FUZZ = 0;
  if constexpr (0 == FUZZ)
  {
    return {x, y};
  }
  else
  {
    return {std::clamp(m_goomRand.GetRandInRange(x - FUZZ, x + FUZZ), 0, m_xMax),
            std::clamp(m_goomRand.GetRandInRange(y - FUZZ, y + FUZZ), 0, m_yMax)};
  }
}

} // namespace GOOM::VISUAL_FX::FILTERS
