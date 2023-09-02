#ifndef GOOM_DEBUG
#define GOOM_DEBUG
#endif

// TODO(glk) - Look at a better way to deal with this - Vitesse::SetVitesse
#if not defined(_MSC_VER)
#include "filter_fx/filter_settings.h"
#else
#pragma warning(push)
#pragma warning(disable : 4296)
#include "filter_fx/filter_settings.h"
#pragma warning(pop)
#endif

#include "control/goom_sound_events.h"
#include "filter_fx/filter_buffer_striper.h"
#include "filter_fx/filter_buffers.h"
#include "filter_fx/filter_zoom_vector.h"
#include "filter_fx/normalized_coords.h"
#include "goom/goom_config.h"
#include "goom/goom_time.h"
#include "goom/goom_types.h"
#include "goom/point2d.h"
#include "goom/sound_info.h"
#include "goom_plugin_info.h"
#include "utils/math/goom_rand.h"
#include "utils/parallel_utils.h"

#include <cstdint>
#include <memory>
#include <utility>

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

using CONTROL::GoomSoundEvents;
using FILTER_FX::FilterEffectsSettings;
using FILTER_FX::FilterZoomVector;
using FILTER_FX::NormalizedCoords;
using FILTER_FX::NormalizedCoordsConverter;
using FILTER_FX::ZoomFilterBuffers;
using FILTER_FX::ZoomFilterBufferStriper;
using UTILS::Parallel;
using UTILS::MATH::GoomRand;

class FilterBuffers : public ZoomFilterBuffers<ZoomFilterBufferStriper>
{
public:
  explicit FilterBuffers(std::unique_ptr<ZoomFilterBufferStriper> filterStriper) noexcept
    : ZoomFilterBuffers<ZoomFilterBufferStriper>(std::move(filterStriper))
  {
  }

  [[nodiscard]] auto GetBufferBuffMidpoint() const noexcept -> Point2dInt
  {
    return ZoomFilterBuffers<ZoomFilterBufferStriper>::GetTransformBufferBuffMidpoint();
  }
  [[nodiscard]] auto GetBufferYLineStart() const noexcept -> uint32_t
  {
    return ZoomFilterBuffers<ZoomFilterBufferStriper>::GetTransformBufferYLineStart();
  }
  [[nodiscard]] auto HaveSettingsChanged() const noexcept -> bool
  {
    return ZoomFilterBuffers<ZoomFilterBufferStriper>::HaveFilterSettingsChanged();
  }
};

namespace
{

// TODO(glk) - Get rid of this!
class ZoomCoordTransforms
{
public:
  explicit ZoomCoordTransforms(const Dimensions& screenDimensions) noexcept;

  [[nodiscard]] auto NormalizedToTranPoint(const NormalizedCoords& normalizedPoint) const noexcept
      -> Point2dInt;

