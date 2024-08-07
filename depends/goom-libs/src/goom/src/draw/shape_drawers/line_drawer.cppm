module;

#include <cstdint>
#include <utility>

export module Goom.Draw.ShaperDrawers.LineDrawer;

import Goom.Draw.GoomDrawBase;
import Goom.Draw.ShaperDrawers.DrawerUtils;
import Goom.Draw.ShaperDrawers.LineDrawThick;
import Goom.Draw.ShaperDrawers.LineDrawWu;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;

export namespace GOOM::DRAW::SHAPE_DRAWERS
{

template<class DrawPixelPolicy>
class LineDrawer
{
public:
  explicit LineDrawer(DrawPixelPolicy&& drawPixelPolicy) noexcept;

  [[nodiscard]] auto GetDrawPixelPolicy() const noexcept -> const DrawPixelPolicy&;
  [[nodiscard]] auto GetDrawPixelPolicy() noexcept -> DrawPixelPolicy&;

  [[nodiscard]] auto GetLineThickness() const noexcept -> uint8_t;
  auto SetLineThickness(uint8_t thickness) noexcept -> void;

  auto DrawLine(const Point2dInt& point1,
                const Point2dInt& point2,
                const MultiplePixels& colors) noexcept -> void;

private:
  DrawPixelPolicy m_drawPixel;
  uint8_t m_thickness = 1U;

  auto DrawWuLine(const Point2dInt& point1,
                  const Point2dInt& point2,
                  const MultiplePixels& colors) noexcept -> void;

  auto DrawThickLine(const Point2dInt& point1,
                     const Point2dInt& point2,
                     const MultiplePixels& colors) noexcept -> void;
};

class LineDrawerNoClippedEndPoints
{
public:
  explicit LineDrawerNoClippedEndPoints(IGoomDraw& draw) noexcept;

  auto SetLineThickness(uint8_t thickness) noexcept -> void;

  auto DrawLine(const Point2dInt& point1,
                const Point2dInt& point2,
                const MultiplePixels& colors) noexcept -> void;

private:
  LineDrawer<PixelDrawerWithClipping> m_lineDrawer;
};

class LineDrawerClippedEndPoints
{
public:
  explicit LineDrawerClippedEndPoints(IGoomDraw& draw) noexcept;

  auto SetLineThickness(uint8_t thickness) noexcept -> void;

  auto DrawLine(const Point2dInt& point1,
                const Point2dInt& point2,
                const MultiplePixels& colors) noexcept -> void;

private:
  Dimensions m_dimensions;
  LineDrawer<PixelDrawerNoClipping> m_lineDrawer;
  ClipTester m_clipTester{m_dimensions, GetClipMargin()};
  [[nodiscard]] auto GetClipMargin() const noexcept -> int32_t;
};

} // namespace GOOM::DRAW::SHAPE_DRAWERS

