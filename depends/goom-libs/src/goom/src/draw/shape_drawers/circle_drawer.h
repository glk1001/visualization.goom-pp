#pragma once

#include "draw/goom_draw.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "point2d.h"

#include <cstdint>
#include <functional>
#include <vector>

namespace GOOM::DRAW::SHAPE_DRAWERS
{

class CircleDrawer
{
public:
  explicit CircleDrawer(IGoomDraw& draw) noexcept;

  auto DrawCircle(const Point2dInt& point, int32_t radius, const Pixel& color) noexcept -> void;
  auto DrawCircle(const Point2dInt& centre,
                  int32_t radius,
                  const std::vector<Pixel>& colors) noexcept -> void;
  auto DrawFilledCircle(const Point2dInt& centre,
                        int32_t radius,
                        const std::vector<Pixel>& colors) noexcept -> void;

private:
  IGoomDraw& m_draw;
  using PlotCirclePointsFunc =
      std::function<void(const Point2dInt& point1, const Point2dInt& point2)>;
  static auto DrawBresenhamCircle(const Point2dInt& centre,
                                  int32_t radius,
                                  const PlotCirclePointsFunc& plot) noexcept -> void;
  auto DrawHorizontalLine(int32_t x1,
                          int32_t y,
                          int32_t x2,
                          const std::vector<Pixel>& colors) noexcept -> void;
};

inline CircleDrawer::CircleDrawer(IGoomDraw& draw) noexcept : m_draw{draw}
{
}

inline auto CircleDrawer::DrawCircle(const Point2dInt& point,
                                     const int32_t radius,
                                     const Pixel& color) noexcept -> void
{
  DrawCircle(point, radius, std::vector<Pixel>{color});
}

} // namespace GOOM::DRAW::SHAPE_DRAWERS