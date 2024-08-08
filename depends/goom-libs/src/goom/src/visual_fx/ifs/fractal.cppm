module;

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <utility>
#include <vector>

module Goom.VisualFx.IfsDancersFx:Fractal;

import Goom.Color.RandomColorMaps;
import Goom.Utils.Graphics.SmallImageBitmaps;
import Goom.Utils.Math.TValues;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Lib.GoomTypes;
import :FractalHits;
import :Similitudes;

using GOOM::COLOR::WeightedRandomColorMaps;
using GOOM::UTILS::GRAPHICS::SmallImageBitmaps;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::NumberRange;
using GOOM::UTILS::MATH::TValue;
using GOOM::UTILS::MATH::U_HALF;

namespace GOOM::VISUAL_FX::IFS
{

class Fractal
{
public:
  Fractal(const Dimensions& dimensions,
          const GoomRand& goomRand,
          const SmallImageBitmaps& smallBitmaps);

  auto Init() -> void;
  auto Reset() -> void;

  auto SetWeightedColorMaps(const WeightedRandomColorMaps& weightedColorMaps) -> void;

  [[nodiscard]] auto GetSpeed() const -> uint32_t;
  auto SetSpeed(uint32_t val) -> void;

  [[nodiscard]] auto GetNextIfsPoints() -> const std::vector<IfsPoint>&;
  [[nodiscard]] auto GetMaxHitCount() const -> uint32_t;

private:
  Similitudes m_similitudes;
  const GoomRand* m_goomRand;

  Flt m_halfWidth;
  Flt m_halfHeight;

  static constexpr auto MAX_COUNT_TIMES_SPEED_RANGE = NumberRange{950U, 1500U};
  uint32_t m_iterationCount                         = 0;
  auto UpdateIterationCount() -> void;

  static constexpr auto INITIAL_SPEED              = 6U;
  uint32_t m_prevSpeed                             = INITIAL_SPEED;
  uint32_t m_speed                                 = INITIAL_SPEED;
  static constexpr auto NUM_SPEED_TRANSITION_STEPS = 500U;
  TValue m_speedTransitionT{
      {.stepType = TValue::StepType::SINGLE_CYCLE, .numSteps = NUM_SPEED_TRANSITION_STEPS}
  };
  uint32_t m_maxCountTimesSpeed = MAX_COUNT_TIMES_SPEED_RANGE.max;

  FractalHits m_hits1;
  FractalHits m_hits2;
  FractalHits* m_prevHits{&m_hits1};
  FractalHits* m_curHits{&m_hits2};
  auto InitHits() -> void;

  auto UpdateMainSimis() -> void;
  [[nodiscard]] auto GetCurrentIfsPoints() -> const std::vector<IfsPoint>&;
  auto DrawFractal() -> void;
  auto Trace(uint32_t curDepth, const FltPoint& point0) -> void;
  [[nodiscard]] static auto AreSimilarPoints(const FltPoint& point1,
                                             const FltPoint& point2) -> bool;
  auto UpdateHits(const Similitude& simi, const FltPoint& point) -> void;
};

} // namespace GOOM::VISUAL_FX::IFS