  [[nodiscard]] static auto TranToScreenPoint(const Point2dInt& tranPoint) noexcept -> Point2dInt;
  [[nodiscard]] static auto ScreenToTranPoint(const Point2dInt& screenPoint) noexcept -> Point2dInt;

private:
  NormalizedCoordsConverter m_normalizedCoordsConverter;
  static constexpr auto DIM_FILTER_COEFFS_EXP = 4U;
};

inline ZoomCoordTransforms::ZoomCoordTransforms(const Dimensions& screenDimensions) noexcept
  : m_normalizedCoordsConverter{
        {screenDimensions.GetWidth() << DIM_FILTER_COEFFS_EXP,
         screenDimensions.GetWidth() << DIM_FILTER_COEFFS_EXP}
}
{
}

inline auto ZoomCoordTransforms::TranToScreenPoint(const Point2dInt& tranPoint) noexcept
    -> Point2dInt
{
  // Note: Truncation here but seems OK. Trying to round adds about 2ms.
  // NOLINTNEXTLINE(hicpp-signed-bitwise)
  return {tranPoint.x >> DIM_FILTER_COEFFS_EXP, tranPoint.y >> DIM_FILTER_COEFFS_EXP};
}

inline auto ZoomCoordTransforms::ScreenToTranPoint(const Point2dInt& screenPoint) noexcept
    -> Point2dInt
{
  // NOLINTNEXTLINE(hicpp-signed-bitwise)
  return {screenPoint.x << DIM_FILTER_COEFFS_EXP, screenPoint.y << DIM_FILTER_COEFFS_EXP};
}

inline auto ZoomCoordTransforms::NormalizedToTranPoint(
    const NormalizedCoords& normalizedPoint) const noexcept -> Point2dInt
{
  return ToPoint2dInt(m_normalizedCoordsConverter.NormalizedToOtherCoordsFlt(normalizedPoint));
}

//#define LARGE_SCREEN_TEST
constexpr auto LARGE_WIDTH  = 3840U;
constexpr auto LARGE_HEIGHT = 2160U;
#ifdef LARGE_SCREEN_TEST
constexpr auto WIDTH  = LARGE_WIDTH;
constexpr auto HEIGHT = LARGE_HEIGHT;
#else
constexpr auto WIDTH  = LARGE_WIDTH / 10U;
constexpr auto HEIGHT = LARGE_HEIGHT / 10U;
#endif
constexpr auto* RESOURCES_DIRECTORY = "";
const auto GOOM_RAND                = GoomRand{};

const auto SOUND_INFO   = SoundInfo{};
const auto GOOM_TIME    = GoomTime{};
const auto SOUND_EVENTS = GoomSoundEvents{GOOM_TIME, SOUND_INFO};
const auto GOOM_INFO    = PluginInfo{
       {WIDTH, HEIGHT},
       GOOM_TIME, SOUND_EVENTS
};
constexpr auto NORMALIZED_COORDS_CONVERTER = NormalizedCoordsConverter{
    {WIDTH, HEIGHT}
};

constexpr auto MID_PT                     = MidpointFromOrigin({WIDTH, HEIGHT});
constexpr auto CONST_ZOOM_VECTOR_COORDS_1 = Point2dInt{16, 40};
constexpr auto CONST_ZOOM_VECTOR_COORDS_2 = Point2dInt{32, 52};
constexpr auto DUMMY_COORDS               = Point2dInt{14, 38};

const auto MAX_TRAN_POINT = ZoomCoordTransforms::ScreenToTranPoint({WIDTH - 1, HEIGHT - 1});
//const auto MID_TRAN_POINT = ZoomCoordTransforms::ScreenToTranPoint(MID_PT);
//
//inline auto GetBuffPos(const int32_t x, const int32_t y) -> size_t
//{
//  return (static_cast<size_t>(y) * WIDTH) + static_cast<size_t>(x);
//}

class TestZoomVector : public FilterZoomVector
{
public:
  explicit TestZoomVector(const bool returnConst) noexcept
    : FilterZoomVector{WIDTH, RESOURCES_DIRECTORY, GOOM_RAND}, m_returnConst{returnConst}
  {
  }

  auto SetFilterEffectsSettings(const FilterEffectsSettings& filterEffectsSettings) noexcept
      -> void override;

  [[nodiscard]] auto GetConstCoords() const noexcept -> const Point2dInt& { return m_constCoords; }
  auto SetConstCoords(const Point2dInt& coords) -> void { m_constCoords = coords; }

  auto SetZoomAdjustment(const float val) noexcept -> void { m_zoomAdjustment = val; }

