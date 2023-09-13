#pragma once

#include "goom/goom_config.h"
#include "goom/goom_time.h"
#include "goom/goom_types.h"

#include <algorithm>
#include <cstdint>
#include <functional>

namespace GOOM::UTILS
{

class Timer
{
public:
  Timer(const GoomTime& goomTime, uint64_t timeLimit, bool setToFinished = false) noexcept;

  [[nodiscard]] auto GetTimeLeft() const noexcept -> uint64_t;
  auto SetTimeLimit(uint64_t timeLimit, bool setToFinished = false) noexcept -> void;
  auto ResetToZero() noexcept -> void;
  auto SetToFinished() noexcept -> void;

  [[nodiscard]] auto JustFinished() const noexcept -> bool;
  [[nodiscard]] auto Finished() const noexcept -> bool;
  [[nodiscard]] auto GetTimeElapsed() const noexcept -> uint64_t;

private:
  const GoomTime* m_goomTime;
  uint64_t m_startTime;
  uint64_t m_timeLimit;
  uint64_t m_targetTime;
  bool m_finished;
};

class OnOffTimer
{
public:
  struct TimerCounts
  {
    uint32_t numOnCount;
    uint32_t numOnCountAfterFailedOff;
    uint32_t numOffCount;
    uint32_t numOffCountAfterFailedOn;
  };
  OnOffTimer(const GoomTime& goomTime, const TimerCounts& timerCounts) noexcept;

  auto Reset() noexcept -> void;
  auto SetTimerCounts(const TimerCounts& timerCounts) noexcept;

  using Action = std::function<bool()>; // return true if action succeeded.
  struct OnAndOffActions
  {
    Action onAction;
    Action offAction;
  };
  auto SetActions(const OnAndOffActions& onAndOffActions) noexcept -> void;

  auto StartOnTimer() noexcept -> void;
  auto StartOffTimer() noexcept -> void;

  auto Stop() noexcept -> void;

