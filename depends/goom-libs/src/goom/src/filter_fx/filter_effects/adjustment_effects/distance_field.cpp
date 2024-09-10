module;

//#undef NO_LOGGING

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

module Goom.FilterFx.FilterEffects.AdjustmentEffects.DistanceField;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.EnumUtils;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Lib.AssertUtils;
import Goom.Lib.Point2d;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::EnumToString;
using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::MATH::GoomRand;
using UTILS::MATH::HALF;
using UTILS::MATH::IsEven;
using UTILS::MATH::NumberRange;
using UTILS::MATH::Sq;
using UTILS::MATH::U_HALF;

static constexpr auto MIN_GRID_WIDTH         = 4U;
static constexpr auto GRID_WIDTH_RANGE_MODE0 = NumberRange{4U, 15U};
static constexpr auto GRID_WIDTH_RANGE_MODE1 = NumberRange{16U, 31U};
static constexpr auto GRID_WIDTH_RANGE_MODE2 = NumberRange{32U, 63U};
static_assert(GRID_WIDTH_RANGE_MODE0.min >= MIN_GRID_WIDTH);
static_assert(GRID_WIDTH_RANGE_MODE1.min > GRID_WIDTH_RANGE_MODE0.min);
static_assert(GRID_WIDTH_RANGE_MODE2.min > GRID_WIDTH_RANGE_MODE1.min);

static constexpr auto AMPLITUDE_RANGE_MODE0 = AmplitudeRange{
    .xRange = {0.05F, 1.101F},
    .yRange = {0.05F, 1.101F},
};
static constexpr auto AMPLITUDE_RANGE_MODE1 = AmplitudeRange{
    .xRange = {0.50F, 2.01F},
    .yRange = {0.50F, 2.01F},
};
static constexpr auto AMPLITUDE_RANGE_MODE2 = AmplitudeRange{
    .xRange = {0.70F, 3.01F},
    .yRange = {0.70F, 3.01F},
};
static constexpr auto FULL_AMPLITUDE_FACTOR            = 2.0F;
static constexpr auto PARTIAL_DIAMOND_AMPLITUDE_FACTOR = 0.1F;

static constexpr auto LERP_TO_ONE_T_RANGE = NumberRange{0.0F, 1.0F};

static constexpr auto PROB_AMPLITUDES_EQUAL              = 0.50F;
static constexpr auto PROB_LERP_TO_ONE_T_S_EQUAL         = 0.95F;
static constexpr auto PROB_USE_DISCONTINUOUS_ZOOM_FACTOR = 0.5F;
static constexpr auto PROB_RANDOM_CENTRE                 = 0.1F;

static constexpr auto GRID_TYPE_FULL_WEIGHT            = 100.0F;
static constexpr auto GRID_TYPE_PARTIAL_X_WEIGHT       = 10.0F;
static constexpr auto GRID_TYPE_PARTIAL_DIAMOND_WEIGHT = 10.0F;
static constexpr auto GRID_TYPE_PARTIAL_RANDOM_WEIGHT  = 10.0F;

DistanceField::DistanceField(const Modes mode, const GoomRand& goomRand) noexcept
  : m_mode{mode},
    m_goomRand{&goomRand},
    m_weightedEffects{
        *m_goomRand,
        {
            {    .key=GridType::FULL,        .weight=GRID_TYPE_FULL_WEIGHT},
            {    .key=GridType::PARTIAL_X,   .weight=GRID_TYPE_PARTIAL_X_WEIGHT},
            {.key=GridType::PARTIAL_DIAMOND, .weight=GRID_TYPE_PARTIAL_DIAMOND_WEIGHT},
            {.key=GridType::PARTIAL_RANDOM,  .weight=GRID_TYPE_PARTIAL_RANDOM_WEIGHT},
        }
    },
    m_params{GetMode0RandomParams()}
{
}

auto DistanceField::SetRandomParams() noexcept -> void
{
  if (m_mode == Modes::MODE0)
  {
    SetMode0RandomParams();
  }
  else if (m_mode == Modes::MODE1)
  {
    SetMode1RandomParams();
  }
  else
  {
    SetMode2RandomParams();
  }
}

auto DistanceField::GetMode0RandomParams() const noexcept -> Params
{
  return GetRandomParams(AMPLITUDE_RANGE_MODE0, GRID_WIDTH_RANGE_MODE0);
}

auto DistanceField::GetMode1RandomParams() const noexcept -> Params
{
  return GetRandomParams(AMPLITUDE_RANGE_MODE1, GRID_WIDTH_RANGE_MODE1);
}