namespace GOOM::DRAW::SHAPE_DRAWERS
{

inline LineDrawerNoClippedEndPoints::LineDrawerNoClippedEndPoints(IGoomDraw& draw) noexcept
  : m_lineDrawer{PixelDrawerWithClipping{draw}}
{
}

inline auto LineDrawerNoClippedEndPoints::SetLineThickness(const uint8_t thickness) noexcept -> void
{
  m_lineDrawer.SetLineThickness(thickness);
}

inline auto LineDrawerNoClippedEndPoints::DrawLine(const Point2dInt& point1,
                                                   const Point2dInt& point2,
                                                   const MultiplePixels& colors) noexcept -> void
{
  m_lineDrawer.DrawLine(point1, point2, colors);
}

inline LineDrawerClippedEndPoints::LineDrawerClippedEndPoints(IGoomDraw& draw) noexcept
  : m_dimensions{draw.GetDimensions()}, m_lineDrawer{PixelDrawerNoClipping{draw}}
{
}

inline auto LineDrawerClippedEndPoints::SetLineThickness(const uint8_t thickness) noexcept -> void
{
  if (thickness == m_lineDrawer.GetLineThickness())
  {
    return;
  }

  m_lineDrawer.SetLineThickness(thickness);
  m_clipTester.SetClipMargin(GetClipMargin());
}

inline auto LineDrawerClippedEndPoints::GetClipMargin() const noexcept -> int32_t
{
  static constexpr auto CLIP_MARGIN_FOR_THIN_LINE = 2;

  return 1U == m_lineDrawer.GetLineThickness() ? CLIP_MARGIN_FOR_THIN_LINE
                                               : m_lineDrawer.GetLineThickness();
}

inline auto LineDrawerClippedEndPoints::DrawLine(const Point2dInt& point1,
                                                 const Point2dInt& point2,
                                                 const MultiplePixels& colors) noexcept -> void
{
  if (m_clipTester.IsOutside(point1) or m_clipTester.IsOutside(point2))
  {
    return;
  }

  m_lineDrawer.DrawLine(point1, point2, colors);
}

template<class DrawPixelPolicy>
inline LineDrawer<DrawPixelPolicy>::LineDrawer(DrawPixelPolicy&& drawPixelPolicy) noexcept
  : m_drawPixel{std::move(drawPixelPolicy)}
{
}

template<class DrawPixelPolicy>
inline auto LineDrawer<DrawPixelPolicy>::GetDrawPixelPolicy() const noexcept
    -> const DrawPixelPolicy&
{
  return m_drawPixel;
}

template<class DrawPixelPolicy>
inline auto LineDrawer<DrawPixelPolicy>::GetDrawPixelPolicy() noexcept -> DrawPixelPolicy&
{
  return m_drawPixel;
}

template<class DrawPixelPolicy>
inline auto LineDrawer<DrawPixelPolicy>::GetLineThickness() const noexcept -> uint8_t
{
  return m_thickness;
}

template<class DrawPixelPolicy>
inline auto LineDrawer<DrawPixelPolicy>::SetLineThickness(const uint8_t thickness) noexcept -> void
{
  Expects(thickness >= 1U);
  m_thickness = thickness;
}

template<class DrawPixelPolicy>
inline auto LineDrawer<DrawPixelPolicy>::DrawLine(const Point2dInt& point1,
                                                  const Point2dInt& point2,
                                                  const MultiplePixels& colors) noexcept -> void
{
  if (1 == m_thickness)
  {
    DrawWuLine(point1, point2, colors);
  }
  else
  {
    DrawThickLine(point1, point2, colors);
  }
}

} // namespace GOOM::DRAW::SHAPE_DRAWERS

namespace GOOM::DRAW::SHAPE_DRAWERS
{

template<class DrawPixelPolicy>
inline auto LineDrawer<DrawPixelPolicy>::DrawWuLine(const Point2dInt& point1,
                                                    const Point2dInt& point2,
                                                    const MultiplePixels& colors) noexcept -> void
{
  Expects(1 == m_thickness);

  const auto plot = [this, &colors](const Point2dInt& point, const float brightness)
  { m_drawPixel.DrawPixels(point, brightness, colors); };

  WU_LINES::WuLine(static_cast<float>(point1.x),
                   static_cast<float>(point1.y),
                   static_cast<float>(point2.x),
                   static_cast<float>(point2.y),
                   plot);
}

template<class DrawPixelPolicy>
inline auto LineDrawer<DrawPixelPolicy>::DrawThickLine(const Point2dInt& point1,
                                                       const Point2dInt& point2,
                                                       const MultiplePixels& colors) noexcept
    -> void
{
  Expects(m_thickness > 1);

  const auto brightness = (0.8F * 2.0F) / static_cast<float>(m_thickness);

  const auto plot = [this, &brightness, &colors](const int32_t x, const int32_t y)
  { m_drawPixel.DrawPixels({x, y}, brightness, colors); };

  const auto getWidth =
      [this]([[maybe_unused]] const int32_t pointNum, [[maybe_unused]] const int32_t lineLength)
  { return static_cast<double>(m_thickness); };

  THICK_LINES::DrawVariableThicknessLine(
      plot, point1.x, point1.y, point2.x, point2.y, getWidth, getWidth);
}

} // namespace GOOM::DRAW::SHAPE_DRAWERS
