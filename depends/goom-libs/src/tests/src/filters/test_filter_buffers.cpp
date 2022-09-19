#ifndef GOOM_DEBUG
#define GOOM_DEBUG
#endif

#include "catch2/catch.hpp"
#include "control/goom_sound_events.h"
#include "filter_fx/filter_buffers.h"
#include "filter_fx/filter_settings.h"
#include "filter_fx/filter_zoom_vector.h"
#include "filter_fx/normalized_coords.h"
#include "goom_config.h"
#include "goom_plugin_info.h"
#include "point2d.h"
#include "sound_info.h"
#include "utils/math/goom_rand.h"
#include "utils/math/misc.h"
#include "utils/parallel_utils.h"

#include <cmath>

namespace GOOM::UNIT_TESTS
{

using CONTROL::GoomSoundEvents;
using FILTER_FX::FilterZoomVector;
using FILTER_FX::NormalizedCoords;
using FILTER_FX::NormalizedCoordsConverter;
using FILTER_FX::ZoomFilterBuffers;
using FILTER_FX::ZoomFilterEffectsSettings;
using UTILS::Parallel;
using UTILS::MATH::GoomRand;

static constexpr auto WIDTH                       = 120U;
static constexpr auto HEIGHT                      = 70U;
static constexpr auto* RESOURCES_DIRECTORY        = "";
static const auto GOOM_RAND                       = GoomRand{};
static constexpr auto NORMALIZED_COORDS_CONVERTER = NormalizedCoordsConverter{
    {WIDTH, HEIGHT},
    ZoomFilterBuffers::MIN_SCREEN_COORD_ABS_VAL
};

static constexpr auto MID_PT                     = MidpointFromOrigin({WIDTH, HEIGHT});
static constexpr auto CONST_ZOOM_VECTOR_COORDS_1 = Point2dInt{16, 40};
static constexpr auto CONST_ZOOM_VECTOR_COORDS_2 = Point2dInt{32, 52};
static constexpr auto DUMMY_COORDS               = Point2dInt{14, 38};

static constexpr auto NORMALIZED_MID_PT =
    NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(MID_PT);

namespace
{

inline auto GetBuffPos(const int32_t x, const int32_t y) -> size_t
{
  return (static_cast<size_t>(y) * WIDTH) + static_cast<size_t>(x);
}

class TestZoomVector : public FilterZoomVector
{
public:
  explicit TestZoomVector(const bool returnConst) noexcept
    : FilterZoomVector{WIDTH, RESOURCES_DIRECTORY, GOOM_RAND, NORMALIZED_COORDS_CONVERTER},
      m_returnConst{returnConst}
  {
  }

  auto SetFilterSettings(const ZoomFilterEffectsSettings& filterEffectsSettings) noexcept
      -> void override;

  [[nodiscard]] auto GetConstCoords() const noexcept -> const Point2dInt& { return m_constCoords; }
  auto SetConstCoords(const Point2dInt& coords) -> void { m_constCoords = coords; }

  auto SetZoomInCoeff(float val) noexcept -> void { m_zoomInCoeff = val; }