auto DistanceField::GetMode2RandomParams() const noexcept -> Params
{
  return GetRandomParams(AMPLITUDE_RANGE_MODE2, GRID_WIDTH_RANGE_MODE2);
}

auto DistanceField::GetRandomParams(const AmplitudeRange& amplitudeRange,
                                    const GridWidthRange& gridWidthRange) const noexcept -> Params
{
  const auto gridType   = m_weightedEffects.GetRandomWeighted();
  const auto gridWidth  = GetGridWidth(gridType, gridWidthRange);
  const auto gridScale  = static_cast<float>(gridWidth) / NormalizedCoords::COORD_WIDTH;
  const auto cellCentre = HALF / gridScale;
  const auto gridArrays = GetGridsArray(gridType, gridWidth);

  const auto amplitude = GetAmplitude(amplitudeRange, gridType, gridWidth, gridArrays);

  const auto xLerpToOneT = m_goomRand->GetRandInRange<LERP_TO_ONE_T_RANGE>();
  const auto yLerpToOneT = m_goomRand->ProbabilityOf<PROB_LERP_TO_ONE_T_S_EQUAL>()
                               ? xLerpToOneT
                               : m_goomRand->GetRandInRange<LERP_TO_ONE_T_RANGE>();
  const auto useDiscontinuousZoomFactor =
      m_goomRand->ProbabilityOf<PROB_USE_DISCONTINUOUS_ZOOM_FACTOR>();

  return {
      .amplitude                  = amplitude,
      .lerpToOneTs                = {.xLerpT = xLerpToOneT, .yLerpT = yLerpToOneT},
      .useDiscontinuousZoomFactor = useDiscontinuousZoomFactor,
      .gridType                   = gridType,
      .gridMax                    = static_cast<int32_t>(gridWidth - 1),
      .gridScale                  = gridScale,
      .cellCentre                 = cellCentre,
      .gridArrays                 = gridArrays,
  };
}

auto DistanceField::GetGridWidth(const GridType gridType,
                                 const GridWidthRange& gridWidthRange) const noexcept -> uint32_t
{
  if ((gridType == GridType::PARTIAL_RANDOM) and (gridWidthRange.min == GRID_WIDTH_RANGE_MODE0.min))
  {
    // For random grid type, wider range looks better.
    return m_goomRand->GetRandInRange<GRID_WIDTH_RANGE_MODE1>();
  }

  const auto gridWidth = m_goomRand->GetRandInRange(gridWidthRange);

  if ((gridType == GridType::PARTIAL_DIAMOND) and IsEven(gridWidth))
  {
    return 1 + gridWidth;
  }

  return gridWidth;
}

auto DistanceField::GetGridsArray(const GridType gridType, const uint32_t gridWidth) const noexcept
    -> Params::GridArrays
{
  if (gridType == GridType::FULL)
  {
    return {.gridPointsWithCentres = DistanceField::GridPointsWithCentres{},
            .gridPointCentresMap   = DistanceField::GridPointMap{}};
  }

  const auto gridPointArray      = GetGridPointsWithCentres(gridType, gridWidth);
  const auto gridPointCentresMap = GetGridPointCentresMap(gridWidth, gridPointArray);

  return {.gridPointsWithCentres = gridPointArray, .gridPointCentresMap = gridPointCentresMap};
}

auto DistanceField::GetGridPointsWithCentres(const GridType gridType,
                                             const uint32_t gridWidth) const noexcept
    -> GridPointsWithCentres
{
  if (gridType == GridType::PARTIAL_X)
  {
    return GetGridPointXArray(gridWidth);
  }
  if (gridType == GridType::PARTIAL_DIAMOND)
  {
    return GetGridPointDiamondArray(gridWidth);
  }
  if (gridType == GridType::PARTIAL_RANDOM)
  {
    return GetGridPointRandomArray(gridWidth);
  }

  std::unreachable();
}

auto DistanceField::GetGridPointXArray(const uint32_t gridWidth) noexcept -> GridPointsWithCentres
{
  auto gridPointArray = GridPointsWithCentres{};

  for (auto y = 0U; y < gridWidth; ++y)
  {
    gridPointArray.emplace_back(GetPoint2dInt(y, y));
    gridPointArray.emplace_back(GetPoint2dInt(y, gridWidth - y - 1));
  }

  return gridPointArray;
}

