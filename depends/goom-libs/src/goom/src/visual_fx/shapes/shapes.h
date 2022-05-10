#pragma once

#include "color/random_colormaps.h"
#include "color/random_colormaps_manager.h"
#include "goom_plugin_info.h"
#include "point2d.h"
#include "shape_parts.h"
#include "utils/math/goom_rand_base.h"

#include <functional>
#include <vector>

namespace GOOM::VISUAL_FX::SHAPES
{

class Shape
{
public:
  struct Params
  {
    float minRadiusFraction;
    float maxRadiusFraction;
    int32_t minShapeDotRadius;
    int32_t maxShapeDotRadius;
    uint32_t maxNumShapePaths;
    Point2dInt zoomMidpoint;
    float minShapePathSpeed;
    float maxShapePathSpeed;
  };

  Shape(const UTILS::MATH::IGoomRand& goomRand,
        const PluginInfo& goomInfo,
        COLOR::RandomColorMapsManager& colorMapsManager,
        const Params& params) noexcept;

  auto SetWeightedMainColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps) noexcept
      -> void;
  auto SetWeightedLowColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps) noexcept
      -> void;
  auto SetWeightedInnerColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps) noexcept
      -> void;

  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;
  auto SetMinMaxShapePathSpeeds(float minShapePathSpeed, float maxShapePathSpeed) noexcept -> void;

  auto Start() noexcept -> void;

  auto Update() noexcept -> void;

  [[nodiscard]] auto GetTotalNumShapePaths() const noexcept -> uint32_t;
  [[nodiscard]] auto GetNumShapeParts() const noexcept -> size_t;
  [[nodiscard]] auto GetShapePart(size_t shapePartNum) const noexcept -> const ShapePart&;

  using ConstShapePartFunction = std::function<void(const ShapePart&)>;
  auto IterateAllShapeParts(const ConstShapePartFunction& shapePartFunction) const noexcept -> void;

  [[nodiscard]] auto HasFirstShapePathJustHitStartBoundary() const noexcept -> bool;
  [[nodiscard]] auto HasFirstShapePathJustHitEndBoundary() const noexcept -> bool;
  [[nodiscard]] auto FirstShapePathAtMeetingPoint() const noexcept -> bool;
  [[nodiscard]] auto FirstShapePathsCloseToMeeting() const noexcept -> bool;
  auto DoRandomChanges() noexcept -> void;
  auto SetShapeSpeeds() noexcept -> void;
  auto SetFixedShapeSpeeds() noexcept -> void;

private:
  const UTILS::MATH::IGoomRand& m_goomRand;
  const PluginInfo& m_goomInfo;
  COLOR::RandomColorMapsManager& m_colorMapsManager;

  static constexpr uint32_t NUM_SHAPE_PARTS = 10;
  std::vector<ShapePart> m_shapeParts;
  [[nodiscard]] auto GetInitialShapeParts(const Params& params) noexcept -> std::vector<ShapePart>;
  [[nodiscard]] auto GetFirstShapePathPositionT() const noexcept -> float;

  static constexpr float STARTING_FIXED_T_MIN_MAX_LERP = 0.5F;
  float m_fixedTMinMaxLerp =
      ShapePart::GetNewRandomMinMaxLerpT(m_goomRand, STARTING_FIXED_T_MIN_MAX_LERP);
  auto SetRandomShapeSpeeds() noexcept -> void;

  [[nodiscard]] auto AllColorMapsValid() const noexcept -> bool;
};

inline auto Shape::GetNumShapeParts() const noexcept -> size_t
{
  return m_shapeParts.size();
}

inline auto Shape::GetShapePart(const size_t shapePartNum) const noexcept -> const ShapePart&
{
  return m_shapeParts.at(shapePartNum);
}

inline auto Shape::IterateAllShapeParts(
    const ConstShapePartFunction& shapePartFunction) const noexcept -> void
{
  std::for_each(begin(m_shapeParts), end(m_shapeParts), shapePartFunction);
}

inline auto Shape::HasFirstShapePathJustHitStartBoundary() const noexcept -> bool
{
  if (0 == m_shapeParts.front().GetNumShapePaths())
  {
    return false;
  }

  return m_shapeParts.front().GetShapePath(0).HasJustHitStartBoundary();
}

inline auto Shape::HasFirstShapePathJustHitEndBoundary() const noexcept -> bool
{
  if (0 == m_shapeParts.front().GetNumShapePaths())
  {
    return false;
  }

  return m_shapeParts.front().GetShapePath(0).HasJustHitEndBoundary();
}

inline auto Shape::FirstShapePathAtMeetingPoint() const noexcept -> bool
{
  return HasFirstShapePathJustHitStartBoundary() || HasFirstShapePathJustHitEndBoundary();
}

inline auto Shape::FirstShapePathsCloseToMeeting() const noexcept -> bool
{
  return m_shapeParts.front().AreShapePathsCloseToMeeting();
}

inline auto Shape::GetFirstShapePathPositionT() const noexcept -> float
{
  if (0 == m_shapeParts.front().GetNumShapePaths())
  {
    return 1.0F;
  }

  return m_shapeParts.front().GetFirstShapePathPositionT();
}

inline auto Shape::SetShapeSpeeds() noexcept -> void
{
  if (constexpr float PROB_FIXED_SPEEDS = 0.95F; m_goomRand.ProbabilityOf(PROB_FIXED_SPEEDS))
  {
    SetFixedShapeSpeeds();
  }
  else
  {
    SetRandomShapeSpeeds();
  }
}

} // namespace GOOM::VISUAL_FX::SHAPES