  [[nodiscard]] auto GetZoomPoint(const NormalizedCoords& coords,
                                  const NormalizedCoords& filterViewportCoords) const noexcept
      -> NormalizedCoords override;

private:
  bool m_returnConst;
  Point2dInt m_constCoords = CONST_ZOOM_VECTOR_COORDS_1;
  float m_zoomAdjustment   = 0.0F;
};

auto TestZoomVector::SetFilterEffectsSettings(
    const FilterEffectsSettings& filterEffectsSettings) noexcept -> void
{
  FilterZoomVector::SetFilterEffectsSettings(filterEffectsSettings);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto TestZoomVector::GetZoomPoint(const NormalizedCoords& coords,
                                  [[maybe_unused]] const NormalizedCoords& filterViewportCoords)
    const noexcept -> NormalizedCoords
{
  if (m_returnConst)
  {
    return NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(m_constCoords);
  }
  return (1.0F - m_zoomAdjustment) * coords;
}

const ZoomCoordTransforms COORD_TRANSFORMS{
    {WIDTH, HEIGHT}
};
const auto IDENTITY_ZOOM_VECTOR = TestZoomVector{false};
const auto CONSTANT_ZOOM_VECTOR = TestZoomVector{true};

auto FullyUpdateDestBuffer(FilterBuffers& filterBuffers) noexcept -> void
{
  filterBuffers.UpdateTransformBuffer();

  filterBuffers.UpdateTransformBuffer();
  while (true)
  {
    filterBuffers.UpdateTransformBuffer();
    if (0 == filterBuffers.GetBufferYLineStart())
    {
      break;
    }
  }

  filterBuffers.UpdateTransformBuffer();
}

[[nodiscard]] auto GetFilterBuffers(Parallel& parallel, const TestZoomVector& zoomVector) noexcept
    -> FilterBuffers
{
  auto filterBufferStriper = std::make_unique<ZoomFilterBufferStriper>(
      parallel,
      GOOM_INFO,
      NORMALIZED_COORDS_CONVERTER,
      [&zoomVector](const NormalizedCoords& normalizedCoords,
                    const NormalizedCoords& viewportCoords)
      { return zoomVector.GetZoomPoint(normalizedCoords, viewportCoords); });

  return FilterBuffers{std::move(filterBufferStriper)};
}

constexpr auto TEST_X          = 10;
constexpr auto TEST_Y          = 50;
constexpr auto TEST_SRCE_POINT = Point2dInt{TEST_X, TEST_Y};
static_assert((0 <= TEST_X) && (TEST_X < WIDTH), "Invalid X");
static_assert((0 <= TEST_Y) && (TEST_Y < WIDTH), "Invalid Y");

constexpr auto DUMMY_NML_COORDS = NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(DUMMY_COORDS);

} // namespace

// NOLINTBEGIN(readability-function-cognitive-complexity)
TEST_CASE("ZoomFilterBuffers Basic")
{
  auto parallel      = Parallel{-1};
  auto filterBuffers = GetFilterBuffers(parallel, IDENTITY_ZOOM_VECTOR);

  filterBuffers.SetTransformBufferMidpoint(MID_PT);
  filterBuffers.Start();

  SECTION("Correct Starting TranBuffersState")
  {
  }
  SECTION("Correct Starting BuffYLineStart")
  {
    REQUIRE(0 == filterBuffers.GetBufferYLineStart());
  }
  SECTION("Correct Starting BuffMidpoint()")
  {
    REQUIRE(MID_PT == filterBuffers.GetBufferBuffMidpoint());
  }
  SECTION("Correct Starting HaveFilterSettingsChanged")
  {
    REQUIRE(!filterBuffers.HaveSettingsChanged());
  }
}

TEST_CASE("ZoomFilterBuffers Calculations - Correct Dest ZoomBufferTranPoint")
{
  auto parallel      = Parallel{-1};
  auto filterBuffers = GetFilterBuffers(parallel, CONSTANT_ZOOM_VECTOR);
  filterBuffers.SetTransformBufferMidpoint(MID_PT);
  filterBuffers.Start();

  static constexpr auto NML_CONST_ZOOM_VECTOR_COORDS_1 =
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1);

  REQUIRE(CONST_ZOOM_VECTOR_COORDS_1 == CONSTANT_ZOOM_VECTOR.GetConstCoords());
  REQUIRE(MID_PT == filterBuffers.GetBufferBuffMidpoint());

