module;

#include <algorithm>
#include <bezier/bezier.h>
#include <cstddef>
#include <cstdint>

module Goom.Utils.Graphics.BezierDrawer;

import Goom.Color.ColorUtils;
import Goom.Draw.ShapeDrawers.BitmapDrawer;
import Goom.Draw.ShaperDrawers.LineDrawer;
import Goom.Utils.Graphics.ImageBitmaps;
import Goom.Lib.GoomGraphic;
import Goom.Lib.Point2d;

namespace GOOM::UTILS::GRAPHICS
{

using COLOR::GetBrighterColor;
using DRAW::SHAPE_DRAWERS::BitmapDrawer;
using DRAW::SHAPE_DRAWERS::LineDrawerClippedEndPoints;

inline auto BezierDrawer::GetImageBitmap(const size_t size) const -> const ImageBitmap&
{
  return m_smallBitmaps->GetImageBitmap(m_currentBitmapName,
                                        std::clamp(size, MIN_DOT_DIAMETER, MAX_DOT_DIAMETER));
}

void BezierDrawer::Draw(const Bezier::Bezier<3>& bezier, const float colorT0, const float colorT1)
{
  auto lineDrawer = LineDrawerClippedEndPoints{*m_draw};
  lineDrawer.SetLineThickness(m_lineThickness);

  const auto colorTStep = (colorT1 - colorT0) / static_cast<float>(m_numBezierSteps - 1);

  const auto tStep = 1.0F / static_cast<float>(m_numBezierSteps - 1);
  auto colorT      = colorT0 + colorTStep;
  auto t           = tStep;
  auto point1      = Point2dInt{.x = static_cast<int32_t>(bezier.valueAt(0.0F, 0)),
                                .y = static_cast<int32_t>(bezier.valueAt(0.0F, 1))};

  for (auto i = 1U; i < m_numBezierSteps; ++i)
  {
    const auto point2 = Point2dInt{.x = static_cast<int32_t>(bezier.valueAt(t, 0)),
                                   .y = static_cast<int32_t>(bezier.valueAt(t, 1))};

    const auto lineColor = GetBrighterColor(10.F, m_lineColorFunc(colorT));
    lineDrawer.DrawLine(point1, point2, {.color1 = lineColor, .color2 = lineColor});

    if (0 == (i % m_dotEveryNumBezierSteps))
    {
      const auto dotColor = GetBrighterColor(10.F, m_dotColorFunc(colorT));
      DrawDot(point2, m_dotDiameter, dotColor);
    }

    point1 = point2;
    t += tStep;
    colorT += colorTStep;
  }
}

void BezierDrawer::DrawDot(const Point2dInt& centre, const uint32_t diameter, const Pixel& color)
{
  const auto getColor = [&color]([[maybe_unused]] const Point2dInt& bitmapPoint, const Pixel& bgnd)
  {
    if (0 == bgnd.A())
    {
      return BLACK_PIXEL;
    }
    return color;
  };

  auto bitmapDrawer = BitmapDrawer{*m_draw};
  bitmapDrawer.Bitmap(centre, GetImageBitmap(diameter), {getColor, getColor});
}

} // namespace GOOM::UTILS::GRAPHICS
