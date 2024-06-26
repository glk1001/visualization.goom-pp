module;

#include <cstdint>
#include <functional>

export module Goom.Draw.ShaperDrawers.LineDrawThick;

export namespace GOOM::DRAW::SHAPE_DRAWERS::THICK_LINES
{

using PlotPointFunc    = std::function<void(int32_t x, int32_t y)>;
using GetLineWidthFunc = std::function<double(int32_t pointNum, int32_t lineLength)>;

auto DrawVariableThicknessLine(const PlotPointFunc& plot,
                               int32_t x0,
                               int32_t y0,
                               int32_t x1,
                               int32_t y1,
                               GetLineWidthFunc getLeftWidth,
                               GetLineWidthFunc getRightWidth) noexcept -> void;

} // namespace GOOM::DRAW::SHAPE_DRAWERS::THICK_LINES