  [[nodiscard]] auto GetZoomInPoint(const NormalizedCoords& coords) const noexcept
      -> NormalizedCoords override;

private:
  const bool m_returnConst;
  Point2dInt m_constCoords = CONST_ZOOM_VECTOR_COORDS_1;
  float m_zoomInCoeff      = 0.0F;
};

auto TestZoomVector::SetFilterSettings(
    const ZoomFilterEffectsSettings& filterEffectsSettings) noexcept -> void
{
  FilterZoomVector::SetFilterSettings(filterEffectsSettings);
}

auto TestZoomVector::GetZoomInPoint(const NormalizedCoords& coords) const noexcept
    -> NormalizedCoords
{
  if (m_returnConst)
  {
    return NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(m_constCoords);
  }
  return (1.0F - m_zoomInCoeff) * coords;
}

using CoordTransforms = ZoomFilterBuffers::CoordTransforms;
const CoordTransforms COORD_TRANSFORMS{NORMALIZED_COORDS_CONVERTER};

auto GetSourcePoint(const ZoomFilterBuffers& filterBuffers, const size_t buffPos) -> Point2dInt
{
  return filterBuffers.GetSourcePointInfo(buffPos).screenPoint;
}

auto FullyUpdateDestBuffer(ZoomFilterBuffers& filterBuffers) noexcept -> void
{
  REQUIRE(ZoomFilterBuffers::TranBuffersState::START_FRESH_TRAN_BUFFERS ==
          filterBuffers.GetTranBuffersState());

  filterBuffers.UpdateTranBuffers();
  REQUIRE(ZoomFilterBuffers::TranBuffersState::TRAN_BUFFERS_READY ==
          filterBuffers.GetTranBuffersState());

  filterBuffers.UpdateTranBuffers();
  while (true)
  {
    filterBuffers.UpdateTranBuffers();
    if (filterBuffers.GetTranBuffYLineStart() == 0)
    {
      break;
    }
  }
  REQUIRE(ZoomFilterBuffers::TranBuffersState::RESET_TRAN_BUFFERS ==
          filterBuffers.GetTranBuffersState());

  filterBuffers.UpdateTranBuffers();
}

} // namespace

TEST_CASE("ZoomFilterBuffers Basic")
{
  static constexpr auto TEST_X          = 10;
  static constexpr auto TEST_Y          = 50;
  static constexpr auto TEST_SRCE_POINT = Point2dInt{TEST_X, TEST_Y};
  static_assert((0 <= TEST_X) && (TEST_X < WIDTH), "Invalid X");
  static_assert((0 <= TEST_Y) && (TEST_Y < WIDTH), "Invalid Y");

  auto parallel        = Parallel{-1};
  auto soundInfo       = SoundInfo{};
  auto goomSoundEvents = GoomSoundEvents{soundInfo};

  const auto goomInfo = PluginInfo{
      {WIDTH, HEIGHT},
      goomSoundEvents
  };
  auto identityZoomVector = TestZoomVector{false};
  auto filterBuffers =
      ZoomFilterBuffers{parallel,
                        goomInfo,
                        NORMALIZED_COORDS_CONVERTER,
                        [&](const NormalizedCoords& normalizedCoords)
                        { return identityZoomVector.GetZoomInPoint(normalizedCoords); }};

  filterBuffers.SetBuffMidpoint(MID_PT);
  filterBuffers.Start();

  static constexpr auto DUMMY_NML_COORDS =
      NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(DUMMY_COORDS);

  SECTION("Correct Starting TranBuffersState")
  {
    REQUIRE(ZoomFilterBuffers::TranBuffersState::START_FRESH_TRAN_BUFFERS ==
            filterBuffers.GetTranBuffersState());
  }
  SECTION("Correct Starting BuffYLineStart")
  {
    REQUIRE(0 == filterBuffers.GetTranBuffYLineStart());
  }
  SECTION("Correct Starting BuffMidpoint()")
  {
    REQUIRE(MID_PT == filterBuffers.GetBuffMidpoint());
  }
  SECTION("Correct Starting HaveFilterSettingsChanged")
  {
    REQUIRE(!filterBuffers.HaveFilterSettingsChanged());
  }
  SECTION("Correct Starting TranLerpFactor")
  {
    REQUIRE(0 == filterBuffers.GetTranLerpFactor());
  }
  SECTION("Correct Starting ZoomBufferTranPoint")
  {
    REQUIRE(DUMMY_NML_COORDS.Equals(identityZoomVector.GetZoomInPoint(DUMMY_NML_COORDS)));

    // TranLerpFactor is zero so starting tranPoint should be srce identity buffer
    const auto expectedSrceIdentityPoint = TEST_SRCE_POINT;
    const auto srcePoint = GetSourcePoint(filterBuffers, GetBuffPos(TEST_X, TEST_Y));
    UNSCOPED_INFO("srcePoint.x = " << srcePoint.x);
    UNSCOPED_INFO("srcePoint.y = " << srcePoint.y);
    REQUIRE(expectedSrceIdentityPoint == srcePoint);
  }
  SECTION("Correct Dest ZoomBufferTranPoint")
  {
    // Lerp to the dest buffer only (by using max lerp).
    filterBuffers.SetTranLerpFactor(filterBuffers.GetMaxTranLerpFactor());
    REQUIRE(filterBuffers.GetMaxTranLerpFactor() == filterBuffers.GetTranLerpFactor());

    REQUIRE(DUMMY_NML_COORDS.Equals(identityZoomVector.GetZoomInPoint(DUMMY_NML_COORDS)));

    // GetSourcePoint uses tranPoint which comes solely from the dest Zoom buffer.
    // Because we are using an identity ZoomVectorFunc, tranPoint should be the identity point.
    const auto expectedSrceIdentityPoint = TEST_SRCE_POINT;
    const auto srcePoint = GetSourcePoint(filterBuffers, GetBuffPos(TEST_X, TEST_Y));
    UNSCOPED_INFO("expectedSrceIdentityPoint.x = " << expectedSrceIdentityPoint.x);
    UNSCOPED_INFO("expectedSrceIdentityPoint.y = " << expectedSrceIdentityPoint.y);
    UNSCOPED_INFO("srcePoint.x = " << srcePoint.x);
    UNSCOPED_INFO("srcePoint.y = " << srcePoint.y);
    REQUIRE(expectedSrceIdentityPoint == srcePoint);
  }
  SECTION("Correct Lerped ZoomBufferTranPoint")
  {
    static constexpr auto T_LERP = 0.5F;
    const auto tranLerpFactor    = static_cast<uint32_t>(
        std::round(T_LERP * static_cast<float>(filterBuffers.GetMaxTranLerpFactor())));

    filterBuffers.SetTranLerpFactor(tranLerpFactor);
    REQUIRE(tranLerpFactor == filterBuffers.GetTranLerpFactor());

    REQUIRE(DUMMY_NML_COORDS.Equals(identityZoomVector.GetZoomInPoint(DUMMY_NML_COORDS)));

    // tranPoint comes solely from the lerp middle of srce and dest Zoom buffer which
    // because we are using an identity ZoomVectorFunc should still be the identity point.
    const auto expectedSrceIdentityPoint = TEST_SRCE_POINT;
    const auto srcePoint = GetSourcePoint(filterBuffers, GetBuffPos(TEST_X, TEST_Y));
    UNSCOPED_INFO("srcePoint.x = " << srcePoint.x);
    UNSCOPED_INFO("srcePoint.y = " << srcePoint.y);
    REQUIRE(expectedSrceIdentityPoint == srcePoint);
  }
}

TEST_CASE("ZoomFilterBuffers Calculations")
{
  auto parallel        = Parallel{-1};
  auto soundInfo       = SoundInfo{};
  auto goomSoundEvents = GoomSoundEvents{soundInfo};

  const auto goomInfo = PluginInfo{
      {WIDTH, HEIGHT},
      goomSoundEvents
  };
  auto constantZoomVector = TestZoomVector{true};
  auto filterBuffers =
      ZoomFilterBuffers{parallel,
                        goomInfo,
                        NORMALIZED_COORDS_CONVERTER,
                        [&](const NormalizedCoords& normalizedCoords)
                        { return constantZoomVector.GetZoomInPoint(normalizedCoords); }};

  filterBuffers.SetBuffMidpoint(MID_PT);
  filterBuffers.Start();

  static constexpr auto DUMMY_NML_COORDS =
      NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(DUMMY_COORDS);
  static constexpr auto NML_CONST_ZOOM_VECTOR_COORDS1 =
      NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1);

