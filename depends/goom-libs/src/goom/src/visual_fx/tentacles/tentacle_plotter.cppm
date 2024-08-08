module;

#include <algorithm>
#include <cstdint>
#include <functional>
#include <vector>

module Goom.VisualFx.TentaclesFx:TentaclePlotter;

import Goom.Draw.GoomDrawBase;
import Goom.Draw.ShaperDrawers.CircleDrawer;
import Goom.Draw.ShaperDrawers.LineDrawer;
import Goom.Utils.Graphics.LineClipper;
import Goom.Utils.Math.TValues;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;
import :Tentacle3d;

using GOOM::DRAW::IGoomDraw;
using GOOM::DRAW::MultiplePixels;
using GOOM::UTILS::GRAPHICS::LineClipper;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::NumberRange;
using GOOM::UTILS::MATH::TValue;

namespace GOOM::VISUAL_FX::TENTACLES
{

class TentaclePlotter
{
public:
  TentaclePlotter() noexcept = delete;
  TentaclePlotter(IGoomDraw& draw, const GoomRand& goomRand) noexcept;

  auto UpdateCameraPosition() noexcept -> void;
  auto SetTentacleLineThickness(uint8_t lineThickness) noexcept -> void;

  using GetColorsFunc = std::function<MultiplePixels(float nodeT)>;
  auto SetGetColorsFunc(const GetColorsFunc& getColorsFunc) noexcept -> void;
  auto SetEndDotColors(const MultiplePixels& endDotColors) noexcept -> void;
  auto SetNodeTOffset(float value) noexcept -> void;

  auto Plot3D(const Tentacle3D& tentacle) noexcept -> void;

private:
  IGoomDraw* m_draw;
  const GoomRand* m_goomRand;
  Vec2dFlt m_screenCentre = ToVec2dFlt(m_draw->GetDimensions().GetCentrePoint());

  DRAW::SHAPE_DRAWERS::LineDrawerNoClippedEndPoints m_lineDrawer{*m_draw};
  DRAW::SHAPE_DRAWERS::CircleDrawer m_circleDrawer{*m_draw};

  LineClipper m_lineClipper{GetLineClipRectangle(1U)};
  [[nodiscard]] auto GetLineClipRectangle(uint8_t lineThickness) const noexcept -> Rectangle2dInt;

  GetColorsFunc m_getColors;
  MultiplePixels m_endDotColors{};
  float m_nodeTOffset = 0.0F;

  static constexpr auto PROJECTION_DISTANCE   = 170.0F;
  static constexpr auto CAMERA_X_OFFSET_RANGE = NumberRange{-10.0F, +10.0F};
  static constexpr auto CAMERA_Y_OFFSET_RANGE = NumberRange{-10.0F, +10.0F};
  static constexpr auto CAMERA_Z_OFFSET_RANGE = NumberRange{+04.0F, // Don't make this any smaller
                                                            +10.1F};
  static_assert(CAMERA_X_OFFSET_RANGE.min < CAMERA_X_OFFSET_RANGE.max);
  static_assert(CAMERA_Y_OFFSET_RANGE.min < CAMERA_Y_OFFSET_RANGE.max);
  static_assert(CAMERA_Z_OFFSET_RANGE.min < CAMERA_Z_OFFSET_RANGE.max);
  V3dFlt m_cameraPosition{.x = 0.0F, .y = 0.0F, .z = CAMERA_Z_OFFSET_RANGE.min};

  auto PlotPoints(const std::vector<V3dFlt>& points3D) -> void;
  struct Line2DInt
  {
    Point2dInt point1;
    Point2dInt point2;
  };
  [[nodiscard]] auto GetPerspectiveProjection(const std::vector<V3dFlt>& points3D) const
      -> std::vector<Line2DInt>;
  [[nodiscard]] auto GetPerspectivePoint(const V3dFlt& point3D) const -> Point2dFlt;
  [[nodiscard]] static auto GetLine2D(const Point2dFlt& point1Flt,
                                      const Point2dFlt& point2Flt) noexcept -> Line2DInt;
  struct Line3DFlt
  {
    V3dFlt point1;
    V3dFlt point2;
  };
  [[nodiscard]] static auto GetLines3D(const std::vector<V3dFlt>& points3D)
      -> std::vector<Line3DFlt>;
};

} // namespace GOOM::VISUAL_FX::TENTACLES

