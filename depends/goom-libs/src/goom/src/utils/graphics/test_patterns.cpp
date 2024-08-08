module Goom.Utils.Graphics.TestPatterns;

import Goom.Color.ColorUtils;
import Goom.Draw.GoomDrawBase;
import Goom.Draw.ShaperDrawers.LineDrawer;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;

namespace GOOM::UTILS::GRAPHICS
{

using DRAW::IGoomDraw;
using GOOM::COLOR::GetBrighterColor;
using GOOM::DRAW::MultiplePixels;
using GOOM::DRAW::SHAPE_DRAWERS::LineDrawerClippedEndPoints;

auto DrawTestPattern(IGoomDraw& draw,
                     const Point2dInt& centre,
                     const Dimensions& dimensions) -> void
{
  const auto lineThickness = 3;
  const auto width         = dimensions.GetIntWidth();
  const auto height        = dimensions.GetIntHeight();
  const auto x0            = lineThickness + (centre.x - (width / 2));
  const auto y0            = lineThickness + (centre.y - (height / 2));
  const auto x1            = (centre.x + ((width - 1) / 2)) - lineThickness;
  const auto y1            = (centre.y + ((height - 1) / 2)) - lineThickness;
  const auto topLeft       = Point2dInt{.x = x0, .y = y0};
  const auto topRight      = Point2dInt{.x = x1, .y = y0};
  const auto bottomLeft    = Point2dInt{.x = x0, .y = y1};
  const auto bottomRight   = Point2dInt{.x = x1, .y = y1};

  static constexpr auto BRIGHTNESS = 10.0F;
  const auto white                 = GetBrighterColor(BRIGHTNESS, WHITE_PIXEL);
  const auto color                 = MultiplePixels{.color1 = white, .color2 = white};

  auto lineDrawer = LineDrawerClippedEndPoints{draw};
  lineDrawer.SetLineThickness(lineThickness);

  lineDrawer.DrawLine(topLeft, topRight, color);
  lineDrawer.DrawLine(topRight, bottomRight, color);
  lineDrawer.DrawLine(bottomRight, bottomLeft, color);
  lineDrawer.DrawLine(bottomLeft, topLeft, color);
  lineDrawer.DrawLine(bottomLeft, topRight, color);
  lineDrawer.DrawLine(bottomRight, topLeft, color);
}

} // namespace GOOM::UTILS::GRAPHICS