  SECTION("Correct Dest ZoomBufferTranPoint")
  {
    REQUIRE(CONST_ZOOM_VECTOR_COORDS_1 == constantZoomVector.GetConstCoords());
    REQUIRE(MID_PT == filterBuffers.GetBuffMidpoint());

    // Lerp to the dest buffer only
    filterBuffers.SetTranLerpFactor(filterBuffers.GetMaxTranLerpFactor());
    REQUIRE(filterBuffers.GetMaxTranLerpFactor() == filterBuffers.GetTranLerpFactor());

    // tranPoint comes solely from the dest Zoom buffer which because we are using a
    // const ZoomVectorFunc, returns a const normalized value
    UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_1.x = " << NML_CONST_ZOOM_VECTOR_COORDS1.GetX());
    UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_1.y = " << NML_CONST_ZOOM_VECTOR_COORDS1.GetY());
    UNSCOPED_INFO("GetZoomInPoint(DUMMY_NML_COORDS).x = "
                  << constantZoomVector.GetZoomInPoint(DUMMY_NML_COORDS).GetX());
    UNSCOPED_INFO("GetZoomInPoint(DUMMY_NML_COORDS).y = "
                  << constantZoomVector.GetZoomInPoint(DUMMY_NML_COORDS).GetY());
    REQUIRE(
        NML_CONST_ZOOM_VECTOR_COORDS1.Equals(constantZoomVector.GetZoomInPoint(DUMMY_NML_COORDS)));
    const auto normalizedMidPt =
        NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(filterBuffers.GetBuffMidpoint());
    const auto expectedTranPoint = COORD_TRANSFORMS.NormalizedToTranPoint(
        normalizedMidPt + NormalizedCoords{NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(
                              CONST_ZOOM_VECTOR_COORDS_1)});
    UNSCOPED_INFO("filterBuffers.GetBuffMidpoint().x = " << filterBuffers.GetBuffMidpoint().x);
    UNSCOPED_INFO("filterBuffers.GetBuffMidpoint().y = " << filterBuffers.GetBuffMidpoint().y);
    UNSCOPED_INFO("normalizedMidPt.x = " << normalizedMidPt.GetX());
    UNSCOPED_INFO("normalizedMidPt.y = " << normalizedMidPt.GetY());
    UNSCOPED_INFO("expectedTranPoint.x = " << expectedTranPoint.x);
    UNSCOPED_INFO("expectedTranPoint.y = " << expectedTranPoint.y);

    const auto expectedSrcePoint = CoordTransforms::TranToScreenPoint(expectedTranPoint);
    UNSCOPED_INFO("expectedSrcePoint.x = " << expectedSrcePoint.x);
    UNSCOPED_INFO("expectedSrcePoint.y = " << expectedSrcePoint.y);

    for (auto buffPos = 0U; buffPos < WIDTH * HEIGHT; buffPos++)
    {
      const auto srcePoint = GetSourcePoint(filterBuffers, buffPos);
      UNSCOPED_INFO("srcePoint.x = " << srcePoint.x);
      UNSCOPED_INFO("srcePoint.y = " << srcePoint.y);

      REQUIRE(expectedSrcePoint == srcePoint);
    }
  }

