module;

#include <cstdint>
#include <vector>

module Goom.Draw.ShapeDrawers.BitmapDrawer;

import Goom.Draw.GoomDrawBase;
import Goom.Utils.Graphics.ImageBitmaps;
import Goom.Lib.Point2d;

namespace GOOM::DRAW::SHAPE_DRAWERS
{

using UTILS::GRAPHICS::ImageBitmap;

auto BitmapDrawer::Bitmap(const Point2dInt& centre,
                          const ImageBitmap& bitmap,
                          const std::vector<GetBitmapColorFunc>& getColors) noexcept -> void
{
  const auto bitmapWidth  = bitmap.GetIntWidth();
  const auto bitmapHeight = bitmap.GetIntHeight();

  auto x0 = centre.x - (bitmapWidth / 2);
  auto y0 = centre.y - (bitmapHeight / 2);
  auto x1 = x0 + (bitmapWidth - 1);
  auto y1 = y0 + (bitmapHeight - 1);

  if ((x0 >= m_draw->GetDimensions().GetIntWidth()) or
      (y0 >= m_draw->GetDimensions().GetIntHeight()) or (x1 < 0) or (y1 < 0))
  {
    return;
  }

  x0 = std::max(x0, 0);
  y0 = std::max(y0, 0);

  if (x1 >= m_draw->GetDimensions().GetIntWidth())
  {
    x1 = m_draw->GetDimensions().GetIntWidth() - 1;
  }
  if (y1 >= m_draw->GetDimensions().GetIntHeight())
  {
    y1 = m_draw->GetDimensions().GetIntHeight() - 1;
  }

  const auto actualBitmapWidth  = static_cast<uint32_t>(x1 - x0) + 1;
  const auto actualBitmapHeight = static_cast<uint32_t>(y1 - y0) + 1;

  const auto setDestPixelRow =
      [this, &x0, &y0, &actualBitmapWidth, &bitmap, &getColors](const uint32_t bitmapY)
  {
    const int buffY  = y0 + static_cast<int>(bitmapY);
    auto finalColors = MultiplePixels{};
    for (auto bitmapX = 0U; bitmapX < actualBitmapWidth; ++bitmapX)
    {
      const auto bitmapColor = bitmap(bitmapX, bitmapY);
      if ((0 == bitmapColor.A()) || (bitmapColor.IsBlack()))
      {
        continue;
      }

      finalColors.color1 = getColors[0](GetPoint2dInt(bitmapX, bitmapY), bitmapColor);
      if (getColors.size() > 1)
      {
        finalColors.color2 = getColors[1](GetPoint2dInt(bitmapX, bitmapY), bitmapColor);
      }

      const auto buffX = x0 + static_cast<int>(bitmapX);
      m_draw->DrawPixels({.x = buffX, .y = buffY}, finalColors);
    }
  };

  for (auto yBitmap = 0U; yBitmap < actualBitmapHeight; ++yBitmap)
  {
    setDestPixelRow(yBitmap);
  }
}

} // namespace GOOM::DRAW::SHAPE_DRAWERS
