//#undef NO_LOGGING

#include "circles.h"

#include "bitmap_getter.h"
#include "circle.h"
#include "color/random_color_maps.h"
#include "dot_paths.h"
#include "goom_plugin_info.h"
#include "helper.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/parametric_functions2d.h"
#include "visual_fx/fx_helper.h"

#include <algorithm>
#include <cstdint>
#include <vector>

namespace GOOM::VISUAL_FX::CIRCLES
{

using COLOR::WeightedRandomColorMaps;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::OscillatingFunction;

static constexpr auto LINE_DOT_DIAMETER = BitmapGetter::MIN_DOT_DIAMETER;
static constexpr auto MIN_DOT_DIAMETER  = BitmapGetter::MIN_DOT_DIAMETER + 4;
static constexpr auto MAX_DOT_DIAMETER  = BitmapGetter::MAX_DOT_DIAMETER;

Circles::Circles(FxHelper& fxHelper,
                 const SmallImageBitmaps& smallBitmaps,
                 const uint32_t numCircles,
                 const std::vector<OscillatingFunction::Params>& pathParams,
                 const std::vector<Circle::Params>& circleParams) noexcept
  : m_goomInfo{&fxHelper.GetGoomInfo()},
    m_goomRand{&fxHelper.GetGoomRand()},
    m_bitmapGetter{fxHelper.GetGoomRand(), smallBitmaps},
    m_numCircles{numCircles},
    m_circles{GetCircles(fxHelper,
                         {LINE_DOT_DIAMETER, MIN_DOT_DIAMETER, MAX_DOT_DIAMETER, &m_bitmapGetter},
                         pathParams,
                         m_numCircles,
                         circleParams)}
{
}

auto Circles::GetCircles(FxHelper& fxHelper,
                         const Helper& helper,
                         const std::vector<OscillatingFunction::Params>& pathParams,
                         const uint32_t numCircles,
                         const std::vector<Circle::Params>& circleParams) noexcept
    -> std::vector<Circle>
{
  auto circles = std::vector<Circle>{};
  circles.reserve(numCircles);

  for (auto i = 0U; i < numCircles; ++i)
  {
    circles.emplace_back(fxHelper, helper, circleParams[i], pathParams[i]);
  }

  return circles;
}

auto Circles::SetWeightedColorMaps(const WeightedRandomColorMaps& weightedMaps,
                                   const WeightedRandomColorMaps& weightedLowMaps) noexcept -> void
{
  std::for_each(begin(m_circles),
                end(m_circles),
                [&weightedMaps, &weightedLowMaps](Circle& circle)
                { circle.SetWeightedColorMaps(weightedMaps, weightedLowMaps); });

  m_bitmapGetter.ChangeCurrentBitmap();
}

auto Circles::ChangeDirection(const DotPaths::Direction newDirection) noexcept -> void
{
  std::for_each(begin(m_circles),
                end(m_circles),
                [&newDirection](Circle& circle) { circle.ChangeDirection(newDirection); });
}

auto Circles::SetPathParams(const std::vector<OscillatingFunction::Params>& pathParams) noexcept
    -> void
{
  for (auto i = 0U; i < m_numCircles; ++i)
  {
    m_circles.at(i).SetPathParams(pathParams.at(i));
  }
}

auto Circles::SetGlobalBrightnessFactors(const std::vector<float>& brightnessFactors) noexcept
    -> void
{
  for (auto i = 0U; i < m_numCircles; ++i)
  {
    m_circles.at(i).SetGlobalBrightnessFactor(brightnessFactors.at(i));
  }
}

auto Circles::Start() noexcept -> void
{
  std::for_each(begin(m_circles), end(m_circles), [](Circle& circle) { circle.Start(); });
}

auto Circles::UpdateAndDraw() noexcept -> void
{
  UpdateAndDrawCircles();
  UpdatePositionSpeed();
}

inline auto Circles::UpdateAndDrawCircles() noexcept -> void
{
  std::for_each(begin(m_circles), end(m_circles), [](Circle& circle) { circle.UpdateAndDraw(); });
}

auto Circles::IncrementTs() noexcept -> void
{
  std::for_each(begin(m_circles), end(m_circles), [](Circle& circle) { circle.IncrementTs(); });
}

auto Circles::UpdatePositionSpeed() noexcept -> void
{
  if (static constexpr auto PROB_NO_SPEED_CHANGE = 0.7F;
      m_goomRand->ProbabilityOf(PROB_NO_SPEED_CHANGE))
  {
    return;
  }

  static constexpr auto MIN_POSITION_STEPS = 100U;
  static constexpr auto MAX_POSITION_STEPS = 1000U;
  const auto newNumSteps                   = std::min(
      MIN_POSITION_STEPS + m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom(), MAX_POSITION_STEPS);

  std::for_each(begin(m_circles),
                end(m_circles),
                [&newNumSteps](Circle& circle) { circle.UpdatePositionSpeed(newNumSteps); });
}

} // namespace GOOM::VISUAL_FX::CIRCLES
