module;

#include <cstdint>
#include <functional>
#include <vector>

export module Goom.Draw.ShaperDrawers.CircleDrawer;

import Goom.Draw.GoomDrawBase;
import Goom.Lib.GoomGraphic;
import Goom.Lib.Point2d;

export namespace GOOM::DRAW::SHAPE_DRAWERS
{

class CircleDrawer
{
public:
  explicit CircleDrawer(IGoomDraw& draw) noexcept;

  auto DrawCircle(const Point2dInt& centre,
                  int32_t radius,
                  const MultiplePixels& colors) noexcept -> void;
  auto DrawFilledCircle(const Point2dInt& centre,
                        int32_t radius,
                        const MultiplePixels& colors) noexcept -> void;

private:
  IGoomDraw* m_draw;
  using PlotCirclePointsFunc =
      std::function<void(const Point2dInt& point1, const Point2dInt& point2)>;
  static auto DrawBresenhamCircle(const Point2dInt& centre,
                                  int32_t radius,
                                  const PlotCirclePointsFunc& plot) noexcept -> void;
  auto DrawHorizontalLine(const Point2dInt& point1,
                          int32_t x2,
                          const MultiplePixels& colors) noexcept -> void;
};

} // namespace GOOM::DRAW::SHAPE_DRAWERS

namespace GOOM::DRAW::SHAPE_DRAWERS
{

inline CircleDrawer::CircleDrawer(IGoomDraw& draw) noexcept : m_draw{&draw}
{
}

} // namespace GOOM::DRAW::SHAPE_DRAWERS
