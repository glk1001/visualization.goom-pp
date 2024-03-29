#pragma once

#include "goom/point2d.h"

#include <cstdint>
#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

class CirclesTentacleLayout
{
public:
  struct LayoutProperties
  {
    float startRadius;
    float endRadius;
    uint32_t numTentacles;
  };

  explicit CirclesTentacleLayout(const LayoutProperties& layoutProperties) noexcept;

  [[nodiscard]] auto GetNumTentacles() const noexcept -> uint32_t;

  [[nodiscard]] auto GetStartPoints() const noexcept -> const std::vector<Point2dFlt>&;
  [[nodiscard]] auto GetEndPoints() const noexcept -> const std::vector<Point2dFlt>&;

  [[nodiscard]] auto GetStartRadius() const noexcept -> float;
  [[nodiscard]] auto GetEndRadius() const noexcept -> float;

  struct CirclePointsProperties
  {
    float radius;
    uint32_t numPoints;
  };
  [[nodiscard]] static auto GetCirclePoints(
      const CirclePointsProperties& circlePointsProperties) noexcept -> std::vector<Point2dFlt>;

private:
  std::vector<Point2dFlt> m_startPoints;
  std::vector<Point2dFlt> m_endPoints;
  float m_startRadius;
  float m_endRadius;
};

inline auto CirclesTentacleLayout::GetNumTentacles() const noexcept -> uint32_t
{
  return static_cast<uint32_t>(m_startPoints.size());
}

inline auto CirclesTentacleLayout::GetStartPoints() const noexcept -> const std::vector<Point2dFlt>&
{
  return m_startPoints;
}

inline auto CirclesTentacleLayout::GetEndPoints() const noexcept -> const std::vector<Point2dFlt>&
{
  return m_endPoints;
}

inline auto CirclesTentacleLayout::GetStartRadius() const noexcept -> float
{
  return m_startRadius;
}

inline auto CirclesTentacleLayout::GetEndRadius() const noexcept -> float
{
  return m_endRadius;
}

} // namespace GOOM::VISUAL_FX::TENTACLES
