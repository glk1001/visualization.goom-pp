#include "catch2/catch.hpp"
#include "filter_fx/filter_buffers_service.h"
#include "filter_fx/filter_colors_service.h"
#include "filter_fx/filter_effects/speed_coefficients_effect_factory.h"
#include "filter_fx/filter_settings.h"
#include "filter_fx/filter_settings_service.h"
#include "filter_fx/filter_zoom_vector.h"
#include "filter_fx/normalized_coords.h"
#include "filter_fx/zoom_filter_fx.h"
#include "goom_plugin_info.h"
#include "utils/math/goom_rand.h"
#include "utils/parallel_utils.h"

#include <memory>

namespace GOOM::UNIT_TESTS
{

using FILTER_FX::FilterBuffersService;
using FILTER_FX::FilterColorsService;
using FILTER_FX::FilterSettingsService;
using FILTER_FX::FilterZoomVector;
using FILTER_FX::NormalizedCoordsConverter;
using FILTER_FX::ZoomFilterBuffers;
using FILTER_FX::ZoomFilterBufferSettings;
using FILTER_FX::ZoomFilterFx;
using FILTER_FX::FILTER_EFFECTS::CreateSpeedCoefficientsEffect;
using UTILS::Parallel;
using UTILS::MATH::GoomRand;

static constexpr size_t WIDTH = 120;
static constexpr size_t HEIGHT = 70;

static constexpr const char* RESOURCES_DIRECTORY = "";

TEST_CASE("ZoomFilterFx", "[ZoomFilterFx]")
{
  Parallel parallel{-1};
  const PluginInfo goomInfo{WIDTH, HEIGHT};
  const GoomRand goomRand{};
  FilterSettingsService filterSettingsService{goomInfo, goomRand, RESOURCES_DIRECTORY,
                                              CreateSpeedCoefficientsEffect};
  const NormalizedCoordsConverter normalizedCoordsConverter{
      WIDTH, HEIGHT, ZoomFilterBuffers::MIN_SCREEN_COORD_ABS_VAL};
  ZoomFilterFx zoomFilterFx{
      parallel, goomInfo,
      std::make_unique<FilterBuffersService>(
          parallel, goomInfo, normalizedCoordsConverter,
          std::make_unique<FilterZoomVector>(WIDTH, RESOURCES_DIRECTORY, goomRand,
                                             normalizedCoordsConverter)),
      std::make_unique<FilterColorsService>()};

  SECTION("Correct initial lerp factor") { REQUIRE(0 == zoomFilterFx.GetTranLerpFactor()); }
  SECTION("Correct lerp factor after an increment")
  {
    const ZoomFilterBufferSettings filterBufferSettings = {127, 1.0F};
    zoomFilterFx.UpdateFilterBufferSettings(filterBufferSettings);
    REQUIRE(127 == zoomFilterFx.GetTranLerpFactor());
  }
}

} // namespace GOOM::UNIT_TESTS
