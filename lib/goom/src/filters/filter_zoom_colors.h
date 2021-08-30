#ifndef VISUALIZATION_GOOM_FILTER_ZOOM_COLORS_H
#define VISUALIZATION_GOOM_FILTER_ZOOM_COLORS_H

#include "filter_buffers.h"
#include "goom_graphic.h"
#include "v2d.h"

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

class ZoomFilterColors
{
public:
  ZoomFilterColors() noexcept = default;

  void SetBuffSettings(const FXBuffSettings& settings);
  void SetBlockWavy(bool val);

  using NeighborhoodCoeffArray = ZoomFilterBuffers::NeighborhoodCoeffArray;
  using NeighborhoodPixelArray = ZoomFilterBuffers::NeighborhoodPixelArray;

  [[nodiscard]] auto GetNewColor(const PixelBuffer& srceBuff,
                                 const ZoomFilterBuffers::SourcePointInfo& sourceInfo) const
      -> Pixel;

private:
  bool m_blockyWavy = false;
  FXBuffSettings m_buffSettings{};

  [[nodiscard]] auto GetFilteredColor(const NeighborhoodCoeffArray& coeffs,
                                      const NeighborhoodPixelArray& pixels) const -> Pixel;
  [[nodiscard]] auto GetMixedColor(const NeighborhoodCoeffArray& coeffs,
                                   const NeighborhoodPixelArray& colors) const -> Pixel;
  [[nodiscard]] auto GetBlockyMixedColor(const NeighborhoodCoeffArray& coeffs,
                                         const NeighborhoodPixelArray& colors) const -> Pixel;
};

inline void ZoomFilterColors::SetBlockWavy(const bool val)
{
  m_blockyWavy = val;
}

inline void ZoomFilterColors::SetBuffSettings(const FXBuffSettings& settings)
{
  m_buffSettings = settings;
}

inline auto ZoomFilterColors::GetNewColor(
    const PixelBuffer& srceBuff, const ZoomFilterBuffers::SourcePointInfo& sourceInfo) const
    -> Pixel
{
  if (sourceInfo.isClipped)
  {
    return Pixel::BLACK;
  }

  const NeighborhoodPixelArray pixelNeighbours = srceBuff.Get4RHBNeighbours(
      static_cast<size_t>(sourceInfo.screenPoint.x), static_cast<size_t>(sourceInfo.screenPoint.y));
  return GetFilteredColor(sourceInfo.coeffs, pixelNeighbours);
}

inline auto ZoomFilterColors::GetFilteredColor(const NeighborhoodCoeffArray& coeffs,
                                               const NeighborhoodPixelArray& pixels) const -> Pixel
{
  if (m_blockyWavy)
  {
    return GetBlockyMixedColor(coeffs, pixels);
  }

  return GetMixedColor(coeffs, pixels);
}

inline auto ZoomFilterColors::GetBlockyMixedColor(const NeighborhoodCoeffArray& coeffs,
                                                  const NeighborhoodPixelArray& colors) const
    -> Pixel
{
  // Changing the color order gives a strange blocky, wavy look.
  // The order col4, col3, col2, col1 gave a black tear - no so good.
  static_assert(4 == ZoomFilterBuffers::NUM_NEIGHBOR_COEFFS, "NUM_NEIGHBOR_COEFFS must be 4.");
  assert(ZoomFilterBuffers::NUM_NEIGHBOR_COEFFS == coeffs.val.size());
  const NeighborhoodPixelArray reorderedColors{colors[0], colors[2], colors[1], colors[3]};
  return GetMixedColor(coeffs, reorderedColors);
}

inline auto ZoomFilterColors::GetMixedColor(const NeighborhoodCoeffArray& coeffs,
                                            const NeighborhoodPixelArray& colors) const -> Pixel
{
  if (coeffs.isZero)
  {
    return Pixel::BLACK;
  }

  uint32_t multR = 0;
  uint32_t multG = 0;
  uint32_t multB = 0;
  for (size_t i = 0; i < coeffs.val.size(); ++i)
  {
    const uint32_t& coeff = coeffs.val[i];
    const auto& color = colors[i];
    multR += static_cast<uint32_t>(color.R()) * coeff;
    multG += static_cast<uint32_t>(color.G()) * coeff;
    multB += static_cast<uint32_t>(color.B()) * coeff;
  }
  uint32_t newR = multR >> 8;
  uint32_t newG = multG >> 8;
  uint32_t newB = multB >> 8;

  constexpr uint32_t MAX_CHANNEL_COLOR = channel_limits<uint32_t>::max();
  constexpr uint8_t MAX_ALPHA = 0xFF;
  if (m_buffSettings.allowOverexposed)
  {
    return Pixel{{/*.r = */ static_cast<uint8_t>(std::min(MAX_CHANNEL_COLOR, newR)),
                  /*.g = */ static_cast<uint8_t>(std::min(MAX_CHANNEL_COLOR, newG)),
                  /*.b = */ static_cast<uint8_t>(std::min(MAX_CHANNEL_COLOR, newB)),
                  /*.a = */ MAX_ALPHA}};
  }

  const uint32_t maxVal = std::max({newR, newG, newB});
  if (maxVal > channel_limits<uint32_t>::max())
  {
    // scale all channels back
    newR = multR / maxVal;
    newG = multG / maxVal;
    newB = multB / maxVal;
  }

  return Pixel{{/*.r = */ static_cast<uint8_t>(newR),
                /*.g = */ static_cast<uint8_t>(newG),
                /*.b = */ static_cast<uint8_t>(newB),
                /*.a = */ MAX_ALPHA}};
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif

#endif //VISUALIZATION_GOOM_FILTER_ZOOM_COLORS_H