  SECTION("Correct Srce/Dest ZoomBufferTranPoint")
  {
    REQUIRE(CONST_ZOOM_VECTOR_COORDS_1 == constantZoomVector.GetConstCoords());
    REQUIRE(MID_PT == filterBuffers.GetBuffMidpoint());

    static constexpr float T_LERP = 0.5F;
    const auto tranLerpFactor     = static_cast<uint32_t>(
        std::round(T_LERP * static_cast<float>(filterBuffers.GetMaxTranLerpFactor())));

    filterBuffers.SetTranLerpFactor(tranLerpFactor);
    REQUIRE(tranLerpFactor == filterBuffers.GetTranLerpFactor());

    for (auto y = 0; y < static_cast<int32_t>(HEIGHT); y++)
    {
      for (auto x = 0; x < static_cast<int32_t>(WIDTH); x++)
      {
        // tranPoint comes from halfway between srce and dest Zoom buffer.
        REQUIRE(NML_CONST_ZOOM_VECTOR_COORDS1.Equals(
            constantZoomVector.GetZoomInPoint(DUMMY_NML_COORDS)));
        const auto normalizedSrcePt =
            NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(Point2dInt{x, y});
        const auto expectedSrceTranPoint = COORD_TRANSFORMS.NormalizedToTranPoint(normalizedSrcePt);
        const auto normalizedMidPt =
            NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(filterBuffers.GetBuffMidpoint());
        UNSCOPED_INFO("filterBuffers.GetBuffMidpoint().x = " << filterBuffers.GetBuffMidpoint().x);
        UNSCOPED_INFO("filterBuffers.GetBuffMidpoint().y = " << filterBuffers.GetBuffMidpoint().y);
        UNSCOPED_INFO("normalizedMidPt.x = " << normalizedMidPt.GetX());
        UNSCOPED_INFO("normalizedMidPt.y = " << normalizedMidPt.GetY());
        const auto expectedDestTranPoint = COORD_TRANSFORMS.NormalizedToTranPoint(
            normalizedMidPt + NormalizedCoords{NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(
                                  CONST_ZOOM_VECTOR_COORDS_1)});
        const auto expectedTranPoint =
            Point2dInt{STD20::lerp(expectedSrceTranPoint.x, expectedDestTranPoint.x, T_LERP),
                       STD20::lerp(expectedSrceTranPoint.y, expectedDestTranPoint.y, T_LERP)};
        UNSCOPED_INFO("expectedTranPoint.x = " << expectedTranPoint.x);
        UNSCOPED_INFO("expectedTranPoint.y = " << expectedTranPoint.y);

        const auto expectedScreenPoint = CoordTransforms::TranToScreenPoint(expectedTranPoint);
        UNSCOPED_INFO("expectedScreenPoint.x = " << expectedScreenPoint.x);
        UNSCOPED_INFO("expectedScreenPoint.y = " << expectedScreenPoint.y);

        const auto screenPoint = GetSourcePoint(filterBuffers, GetBuffPos(x, y));
        UNSCOPED_INFO("srcePoint.x = " << screenPoint.x);
        UNSCOPED_INFO("srcePoint.y = " << screenPoint.y);

        REQUIRE(expectedScreenPoint == screenPoint);
      }
    }
  }
}

