module;

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <mdspan.hpp>
#ifdef _MSC_VER // TODO(glk): Issuw with mdspan and MSVC. See at bottom.
#include <utility>
#endif

export module Goom.FilterFx.FilterEffects.AdjustmentEffects.FlowFieldGrid;

import Goom.FilterFx.NormalizedCoords;
import Goom.Lib.AssertUtils;
import Goom.Lib.Point2d;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class FlowFieldGrid
{
public:
  static constexpr auto GRID_HEIGHT = 500U;
  static constexpr auto GRID_WIDTH  = 500U;

  struct PolarCoords
  {
    float angle{};
    float radius{};
  };

  template<class BinaryFunc>
  auto Initialize(BinaryFunc func) -> void;

  [[nodiscard]] auto GetPolarCoords(const NormalizedCoords& coords) const noexcept -> PolarCoords;

private:
  std::array<PolarCoords, static_cast<size_t>(GRID_WIDTH) * static_cast<size_t>(GRID_HEIGHT)>
      m_gridArray{};
  std::mdspan<PolarCoords, std::extents<uint64_t, GRID_HEIGHT, GRID_WIDTH>> m_grid2dSpan{
      m_gridArray.data()};
  NormalizedCoordsConverter m_normalizedCoordsToGridConverter{
      {GRID_WIDTH, GRID_HEIGHT}
  };
};

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

#ifdef _MSC_VER // TODO(glk): Issue with mdspan and MSVC.
template<class BinaryFunc>
auto FlowFieldGrid::Initialize([[maybe_unused]] const BinaryFunc func) -> void
{
  std::unreachable();
}
#else
template<class BinaryFunc>
auto FlowFieldGrid::Initialize(const BinaryFunc func) -> void
{
  Expects(m_grid2dSpan.extent(0) == GRID_HEIGHT);
  Expects(m_grid2dSpan.extent(1) == GRID_WIDTH);

  for (auto y = 0U; y < GRID_HEIGHT; ++y)
  {
    for (auto x = 0U; x < GRID_WIDTH; ++x)
    {
      m_grid2dSpan[y, x] = func(x, y);
    }
  }
}
#endif

#ifdef _MSC_VER // TODO(glk): Issue with mdspan and MSVC.
inline auto FlowFieldGrid::GetPolarCoords(
    [[maybe_unused]] const NormalizedCoords& coords) const noexcept -> PolarCoords
{
  std::unreachable();
}
#else
inline auto FlowFieldGrid::GetPolarCoords(const NormalizedCoords& coords) const noexcept
    -> PolarCoords
{
  const auto gridCoords =
      ToPoint2dInt(m_normalizedCoordsToGridConverter.NormalizedToOtherCoordsFlt(coords));

  return m_grid2dSpan
      [static_cast<uint32_t>(std::clamp(gridCoords.y, 0, static_cast<int32_t>(GRID_HEIGHT - 1))),
       static_cast<uint32_t>(std::clamp(gridCoords.x, 0, static_cast<int32_t>(GRID_WIDTH - 1)))];
}
#endif

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