  auto Update() noexcept -> void;
  auto TryToChangeState() noexcept -> void;

private:
  TimerCounts m_timerCounts;
  Timer m_onTimer;
  Timer m_offTimer;
  Action m_onAction  = nullptr;
  Action m_offAction = nullptr;
  enum class TimerState : UnderlyingEnumType
  {
    NO_TIMERS_ACTIVE,
    ON_TIMER_ACTIVE,
    OFF_TIMER_ACTIVE,
  };
  TimerState m_timerState = TimerState::NO_TIMERS_ACTIVE;
  auto ChangeStateToOff() -> void;
  auto ChangeStateToOn() -> void;
};

inline Timer::Timer(const GoomTime& goomTime,
                    const uint64_t timeLimit,
                    const bool setToFinished) noexcept
  : m_goomTime{&goomTime},
    m_startTime{m_goomTime->GetCurrentTime()},
    m_timeLimit{timeLimit},
    m_targetTime{m_startTime + timeLimit},
    m_finished{setToFinished}
{
}

inline auto Timer::GetTimeElapsed() const noexcept -> uint64_t
{
  return std::min(m_timeLimit, m_goomTime->GetElapsedTimeSince(m_startTime));
}

inline auto Timer::GetTimeLeft() const noexcept -> uint64_t
{
  if (m_finished or (m_goomTime->GetCurrentTime() >= m_targetTime))
  {
    return 0U;
  }
  return m_targetTime - m_goomTime->GetCurrentTime();
}

inline auto Timer::ResetToZero() noexcept -> void
{
  m_startTime  = m_goomTime->GetCurrentTime();
  m_targetTime = m_startTime + m_timeLimit;
  m_finished   = false;
}

inline auto Timer::SetToFinished() noexcept -> void
{
  m_finished = true;
}

inline auto Timer::SetTimeLimit(const uint64_t timeLimit, const bool setToFinished) noexcept -> void
{
  m_startTime  = m_goomTime->GetCurrentTime();
  m_timeLimit  = timeLimit;
  m_targetTime = m_startTime + timeLimit;
  m_finished   = setToFinished;
}

inline auto Timer::JustFinished() const noexcept -> bool
{
  return m_goomTime->GetCurrentTime() == m_targetTime;
}

inline auto Timer::Finished() const noexcept -> bool
{
  return m_finished or (m_goomTime->GetCurrentTime() >= m_targetTime);
}

inline OnOffTimer::OnOffTimer(const GoomTime& goomTime, const TimerCounts& timerCounts) noexcept
  : m_timerCounts{timerCounts},
    m_onTimer{goomTime, m_timerCounts.numOnCount, true},
    m_offTimer{goomTime, m_timerCounts.numOffCount, true}
{
}

inline auto OnOffTimer::Reset() noexcept -> void
{
  m_onTimer.ResetToZero();
  m_offTimer.ResetToZero();
  m_onTimer.SetToFinished();
  m_offTimer.SetToFinished();
  m_timerState = TimerState::NO_TIMERS_ACTIVE;
}

inline auto OnOffTimer::SetTimerCounts(const TimerCounts& timerCounts) noexcept
{
  m_timerCounts = timerCounts;
  m_onTimer.SetTimeLimit(m_timerCounts.numOnCount, true);
  m_offTimer.SetTimeLimit(m_timerCounts.numOffCount, true);
}

inline auto OnOffTimer::SetActions(const OnAndOffActions& onAndOffActions) noexcept -> void
{
  m_onAction  = onAndOffActions.onAction;
  m_offAction = onAndOffActions.offAction;
}

inline auto OnOffTimer::StartOnTimer() noexcept -> void
{
  Expects(m_timerState == TimerState::NO_TIMERS_ACTIVE);
  Expects(m_onTimer.Finished());
  Expects(m_offTimer.Finished());
  Expects(m_onAction != nullptr);
  Expects(m_offAction != nullptr);

  m_timerState = TimerState::ON_TIMER_ACTIVE;
  m_onTimer.ResetToZero();
  m_onAction();

  Ensures(not m_onTimer.Finished());
  Ensures(m_offTimer.Finished());
}

inline auto OnOffTimer::StartOffTimer() noexcept -> void
{
  Expects(m_timerState == TimerState::NO_TIMERS_ACTIVE);
  Expects(m_onTimer.Finished());
  Expects(m_offTimer.Finished());
  Expects(m_onAction != nullptr);
  Expects(m_offAction != nullptr);

  m_timerState = TimerState::OFF_TIMER_ACTIVE;
  m_offTimer.ResetToZero();
  m_offAction();

  Ensures(not m_offTimer.Finished());
  Ensures(m_onTimer.Finished());
}

inline auto OnOffTimer::Stop() noexcept -> void
{
  if (TimerState::ON_TIMER_ACTIVE == m_timerState)
  {
    m_offAction();
  }
  else if (TimerState::OFF_TIMER_ACTIVE == m_timerState)
  {
    m_onAction();
  }
  m_timerState = TimerState::NO_TIMERS_ACTIVE;
  m_onTimer.SetToFinished();
  m_offTimer.SetToFinished();

  Ensures(m_onTimer.Finished());
  Ensures(m_offTimer.Finished());
  Ensures(m_timerState == TimerState::NO_TIMERS_ACTIVE);
}

inline auto OnOffTimer::TryToChangeState() noexcept -> void
{
  if (TimerState::ON_TIMER_ACTIVE == m_timerState)
  {
    ChangeStateToOff();
  }
  else if (TimerState::OFF_TIMER_ACTIVE == m_timerState)
  {
    ChangeStateToOn();
  }
}

inline auto OnOffTimer::Update() noexcept -> void
{
  if (TimerState::ON_TIMER_ACTIVE == m_timerState)
  {
    Expects(m_offTimer.Finished());
    if (m_onTimer.Finished())
    {
      ChangeStateToOff();
    }
  }
  else if (TimerState::OFF_TIMER_ACTIVE == m_timerState)
  {
    Expects(m_onTimer.Finished());
    if (m_offTimer.Finished())
    {
      ChangeStateToOn();
    }
  }
}

inline auto OnOffTimer::ChangeStateToOff() -> void
{
  Expects(m_offTimer.Finished());

  if (not m_offAction())
  {
    m_onTimer.SetTimeLimit(m_timerCounts.numOnCountAfterFailedOff);
    Ensures(not m_onTimer.Finished());
    Ensures(m_offTimer.Finished());
    return;
  }

  m_onTimer.SetToFinished();
  m_offTimer.SetTimeLimit(m_timerCounts.numOffCount);
  m_timerState = TimerState::OFF_TIMER_ACTIVE;

  Ensures(not m_offTimer.Finished());
  Ensures(m_onTimer.Finished());
}

inline auto OnOffTimer::ChangeStateToOn() -> void
{
  Expects(m_onTimer.Finished());

  if (not m_onAction())
  {
    m_offTimer.SetTimeLimit(m_timerCounts.numOffCountAfterFailedOn);
    Ensures(not m_offTimer.Finished());
    Ensures(m_onTimer.Finished());
    return;
  }

  m_offTimer.SetToFinished();
  m_onTimer.SetTimeLimit(m_timerCounts.numOnCount);
  m_timerState = TimerState::ON_TIMER_ACTIVE;

  Ensures(not m_onTimer.Finished());
  Ensures(m_offTimer.Finished());
}

} // namespace GOOM::UTILS