  // tranPoint comes solely from the dest Zoom buffer which because we are using a
  // const ZoomVectorFunc, returns a const normalized value
  UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_1.x = " << NML_CONST_ZOOM_VECTOR_COORDS_1.GetX());
  UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_1.y = " << NML_CONST_ZOOM_VECTOR_COORDS_1.GetY());
  UNSCOPED_INFO("GetZoomPoint(DUMMY_NML_COORDS).x = "
                << CONSTANT_ZOOM_VECTOR.GetZoomPoint(DUMMY_NML_COORDS, DUMMY_NML_COORDS).GetX());
  UNSCOPED_INFO("GetZoomPoint(DUMMY_NML_COORDS).y = "
                << CONSTANT_ZOOM_VECTOR.GetZoomPoint(DUMMY_NML_COORDS, DUMMY_NML_COORDS).GetY());
  REQUIRE(NML_CONST_ZOOM_VECTOR_COORDS_1.Equals(
      CONSTANT_ZOOM_VECTOR.GetZoomPoint(DUMMY_NML_COORDS, DUMMY_NML_COORDS)));
  const auto normalizedMidPt =
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(filterBuffers.GetBufferBuffMidpoint());
  const auto expectedNmlCoord1 = NormalizedCoords{
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1)};
  const auto expectedUnclippedTranPoint =
      COORD_TRANSFORMS.NormalizedToTranPoint(normalizedMidPt + expectedNmlCoord1);
  // NOLINTNEXTLINE(readability-suspicious-call-argument)
  const auto expectedTranPoint = clamp(expectedUnclippedTranPoint, {0, 0}, MAX_TRAN_POINT);
  UNSCOPED_INFO("filterBuffers.GetTransformBufferBuffMidpoint().x = "
                << filterBuffers.GetBufferBuffMidpoint().x);
  UNSCOPED_INFO("filterBuffers.GetTransformBufferBuffMidpoint().y = "
                << filterBuffers.GetBufferBuffMidpoint().y);
  UNSCOPED_INFO("normalizedMidPt.x = " << normalizedMidPt.GetX());
  UNSCOPED_INFO("normalizedMidPt.y = " << normalizedMidPt.GetY());
  UNSCOPED_INFO("expectedNmlCoord1.x = " << expectedNmlCoord1.GetX());
  UNSCOPED_INFO("expectedNmlCoord1.y = " << expectedNmlCoord1.GetY());
  UNSCOPED_INFO("expectedUnclippedTranPoint.x = " << expectedUnclippedTranPoint.x);
  UNSCOPED_INFO("expectedUnclippedTranPoint.y = " << expectedUnclippedTranPoint.y);
  UNSCOPED_INFO("expectedTranPoint.x = " << expectedTranPoint.x);
  UNSCOPED_INFO("expectedTranPoint.y = " << expectedTranPoint.y);
}

namespace
{

auto TestCorrectStripesBasicValues(const FilterBuffers& filterBuffers) -> void
{
  static constexpr auto NML_CONST_ZOOM_VECTOR_COORDS1 =
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1);

  REQUIRE(CONST_ZOOM_VECTOR_COORDS_1 == CONSTANT_ZOOM_VECTOR.GetConstCoords());
  REQUIRE(MID_PT == filterBuffers.GetBufferBuffMidpoint());
  REQUIRE(NML_CONST_ZOOM_VECTOR_COORDS1.Equals(
      CONSTANT_ZOOM_VECTOR.GetZoomPoint(DUMMY_NML_COORDS, DUMMY_NML_COORDS)));

  REQUIRE(not filterBuffers.HaveSettingsChanged());
}