TEST_CASE("ZoomFilterBuffers Stripes")
{
  static constexpr auto TEST_X = 10;
  static constexpr auto TEST_Y = 50;
  static_assert((0 <= TEST_X) && (TEST_X < WIDTH), "Invalid X");
  static_assert((0 <= TEST_Y) && (TEST_Y < WIDTH), "Invalid Y");

  auto parallel        = Parallel{0};
  auto soundInfo       = SoundInfo{};
  auto goomSoundEvents = GoomSoundEvents{soundInfo};

  const auto goomInfo = PluginInfo{
      {WIDTH, HEIGHT},
      goomSoundEvents
  };
  auto constantZoomVector = TestZoomVector{true};
  auto filterBuffers =
      ZoomFilterBuffers{parallel,
                        goomInfo,
                        NORMALIZED_COORDS_CONVERTER,
                        [&](const NormalizedCoords& normalizedCoords)
                        { return constantZoomVector.GetZoomInPoint(normalizedCoords); }};

  filterBuffers.SetBuffMidpoint(MID_PT);
  filterBuffers.Start();

  static constexpr auto DUMMY_NML_COORDS =
      NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(DUMMY_COORDS);
  static constexpr auto NML_CONST_ZOOM_VECTOR_COORDS1 =
      NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1);
  static constexpr auto NML_CONST_ZOOM_VECTOR_COORDS2 =
      NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_2);

  SECTION("ZoomBuffer Stripe")
  {
    REQUIRE(ZoomFilterBuffers::TranBuffersState::START_FRESH_TRAN_BUFFERS ==
            filterBuffers.GetTranBuffersState());
    REQUIRE(not filterBuffers.HaveFilterSettingsChanged());

    REQUIRE(CONST_ZOOM_VECTOR_COORDS_1 == constantZoomVector.GetConstCoords());
    REQUIRE(MID_PT == filterBuffers.GetBuffMidpoint());
    REQUIRE(
        NML_CONST_ZOOM_VECTOR_COORDS1.Equals(constantZoomVector.GetZoomInPoint(DUMMY_NML_COORDS)));

    // Make sure dest buffer is completely copied to srce buffer at end of update.
    filterBuffers.SetTranLerpFactor(filterBuffers.GetMaxTranLerpFactor());
    REQUIRE(filterBuffers.GetMaxTranLerpFactor() == filterBuffers.GetTranLerpFactor());

    constantZoomVector.SetConstCoords(CONST_ZOOM_VECTOR_COORDS_2);
    REQUIRE(CONST_ZOOM_VECTOR_COORDS_2 == constantZoomVector.GetConstCoords());
    REQUIRE(
        NML_CONST_ZOOM_VECTOR_COORDS2.Equals(constantZoomVector.GetZoomInPoint(DUMMY_NML_COORDS)));
    filterBuffers.NotifyFilterSettingsHaveChanged();
    REQUIRE(filterBuffers.HaveFilterSettingsChanged());
    FullyUpdateDestBuffer(filterBuffers);
    REQUIRE(ZoomFilterBuffers::TranBuffersState::START_FRESH_TRAN_BUFFERS ==
            filterBuffers.GetTranBuffersState());
    REQUIRE(0 == filterBuffers.GetTranLerpFactor());
    REQUIRE(0 == filterBuffers.GetTranBuffYLineStart());
    REQUIRE(CONST_ZOOM_VECTOR_COORDS_2 == constantZoomVector.GetConstCoords());
    REQUIRE(MID_PT == filterBuffers.GetBuffMidpoint());

    const auto normalizedMidPt =
        NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(filterBuffers.GetBuffMidpoint());

    // Get srce buffer points - should be all CONST_ZOOM_VECTOR_COORDS_1
    filterBuffers.SetTranLerpFactor(0);
    REQUIRE(0 == filterBuffers.GetTranLerpFactor());
    const auto expectedSrceTranPoint = COORD_TRANSFORMS.NormalizedToTranPoint(
        normalizedMidPt +
        NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1));
    const auto expectedSrcePoint = CoordTransforms::TranToScreenPoint(expectedSrceTranPoint);
    UNSCOPED_INFO("expectedSrceTranPoint.x = " << expectedSrceTranPoint.x);
    UNSCOPED_INFO("expectedSrceTranPoint.y = " << expectedSrceTranPoint.y);
    UNSCOPED_INFO("expectedSrcePoint.x = " << expectedSrcePoint.x);
    UNSCOPED_INFO("expectedSrcePoint.y = " << expectedSrcePoint.y);

    for (auto buffPos = 0U; buffPos < WIDTH * HEIGHT; buffPos++)
    {
      const auto srcePoint = GetSourcePoint(filterBuffers, GetBuffPos(TEST_X, TEST_Y));
      UNSCOPED_INFO("srcePoint.x = " << srcePoint.x);
      UNSCOPED_INFO("srcePoint.y = " << srcePoint.y);

      REQUIRE(expectedSrcePoint == srcePoint);
    }

    // Get dest buffer points - should be all CONST_ZOOM_VECTOR_COORDS_2
    filterBuffers.SetTranLerpFactor(filterBuffers.GetMaxTranLerpFactor());
    REQUIRE(filterBuffers.GetMaxTranLerpFactor() == filterBuffers.GetTranLerpFactor());
    const auto expectedDestTranPoint =
        COORD_TRANSFORMS.NormalizedToTranPoint(normalizedMidPt + NML_CONST_ZOOM_VECTOR_COORDS2);
    const auto expectedDestPoint = CoordTransforms::TranToScreenPoint(expectedDestTranPoint);
    UNSCOPED_INFO("normalizedMidPt.x = " << normalizedMidPt.GetX());
    UNSCOPED_INFO("normalizedMidPt.y = " << normalizedMidPt.GetY());
    UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_2.x = " << NML_CONST_ZOOM_VECTOR_COORDS2.GetX());
    UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_2.y = " << NML_CONST_ZOOM_VECTOR_COORDS2.GetY());
    UNSCOPED_INFO("expectedDestTranPoint.x = " << expectedDestTranPoint.x);
    UNSCOPED_INFO("expectedDestTranPoint.y = " << expectedDestTranPoint.y);
    UNSCOPED_INFO("expectedDestPoint.x = " << expectedDestPoint.x);
    UNSCOPED_INFO("expectedDestPoint.y = " << expectedDestPoint.y);

    for (auto buffPos = 0U; buffPos < WIDTH * HEIGHT; buffPos++)
    {
      const auto destPoint = GetSourcePoint(filterBuffers, buffPos);
      UNSCOPED_INFO("destPoint.x = " << destPoint.x);
      UNSCOPED_INFO("destPoint.y = " << destPoint.y);

      REQUIRE(expectedDestPoint == destPoint);
    }
  }
}

