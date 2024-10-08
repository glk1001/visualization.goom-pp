module;

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <utility>

module Goom.Draw.GoomDrawToContainer;

import Goom.Color.ColorUtils;
import Goom.Draw.GoomDrawBase;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;

namespace GOOM::DRAW
{

using COLOR::GetBrighterColorInt;

GoomDrawToContainer::GoomDrawToContainer(const Dimensions& dimensions) noexcept
  : IGoomDraw{dimensions}, m_xyPixelList(dimensions.GetHeight())
{
  for (auto& xPixelList : m_xyPixelList)
  {
    xPixelList.resize(dimensions.GetWidth());
  }
}

auto GoomDrawToContainer::ClearAll() noexcept -> void
{
  m_orderedXYPixelList.clear();

  for (auto& xPixelList : m_xyPixelList)
  {
    for (auto& colorsList : xPixelList)
    {
      colorsList.count = 0;
    }
  }
}

inline auto GoomDrawToContainer::GetWriteableColorsList(const Point2dInt& point) noexcept
    -> ColorsList&
{
  return m_xyPixelList.at(static_cast<size_t>(point.y)).at(static_cast<size_t>(point.x));
}

auto GoomDrawToContainer::DrawPixelsUnblended(
    [[maybe_unused]] const Point2dInt& point,
    [[maybe_unused]] const MultiplePixels& colors) noexcept -> void
{
  std::unreachable();
}

auto GoomDrawToContainer::DrawPixelsToDevice(const Point2dInt& point,
                                             const MultiplePixels& colors) noexcept -> void
{
  auto& colorsList = GetWriteableColorsList(point);

  if (colorsList.count == colorsList.colorsArray.size())
  {
    return;
  }

  // NOTE: Just save the first pixel in 'colors'. May need to improve this.
  const auto newColor = GetBrighterColorInt(GetIntBuffIntensity(), colors.color1);

  colorsList.colorsArray.at(colorsList.count) = newColor;
  ++colorsList.count;
  if (1 == colorsList.count)
  {
    m_orderedXYPixelList.emplace_back(point);
  }
}

auto GoomDrawToContainer::ResizeChangedCoordsKeepingNewest(const size_t numToKeep) noexcept -> void
{
  Expects(numToKeep <= m_orderedXYPixelList.size());

  const auto eraseFrom = cbegin(m_orderedXYPixelList);
  const auto eraseTo   = cbegin(m_orderedXYPixelList) +
                       static_cast<std::ptrdiff_t>(m_orderedXYPixelList.size() - numToKeep);

  for (auto coords = eraseFrom; coords != eraseTo; ++coords)
  {
    GetWriteableColorsList(*coords).count = 0;
  }

  m_orderedXYPixelList.erase(eraseFrom, eraseTo);
  m_orderedXYPixelList.resize(numToKeep);
}

auto GoomDrawToContainer::IterateChangedCoordsNewToOld(const CoordsFunc& func) const noexcept
    -> void
{
  const auto runFunc = [this, &func](const size_t i)
  {
    const auto& coords     = m_orderedXYPixelList[i];
    const auto& colorsList = GetColorsList(coords);
    func(coords, colorsList);
  };

  // Start with the newest coords added.
  const auto maxIndex = static_cast<int32_t>(m_orderedXYPixelList.size() - 1);
  for (auto i = maxIndex; i >= 0; --i)
  {
    runFunc(static_cast<size_t>(i));
  }
}

} // namespace GOOM::DRAW
