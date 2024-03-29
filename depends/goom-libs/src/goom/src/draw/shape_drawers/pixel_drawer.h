#pragma once

#include "draw/goom_draw.h"
#include "drawer_utils.h"
#include "goom/goom_config.h"
#include "goom/point2d.h"

namespace GOOM::DRAW::SHAPE_DRAWERS
{

class PixelDrawer
{
public:
  explicit PixelDrawer(IGoomDraw& draw) noexcept;

  auto DrawPixels(const Point2dInt& point, const MultiplePixels& colors) noexcept -> void;
  auto DrawPixelsClipped(const Point2dInt& point, const MultiplePixels& colors) noexcept -> void;

private:
  IGoomDraw* m_draw;
  ClipTester m_clipTester{m_draw->GetDimensions(), 0};
};

inline PixelDrawer::PixelDrawer(IGoomDraw& draw) noexcept : m_draw{&draw}
{
}

inline auto PixelDrawer::DrawPixels(const Point2dInt& point, const MultiplePixels& colors) noexcept
    -> void
{
  m_draw->DrawPixels(point, colors);
}

inline auto PixelDrawer::DrawPixelsClipped(const Point2dInt& point,
                                           const MultiplePixels& colors) noexcept -> void
{
  if (m_clipTester.IsOutside(point))
  {
    return;
  }

  m_draw->DrawPixels(point, colors);
}

} // namespace GOOM::DRAW::SHAPE_DRAWERS