auto DistanceField::GetGridPointDiamondArray(const uint32_t gridWidth) noexcept
    -> GridPointsWithCentres
{
  auto gridPointArray = GridPointsWithCentres{};

  gridPointArray.emplace_back(GetPoint2dInt(0U, (U_HALF * gridWidth) - 1));
  gridPointArray.emplace_back(GetPoint2dInt((U_HALF * gridWidth) - 1, gridWidth - 1));
  gridPointArray.emplace_back(GetPoint2dInt(gridWidth - 1, (U_HALF * gridWidth) - 1));
  gridPointArray.emplace_back(GetPoint2dInt((U_HALF * gridWidth) - 1, 0U));

  return gridPointArray;
}

auto DistanceField::GetGridPointRandomArray(const uint32_t gridWidth) const noexcept
    -> GridPointsWithCentres
{
  static constexpr auto MAX_TRIES = 100U;

  for (auto i = 0U; i < MAX_TRIES; ++i)
  {
    auto gridPointArray = TryGetGridPointRandomArray(gridWidth);
    if (not gridPointArray.empty())
    {
      return gridPointArray;
    }
  }

  std::unreachable();
}

auto DistanceField::TryGetGridPointRandomArray(const uint32_t gridWidth) const noexcept
    -> GridPointsWithCentres
{
  auto gridPointArray = GridPointsWithCentres{};

  for (auto y = 0U; y < gridWidth; ++y)
  {
    for (auto x = 0U; x < gridWidth; ++x)
    {
      if (m_goomRand->ProbabilityOf<PROB_RANDOM_CENTRE>())
      {
        gridPointArray.emplace_back(GetPoint2dInt(x, y));
      }
    }
  }

  return gridPointArray;
}

auto DistanceField::GetGridPointCentresMap(
    const uint32_t gridWidth, const GridPointsWithCentres& gridPointsWithCentres) noexcept
    -> GridPointMap
{
  Expects(gridWidth > 0U);
  Expects(not gridPointsWithCentres.empty());

  auto gridPointCentresMap = GridPointMap(gridWidth);

  for (auto y = 0U; y < gridWidth; ++y)
  {
    gridPointCentresMap[y].resize(gridWidth);
    for (auto x = 0U; x < gridWidth; ++x)
    {
      gridPointCentresMap[y][x] =
          FindNearestGridPointsWithCentres(GetPoint2dInt(x, y), gridPointsWithCentres);
    }
  }

  return gridPointCentresMap;
}

inline auto DistanceField::FindNearestGridPointsWithCentres(
    const Point2dInt& gridPoint, const GridPointsWithCentres& gridPointsWithCentres) noexcept
    -> GridCentresList
{
  Expects(not gridPointsWithCentres.empty());

  auto minDistancePoints = GridCentresList{};
  auto minDistanceSq     = std::numeric_limits<int32_t>::max();

  for (const auto& centrePoint : gridPointsWithCentres)
  {
    const auto distanceSq = SqDistance(gridPoint, centrePoint);
    if (distanceSq < minDistanceSq)
    {
      minDistanceSq     = distanceSq;
      minDistancePoints = GridCentresList{centrePoint};
    }
    else if (distanceSq == minDistanceSq)
    {
      minDistancePoints.emplace_back(centrePoint);
    }
  }

  Ensures(not minDistancePoints.empty());

  return minDistancePoints;
}

inline auto DistanceField::GetAmplitude(const AmplitudeRange& amplitudeRange,
                                        const GridType gridType,
                                        const uint32_t gridWidth,
                                        const Params::GridArrays& gridArrays) const noexcept
    -> Amplitude
{
  const auto xAmplitude = m_goomRand->GetRandInRange(amplitudeRange.xRange);
  const auto yAmplitude = m_goomRand->ProbabilityOf<PROB_AMPLITUDES_EQUAL>()
                              ? xAmplitude
                              : m_goomRand->GetRandInRange(amplitudeRange.yRange);

  const auto amplitudeFactor = GetAmplitudeFactor(gridType, gridWidth, gridArrays);

  return {amplitudeFactor * xAmplitude, amplitudeFactor * yAmplitude};
}

inline auto DistanceField::GetAmplitudeFactor(const GridType gridType,
                                              const uint32_t gridWidth,
                                              const Params::GridArrays& gridArrays) noexcept
    -> float
{
  if (gridType == GridType::PARTIAL_X)
  {
    return 1.0F + std::log2(static_cast<float>(gridWidth));
  }
  if (gridType == GridType::PARTIAL_DIAMOND)
  {
    return PARTIAL_DIAMOND_AMPLITUDE_FACTOR;
  }
  if (gridType == GridType::PARTIAL_RANDOM)
  {
    return std::log2(static_cast<float>(gridArrays.gridPointsWithCentres.size()));
  }

  return FULL_AMPLITUDE_FACTOR * std::log2(static_cast<float>(gridWidth));
}

