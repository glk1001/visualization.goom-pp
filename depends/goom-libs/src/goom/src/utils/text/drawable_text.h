#pragma once

#include "draw/shape_drawers/text_drawer.h"
#include "goom/point2d.h"

#include <cstdint>
#include <string>
#include <vector>

namespace GOOM::UTILS::TEXT
{

[[nodiscard]] auto GetLinesOfWords(const std::string& text, uint32_t maxLineLength)
    -> std::vector<std::string>;

[[nodiscard]] auto GetLeftAlignedPenForCentringStringAt(DRAW::SHAPE_DRAWERS::TextDrawer& textDrawer,
                                                        const std::string& text,
                                                        int32_t fontSize,
                                                        const Point2dInt& centreAt) -> Point2dInt;

} // namespace GOOM::UTILS::TEXT
