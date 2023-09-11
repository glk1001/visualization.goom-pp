#ifndef GOOM_DEBUG
#define GOOM_DEBUG
#endif

#include "goom/goom_config.h"
#include "goom/goom_lerp_data.h"
#include "goom/math20.h"

#if __clang_major__ >= 16 // NOLINT: Can't include header for this.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wunsafe-buffer-usage"
#endif
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#if __clang_major__ >= 16 // NOLINT: Can't include header for this.
#pragma GCC diagnostic pop
#endif

#include <cstdint>

namespace GOOM::UNIT_TESTS
{

using Catch::Approx;

namespace
{

auto UpdateLerpData(GoomLerpData& lerpData, const uint32_t num)
{
  for (auto i = 0U; i < num; ++i)
  {
    lerpData.Update();
  }
}

} // namespace

// NOLINTBEGIN(readability-function-cognitive-complexity)

TEST_CASE("LerpData Simple")
{
  auto lerpData = GoomLerpData{};

  REQUIRE(lerpData.GetLerpFactor() == Approx(0.0F));
  REQUIRE(lerpData.GetIncrement() == Approx(0.0F));
  REQUIRE(lerpData.GetLerpToOneFactor() == Approx(0.0F));

  static constexpr auto TEST_INCREMENT = 0.1F;
  lerpData.SetIncrement(TEST_INCREMENT);
  lerpData.Update();
  REQUIRE(lerpData.GetIncrement() == Approx(TEST_INCREMENT));
  REQUIRE(lerpData.GetLerpFactor() == Approx(TEST_INCREMENT));
  lerpData.Update();
  REQUIRE(lerpData.GetIncrement() == Approx(TEST_INCREMENT));
  REQUIRE(lerpData.GetLerpFactor() == Approx(TEST_INCREMENT + TEST_INCREMENT));

  lerpData.Reset();
  REQUIRE(lerpData.GetLerpFactor() == Approx(0.0F));
  REQUIRE(lerpData.GetIncrement() == Approx(0.0F));
  REQUIRE(lerpData.GetLerpToOneFactor() == Approx(0.0F));

  static constexpr auto TEST_LERP_TO_ONE_FACTOR = 0.7F;
  lerpData.SetLerpToOneFactor(TEST_LERP_TO_ONE_FACTOR);
  lerpData.Update();
  REQUIRE(lerpData.GetLerpFactor() == Approx(TEST_LERP_TO_ONE_FACTOR));
  REQUIRE(lerpData.GetIncrement() == Approx(0.0F));
  REQUIRE(lerpData.GetLerpToOneFactor() == Approx(TEST_LERP_TO_ONE_FACTOR));
}

TEST_CASE("LerpData More Complicated")
{
  auto lerpData = GoomLerpData{};

  REQUIRE(lerpData.GetLerpFactor() == Approx(0.0F));
  REQUIRE(lerpData.GetIncrement() == Approx(0.0F));
  REQUIRE(lerpData.GetLerpToOneFactor() == Approx(0.0F));

  static constexpr auto TEST_INCREMENT = 0.1F;
  lerpData.SetIncrement(TEST_INCREMENT);

  static constexpr auto NUM_INCS_JUST_BEFORE_END =
      static_cast<uint32_t>(1.0F / TEST_INCREMENT) - 1U;
  UpdateLerpData(lerpData, NUM_INCS_JUST_BEFORE_END);

  REQUIRE(lerpData.GetLerpFactor() == Approx(1.0F - TEST_INCREMENT));
  lerpData.Update();
  REQUIRE(lerpData.GetLerpFactor() == Approx(1.0F));
  lerpData.Update();
  REQUIRE(lerpData.GetLerpFactor() == Approx(1.0F));

  lerpData.SetLerpFactor(0.0F);
  lerpData.SetIncrement(0.0F);
  static constexpr auto TEST_LERP_TO_ONE_FACTOR   = 0.2F;
  static constexpr auto NUM_LERPS_JUST_BEFORE_END = 7U;
  lerpData.SetLerpToOneFactor(TEST_LERP_TO_ONE_FACTOR);
  UpdateLerpData(lerpData, NUM_LERPS_JUST_BEFORE_END);
  REQUIRE(lerpData.GetLerpFactor() <= 1.0F - TEST_LERP_TO_ONE_FACTOR);
  static constexpr auto NUM_TO_CONVERGE_TO_ONE = 45U;
  UpdateLerpData(lerpData, NUM_TO_CONVERGE_TO_ONE);
  REQUIRE(lerpData.GetLerpFactor() == Approx(1.0F));

  lerpData.Reset();
  lerpData.SetIncrement(TEST_INCREMENT);
  lerpData.SetLerpToOneFactor(TEST_LERP_TO_ONE_FACTOR);
  lerpData.Update();
  static constexpr auto EXPECTED_LERP_FACTOR =
      STD20::lerp(TEST_INCREMENT, 1.0F, TEST_LERP_TO_ONE_FACTOR);
  REQUIRE(lerpData.GetLerpFactor() == Approx(EXPECTED_LERP_FACTOR));

  lerpData.SetLerpToOneFactor(0.0F);
  lerpData.Update();
  REQUIRE(lerpData.GetLerpFactor() == Approx(EXPECTED_LERP_FACTOR + TEST_INCREMENT));
}

TEST_CASE("LerpData Negative Increment")
{
  auto lerpData = GoomLerpData{};

  REQUIRE(lerpData.GetLerpFactor() == Approx(0.0F));
  REQUIRE(lerpData.GetIncrement() == Approx(0.0F));
  REQUIRE(lerpData.GetLerpToOneFactor() == Approx(0.0F));

  static constexpr auto TEST_INCREMENT = -0.1F;
  lerpData.SetIncrement(TEST_INCREMENT);
  lerpData.SetLerpFactor(1.0F);

  lerpData.Update();
  REQUIRE(lerpData.GetLerpFactor() == Approx(1.0F + TEST_INCREMENT));

  static constexpr auto NUM_INCS_TO_START = static_cast<uint32_t>(1.0F / (-TEST_INCREMENT)) - 2U;
  UpdateLerpData(lerpData, NUM_INCS_TO_START);
  REQUIRE(lerpData.GetLerpFactor() == Approx(-TEST_INCREMENT));

  lerpData.Update();
  REQUIRE(lerpData.GetLerpFactor() == Approx(0.0F));
  lerpData.Update();
  REQUIRE(lerpData.GetLerpFactor() == Approx(0.0F));
}

// NOLINTEND(readability-function-cognitive-complexity)

} // namespace GOOM::UNIT_TESTS
