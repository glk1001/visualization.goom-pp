#pragma once

#include "filter_fx/common_types.h"
#include "filter_fx/normalized_coords.h"
#include "filter_fx/zoom_adjustment_effect.h"
#include "goom/goom_types.h"
#include "goom/point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

#include <cstdint>
#include <vector>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class DistanceField : public IZoomAdjustmentEffect
{
public:
  enum class Modes : UnderlyingEnumType
  {
    MODE0,
    MODE1,
    MODE2
  };
  DistanceField(Modes mode, const UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Point2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> UTILS::NameValuePairs override;

  enum class GridType : UnderlyingEnumType
  {
    FULL,
    PARTIAL_X,
    PARTIAL_DIAMOND,
    PARTIAL_RANDOM,
    _num // unused, and marks the enum end
  };
  using GridPointsWithCentres = std::vector<Point2dInt>;
  using GridCentresList       = std::vector<Point2dInt>;
  using GridPointMap          = std::vector<std::vector<GridCentresList>>;
  struct Params
  {
    Amplitude amplitude;
    GridType gridType;
    int32_t gridMax;
    float gridScale;
    float cellCentre;
    struct GridArrays
    {
      GridPointsWithCentres gridPointsWithCentres;
      GridPointMap gridPointCentresMap;
    };
    GridArrays gridArrays;
  };
  [[nodiscard]] auto GetParams() const noexcept -> const Params&;

protected:
  auto SetParams(const Params& params) noexcept -> void;

private:
  Modes m_mode;
  const UTILS::MATH::IGoomRand* m_goomRand;
  UTILS::MATH::Weights<GridType> m_weightedEffects;

  Params m_params;
  auto SetMode0RandomParams() noexcept -> void;
  auto SetMode1RandomParams() noexcept -> void;
  auto SetMode2RandomParams() noexcept -> void;

  using GridWidthRange = UTILS::MATH::IGoomRand::NumberRange<uint32_t>;
  auto SetRandomParams(const AmplitudeRange& amplitudeRange,
                       const GridWidthRange& gridWidthRange) noexcept -> void;

  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const noexcept -> Point2dFlt;
  [[nodiscard]] auto GetGridWidth(GridType gridType,
                                  const GridWidthRange& gridWidthRange) const noexcept -> uint32_t;
  [[nodiscard]] auto GetGridsArray(GridType gridType, uint32_t gridWidth) const noexcept
      -> Params::GridArrays;
  [[nodiscard]] auto GetGridPointsWithCentres(GridType gridType, uint32_t gridWidth) const noexcept
      -> GridPointsWithCentres;
  [[nodiscard]] static auto GetGridPointCentresMap(
      uint32_t gridWidth, const GridPointsWithCentres& gridPointsWithCentres) noexcept
      -> GridPointMap;
  [[nodiscard]] static auto FindNearestGridPointsWithCentres(
      const Point2dInt& gridPoint, const GridPointsWithCentres& gridPointsWithCentres) noexcept
      -> GridCentresList;
  [[nodiscard]] static auto GetGridPointXArray(uint32_t gridWidth) noexcept
      -> GridPointsWithCentres;
  [[nodiscard]] static auto GetGridPointDiamondArray(uint32_t gridWidth) noexcept
      -> GridPointsWithCentres;
  [[nodiscard]] auto GetGridPointRandomArray(uint32_t gridWidth) const noexcept
      -> GridPointsWithCentres;
  [[nodiscard]] auto TryGetGridPointRandomArray(uint32_t gridWidth) const noexcept
      -> GridPointsWithCentres;
  [[nodiscard]] auto GetAmplitude(const AmplitudeRange& amplitudeRange,
                                  GridType gridType,
                                  uint32_t gridWidth,
                                  const Params::GridArrays& gridArrays) const noexcept -> Amplitude;
  [[nodiscard]] static auto GetAmplitudeFactor(GridType gridType,
                                               uint32_t gridWidth,
                                               const Params::GridArrays& gridArrays) noexcept
      -> float;

  [[nodiscard]] auto GetDistanceSquaredFromClosestPoint(
      const NormalizedCoords& point) const noexcept -> float;
  [[nodiscard]] auto GetCorrespondingGridPoint(const NormalizedCoords& point) const noexcept
      -> Point2dInt;
  [[nodiscard]] auto GetNearsetGridPointsWithCentres(const Point2dInt& gridPoint) const noexcept
      -> GridCentresList;
  [[nodiscard]] auto GetNormalizedGridPointCentres(
      const GridCentresList& gridPointsWithCentres) const noexcept -> std::vector<NormalizedCoords>;
  [[nodiscard]] auto GetNormalizedGridPointCentre(
      const Point2dInt& gridPointWithCentre) const noexcept -> NormalizedCoords;
  [[nodiscard]] static auto GetMinDistanceSquared(
      const NormalizedCoords& point, const std::vector<NormalizedCoords>& centres) noexcept
      -> float;
};

inline auto DistanceField::GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
    -> Point2dFlt
{
  const auto velocity = GetVelocity(coords);

  return {coords.GetX() * velocity.x, coords.GetY() * velocity.y};
}

inline auto DistanceField::GetVelocity(const NormalizedCoords& coords) const noexcept -> Point2dFlt
{
  const auto sqDistFromClosestPoint = GetDistanceSquaredFromClosestPoint(coords);

  return {GetBaseZoomAdjustment().x + (m_params.amplitude.x * sqDistFromClosestPoint),
          GetBaseZoomAdjustment().y + (m_params.amplitude.y * sqDistFromClosestPoint)};
}

inline auto DistanceField::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline auto DistanceField::SetParams(const Params& params) noexcept -> void
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
