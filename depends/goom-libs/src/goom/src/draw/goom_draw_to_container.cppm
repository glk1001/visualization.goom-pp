module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

export module Goom.Draw.GoomDrawToContainer;

import Goom.Draw.GoomDrawBase;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;

export namespace GOOM::DRAW
{

class GoomDrawToContainer : public IGoomDraw
{
public:
  explicit GoomDrawToContainer(const Dimensions& dimensions) noexcept;

  [[nodiscard]] auto GetPixel(const Point2dInt& point) const noexcept -> Pixel override;
  [[noreturn]] auto DrawPixelsUnblended(const Point2dInt& point,
                                        const MultiplePixels& colors) noexcept -> void override;

  [[nodiscard]] auto GetPixels(const Point2dInt& point) const noexcept -> MultiplePixels;

  static constexpr size_t MAX_NUM_COLORS_LIST = 3;
  using ColorsArray                           = std::array<Pixel, MAX_NUM_COLORS_LIST>;
  struct ColorsList
  {
    uint8_t count = 0;
    ColorsArray colorsArray{};
  };
  [[nodiscard]] auto GetNumChangedCoords() const noexcept -> size_t;
  [[nodiscard]] auto GetChangedCoordsList() const noexcept -> const std::vector<Point2dInt>&;
  // IMPORTANT: The above is ordered from oldest to newest.
  [[nodiscard]] auto GetColorsList(const Point2dInt& point) const noexcept -> const ColorsList&;

  using CoordsFunc = std::function<void(const Point2dInt& point, const ColorsList& colorsList)>;
  // NOTE: 'func' must be thread-safe.
  auto IterateChangedCoordsNewToOld(const CoordsFunc& func) const noexcept -> void;

  auto ResizeChangedCoordsKeepingNewest(size_t numToKeep) noexcept -> void;
  auto ClearAll() noexcept -> void;

protected:
  auto DrawPixelsToDevice(const Point2dInt& point,
                          const MultiplePixels& colors) noexcept -> void override;

private:
  std::vector<std::vector<ColorsList>> m_xyPixelList;
  std::vector<Point2dInt> m_orderedXYPixelList;
  [[nodiscard]] auto GetWriteableColorsList(const Point2dInt& point) noexcept -> ColorsList&;
  [[nodiscard]] auto GetLastDrawnColor(const Point2dInt& point) const noexcept -> Pixel;
  [[nodiscard]] auto GetLastDrawnColors(const Point2dInt& point) const noexcept -> MultiplePixels;
};

} // namespace GOOM::DRAW

namespace GOOM::DRAW
{

inline auto GoomDrawToContainer::GetPixel(const Point2dInt& point) const noexcept -> Pixel
{
  return GetLastDrawnColor(point);
}

inline auto GoomDrawToContainer::GetPixels(const Point2dInt& point) const noexcept -> MultiplePixels
{
  return GetLastDrawnColors(point);
}

inline auto GoomDrawToContainer::GetColorsList(const Point2dInt& point) const noexcept
    -> const ColorsList&
{
  return m_xyPixelList.at(static_cast<size_t>(point.y)).at(static_cast<size_t>(point.x));
}

inline auto GoomDrawToContainer::GetNumChangedCoords() const noexcept -> size_t
{
  return m_orderedXYPixelList.size();
}

inline auto GoomDrawToContainer::GetChangedCoordsList() const noexcept
    -> const std::vector<Point2dInt>&
{
  return m_orderedXYPixelList;
}

inline auto GoomDrawToContainer::GetLastDrawnColor(const Point2dInt& point) const noexcept -> Pixel
{
  const auto& colorsList = GetColorsList(point);
  if (0 == colorsList.count)
  {
    return BLACK_PIXEL;
  }
  return colorsList.colorsArray.at(static_cast<size_t>(colorsList.count - 1));
}

inline auto GoomDrawToContainer::GetLastDrawnColors(const Point2dInt& point) const noexcept
    -> MultiplePixels
{
  return {.color1 = GetLastDrawnColor(point), .color2 = BLACK_PIXEL};
}

} // namespace GOOM::DRAW