auto TestCorrectStripesFullyUpdate(FilterBuffers& filterBuffers,
                                   const TestZoomVector& constantZoomVector) -> void
{
  // Make sure dest buffer is completely copied to srce buffer at end of update.
  filterBuffers.NotifyFilterSettingsHaveChanged();
  REQUIRE(filterBuffers.HaveSettingsChanged());

  FullyUpdateDestBuffer(filterBuffers);
  REQUIRE(0 == filterBuffers.GetBufferYLineStart());
  REQUIRE(CONST_ZOOM_VECTOR_COORDS_2 == constantZoomVector.GetConstCoords());
  REQUIRE(MID_PT == filterBuffers.GetBufferBuffMidpoint());
}

[[nodiscard]] auto TestCorrectStripesGetExpectedDestPoint(const FilterBuffers& filterBuffers,
                                                          const TestZoomVector& constantZoomVector)
    -> Point2dInt
{
  // Get dest buffer points - should be all CONST_ZOOM_VECTOR_COORDS_2

  const auto normalizedMidPt =
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(filterBuffers.GetBufferBuffMidpoint());

  static constexpr auto NML_CONST_ZOOM_VECTOR_COORDS2 =
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_2);
  REQUIRE(NML_CONST_ZOOM_VECTOR_COORDS2.Equals(
      constantZoomVector.GetZoomPoint(DUMMY_NML_COORDS, DUMMY_NML_COORDS)));

  const auto expectedDestTranPoint =
      COORD_TRANSFORMS.NormalizedToTranPoint(normalizedMidPt + NML_CONST_ZOOM_VECTOR_COORDS2);
  const auto expectedUnclippedDestPoint =
      ZoomCoordTransforms::TranToScreenPoint(expectedDestTranPoint);
  const auto expectedDestPoint =
      clamp(expectedUnclippedDestPoint,
            {0, 0},
            {static_cast<int32_t>(WIDTH - 1U), static_cast<int32_t>(HEIGHT - 1U)});
  UNSCOPED_INFO("normalizedMidPt.x = " << normalizedMidPt.GetX());
  UNSCOPED_INFO("normalizedMidPt.y = " << normalizedMidPt.GetY());
  UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_2.x = " << NML_CONST_ZOOM_VECTOR_COORDS2.GetX());
  UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_2.y = " << NML_CONST_ZOOM_VECTOR_COORDS2.GetY());
  UNSCOPED_INFO("expectedDestTranPoint.x = " << expectedDestTranPoint.x);
  UNSCOPED_INFO("expectedDestTranPoint.y = " << expectedDestTranPoint.y);
  UNSCOPED_INFO("expectedUnclippedDestPoint.x = " << expectedUnclippedDestPoint.x);
  UNSCOPED_INFO("expectedUnclippedDestPoint.y = " << expectedUnclippedDestPoint.y);
  UNSCOPED_INFO("expectedDestPoint.x = " << expectedDestPoint.x);
  UNSCOPED_INFO("expectedDestPoint.y = " << expectedDestPoint.y);

  return expectedDestPoint;
}

auto TestCorrectStripesDestPoint([[maybe_unused]] FilterBuffers& filterBuffers,
                                 [[maybe_unused]] const uint32_t buffPos,
                                 [[maybe_unused]] const Point2dInt& expectedDestPoint) -> void
{
  //TODO(glk) - fix this
  //  const auto destPoint = GetSourcePoint(filterBuffers, buffPos);
  //  UNSCOPED_INFO("destPoint.x = " << destPoint.x);
  //  UNSCOPED_INFO("destPoint.y = " << destPoint.y);
  //
  //  REQUIRE(expectedDestPoint == destPoint);
}

} // namespace

TEST_CASE("ZoomFilterBuffers Stripes")
{
  auto constantZoomVector = TestZoomVector{true};

  auto parallel      = Parallel{-1};
  auto filterBuffers = GetFilterBuffers(parallel, constantZoomVector);
  filterBuffers.SetTransformBufferMidpoint(MID_PT);
  filterBuffers.Start();

  TestCorrectStripesBasicValues(filterBuffers);

  constantZoomVector.SetConstCoords(CONST_ZOOM_VECTOR_COORDS_2);
  REQUIRE(CONST_ZOOM_VECTOR_COORDS_2 == constantZoomVector.GetConstCoords());
  TestCorrectStripesFullyUpdate(filterBuffers, constantZoomVector);

  const auto expectedDestPoint =
      TestCorrectStripesGetExpectedDestPoint(filterBuffers, constantZoomVector);
  for (auto buffPos = 0U; buffPos < (WIDTH * HEIGHT); ++buffPos)
  {
    TestCorrectStripesDestPoint(filterBuffers, buffPos, expectedDestPoint);
  }
}

