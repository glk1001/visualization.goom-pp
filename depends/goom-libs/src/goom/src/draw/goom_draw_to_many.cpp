#include "goom_draw_to_many.h"

#include "goom_graphic.h"

#undef NDEBUG
#include <cassert>
#include <cstdint>
#include <vector>

namespace GOOM::DRAW
{

GoomDrawToMany::GoomDrawToMany(const uint32_t screenWidth,
                               const uint32_t screenHeight,
                               const std::vector<IGoomDraw*>& manyDraws)
  : IGoomDraw{screenWidth, screenHeight}, m_manyDraws{manyDraws}
{
  assert(!m_manyDraws.empty());
}

auto GoomDrawToMany::GetPixel(const int32_t x, const int32_t y) const -> Pixel
{
  const IGoomDraw* const draw = m_manyDraws[0];
  return draw->GetPixel(x, y);
}

void GoomDrawToMany::DrawPixelsUnblended(const int32_t x,
                                         const int32_t y,
                                         const std::vector<Pixel>& colors)
{
  for (auto* const draw : m_manyDraws)
  {
    draw->DrawPixelsUnblended(x, y, colors);
  }
}

void GoomDrawToMany::DrawPixelsToDevice(const int32_t x,
                                        const int32_t y,
                                        const std::vector<Pixel>& colors,
                                        [[maybe_unused]] const uint32_t intBuffIntensity)
{
  for (auto* const draw : m_manyDraws)
  {
    draw->DrawPixels(x, y, colors);
  }
}

} // namespace GOOM::DRAW