auto DistanceField::GetDistanceSquaredFromClosestPoint(const NormalizedCoords& point) const noexcept
    -> float
{
  const auto gridPoint = GetCorrespondingGridPoint(point);

  if (m_params.gridType == GridType::FULL)
  {
    const auto normalizedGridCentre = GetNormalizedGridPointCentre(gridPoint);
    return SqDistance(point, normalizedGridCentre);
  }

  const auto gridPointsWithCentres = GetNearsetGridPointsWithCentres(gridPoint);
  const auto normalizedGridCentres = GetNormalizedGridPointCentres(gridPointsWithCentres);

  return GetMinDistanceSquared(point, normalizedGridCentres);
}

inline auto DistanceField::GetCorrespondingGridPoint(const NormalizedCoords& point) const noexcept
    -> Point2dInt
{
  const auto x = static_cast<int32_t>(
      std::floor(m_params.gridScale * (point.GetX() - NormalizedCoords::MIN_COORD)));
  const auto y = static_cast<int32_t>(
      std::floor(m_params.gridScale * (point.GetY() - NormalizedCoords::MIN_COORD)));

  return {.x = std::clamp(x, 0, m_params.gridMax), .y = std::clamp(y, 0, m_params.gridMax)};
}

inline auto DistanceField::GetNearsetGridPointsWithCentres(
    const Point2dInt& gridPoint) const noexcept -> GridCentresList
{
  if (m_params.gridType == GridType::FULL)
  {
    return {gridPoint};
  }

  const auto x = static_cast<uint32_t>(gridPoint.x);
  const auto y = static_cast<uint32_t>(gridPoint.y);

  return m_params.gridArrays.gridPointCentresMap.at(y).at(x);
}

inline auto DistanceField::GetNormalizedGridPointCentres(
    const GridCentresList& gridPointsWithCentres) const noexcept -> std::vector<NormalizedCoords>
{
  auto normalizedCentres = std::vector<NormalizedCoords>{};

  for (const auto& gridPoint : gridPointsWithCentres)
  {
    normalizedCentres.emplace_back(GetNormalizedGridPointCentre(gridPoint));
  }

  return normalizedCentres;
}

inline auto DistanceField::GetNormalizedGridPointCentre(
    const Point2dInt& gridPointWithCentre) const noexcept -> NormalizedCoords
{
  return {(NormalizedCoords::MIN_COORD +
           (static_cast<float>(gridPointWithCentre.x) / m_params.gridScale)) +
              m_params.cellCentre,
          (NormalizedCoords::MIN_COORD +
           (static_cast<float>(gridPointWithCentre.y) / m_params.gridScale)) +
              m_params.cellCentre};
}

inline auto DistanceField::GetMinDistanceSquared(
    const NormalizedCoords& point, const std::vector<NormalizedCoords>& centres) noexcept -> float
{
  static constexpr auto MAX_DISTANCE_SQUARED = 1.0F + (2.0F * Sq(NormalizedCoords::COORD_WIDTH));
  auto minDistanceSquared                    = MAX_DISTANCE_SQUARED;

  for (const auto& centre : centres)
  {
    minDistanceSquared = std::min(SqDistance(point, centre), minDistanceSquared);
  }

  if (minDistanceSquared >= MAX_DISTANCE_SQUARED)
  {
    minDistanceSquared = MAX_DISTANCE_SQUARED - UTILS::MATH::SMALL_FLOAT;
  }
  Ensures(minDistanceSquared < MAX_DISTANCE_SQUARED);

  return minDistanceSquared;
}

auto DistanceField::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  const auto fullParamGroup = GetFullParamGroup({PARAM_GROUP, "Dist Field"});
  const auto gridWidth      = m_params.gridMax + 1;
  return {
      GetPair(fullParamGroup, "GridType", EnumToString(m_params.gridType)),
      GetPair(fullParamGroup, "GridWidth", gridWidth),
      GetPair(fullParamGroup,
              "Grid Centres",
              m_params.gridType == GridType::FULL
                  ? static_cast<size_t>(Sq(gridWidth))
                  : m_params.gridArrays.gridPointsWithCentres.size()),
      GetPair(fullParamGroup,
              "amplitude",
              Point2dFlt{.x = m_params.amplitude.x, .y = m_params.amplitude.y}),
      GetPair(fullParamGroup,
              "lerpToOneTs",
              Point2dFlt{.x = m_params.lerpToOneTs.xLerpT, .y = m_params.lerpToOneTs.yLerpT}),
      GetPair(fullParamGroup, "discon zoom", m_params.useDiscontinuousZoomFactor),
  };
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
