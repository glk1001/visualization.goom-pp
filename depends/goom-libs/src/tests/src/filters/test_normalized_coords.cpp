#include "filter_fx/normalized_coords.h"
#include "goom/point2d.h"
#include "utils/math/misc.h"

#include <cstdint>

#if __clang_major__ >= 16 // NOLINT: Can't include header for this.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wunsafe-buffer-usage"
#endif
#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#if __clang_major__ >= 16 // NOLINT: Can't include header for this.
#pragma GCC diagnostic pop
#endif

namespace GOOM::UNIT_TESTS
{

using FILTER_FX::NormalizedCoords;
using FILTER_FX::NormalizedCoordsConverter;
using UTILS::MATH::FloatsEqual;
using UTILS::MATH::HALF;

namespace
{

constexpr auto WIDTH                       = 1280U;
constexpr auto HEIGHT                      = 720U;
constexpr auto NORMALIZED_COORDS_CONVERTER = NormalizedCoordsConverter{
    {WIDTH, HEIGHT}
};

} // namespace

// NOLINTBEGIN(readability-function-cognitive-complexity)
TEST_CASE("Normalized Coords Values")
{
  SECTION("Min coords")
  {
    static constexpr auto COORDS =
        NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(Point2dInt{0, 0});
    UNSCOPED_INFO("coords.GetX() = " << COORDS.GetX());
    REQUIRE(FloatsEqual(COORDS.GetX(), NormalizedCoords::MIN_COORD));
    UNSCOPED_INFO("coords.GetY() = " << COORDS.GetY());
    REQUIRE(FloatsEqual(COORDS.GetY(), NormalizedCoords::MIN_COORD));

    const auto screenCoords =
        ToPoint2dInt(NORMALIZED_COORDS_CONVERTER.NormalizedToOtherCoordsFlt(COORDS));
    UNSCOPED_INFO("screenCoords.x = " << screenCoords.x);
    REQUIRE(screenCoords.x == 0);
    UNSCOPED_INFO("screenCoords.y = " << screenCoords.y);
    REQUIRE(screenCoords.y == 0);
  }

  SECTION("Max coords")
  {
    static constexpr auto COORDS =
        NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(Point2dInt{WIDTH - 1U, HEIGHT - 1U});
    UNSCOPED_INFO("coords.GetX() = " << COORDS.GetX());
    REQUIRE(FloatsEqual(COORDS.GetX(), NormalizedCoords::MAX_COORD));
    static constexpr auto MAX_Y =
        NormalizedCoords::MIN_COORD +
        (NormalizedCoords::COORD_WIDTH) *
            (static_cast<float>(HEIGHT - 1) / static_cast<float>(WIDTH - 1));
    UNSCOPED_INFO("coords.GetY() = " << COORDS.GetY());
    UNSCOPED_INFO("maxY = " << MAX_Y);
    REQUIRE(FloatsEqual(COORDS.GetY(), MAX_Y));

    const auto screenCoords =
        ToPoint2dInt(NORMALIZED_COORDS_CONVERTER.NormalizedToOtherCoordsFlt(COORDS));
    UNSCOPED_INFO("screenCoords.x = " << screenCoords.x);
    REQUIRE(screenCoords.x == WIDTH - 1);
    UNSCOPED_INFO("screenCoords.y = " << screenCoords.y);
    REQUIRE(screenCoords.y == HEIGHT - 1);
  }

  SECTION("Zero coords (middle)")
  {
    static constexpr auto EPSILON = 0.002F;
    static constexpr auto COORDS =
        NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(Point2dInt{WIDTH / 2, WIDTH / 2});
    UNSCOPED_INFO("coords.GetX() = " << COORDS.GetX());
    REQUIRE(FloatsEqual(COORDS.GetX(), 0.0F, EPSILON));
    UNSCOPED_INFO("coords.GetY() = " << COORDS.GetY());
    REQUIRE(FloatsEqual(COORDS.GetY(), 0.0F, EPSILON));

    const auto screenCoords =
        ToPoint2dInt(NORMALIZED_COORDS_CONVERTER.NormalizedToOtherCoordsFlt(COORDS));
    UNSCOPED_INFO("screenCoords.x = " << screenCoords.x);
    REQUIRE(screenCoords.x == WIDTH / 2);
    UNSCOPED_INFO("screenCoords.y = " << screenCoords.y);
    REQUIRE(screenCoords.y == WIDTH / 2);
  }

  SECTION("From normalized")
  {
    static constexpr auto X_COORD = 0.5F;
    static constexpr auto Y_COORD = 0.3F;
    static constexpr auto COORDS  = NormalizedCoords{X_COORD, Y_COORD};
    UNSCOPED_INFO("coords.GetX() = " << COORDS.GetX());
    REQUIRE(FloatsEqual(COORDS.GetX(), X_COORD));
    UNSCOPED_INFO("coords.GetY() = " << COORDS.GetY());
    REQUIRE(FloatsEqual(COORDS.GetY(), Y_COORD));

    const auto screenCoords =
        ToPoint2dInt(NORMALIZED_COORDS_CONVERTER.NormalizedToOtherCoordsFlt(COORDS));
    static constexpr auto X = static_cast<int32_t>(
        static_cast<float>(WIDTH - 1) *
        ((COORDS.GetX() - NormalizedCoords::MIN_COORD) / (NormalizedCoords::COORD_WIDTH)));
    UNSCOPED_INFO("screenCoords.x = " << screenCoords.x);
    UNSCOPED_INFO("X = " << X);
    REQUIRE(screenCoords.x == X);
  }
}
// NOLINTEND(readability-function-cognitive-complexity)

TEST_CASE("Normalized Coords Update")
{
  SECTION("IncX")
  {
    auto coords = NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(Point2dInt{0, 0});
    REQUIRE(FloatsEqual(coords.GetX(), NormalizedCoords::MIN_COORD));
    REQUIRE(FloatsEqual(coords.GetY(), NormalizedCoords::MIN_COORD));
    static constexpr auto STEP_SIZE = 0.11F;
    coords.IncX(STEP_SIZE);
    UNSCOPED_INFO("coords.GetX() = " << coords.GetX());
    REQUIRE(FloatsEqual(coords.GetX(), NormalizedCoords::MIN_COORD + STEP_SIZE));
  }
}

// NOLINTBEGIN(readability-function-cognitive-complexity)
TEST_CASE("Normalized Coords Operations", )
{
  SECTION("Plus")
  {
    static constexpr auto X_COORD = 0.5F;
    static constexpr auto Y_COORD = 0.3F;
    auto coords                   = NormalizedCoords{X_COORD, Y_COORD};
    coords += NormalizedCoords{X_COORD, Y_COORD};
    UNSCOPED_INFO("coords.GetX() = " << coords.GetX());
    REQUIRE(FloatsEqual(coords.GetX(), (X_COORD + X_COORD)));
    UNSCOPED_INFO("coords.GetY() = " << coords.GetY());
    REQUIRE(FloatsEqual(coords.GetY(), (Y_COORD + Y_COORD)));
  }

  SECTION("Minus")
  {
    auto coords                   = NormalizedCoords{1.0F, 1.0F};
    static constexpr auto X_COORD = 0.5F;
    static constexpr auto Y_COORD = 0.3F;
    coords -= NormalizedCoords{X_COORD, Y_COORD};
    UNSCOPED_INFO("coords.GetX() = " << coords.GetX());
    REQUIRE(FloatsEqual(coords.GetX(), (1.0F - X_COORD)));
    UNSCOPED_INFO("coords.GetY() = " << coords.GetY());
    REQUIRE(FloatsEqual(coords.GetY(), (1.0F - Y_COORD)));
  }

  SECTION("Scalar Mult")
  {
    static constexpr auto X_COORD = 0.5F;
    static constexpr auto Y_COORD = 0.3F;
    auto coords                   = NormalizedCoords{X_COORD, Y_COORD};
    coords *= HALF;
    UNSCOPED_INFO("coords.GetX() = " << coords.GetX());
    REQUIRE(FloatsEqual(coords.GetX(), HALF * X_COORD));
    UNSCOPED_INFO("coords.GetY() = " << coords.GetY());
    REQUIRE(FloatsEqual(coords.GetY(), HALF * Y_COORD));
  }
}
// NOLINTEND(readability-function-cognitive-complexity)

} // namespace GOOM::UNIT_TESTS