namespace GOOM::VISUAL_FX::IFS
{

inline auto Fractal::GetMaxHitCount() const -> uint32_t
{
  return m_curHits->GetMaxHitCount();
}

Fractal::Fractal(const Dimensions& dimensions,
                 const GoomRand& goomRand,
                 const SmallImageBitmaps& smallBitmaps)
  : m_similitudes{goomRand, smallBitmaps},
    m_goomRand{&goomRand},
    m_halfWidth{static_cast<Flt>(U_HALF * (dimensions.GetWidth() - 1))},
    m_halfHeight{static_cast<Flt>(U_HALF * (dimensions.GetHeight() - 1))},
    m_hits1{dimensions},
    m_hits2{dimensions}
{
  m_speedTransitionT.Reset(TValue::MAX_T_VALUE);

  InitHits();
  Reset();
}

auto Fractal::SetWeightedColorMaps(const WeightedRandomColorMaps& weightedColorMaps) -> void
{
  m_similitudes.SetWeightedColorMaps(weightedColorMaps);
}

auto Fractal::Init() -> void
{
  InitHits();
  m_similitudes.Init();
}

inline auto Fractal::InitHits() -> void
{
  m_prevHits->Reset();
  m_curHits->Reset();
}

auto Fractal::GetSpeed() const -> uint32_t
{
  if (m_speedTransitionT.IsStopped())
  {
    return m_speed;
  }
  return static_cast<uint32_t>(std::lerp(m_prevSpeed, m_speed, m_speedTransitionT()));
}

auto Fractal::SetSpeed(const uint32_t val) -> void
{
  m_prevSpeed = GetSpeed();
  m_speed     = val;
  m_speedTransitionT.Reset();
}

auto Fractal::Reset() -> void
{
  m_maxCountTimesSpeed = m_goomRand->GetRandInRange<MAX_COUNT_TIMES_SPEED_RANGE>();

  m_similitudes.ResetCurrentIfsFunc();
}

auto Fractal::GetNextIfsPoints() -> const std::vector<IfsPoint>&
{
  m_speedTransitionT.Increment();

  UpdateMainSimis();

  const auto& currentIfsPoints = GetCurrentIfsPoints();

  UpdateIterationCount();

  return currentIfsPoints;
}

inline auto Fractal::GetCurrentIfsPoints() -> const std::vector<IfsPoint>&
{
  m_curHits->Reset();
  DrawFractal();
  const auto& currentBuffer = m_curHits->GetBuffer();
  std::swap(m_prevHits, m_curHits);
  return currentBuffer;
}

inline auto Fractal::UpdateIterationCount() -> void
{
  if (m_iterationCount < (m_maxCountTimesSpeed / GetSpeed()))
  {
    ++m_iterationCount;
    return;
  }

  m_similitudes.IterateSimis();

  m_iterationCount = 0;
}

auto Fractal::DrawFractal() -> void
{
  const auto& mainSimiGroup = m_similitudes.GetMainSimiGroup();
  const auto numSimis       = m_similitudes.GetNumSimis();

  for (auto i = 0U; i < numSimis; ++i)
  {
    const auto point0 = mainSimiGroup[i].GetCPoint();

    for (auto j = 0U; j < numSimis; ++j)
    {
      if (i != j)
      {
        const auto point = m_similitudes.Transform(mainSimiGroup[j], point0);
        Trace(m_similitudes.GetSimiDepth(), point);
      }
    }
  }
}

inline auto Fractal::UpdateMainSimis() -> void
{
  const auto uValue =
      static_cast<Dbl>(m_iterationCount * GetSpeed()) / static_cast<Dbl>(m_maxCountTimesSpeed);
  m_similitudes.UpdateMainSimis(uValue);
}

// https://glk.youtrack.cloud/issue/G-92
// NOLINTNEXTLINE(misc-no-recursion)
auto Fractal::Trace(const uint32_t curDepth, const FltPoint& point0) -> void
{
  const auto& mainSimiGroup = m_similitudes.GetMainSimiGroup();
  const auto numSimis       = m_similitudes.GetNumSimis();

  for (auto i = 0U; i < numSimis; ++i)
  {
    const auto point = m_similitudes.Transform(mainSimiGroup[i], point0);

    UpdateHits(mainSimiGroup[i], point);

    if (0 == curDepth)
    {
      continue;
    }
    if (AreSimilarPoints(point0, point))
    {
      continue;
    }

    Trace(curDepth - 1, point);
  }
}

inline auto Fractal::AreSimilarPoints(const FltPoint& point1, const FltPoint& point2) -> bool
{
  // TODO(glk) What's going on here?
  static constexpr auto CUTOFF = 16;
  return (std::abs(point1.x - point2.x) < CUTOFF) || (std::abs(point1.y - point2.y) < CUTOFF);
}

inline auto Fractal::UpdateHits(const Similitude& simi, const FltPoint& point) -> void
{
  const auto x = m_halfWidth + DivBy2Units(point.x * m_halfWidth);
  const auto y = m_halfHeight - DivBy2Units(point.y * m_halfHeight);
  m_curHits->AddHit(x, y, simi);
}

} // namespace GOOM::VISUAL_FX::IFS