TEST_CASE("ZoomFilterBuffers ZoomIn")
{
  static constexpr auto TEST_X          = 10;
  static constexpr auto TEST_Y          = 50;
  static constexpr auto TEST_SRCE_POINT = Point2dInt{TEST_X, TEST_Y};
  static_assert((0 <= TEST_X) && (TEST_X < WIDTH), "Invalid X");
  static_assert((0 <= TEST_Y) && (TEST_Y < WIDTH), "Invalid Y");
  static constexpr auto TEST_SRCE_NML_COORDS =
      NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(TEST_SRCE_POINT);

  auto parallel        = Parallel{-1};
  auto soundInfo       = SoundInfo{};
  auto goomSoundEvents = GoomSoundEvents{soundInfo};

  const auto goomInfo = PluginInfo{
      {WIDTH, HEIGHT},
      goomSoundEvents
  };
  auto zoomVector    = TestZoomVector{false};
  auto filterBuffers = ZoomFilterBuffers{parallel,
                                         goomInfo,
                                         NORMALIZED_COORDS_CONVERTER,
                                         [&](const NormalizedCoords& normalizedCoords)
                                         { return zoomVector.GetZoomInPoint(normalizedCoords); }};

  filterBuffers.SetBuffMidpoint(MID_PT);
  filterBuffers.Start();

  SECTION("Correct Zoomed In Dest ZoomBufferTranPoint")
  {
    // Lerp to the dest buffer only (by using max lerp).
    filterBuffers.SetTranLerpFactor(filterBuffers.GetMaxTranLerpFactor());
    REQUIRE(filterBuffers.GetMaxTranLerpFactor() == filterBuffers.GetTranLerpFactor());

    REQUIRE(TEST_SRCE_NML_COORDS.Equals(zoomVector.GetZoomInPoint(TEST_SRCE_NML_COORDS)));

    static constexpr auto ZOOM_IN_COEFF1  = 0.2F;
    static constexpr auto ZOOM_IN_FACTOR1 = 1.0F - ZOOM_IN_COEFF1;
    zoomVector.SetZoomInCoeff(ZOOM_IN_COEFF1);
    REQUIRE((ZOOM_IN_FACTOR1 * TEST_SRCE_NML_COORDS)
                .Equals(zoomVector.GetZoomInPoint(TEST_SRCE_NML_COORDS)));

    // GetSourcePoint uses tranPoint which comes solely from the dest Zoom buffer.
    // Because we are using a zoomed in ZoomVectorFunc, tranPoint should be zoomed in.
    filterBuffers.NotifyFilterSettingsHaveChanged();
    REQUIRE(filterBuffers.HaveFilterSettingsChanged());
    FullyUpdateDestBuffer(filterBuffers);
    REQUIRE(ZoomFilterBuffers::TranBuffersState::START_FRESH_TRAN_BUFFERS ==
            filterBuffers.GetTranBuffersState());
    filterBuffers.SetTranLerpFactor(filterBuffers.GetMaxTranLerpFactor());
    REQUIRE(filterBuffers.GetMaxTranLerpFactor() == filterBuffers.GetTranLerpFactor());

    const auto expectedZoomedInSrcePointFlt =
        NORMALIZED_COORDS_CONVERTER.NormalizedToScreenCoordsFlt(
            (ZOOM_IN_FACTOR1 * (TEST_SRCE_NML_COORDS - NORMALIZED_MID_PT)) + NORMALIZED_MID_PT);
    // TODO(glk) - Note truncation here. Is this an issue with 'tran' int32_t buffers?
    const auto expectedZoomedInSrcePoint =
        Point2dInt{static_cast<int32_t>(expectedZoomedInSrcePointFlt.x),
                   static_cast<int32_t>(expectedZoomedInSrcePointFlt.y)};
    const auto srcePoint = GetSourcePoint(filterBuffers, GetBuffPos(TEST_X, TEST_Y));
    UNSCOPED_INFO("expectedZoomedInSrcePoint.x = " << expectedZoomedInSrcePoint.x);
    UNSCOPED_INFO("expectedZoomedInSrcePoint.y = " << expectedZoomedInSrcePoint.y);
    UNSCOPED_INFO("srcePoint.x = " << srcePoint.x);
    UNSCOPED_INFO("srcePoint.y = " << srcePoint.y);
    REQUIRE(expectedZoomedInSrcePoint == srcePoint);
  }
}

