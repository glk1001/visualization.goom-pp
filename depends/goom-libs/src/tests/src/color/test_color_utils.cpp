// NOLINTBEGIN(cert-err58-cpp): Catch2 3.6.0 issue

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <cstdint>

import Goom.Color.ColorUtils;
import Goom.Utils.Graphics.PixelUtils;
import Goom.Lib.GoomGraphic;

namespace GOOM::UNIT_TESTS
{

using COLOR::GetBrighterChannelColor;
using COLOR::GetBrighterColor;
using COLOR::GetLightenedColor;
using COLOR::GetRgbColorLerp;
using GOOM::UTILS::GRAPHICS::CHANNEL_COLOR_SCALAR_DIVISOR;

// NOLINTBEGIN(bugprone-chained-comparison): Catch2 needs to fix this.
// NOLINTBEGIN(readability-function-cognitive-complexity)
TEST_CASE("Test max channels")
{
  static constexpr auto CHANNEL_MAX = 255;
  REQUIRE(channel_limits<uint8_t>::min() == 0);
  REQUIRE(channel_limits<uint8_t>::max() == CHANNEL_MAX);
  REQUIRE(channel_limits<uint32_t>::min() == 0);
  REQUIRE(channel_limits<uint32_t>::max() == CHANNEL_MAX);
  REQUIRE(channel_limits<int>::min() == 0);
  REQUIRE(channel_limits<int>::max() == CHANNEL_MAX);
  REQUIRE(channel_limits<float>::min() == 0.0F);
  REQUIRE(channel_limits<float>::max() == static_cast<float>(CHANNEL_MAX));
}

TEST_CASE("Color channels are brightened")
{
  REQUIRE(GetBrighterChannelColor(100U, 2U) == (100U * 2U) / CHANNEL_COLOR_SCALAR_DIVISOR);
  REQUIRE(GetBrighterChannelColor(11U, 20U) == (11U * 20U) / CHANNEL_COLOR_SCALAR_DIVISOR);
  REQUIRE(GetBrighterChannelColor(0U, 20U) == 0U);
  REQUIRE(
      GetBrighterChannelColor(100U, 20U) ==
      std::clamp(0U, (100U * 20U) / CHANNEL_COLOR_SCALAR_DIVISOR, CHANNEL_COLOR_SCALAR_DIVISOR));
}

TEST_CASE("Colors are brightened")
{
  static constexpr auto RED   = 100;
  static constexpr auto GREEN = 50;
  static constexpr auto BLUE  = 20;
  static constexpr auto COLOR = Pixel{
      {.red = RED, .green = GREEN, .blue = BLUE}
  };

  auto brighterColor = GetBrighterColor(1.0F, COLOR);
  REQUIRE(brighterColor.R() == RED);
  REQUIRE(brighterColor.G() == GREEN);
  REQUIRE(brighterColor.B() == BLUE);

  static constexpr auto HALF_BRIGHTNESS = 0.5F;
  brighterColor                         = GetBrighterColor(HALF_BRIGHTNESS, COLOR);
  REQUIRE(brighterColor.R() == (RED / 2));
  REQUIRE(brighterColor.G() == (GREEN / 2));
  REQUIRE(brighterColor.B() == (BLUE / 2));

  static constexpr auto SMALL_BRIGHTNESS = 0.01F;
  brighterColor                          = GetBrighterColor(SMALL_BRIGHTNESS, COLOR);
  REQUIRE(brighterColor.R() == 1);
  REQUIRE(brighterColor.G() == 0);
  REQUIRE(brighterColor.B() == 0);
}

TEST_CASE("Color Lerp")
{
  static constexpr auto LERP_T1   = 0.5F;
  const auto lerpedColor1         = GetRgbColorLerp(BLACK_PIXEL, WHITE_PIXEL, LERP_T1);
  const auto expectedLerpedColor1 = std::lround(LERP_T1 * static_cast<float>(MAX_COLOR_VAL));
  REQUIRE(lerpedColor1.R() == expectedLerpedColor1);
  REQUIRE(lerpedColor1.G() == expectedLerpedColor1);
  REQUIRE(lerpedColor1.B() == expectedLerpedColor1);

  static constexpr auto LERP_T2   = 0.333333F;
  const auto lerpedColor2         = GetRgbColorLerp(BLACK_PIXEL, WHITE_PIXEL, LERP_T2);
  const auto expectedLerpedColor2 = std::lround(LERP_T2 * static_cast<float>(MAX_COLOR_VAL));
  REQUIRE(lerpedColor2.R() == expectedLerpedColor2);
  REQUIRE(lerpedColor2.G() == expectedLerpedColor2);
  REQUIRE(lerpedColor2.B() == expectedLerpedColor2);
}

TEST_CASE("Lighten")
{
  static constexpr auto RED   = 100;
  static constexpr auto GREEN = 0;
  static constexpr auto BLUE  = 0;
  static constexpr auto COLOR = Pixel{
      {.red = RED, .green = GREEN, .blue = BLUE}
  };

  const auto lightenedColor = GetLightenedColor(COLOR, 10.0);
  REQUIRE(static_cast<uint32_t>(lightenedColor.R()) == 50);
  REQUIRE(static_cast<uint32_t>(lightenedColor.G()) == 0);
  REQUIRE(static_cast<uint32_t>(lightenedColor.B()) == 0);
}

TEST_CASE("Lightened color")
{
  static constexpr auto RED   = 100;
  static constexpr auto GREEN = 50;
  static constexpr auto BLUE  = 20;
  static constexpr auto COLOR = Pixel{
      {.red = RED, .green = GREEN, .blue = BLUE}
  };

  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

  auto lightenedColor = GetLightenedColor(COLOR, 0.5);
  REQUIRE(lightenedColor.R() == 0);
  REQUIRE(lightenedColor.G() == 0);
  REQUIRE(lightenedColor.B() == 0);

  lightenedColor = GetLightenedColor(COLOR, 1.0);
  REQUIRE(lightenedColor.R() == 0);
  REQUIRE(lightenedColor.G() == 0);
  REQUIRE(lightenedColor.B() == 0);

  lightenedColor = GetLightenedColor(COLOR, 2.0);
  REQUIRE(lightenedColor.R() == 15);
  REQUIRE(lightenedColor.G() == 7);
  REQUIRE(lightenedColor.B() == 3);

  lightenedColor = GetLightenedColor(COLOR, 5.0);
  REQUIRE(lightenedColor.R() == 34);
  REQUIRE(lightenedColor.G() == 17);
  REQUIRE(lightenedColor.B() == 6);

  lightenedColor = GetLightenedColor(COLOR, 10.0);
  REQUIRE(lightenedColor.R() == 50);
  REQUIRE(lightenedColor.G() == 25);
  REQUIRE(lightenedColor.B() == 10);

  static constexpr auto COLOR2 = WHITE_PIXEL;
  lightenedColor               = GetLightenedColor(COLOR2, 1.0);
  REQUIRE(lightenedColor.R() == 0);
  REQUIRE(lightenedColor.G() == 0);
  REQUIRE(lightenedColor.B() == 0);

  lightenedColor = GetLightenedColor(COLOR2, 2.0);
  REQUIRE(lightenedColor.R() == 38);
  REQUIRE(lightenedColor.G() == 38);
  REQUIRE(lightenedColor.B() == 38);

  lightenedColor = GetLightenedColor(COLOR2, 5.0);
  REQUIRE(lightenedColor.R() == 89);
  REQUIRE(lightenedColor.G() == 89);
  REQUIRE(lightenedColor.B() == 89);

  lightenedColor = GetLightenedColor(COLOR2, 10.0);
  REQUIRE(lightenedColor.R() == 127);
  REQUIRE(lightenedColor.G() == 127);
  REQUIRE(lightenedColor.B() == 127);

  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
}
// NOLINTEND(readability-function-cognitive-complexity)
// NOLINTEND(bugprone-chained-comparison)

} // namespace GOOM::UNIT_TESTS

// NOLINTEND(cert-err58-cpp): Catch2 3.6.0 issue