namespace GOOM::VISUAL_FX::TENTACLES
{

inline auto TentaclePlotter::SetGetColorsFunc(const GetColorsFunc& getColorsFunc) noexcept -> void
{
  m_getColors = getColorsFunc;
}

inline auto TentaclePlotter::SetEndDotColors(const DRAW::MultiplePixels& endDotColors) noexcept
    -> void
{
  m_endDotColors = endDotColors;
}

inline auto TentaclePlotter::SetNodeTOffset(const float value) noexcept -> void
{
  m_nodeTOffset = value;
}

inline auto TentaclePlotter::UpdateCameraPosition() noexcept -> void
{
  m_cameraPosition = {.x = m_goomRand->GetRandInRange<CAMERA_X_OFFSET_RANGE>(),
                      .y = m_goomRand->GetRandInRange<CAMERA_Y_OFFSET_RANGE>(),
                      .z = m_goomRand->GetRandInRange<CAMERA_Z_OFFSET_RANGE>()};
}

TentaclePlotter::TentaclePlotter(IGoomDraw& draw, const GoomRand& goomRand) noexcept
  : m_draw{&draw}, m_goomRand{&goomRand}
{
}

auto TentaclePlotter::SetTentacleLineThickness(const uint8_t lineThickness) noexcept -> void
{
  m_lineDrawer.SetLineThickness(lineThickness);
  m_lineClipper.SetClipRectangle(GetLineClipRectangle(lineThickness));
}

inline auto TentaclePlotter::GetLineClipRectangle(const uint8_t lineThickness) const noexcept
    -> Rectangle2dInt
{
  const auto clipMargin  = static_cast<int32_t>(lineThickness + 1U);
  const auto topLeft     = Point2dInt{.x = clipMargin, .y = clipMargin};
  const auto bottomRight = Point2dInt{.x = m_draw->GetDimensions().GetIntWidth() - clipMargin,
                                      .y = m_draw->GetDimensions().GetIntHeight() - clipMargin};

  return {.topLeft = topLeft, .bottomRight = bottomRight};
}

auto TentaclePlotter::Plot3D(const Tentacle3D& tentacle) noexcept -> void
{
  const auto points3D = tentacle.GetTentacleVertices(m_cameraPosition);

  PlotPoints(points3D);
}

inline auto TentaclePlotter::PlotPoints(const std::vector<V3dFlt>& points3D) -> void
{
  const auto lines2D = GetPerspectiveProjection(points3D);

  const auto numNodes = static_cast<uint32_t>(lines2D.size());
  if (0 == numNodes)
  {
    return;
  }

  auto nodeT = TValue{
      {.stepType  = TValue::StepType::CONTINUOUS_REVERSIBLE,
       .numSteps  = numNodes,
       .startingT = m_nodeTOffset}
  };
  for (const auto& line : lines2D)
  {
    const auto colors = m_getColors(nodeT());
    m_lineDrawer.DrawLine(line.point1, line.point2, colors);
    nodeT.Increment();
  }

  static constexpr auto END_DOT_RADIUS = 1;
  m_circleDrawer.DrawFilledCircle(lines2D.back().point2, END_DOT_RADIUS, m_endDotColors);
}

auto TentaclePlotter::GetPerspectiveProjection(const std::vector<V3dFlt>& points3D) const
    -> std::vector<Line2DInt>
{
  const auto lines3D = GetLines3D(points3D);

  auto lines2D = std::vector<Line2DInt>{};
  for (const auto& line3D : lines3D)
  {
    if (static constexpr auto MIN_Z = 2.0F;
        (line3D.point1.z <= MIN_Z) or (line3D.point2.z <= MIN_Z))
    {
      continue;
    }

    const auto pointFlt1 = GetPerspectivePoint(line3D.point1);
    const auto pointFlt2 = GetPerspectivePoint(line3D.point2);

    const auto clippedLine =
        m_lineClipper.GetClippedLine({.point1 = pointFlt1, .point2 = pointFlt2});
    if (clippedLine.clipResult == LineClipper::ClipResult::REJECTED)
    {
      continue;
    }

    const auto line2D = GetLine2D(clippedLine.line.point1, clippedLine.line.point2);

    lines2D.emplace_back(line2D);
  }

  return lines2D;
}

inline auto TentaclePlotter::GetPerspectivePoint(const V3dFlt& point3D) const -> Point2dFlt
{
  const auto perspectiveFactor = PROJECTION_DISTANCE / point3D.z;
  const auto xProj             = perspectiveFactor * point3D.x;
  const auto yProj             = perspectiveFactor * point3D.y;

  return Point2dFlt{.x = xProj, .y = -yProj} + m_screenCentre;
}

inline auto TentaclePlotter::GetLines3D(const std::vector<V3dFlt>& points3D)
    -> std::vector<Line3DFlt>
{
  auto lines3D = std::vector<Line3DFlt>{};
  if (points3D.size() < 2)
  {
    return lines3D;
  }

  const auto numPointsMinus1 = points3D.size() - 1;
  for (auto i = 0U; i < numPointsMinus1; ++i)
  {
    lines3D.emplace_back(Line3DFlt{.point1 = points3D[i], .point2 = points3D[i + 1]});
  }

  return lines3D;
}

inline auto TentaclePlotter::GetLine2D(const Point2dFlt& point1Flt,
                                       const Point2dFlt& point2Flt) noexcept -> Line2DInt
{
  auto line2D = Line2DInt{.point1 = ToPoint2dInt(point1Flt), .point2 = ToPoint2dInt(point2Flt)};

  if (line2D.point1 == line2D.point2)
  {
    return line2D;
  }

  // TODO(glk) - What about the last line??
  // We are drawing joined lines and we don't want to re-plot the last
  // point of the previous line. So back up one pixel from 'point2'.
  const auto dx = std::clamp(line2D.point2.x - line2D.point1.x, -1, +1);
  const auto dy = std::clamp(line2D.point2.y - line2D.point1.y, -1, +1);
  line2D.point2.x -= dx;
  line2D.point2.y -= dy;

  return line2D;
}

} // namespace GOOM::VISUAL_FX::TENTACLES
