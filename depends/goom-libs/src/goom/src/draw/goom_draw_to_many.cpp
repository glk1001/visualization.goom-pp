module;

#include <vector>

module Goom.Draw.GoomDrawToMany;

import Goom.Draw.GoomDrawBase;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;

namespace GOOM::DRAW
{

GoomDrawToMany::GoomDrawToMany(const Dimensions& dimensions,
                               const std::vector<IGoomDraw*>& manyDraws) noexcept
  : IGoomDraw{dimensions}, m_manyDraws{manyDraws}
{
  Expects(not manyDraws.empty());
}

auto GoomDrawToMany::GetPixel(const Point2dInt& point) const noexcept -> Pixel
{
  return m_manyDraws[0]->GetPixel(point);
}

auto GoomDrawToMany::DrawPixelsUnblended(const Point2dInt& point,
                                         const MultiplePixels& colors) noexcept -> void
{
  for (auto* const draw : m_manyDraws)
  {
    draw->DrawPixelsUnblended(point, colors);
  }
}

auto GoomDrawToMany::DrawPixelsToDevice(const Point2dInt& point,
                                        const MultiplePixels& colors) noexcept -> void
{
  for (auto* const draw : m_manyDraws)
  {
    draw->DrawPixels(point, colors);
  }
}

} // namespace GOOM::DRAW