TEST_CASE("ZoomFilterBuffers Adjustment")
{
  static constexpr auto TEST_SRCE_NML_COORDS =
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(TEST_SRCE_POINT);

  auto zoomVector = TestZoomVector{false};

  auto parallel      = Parallel{-1};
  auto filterBuffers = GetFilterBuffers(parallel, zoomVector);

  filterBuffers.SetTransformBufferMidpoint(MID_PT);
  filterBuffers.Start();

  SECTION("Correct Zoomed In Dest ZoomBufferTranPoint")
  {
    REQUIRE(TEST_SRCE_NML_COORDS.Equals(
        zoomVector.GetZoomPoint(TEST_SRCE_NML_COORDS, TEST_SRCE_NML_COORDS)));

    static constexpr auto ZOOM_ADJUSTMENT1 = 0.2F;
    static constexpr auto ZOOM_FACTOR1     = 1.0F - ZOOM_ADJUSTMENT1;
    zoomVector.SetZoomAdjustment(ZOOM_ADJUSTMENT1);
    REQUIRE((ZOOM_FACTOR1 * TEST_SRCE_NML_COORDS)
                .Equals(zoomVector.GetZoomPoint(TEST_SRCE_NML_COORDS, TEST_SRCE_NML_COORDS)));

    // GetSourcePoint uses tranPoint which comes solely from the dest Zoom buffer.
    // Because we are using a zoomed in ZoomVectorFunc, tranPoint should be zoomed in.
    filterBuffers.NotifyFilterSettingsHaveChanged();
    REQUIRE(filterBuffers.HaveSettingsChanged());
    FullyUpdateDestBuffer(filterBuffers);

    //    const auto expectedTranPoint = ZoomCoordTransforms::ScreenToTranPoint(TEST_SRCE_POINT);
    //    const auto expectedZoomedInTranPoint = Point2dInt{
    //        static_cast<int32_t>(ZOOM_FACTOR1 *
    //                             static_cast<float>(expectedTranPoint.x - MID_TRAN_POINT.x)) +
    //            MID_TRAN_POINT.x,
    //        static_cast<int32_t>(ZOOM_FACTOR1 *
    //                             static_cast<float>(expectedTranPoint.y - MID_TRAN_POINT.y)) +
    //            MID_TRAN_POINT.y};
    //    const auto expectedZoomedInSrcePoint =
    //        ZoomCoordTransforms::TranToScreenPoint(expectedZoomedInTranPoint);
    //    const auto srcePoint = GetSourcePoint(filterBuffers, GetBuffPos(TEST_X, TEST_Y));
    //    UNSCOPED_INFO("expectedTranPoint.x = " << expectedTranPoint.x);
    //    UNSCOPED_INFO("expectedTranPoint.y = " << expectedTranPoint.y);
    //    UNSCOPED_INFO("expectedZoomedInTranPoint.x = " << expectedZoomedInTranPoint.x);
    //    UNSCOPED_INFO("expectedZoomedInTranPoint.y = " << expectedZoomedInTranPoint.y);
    //    UNSCOPED_INFO("expectedZoomedInSrcePoint.x = " << expectedZoomedInSrcePoint.x);
    //    UNSCOPED_INFO("expectedZoomedInSrcePoint.y = " << expectedZoomedInSrcePoint.y);
    //    UNSCOPED_INFO("srcePoint.x = " << srcePoint.x);
    //    UNSCOPED_INFO("srcePoint.y = " << srcePoint.y);
    //    REQUIRE(expectedZoomedInSrcePoint == srcePoint);
  }
}