TEST_CASE("ZoomFilterBuffers Clipping")
{
  auto parallel        = Parallel{-1};
  auto soundInfo       = SoundInfo{};
  auto goomSoundEvents = GoomSoundEvents{soundInfo};

  const auto goomInfo = PluginInfo{
      {WIDTH, HEIGHT},
      goomSoundEvents
  };
  auto constantZoomVector = TestZoomVector{true};
  auto filterBuffers =
      ZoomFilterBuffers{parallel,
                        goomInfo,
                        NORMALIZED_COORDS_CONVERTER,
                        [&](const NormalizedCoords& normalizedCoords)
                        { return constantZoomVector.GetZoomInPoint(normalizedCoords); }};

  filterBuffers.SetBuffMidpoint({0, 0});
  filterBuffers.Start();

  static constexpr auto DUMMY_NML_COORDS =
      NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(DUMMY_COORDS);
  static constexpr auto NML_CONST_ZOOM_VECTOR_COORDS1 =
      NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1);

  SECTION("Clipped ZoomBufferTranPoint")
  {
    REQUIRE(Point2dInt{0, 0} == filterBuffers.GetBuffMidpoint());

    // Lerp to the dest buffer only
    filterBuffers.SetTranLerpFactor(filterBuffers.GetMaxTranLerpFactor());
    REQUIRE(filterBuffers.GetMaxTranLerpFactor() == filterBuffers.GetTranLerpFactor());

    // tranPoint comes solely from the dest Zoom buffer which because we are using a
    // const ZoomVectorFunc, returns a const normalized value
    UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_1.x = " << NML_CONST_ZOOM_VECTOR_COORDS1.GetX());
    UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_1.y = " << NML_CONST_ZOOM_VECTOR_COORDS1.GetY());
    UNSCOPED_INFO("GetZoomInPoint(DUMMY_NML_COORDS).x = "
                  << constantZoomVector.GetZoomInPoint(DUMMY_NML_COORDS).GetX());
    UNSCOPED_INFO("GetZoomInPoint(DUMMY_NML_COORDS).y = "
                  << constantZoomVector.GetZoomInPoint(DUMMY_NML_COORDS).GetY());
    REQUIRE(
        NML_CONST_ZOOM_VECTOR_COORDS1.Equals(constantZoomVector.GetZoomInPoint(DUMMY_NML_COORDS)));
    const auto normalizedMidPt =
        NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(filterBuffers.GetBuffMidpoint());
    const auto expectedTranPoint = COORD_TRANSFORMS.NormalizedToTranPoint(
        normalizedMidPt +
        NORMALIZED_COORDS_CONVERTER.ScreenToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1));
    // Because mid-point is zero, the trans point is negative and therefore clipped.
    REQUIRE(expectedTranPoint.x < 0);
    REQUIRE(expectedTranPoint.y < 0);

    UNSCOPED_INFO("filterBuffers.GetBuffMidpoint().x = " << filterBuffers.GetBuffMidpoint().x);
    UNSCOPED_INFO("filterBuffers.GetBuffMidpoint().y = " << filterBuffers.GetBuffMidpoint().y);
    UNSCOPED_INFO("normalizedMidPt.x = " << normalizedMidPt.GetX());
    UNSCOPED_INFO("normalizedMidPt.y = " << normalizedMidPt.GetY());
    UNSCOPED_INFO("expectedTranPoint.x = " << expectedTranPoint.x);
    UNSCOPED_INFO("expectedTranPoint.y = " << expectedTranPoint.y);

    const auto destPoint = filterBuffers.GetSourcePointInfo(0);
    UNSCOPED_INFO("destPoint.isClipped = " << destPoint.isClipped);
    UNSCOPED_INFO("destPoint.screenPoint.x = " << destPoint.screenPoint.x);
    UNSCOPED_INFO("destPoint.screenPoint.y = " << destPoint.screenPoint.y);
    REQUIRE(destPoint.isClipped);
    REQUIRE(0 == destPoint.screenPoint.x);
    REQUIRE(0 == destPoint.screenPoint.y);

    // TODO(glk) Test coeff values
  }
}

} // namespace GOOM::UNIT_TESTS
