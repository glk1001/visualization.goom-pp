// NOLINTBEGIN(cert-err58-cpp): Catch2 3.6.0 issue

#include <algorithm>
#include <array>
#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <format>
#include <vector>

import Goom.Draw.GoomDrawBase;
import Goom.Draw.GoomDrawToContainer;
import Goom.Lib.GoomGraphic;
import Goom.Lib.Point2d;

namespace GOOM::UNIT_TESTS
{

using DRAW::GoomDrawToContainer;
using DRAW::MultiplePixels;
using ColorsList = GoomDrawToContainer::ColorsList;

static constexpr auto WIDTH  = 100;
static constexpr auto HEIGHT = 100;

struct PixelInfo
{
  Point2dInt point{};
  MultiplePixels colors{};
};

// NOLINTBEGIN(bugprone-chained-comparison): Catch2 needs to fix this.
// NOLINTBEGIN(misc-const-correctness)

namespace
{

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void CheckPixels(const std::vector<PixelInfo>& changedPixels,
                 const std::vector<PixelInfo>& expectedPixels)
{
  // '1' is old, 'expectedPixels.size() - 1' is new.
  for (auto i = 0U; i < expectedPixels.size(); ++i)
  {
    const auto& point  = expectedPixels[i].point;
    const auto& colors = expectedPixels[i].colors;

    const auto& coords = changedPixels[i];

    INFO(std::format("i = {}, coords = ({}, {}), (x, y) = ({}, {})",
                     i,
                     coords.point.x,
                     coords.point.y,
                     point.x,
                     point.y));
    REQUIRE(coords.point.x == point.x);
    REQUIRE(coords.point.y == point.y);

    INFO(std::format("coords.colors[{}] = ({}, {}, {}, {}), colors[{}] = ({}, {}, {}, {})",
                     0,
                     coords.colors.color1.R(),
                     coords.colors.color1.G(),
                     coords.colors.color1.B(),
                     coords.colors.color1.A(),
                     0,
                     colors.color1.R(),
                     colors.color1.G(),
                     colors.color1.B(),
                     colors.color1.A()));
    REQUIRE(coords.colors.color1 == colors.color1);
    REQUIRE(coords.colors.color2 == BLACK_PIXEL);
  }
}

void CheckContainer(const GoomDrawToContainer& draw, const std::vector<PixelInfo>& expectedPixels)
{
  INFO(std::format("draw.GetNumChangedCoords() = {}", draw.GetNumChangedCoords()));
  REQUIRE(draw.GetNumChangedCoords() == expectedPixels.size());

  auto changedPixels       = std::vector<PixelInfo>{};
  const auto emplaceCoords = [&](const Point2dInt& point, const ColorsList& colorsList)
  {
    changedPixels.emplace_back(PixelInfo{
        .point = point, .colors = {.color1 = colorsList.colorsArray[0], .color2 = BLACK_PIXEL}
    });
  };
  draw.IterateChangedCoordsNewToOld(emplaceCoords);
  REQUIRE(changedPixels.size() == expectedPixels.size());

  CheckPixels(changedPixels, expectedPixels);
}

[[nodiscard]] auto GetPixelInfo(uint32_t pixelNum) -> PixelInfo
{
  const auto point =
      Point2dInt{.x = static_cast<int32_t>(pixelNum), .y = static_cast<int32_t>(pixelNum)};
  const auto chan0  = static_cast<PixelChannelType>(pixelNum);
  const auto chan1  = static_cast<PixelChannelType>(pixelNum + 1);
  const auto color0 = Pixel{
      {.red = chan0, .green = chan0, .blue = chan0, .alpha = 255U}
  };
  const auto color1 = Pixel{
      {.red = chan1, .green = chan1, .blue = chan1, .alpha = 0U}
  };
  const auto colors = MultiplePixels{.color1 = color0, .color2 = color1};

  return {.point = point, .colors = colors};
}

[[nodiscard]] auto GetPixelsNewToOld(GoomDrawToContainer* const draw,
                                     const uint32_t numChanged) noexcept -> std::vector<PixelInfo>
{
  auto pixelsNewToOld = std::vector<PixelInfo>{};

  // Add some changed coords - '1' is old, 'numChanged' is new.
  for (auto i = 1U; i <= numChanged; ++i)
  {
    const auto pixelInfo = GetPixelInfo(i);

    pixelsNewToOld.emplace_back(pixelInfo);

    draw->DrawPixels(pixelInfo.point, pixelInfo.colors);
    REQUIRE(draw->GetPixels(pixelInfo.point).color1 == pixelInfo.colors.color1);
    REQUIRE(draw->GetPixels(pixelInfo.point).color2 == BLACK_PIXEL);
  }
  std::ranges::reverse(pixelsNewToOld);

  return pixelsNewToOld;
}

auto FillDrawContainer(GoomDrawToContainer* const draw, const uint32_t numChanged)
    -> std::vector<PixelInfo>
{
  auto pixelsNewToOld = GetPixelsNewToOld(draw, numChanged);

  REQUIRE(pixelsNewToOld.size() == numChanged);
  REQUIRE(pixelsNewToOld.front().point.x == static_cast<int32_t>(numChanged));
  REQUIRE(pixelsNewToOld.front().point.y == static_cast<int32_t>(numChanged));
  REQUIRE(pixelsNewToOld.back().point.x == 1);
  REQUIRE(pixelsNewToOld.back().point.y == 1);

  return pixelsNewToOld;
}

} // namespace

TEST_CASE("Test DrawMovingText to Container")
{
  GoomDrawToContainer draw{
      {WIDTH, HEIGHT}
  };

  draw.SetBuffIntensity(1.0F);

  static constexpr auto NUM_CHANGED_COORDS    = 5U;
  const std::vector<PixelInfo> pixelsNewToOld = FillDrawContainer(&draw, NUM_CHANGED_COORDS);

  auto i = static_cast<int32_t>(NUM_CHANGED_COORDS);
  for (const auto& pixelInfo : pixelsNewToOld)
  {
    REQUIRE(pixelInfo.point.x == i);
    REQUIRE(pixelInfo.point.y == i);
    --i;
  }
  CheckContainer(draw, pixelsNewToOld);
}

TEST_CASE("Test Resized DrawMovingText to Container")
{
  GoomDrawToContainer draw{
      {WIDTH, HEIGHT}
  };
  draw.SetBuffIntensity(1.0F);

  static constexpr auto NUM_CHANGED_COORDS = 5U;
  std::vector<PixelInfo> pixelsNewToOld    = FillDrawContainer(&draw, NUM_CHANGED_COORDS);

  static constexpr auto NEW_SIZE = NUM_CHANGED_COORDS / 2U;
  draw.ResizeChangedCoordsKeepingNewest(NEW_SIZE);
  pixelsNewToOld.resize(NEW_SIZE);

  auto i = static_cast<int32_t>(NUM_CHANGED_COORDS);
  for (const auto& pixelInfo : pixelsNewToOld)
  {
    REQUIRE(pixelInfo.point.x == i);
    REQUIRE(pixelInfo.point.y == i);
    --i;
  }
  CheckContainer(draw, pixelsNewToOld);
}

TEST_CASE("Test DrawMovingText to Container with Duplicates")
{
  auto draw = GoomDrawToContainer{
      {WIDTH, HEIGHT}
  };

  draw.SetBuffIntensity(1.0F);

  static constexpr auto NUM_CHANGED_COORDS = 5U;
  std::vector<PixelInfo> pixelsNewToOld    = FillDrawContainer(&draw, NUM_CHANGED_COORDS);

  const auto color0 = Pixel{
      {.red = 10, .green = 10, .blue = 10, .alpha = 255U}
  };
  const auto color1 = Pixel{
      {.red = 11, .green = 11, .blue = 11, .alpha = 0U}
  };
  const auto colors = MultiplePixels{.color1 = color0, .color2 = color1};
  const auto oldest = pixelsNewToOld.back();
  draw.DrawPixels(oldest.point, colors);
  REQUIRE(draw.GetNumChangedCoords() == NUM_CHANGED_COORDS);

  const auto& colorsListOldest = draw.GetColorsList(oldest.point);
  REQUIRE(2 == colorsListOldest.count);

  const auto& coords0     = draw.GetChangedCoordsList()[0];
  const auto& colorsList0 = draw.GetColorsList(coords0);
  REQUIRE(colorsListOldest.count == colorsList0.count);
  REQUIRE(colorsListOldest.colorsArray == colorsList0.colorsArray);

  draw.ResizeChangedCoordsKeepingNewest(NUM_CHANGED_COORDS - 1);
  REQUIRE(draw.GetNumChangedCoords() == NUM_CHANGED_COORDS - 1);
  REQUIRE(0 == draw.GetColorsList(oldest.point).count);
}

TEST_CASE("Test DrawMovingText ClearAll", "[GoomDrawToContainerClearAll]")
{
  GoomDrawToContainer draw{
      {WIDTH, HEIGHT}
  };

  draw.SetBuffIntensity(1.0F);

  static constexpr auto NUM_CHANGED_COORDS = 5U;
  const auto pixelsNewToOld                = FillDrawContainer(&draw, NUM_CHANGED_COORDS);

  for (const auto& pixelInfo : pixelsNewToOld)
  {
    const auto& colorsList = draw.GetColorsList(pixelInfo.point);
    REQUIRE(0 != colorsList.count);
  }

  draw.ClearAll();

  REQUIRE(draw.GetNumChangedCoords() == 0);
  REQUIRE(draw.GetChangedCoordsList().empty());

  for (const auto& pixelInfo : pixelsNewToOld)
  {
    const auto& colorsList = draw.GetColorsList(pixelInfo.point);
    REQUIRE(0 == colorsList.count);
  }
}

// NOLINTEND(misc-const-correctness)
// NOLINTEND(bugprone-chained-comparison)

} // namespace GOOM::UNIT_TESTS

// NOLINTEND(cert-err58-cpp): Catch2 3.6.0 issue