TEST_CASE("ZoomFilterBuffers Clipping")
{
  auto parallel      = Parallel{-1};
  auto filterBuffers = GetFilterBuffers(parallel, CONSTANT_ZOOM_VECTOR);
  filterBuffers.SetTransformBufferMidpoint({0, 0});
  filterBuffers.Start();

  static constexpr auto NML_CONST_ZOOM_VECTOR_COORDS1 =
      NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1);

  SECTION("Clipped ZoomBufferTranPoint")
  {
    REQUIRE(Point2dInt{0, 0} == filterBuffers.GetBufferBuffMidpoint());

    // tranPoint comes solely from the dest Zoom buffer which because we are using a
    // const ZoomVectorFunc, returns a const normalized value
    UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_1.x = " << NML_CONST_ZOOM_VECTOR_COORDS1.GetX());
    UNSCOPED_INFO("NML_CONST_ZOOM_VECTOR_COORDS_1.y = " << NML_CONST_ZOOM_VECTOR_COORDS1.GetY());
    UNSCOPED_INFO("GetZoomPoint(DUMMY_NML_COORDS).x = "
                  << CONSTANT_ZOOM_VECTOR.GetZoomPoint(DUMMY_NML_COORDS, DUMMY_NML_COORDS).GetX());
    UNSCOPED_INFO("GetZoomPoint(DUMMY_NML_COORDS).y = "
                  << CONSTANT_ZOOM_VECTOR.GetZoomPoint(DUMMY_NML_COORDS, DUMMY_NML_COORDS).GetY());
    REQUIRE(NML_CONST_ZOOM_VECTOR_COORDS1.Equals(
        CONSTANT_ZOOM_VECTOR.GetZoomPoint(DUMMY_NML_COORDS, DUMMY_NML_COORDS)));
    const auto normalizedMidPt =
        NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(filterBuffers.GetBufferBuffMidpoint());
    const auto expectedTranPoint = COORD_TRANSFORMS.NormalizedToTranPoint(
        normalizedMidPt +
        NORMALIZED_COORDS_CONVERTER.OtherToNormalizedCoords(CONST_ZOOM_VECTOR_COORDS_1));
    // Because mid-point is zero, the trans point is negative and therefore clipped.
    REQUIRE(expectedTranPoint.x < 0);
    REQUIRE(expectedTranPoint.y < 0);

    UNSCOPED_INFO("filterBuffers.GetTransformBufferBuffMidpoint().x = "
                  << filterBuffers.GetBufferBuffMidpoint().x);
    UNSCOPED_INFO("filterBuffers.GetTransformBufferBuffMidpoint().y = "
                  << filterBuffers.GetBufferBuffMidpoint().y);
    UNSCOPED_INFO("normalizedMidPt.x = " << normalizedMidPt.GetX());
    UNSCOPED_INFO("normalizedMidPt.y = " << normalizedMidPt.GetY());
    UNSCOPED_INFO("expectedTranPoint.x = " << expectedTranPoint.x);
    UNSCOPED_INFO("expectedTranPoint.y = " << expectedTranPoint.y);

    //    const auto destPoint = filterBuffers.GetSourcePointInfo(0);
    //    UNSCOPED_INFO("destPoint.isClipped = " << destPoint.isClipped);
    //    UNSCOPED_INFO("destPoint.screenPoint.x = " << destPoint.screenPoint.x);
    //    UNSCOPED_INFO("destPoint.screenPoint.y = " << destPoint.screenPoint.y);
    //    REQUIRE(destPoint.isClipped);
    //    REQUIRE(0 == destPoint.screenPoint.x);
    //    REQUIRE(0 == destPoint.screenPoint.y);

    // TODO(glk) Test coeff values
  }
}
// NOLINTEND(readability-function-cognitive-complexity)

} // namespace GOOM::UNIT_TESTS
