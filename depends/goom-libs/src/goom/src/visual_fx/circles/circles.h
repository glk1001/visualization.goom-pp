#pragma once

#include "bitmap_getter.h"
#include "circle.h"
#include "color/random_color_maps.h"
#include "dot_paths.h"
#include "goom_plugin_info.h"
#include "helper.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/parametric_functions2d.h"
#include "visual_fx/fx_helper.h"

#include <cstdint>
#include <vector>

namespace GOOM::VISUAL_FX::CIRCLES
{

class Circles
{
public:
  Circles(FxHelper& fxHelper,
          const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps,
          uint32_t numCircles,
          const std::vector<UTILS::MATH::OscillatingFunction::Params>& pathParams,
          const std::vector<Circle::Params>& circleParams) noexcept;

  auto SetWeightedColorMaps(const COLOR::WeightedRandomColorMaps& weightedMaps,
                            const COLOR::WeightedRandomColorMaps& weightedLowMaps) noexcept -> void;
  [[nodiscard]] auto GetCurrentDirection() const noexcept -> DotPaths::Direction;
  auto ChangeDirection(DotPaths::Direction newDirection) noexcept -> void;
  auto SetPathParams(
      const std::vector<UTILS::MATH::OscillatingFunction::Params>& pathParams) noexcept -> void;
  auto SetGlobalBrightnessFactors(const std::vector<float>& brightnessFactors) noexcept -> void;

  auto Start() noexcept -> void;
  auto UpdateAndDraw() noexcept -> void;
  auto IncrementTs() noexcept -> void;

  [[nodiscard]] auto HasPositionTJustHitStartBoundary() const noexcept -> bool;
  [[nodiscard]] auto HasPositionTJustHitEndBoundary() const noexcept -> bool;

private:
  const PluginInfo* m_goomInfo;
  const UTILS::MATH::IGoomRand* m_goomRand;
  BitmapGetter m_bitmapGetter;

  uint32_t m_numCircles;
  std::vector<Circle> m_circles;
  [[nodiscard]] static auto GetCircles(
      FxHelper& fxHelper,
      const Helper& helper,
      const std::vector<UTILS::MATH::OscillatingFunction::Params>& pathParams,
      uint32_t numCircles,
      const std::vector<Circle::Params>& circleParams) noexcept -> std::vector<Circle>;
  auto UpdatePositionSpeed() noexcept -> void;
  auto UpdateAndDrawCircles() noexcept -> void;
};

inline auto Circles::GetCurrentDirection() const noexcept -> DotPaths::Direction
{
  return m_circles.front().GetCurrentDirection();
}

inline auto Circles::HasPositionTJustHitStartBoundary() const noexcept -> bool
{
  return m_circles.front().HasPositionTJustHitStartBoundary();
}

inline auto Circles::HasPositionTJustHitEndBoundary() const noexcept -> bool
{
  return m_circles.front().HasPositionTJustHitEndBoundary();
}

} // namespace GOOM::VISUAL_FX::CIRCLES
