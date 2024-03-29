#include "lsys_paths.h"

#include "goom/point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/parametric_functions2d.h"
#include "utils/math/paths.h"
#include "utils/t_values.h"

#include <cstdint>
#include <memory>
#include <utility>

namespace GOOM::VISUAL_FX::L_SYSTEM
{

using UTILS::TValue;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::OscillatingFunction;
using UTILS::MATH::OscillatingPath;
using UTILS::MATH::StartAndEndPos;

LSysPath::LSysPath(const IGoomRand& goomRand) noexcept : m_goomRand{&goomRand}
{
}

auto LSysPath::Init() noexcept -> void
{
  m_direction      = Direction::TO_TARGET;
  m_lSysPathTarget = m_lSysPathStart;
  m_pathsToAndFrom->pathToTarget.Reset(0.0F);
  m_newLSysPathStart = true;
  ChangeToToPath();
}

auto LSysPath::SetPathStart(const Point2dInt& pathStart) noexcept -> void
{
  if (pathStart == m_lSysPathStart)
  {
    return;
  }

  m_newLSysPathStart = true;
  m_lSysPathStart    = pathStart;
}

auto LSysPath::SetPathTarget(const Point2dInt& pathTarget) noexcept -> void
{
  if (pathTarget == m_lSysPathTarget)
  {
    return;
  }

  m_newLSysPathTarget = true;
  m_lSysPathTarget    = pathTarget;
}

auto LSysPath::SetPathNumSteps(const uint32_t numSteps) noexcept -> void
{
  m_pathNumSteps = numSteps;
}

auto LSysPath::GetPathsToAndFrom() const noexcept -> std::unique_ptr<PathsToAndFrom>
{
  const auto pathParams = GetPathParams();

  // clang-format off
  auto pathsToAndFrom = PathsToAndFrom{
      OscillatingPath{std::make_unique<TValue>(
                     TValue::NumStepsProperties{
                          TValue::StepType::SINGLE_CYCLE, m_pathNumSteps}),
                      StartAndEndPos{ToPoint2dFlt(m_lSysPathStart), ToPoint2dFlt(m_lSysPathTarget)},
                      pathParams},
      OscillatingPath{std::make_unique<TValue>(
                      TValue::NumStepsProperties{
                          TValue::StepType::SINGLE_CYCLE, m_pathNumSteps}),
                      StartAndEndPos{ToPoint2dFlt(m_lSysPathTarget), ToPoint2dFlt(m_lSysPathStart)},
                      pathParams}
  };
  // clang-format on

  return std::make_unique<PathsToAndFrom>(std::move(pathsToAndFrom));
}

auto LSysPath::GetPathParams() const noexcept -> OscillatingFunction::Params
{
  static constexpr auto MIN_PATH_AMPLITUDE = 90.0F;
  static constexpr auto MAX_PATH_AMPLITUDE = 110.0F;
  static constexpr auto MIN_PATH_X_FREQ    = 0.9F;
  static constexpr auto MAX_PATH_X_FREQ    = 2.0F;
  static constexpr auto MIN_PATH_Y_FREQ    = 0.9F;
  static constexpr auto MAX_PATH_Y_FREQ    = 2.0F;

  return {
      m_goomRand->GetRandInRange(MIN_PATH_AMPLITUDE, MAX_PATH_AMPLITUDE),
      m_goomRand->GetRandInRange(MIN_PATH_X_FREQ, MAX_PATH_X_FREQ),
      m_goomRand->GetRandInRange(MIN_PATH_Y_FREQ, MAX_PATH_Y_FREQ),
  };
}

auto LSysPath::GetNextPathPosition() const noexcept -> Point2dInt
{
  if (m_direction == Direction::TO_TARGET)
  {
    return m_pathsToAndFrom->pathToTarget.GetNextPoint();
  }

  return m_pathsToAndFrom->pathFromTarget.GetNextPoint();
}

auto LSysPath::IncrementPositionT() noexcept -> void
{
  if (m_direction == Direction::TO_TARGET)
  {
    m_pathsToAndFrom->pathToTarget.IncrementT();
    if (m_pathsToAndFrom->pathToTarget.IsStopped())
    {
      ChangeToFromPath();
    }
  }
  else
  {
    m_pathsToAndFrom->pathFromTarget.IncrementT();
    if (m_pathsToAndFrom->pathFromTarget.IsStopped())
    {
      ChangeToToPath();
    }
  }
}

inline auto LSysPath::ChangeToFromPath() noexcept -> void
{
  m_direction = Direction::FROM_TARGET;
  if ((not m_newLSysPathTarget) and (not m_newLSysPathStart))
  {
    m_pathsToAndFrom->pathFromTarget.Reset(0.0F);
  }
  else
  {
    UpdatePaths();
  }
}

inline auto LSysPath::ChangeToToPath() noexcept -> void
{
  m_direction = Direction::TO_TARGET;
  if ((not m_newLSysPathTarget) and (not m_newLSysPathStart))
  {
    m_pathsToAndFrom->pathToTarget.Reset(0.0F);
  }
  else
  {
    UpdatePaths();
  }
}

inline auto LSysPath::UpdatePaths() noexcept -> void
{
  m_pathsToAndFrom    = GetPathsToAndFrom();
  m_newLSysPathTarget = false;
  m_newLSysPathStart  = false;
}

} // namespace GOOM::VISUAL_FX::L_SYSTEM
