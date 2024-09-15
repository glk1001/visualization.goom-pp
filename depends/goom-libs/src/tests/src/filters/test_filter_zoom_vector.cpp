// NOLINTBEGIN(cert-err58-cpp): Catch2 3.6.0 issue

// TODO(glk) - Look at a better way to deal with this - Vitesse::SetVitesse
#if not defined(_MSC_VER)
#else
#pragma warning(push)
#pragma warning(disable : 4296)
#pragma warning(pop)
#endif

#include <catch2/catch_approx.hpp>
#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <memory>

import Goom.FilterFx.AfterEffects.TheEffects.Rotation;
import Goom.FilterFx.AfterEffects.AfterEffectsStates;
import Goom.FilterFx.AfterEffects.AfterEffectsTypes;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.UniformZoomAdjustmentEffect;
import Goom.FilterFx.FilterEffects.ZoomVectorEffects;
import Goom.FilterFx.FilterSettings;
import Goom.FilterFx.FilterSpeed;
import Goom.FilterFx.FilterZoomVector;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRand;

namespace GOOM::UNIT_TESTS
{

using Catch::Approx;
using FILTER_FX::FilterEffectsSettings;
using FILTER_FX::FilterZoomVector;
using FILTER_FX::NormalizedCoords;
using FILTER_FX::Vitesse;
using FILTER_FX::AFTER_EFFECTS::AfterEffectsTypes;
using FILTER_FX::AFTER_EFFECTS::HypercosOverlayMode;
using FILTER_FX::AFTER_EFFECTS::RotationAdjustments;
using FILTER_FX::FILTER_EFFECTS::UniformZoomAdjustmentEffect;
using FILTER_FX::FILTER_EFFECTS::ZoomVectorEffects;
using UTILS::EnumMap;
using UTILS::MATH::GoomRand;

namespace
{

class TestZoomVectorEffects : public ZoomVectorEffects
{
public:
  static constexpr auto RAW_BASE_ZOOM_ADJUSTMENT_FACTOR =
      ZoomVectorEffects::RAW_BASE_ZOOM_ADJUSTMENT_FACTOR;
};

constexpr auto WIDTH                = 120;
constexpr auto* RESOURCES_DIRECTORY = "";
const auto GOOM_RAND                = GoomRand{};

[[nodiscard]] constexpr auto GetRelativeSpeed(const uint32_t intSpeed, const bool reverseSpeed)
    -> float
{
  constexpr auto MAX_INT_SPEED = static_cast<float>(Vitesse::MAXIMUM_SPEED);
  const auto absRelativeSpeed  = static_cast<float>(intSpeed) / MAX_INT_SPEED;
  return reverseSpeed ? -absRelativeSpeed : absRelativeSpeed;
}

[[nodiscard]] constexpr auto GetZoomAdjustment(const float relativeSpeed) -> float
{
  constexpr auto BASE_ZOOM_ADJUSTMENT_FACTOR =
      TestZoomVectorEffects::RAW_BASE_ZOOM_ADJUSTMENT_FACTOR;
  return BASE_ZOOM_ADJUSTMENT_FACTOR * (1.0F + relativeSpeed);
}

[[nodiscard]] auto GetZoomFilterEffectsSettings() -> FilterEffectsSettings
{
  static constexpr auto DEFAULT_ZOOM_MID_X                          = 16;
  static constexpr auto DEFAULT_ZOOM_MID_Y                          = 1;
  static constexpr auto DEFAULT_MULTIPLIER_EFFECT_IS_ACTIVE         = false;
  static constexpr auto DEFAULT_MULTIPLIER_EFFECT_X_FREQ            = 1.0F;
  static constexpr auto DEFAULT_MULTIPLIER_EFFECT_Y_FREQ            = 1.0F;
  static constexpr auto DEFAULT_MULTIPLIER_EFFECT_X_AMPLITUDE       = 1.0F;
  static constexpr auto DEFAULT_MULTIPLIER_EFFECT_Y_AMPLITUDE       = 1.0F;
  static constexpr auto DEFAULT_LERP_ADJUSTMENT_TO_COORDS           = 0.5F;
  static constexpr auto DEFAULT_MAX_ZOOM_ADJUSTMENT                 = 2.01F;
  static constexpr auto UNIT_BASE_ZOOM_ADJUSTMENT_FACTOR_MULTIPLIER = 1.0F;
  static constexpr auto DEFAULT_AFTER_EFFECTS_VELOCITY_CONTRIBUTION = 0.5F;

  static constexpr auto ALL_OFF_AFTER_EFFECTS_STATES = EnumMap<AfterEffectsTypes, bool>{{{
      {AfterEffectsTypes::HYPERCOS, false},
      {AfterEffectsTypes::IMAGE_VELOCITY, false},
      {AfterEffectsTypes::NOISE, false},
      {AfterEffectsTypes::PLANES, false},
      {AfterEffectsTypes::ROTATION, false},
      {AfterEffectsTypes::TAN_EFFECT, false},
      {AfterEffectsTypes::XY_LERP_EFFECT, false},
  }}};

  return FilterEffectsSettings{
      .vitesse                            = Vitesse{},
      .maxZoomAdjustment                  = DEFAULT_MAX_ZOOM_ADJUSTMENT,
      .baseZoomAdjustmentFactorMultiplier = UNIT_BASE_ZOOM_ADJUSTMENT_FACTOR_MULTIPLIER,
      .afterEffectsVelocityMultiplier     = DEFAULT_AFTER_EFFECTS_VELOCITY_CONTRIBUTION,
      .zoomAdjustmentEffect               = std::make_shared<UniformZoomAdjustmentEffect>(),
      .okToChangeFilterSettings           = nullptr,
      .zoomMidpoint                       = {.x = DEFAULT_ZOOM_MID_X, .y = DEFAULT_ZOOM_MID_Y},
      .filterMultiplierEffectsSettings    = {.isActive   = DEFAULT_MULTIPLIER_EFFECT_IS_ACTIVE,
                                                    .xFreq      = DEFAULT_MULTIPLIER_EFFECT_X_FREQ,
                                                    .yFreq      = DEFAULT_MULTIPLIER_EFFECT_Y_FREQ,
                                                    .xAmplitude = DEFAULT_MULTIPLIER_EFFECT_X_AMPLITUDE,
                                                    .yAmplitude = DEFAULT_MULTIPLIER_EFFECT_Y_AMPLITUDE,
                                                    .lerpZoomAdjustmentToCoords =
                                                 DEFAULT_LERP_ADJUSTMENT_TO_COORDS},
      .afterEffectsSettings =
          {
                                                    .hypercosOverlayMode = HypercosOverlayMode::NONE,
                                                    .isActive            = ALL_OFF_AFTER_EFFECTS_STATES,
                                                    .rotationAdjustments = RotationAdjustments{},
                                                    },
  };
}

// NOLINTBEGIN(bugprone-chained-comparison): Catch2 needs to fix this.

auto TestZoomAdjustment(FilterZoomVector& filterZoomVector,
                        FilterEffectsSettings& filterSettings,
                        const bool reverseSpeed,
                        const uint32_t speedInc)
{
  static constexpr auto COORDS = NormalizedCoords{1.0F, 1.0F};

  UNSCOPED_INFO("speedInc = " << speedInc);

  const auto intSpeed      = Vitesse::STOP_SPEED + speedInc;
  const auto relativeSpeed = GetRelativeSpeed(intSpeed, reverseSpeed);
  UNSCOPED_INFO("relativeSpeed = " << relativeSpeed);
  REQUIRE(-1.0F <= relativeSpeed);
  REQUIRE(relativeSpeed <= 1.0F);
  filterSettings.vitesse.SetVitesse(intSpeed);
  REQUIRE(filterSettings.vitesse.GetRelativeSpeed() == Approx(relativeSpeed));

  const auto baseZoomAdjustment     = GetZoomAdjustment(relativeSpeed);
  const auto zoomFactor             = 1.0F - baseZoomAdjustment;
  const auto expectedZoomAdjustment = NormalizedCoords{zoomFactor, zoomFactor};
  UNSCOPED_INFO("baseZoomAdjustment = " << baseZoomAdjustment);
  UNSCOPED_INFO("zoomFactor = " << zoomFactor);

  filterZoomVector.SetFilterEffectsSettings(filterSettings);
  const auto zoomAdjustment = filterZoomVector.GetZoomPoint(COORDS);
  REQUIRE(zoomAdjustment.GetX() == Approx(expectedZoomAdjustment.GetX()));
  REQUIRE(zoomAdjustment.GetY() == Approx(expectedZoomAdjustment.GetY()));
}

} // namespace

TEST_CASE("FilterZoomVector")
{
  auto filterZoomVector = FilterZoomVector{WIDTH, RESOURCES_DIRECTORY, GOOM_RAND};

  auto filterSettings = GetZoomFilterEffectsSettings();

  SECTION("Zero Speed")
  {
    const auto coords = NormalizedCoords{1.0F, 1.0F};

    filterSettings.vitesse.SetVitesse(Vitesse::STOP_SPEED);
    REQUIRE(filterSettings.vitesse.GetRelativeSpeed() == Approx(0.0F));
    const auto baseZoomAdjustment     = GetZoomAdjustment(0.0F);
    const auto zoomFactor             = 1.0F - baseZoomAdjustment;
    const auto expectedZoomAdjustment = zoomFactor * coords;

    filterZoomVector.SetFilterEffectsSettings(filterSettings);
    REQUIRE(filterZoomVector.GetZoomPoint(coords).GetX() == Approx(expectedZoomAdjustment.GetX()));
    REQUIRE(filterZoomVector.GetZoomPoint(coords).GetY() == Approx(expectedZoomAdjustment.GetY()));
  }

  SECTION("Non-zero Speed")
  {
    static constexpr auto NUM_SPEEDS = 2U;

    for (auto i = 0U; i < NUM_SPEEDS; ++i)
    {
      const auto reverseSpeed = 1 == i;
      filterSettings.vitesse.SetReverseVitesse(reverseSpeed);

      for (auto speedInc = 0U; speedInc <= Vitesse::MAXIMUM_SPEED; ++speedInc)
      {
        TestZoomAdjustment(filterZoomVector, filterSettings, reverseSpeed, speedInc);
      }
    }
  }
}

// NOLINTEND(bugprone-chained-comparison)

} // namespace GOOM::UNIT_TESTS

// NOLINTEND(cert-err58-cpp): Catch2 3.6.0 issue
