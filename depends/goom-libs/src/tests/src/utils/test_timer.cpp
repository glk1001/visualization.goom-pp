// NOLINTBEGIN(cert-err58-cpp): Catch2 3.6.0 issue

#undef NO_LOGGING

#include "goom/goom_logger.h"

#include <catch2/catch_test_macros.hpp>
#include <cstdint>

import Goom.Utils.DebuggingLogger;
import Goom.Utils.GoomTime;
import Goom.Utils.Timer;

namespace GOOM::UNIT_TESTS
{

using UTILS::GoomTime;
using UTILS::OnOffTimer;
using UTILS::Timer;

// NOLINTBEGIN(bugprone-chained-comparison): Catch2 needs to fix this.
// NOLINTBEGIN(readability-function-cognitive-complexity)

namespace
{

auto UpdateTime(GoomTime& goomTime, uint32_t count)
{
  for (auto i = 0U; i < count; ++i)
  {
    goomTime.UpdateTime();
  }
}

} // namespace

TEST_CASE("Timer Simple")
{
  auto goomTime                    = GoomTime{};
  static constexpr auto TIME_LIMIT = 20U;
  auto timer                       = Timer{goomTime, TIME_LIMIT};

  REQUIRE(not timer.Finished());
  UpdateTime(goomTime, TIME_LIMIT - 1);
  REQUIRE(not timer.Finished());
  REQUIRE(timer.GetTimeElapsed() == TIME_LIMIT - 1);
  REQUIRE(timer.GetTimeLeft() == 1);

  goomTime.UpdateTime();
  REQUIRE(timer.JustFinished());
  REQUIRE(timer.Finished());
  REQUIRE(timer.GetTimeElapsed() == TIME_LIMIT);
  REQUIRE(timer.GetTimeLeft() == 0);

  goomTime.UpdateTime();
  REQUIRE(not timer.JustFinished());
  REQUIRE(timer.Finished());
  REQUIRE(timer.GetTimeElapsed() == TIME_LIMIT);
  REQUIRE(timer.GetTimeLeft() == 0);
}

TEST_CASE("Timer Reset")
{
  auto goomTime                    = GoomTime{};
  static constexpr auto TIME_LIMIT = 20U;
  auto timer                       = Timer{goomTime, TIME_LIMIT};

  REQUIRE(not timer.JustFinished());
  REQUIRE(not timer.Finished());
  REQUIRE(timer.GetTimeElapsed() == 0);
  REQUIRE(timer.GetTimeLeft() == TIME_LIMIT);

  goomTime.UpdateTime();
  REQUIRE(not timer.JustFinished());
  REQUIRE(not timer.Finished());
  REQUIRE(timer.GetTimeElapsed() == 1);
  REQUIRE(timer.GetTimeLeft() == (TIME_LIMIT - 1));

  goomTime.UpdateTime();
  timer.ResetToZero();
  REQUIRE(not timer.JustFinished());
  REQUIRE(not timer.Finished());
  REQUIRE(timer.GetTimeElapsed() == 0);
  REQUIRE(timer.GetTimeLeft() == TIME_LIMIT);

  UpdateTime(goomTime, TIME_LIMIT - 1);

  goomTime.UpdateTime();
  REQUIRE(timer.JustFinished());
  REQUIRE(timer.Finished());
  REQUIRE(timer.GetTimeElapsed() == TIME_LIMIT);
  REQUIRE(timer.GetTimeLeft() == 0);

  goomTime.UpdateTime();
  REQUIRE(not timer.JustFinished());
  REQUIRE(timer.Finished());
  REQUIRE(timer.GetTimeElapsed() == TIME_LIMIT);
  REQUIRE(timer.GetTimeLeft() == 0);
}

TEST_CASE("Timer Limit Change")
{
  auto goomTime                    = GoomTime{};
  static constexpr auto TIME_LIMIT = 20U;
  auto timer                       = Timer{goomTime, TIME_LIMIT};

  REQUIRE(not timer.JustFinished());
  REQUIRE(not timer.Finished());
  REQUIRE(timer.GetTimeElapsed() == 0);
  REQUIRE(timer.GetTimeLeft() == TIME_LIMIT);

  goomTime.UpdateTime();
  REQUIRE(not timer.JustFinished());
  REQUIRE(not timer.Finished());
  REQUIRE(timer.GetTimeElapsed() == 1);
  REQUIRE(timer.GetTimeLeft() == (TIME_LIMIT - 1));

  static constexpr auto NEW_TIME_LIMIT = 10U;
  timer.SetTimeLimit(NEW_TIME_LIMIT);

  REQUIRE(not timer.JustFinished());
  REQUIRE(not timer.Finished());
  REQUIRE(timer.GetTimeElapsed() == 1);
  REQUIRE(timer.GetTimeLeft() == NEW_TIME_LIMIT - 1);

  UpdateTime(goomTime, NEW_TIME_LIMIT - 2);

  goomTime.UpdateTime();
  REQUIRE(timer.JustFinished());
  REQUIRE(timer.Finished());
  REQUIRE(timer.GetTimeElapsed() == NEW_TIME_LIMIT);
  REQUIRE(timer.GetTimeLeft() == 0);

  goomTime.UpdateTime();
  REQUIRE(not timer.JustFinished());
  REQUIRE(timer.Finished());
  REQUIRE(timer.GetTimeElapsed() == NEW_TIME_LIMIT);
  REQUIRE(timer.GetTimeLeft() == 0);

  timer.ResetToZero();
  timer.SetTimeLimit(TIME_LIMIT);
  REQUIRE(not timer.JustFinished());
  REQUIRE(not timer.Finished());
  REQUIRE(timer.GetTimeElapsed() == 0);
  REQUIRE(timer.GetTimeLeft() == TIME_LIMIT);

  goomTime.UpdateTime();
  REQUIRE(not timer.JustFinished());
  REQUIRE(not timer.Finished());
  REQUIRE(timer.GetTimeElapsed() == 1);
  REQUIRE(timer.GetTimeLeft() == (TIME_LIMIT - 1));

  timer.SetTimeLimitAndResetToZero(NEW_TIME_LIMIT);

  REQUIRE(not timer.JustFinished());
  REQUIRE(not timer.Finished());
  REQUIRE(timer.GetTimeElapsed() == 0);
  REQUIRE(timer.GetTimeLeft() == NEW_TIME_LIMIT);

  UpdateTime(goomTime, NEW_TIME_LIMIT - 1);

  goomTime.UpdateTime();
  REQUIRE(timer.JustFinished());
  REQUIRE(timer.Finished());
  REQUIRE(timer.GetTimeElapsed() == NEW_TIME_LIMIT);
  REQUIRE(timer.GetTimeLeft() == 0);

  goomTime.UpdateTime();
  REQUIRE(not timer.JustFinished());
  REQUIRE(timer.Finished());
  REQUIRE(timer.GetTimeElapsed() == NEW_TIME_LIMIT);
  REQUIRE(timer.GetTimeLeft() == 0);
}

TEST_CASE("Timer Set To Finished")
{
  auto goomTime                    = GoomTime{};
  static constexpr auto TIME_LIMIT = 20U;
  auto timer                       = Timer{goomTime, TIME_LIMIT, true};

  REQUIRE(not timer.JustFinished());
  REQUIRE(timer.Finished());
  REQUIRE(timer.GetTimeElapsed() == 0);

  goomTime.UpdateTime();
  REQUIRE(not timer.JustFinished());
  REQUIRE(timer.Finished());
  REQUIRE(timer.GetTimeElapsed() == 1);

  timer.ResetToZero();
  REQUIRE(not timer.JustFinished());
  REQUIRE(not timer.Finished());
  REQUIRE(timer.GetTimeElapsed() == 0);
  REQUIRE(timer.GetTimeLeft() == TIME_LIMIT);

  goomTime.UpdateTime();
  REQUIRE(timer.GetTimeElapsed() == 1);
  REQUIRE(timer.GetTimeLeft() == (TIME_LIMIT - 1));

  timer.SetToFinished();
  REQUIRE(not timer.JustFinished());
  REQUIRE(timer.Finished());
  REQUIRE(timer.GetTimeElapsed() == 1);
  REQUIRE(timer.GetTimeLeft() == 0);

  timer.ResetToZero();
  REQUIRE(not timer.JustFinished());
  REQUIRE(not timer.Finished());
  REQUIRE(timer.GetTimeElapsed() == 0);
  REQUIRE(timer.GetTimeLeft() == TIME_LIMIT);

  UpdateTime(goomTime, TIME_LIMIT);
  REQUIRE(timer.JustFinished());
  REQUIRE(timer.Finished());
  REQUIRE(timer.GetTimeElapsed() == TIME_LIMIT);
  REQUIRE(timer.GetTimeLeft() == 0U);
}

TEST_CASE("OnOffTimer")
{
  auto goomTime                                       = GoomTime{};
  static constexpr auto NUM_ON_COUNT                  = 20U;
  static constexpr auto NUM_ON_COUNT_AFTER_FAILED_OFF = 0U;
  static constexpr auto NUM_OFF_COUNT                 = 10U;
  static constexpr auto NUM_OFF_COUNT_AFTER_FAILED_ON = 0U;
  static constexpr auto TIMER_COUNTS                  = OnOffTimer::TimerCounts{
                       .numOnCount               = NUM_ON_COUNT,
                       .numOnCountAfterFailedOff = NUM_ON_COUNT_AFTER_FAILED_OFF,
                       .numOffCount              = NUM_OFF_COUNT,
                       .numOffCountAfterFailedOn = NUM_OFF_COUNT_AFTER_FAILED_ON,
  };
  auto onOffTimer = OnOffTimer{goomTime, TIMER_COUNTS};

  bool onActionCalled = false;
  const auto onAction = [&onActionCalled]()
  {
    onActionCalled = true;
    return true;
  };
  bool offActionCalled = false;
  const auto offAction = [&offActionCalled]()
  {
    offActionCalled = true;
    return true;
  };

  onOffTimer.SetActions({.onAction = onAction, .offAction = offAction});

  // Run 'on' timer.
  REQUIRE(onOffTimer.GetTimerState() == OnOffTimer::TimerState::NO_TIMERS_ACTIVE);
  REQUIRE(onOffTimer.GetOnTimer().Finished());
  REQUIRE(onOffTimer.GetOffTimer().Finished());
  REQUIRE(not onActionCalled);
  REQUIRE(not offActionCalled);
  onOffTimer.StartOnTimer();
  REQUIRE(onOffTimer.GetTimerState() == OnOffTimer::TimerState::ON_TIMER_ACTIVE);
  REQUIRE(not onOffTimer.GetOnTimer().Finished());
  REQUIRE(onOffTimer.GetOffTimer().Finished());
  REQUIRE(onOffTimer.GetOnTimer().GetTimeLeft() == NUM_ON_COUNT);
  REQUIRE(onActionCalled);
  REQUIRE(not offActionCalled);
  onActionCalled = false;
  UpdateTime(goomTime, 1U);
  REQUIRE(onOffTimer.GetTimerState() == OnOffTimer::TimerState::ON_TIMER_ACTIVE);
  REQUIRE(onOffTimer.GetOnTimer().GetTimeLeft() == (NUM_ON_COUNT - 1U));
  UpdateTime(goomTime, NUM_ON_COUNT - 1U);
  REQUIRE(not offActionCalled);
  REQUIRE(not onActionCalled);
  onOffTimer.Update();
  REQUIRE(onOffTimer.GetTimerState() == OnOffTimer::TimerState::OFF_TIMER_ACTIVE);
  REQUIRE(onOffTimer.GetOnTimer().Finished());
  REQUIRE(not onOffTimer.GetOffTimer().Finished());
  REQUIRE(onOffTimer.GetOffTimer().GetTimeLeft() == NUM_OFF_COUNT);
  REQUIRE(not onActionCalled);
  REQUIRE(offActionCalled);

  // Should now have switched to 'off' timer.
  onActionCalled  = false;
  offActionCalled = false;
  UpdateTime(goomTime, 1U);
  REQUIRE(onOffTimer.GetTimerState() == OnOffTimer::TimerState::OFF_TIMER_ACTIVE);
  REQUIRE(onOffTimer.GetOffTimer().GetTimeLeft() == (NUM_OFF_COUNT - 1U));
  UpdateTime(goomTime, NUM_OFF_COUNT - 1U);
  REQUIRE(not onActionCalled);
  REQUIRE(not offActionCalled);
  onOffTimer.Update();
  REQUIRE(onOffTimer.GetTimerState() == OnOffTimer::TimerState::ON_TIMER_ACTIVE);
  REQUIRE(not onOffTimer.GetOnTimer().Finished());
  REQUIRE(onOffTimer.GetOffTimer().Finished());
  REQUIRE(onActionCalled);
  REQUIRE(not offActionCalled);

  // Run 'off' timer.
  onActionCalled  = false;
  offActionCalled = false;
  onOffTimer.Reset();
  REQUIRE(onOffTimer.GetTimerState() == OnOffTimer::TimerState::NO_TIMERS_ACTIVE);
  REQUIRE(onOffTimer.GetOnTimer().Finished());
  REQUIRE(onOffTimer.GetOffTimer().Finished());
  REQUIRE(not onActionCalled);
  REQUIRE(not offActionCalled);
  onOffTimer.StartOffTimer();
  REQUIRE(onOffTimer.GetTimerState() == OnOffTimer::TimerState::OFF_TIMER_ACTIVE);
  REQUIRE(onOffTimer.GetOnTimer().Finished());
  REQUIRE(not onOffTimer.GetOffTimer().Finished());
  REQUIRE(not onActionCalled);
  REQUIRE(offActionCalled);
  offActionCalled = false;
  UpdateTime(goomTime, NUM_OFF_COUNT);
  REQUIRE(not onActionCalled);
  REQUIRE(not offActionCalled);
  onOffTimer.Update();
  REQUIRE(onOffTimer.GetTimerState() == OnOffTimer::TimerState::ON_TIMER_ACTIVE);
  REQUIRE(not onOffTimer.GetOnTimer().Finished());
  REQUIRE(onOffTimer.GetOffTimer().Finished());
  REQUIRE(onActionCalled);
  REQUIRE(not offActionCalled);

  // Should now have switched to 'on' timer.
  onActionCalled  = false;
  offActionCalled = false;
  UpdateTime(goomTime, NUM_ON_COUNT);
  REQUIRE(not onActionCalled);
  REQUIRE(not offActionCalled);
  onOffTimer.Update();
  REQUIRE(onOffTimer.GetTimerState() == OnOffTimer::TimerState::OFF_TIMER_ACTIVE);
  REQUIRE(onOffTimer.GetOnTimer().Finished());
  REQUIRE(not onOffTimer.GetOffTimer().Finished());
  REQUIRE(not onActionCalled);
  REQUIRE(offActionCalled);

  // Run 'off' timer and make sure 'Stop' works.
  onActionCalled  = false;
  offActionCalled = false;
  onOffTimer.Reset();
  onOffTimer.SetTimerCounts(TIMER_COUNTS);
  REQUIRE(not onActionCalled);
  REQUIRE(not offActionCalled);
  onOffTimer.StartOffTimer();
  REQUIRE(onOffTimer.GetTimerState() == OnOffTimer::TimerState::OFF_TIMER_ACTIVE);
  REQUIRE(onOffTimer.GetOnTimer().Finished());
  REQUIRE(not onOffTimer.GetOffTimer().Finished());
  REQUIRE(not onActionCalled);
  REQUIRE(offActionCalled);
  onActionCalled  = false;
  offActionCalled = false;
  onOffTimer.Stop();
  REQUIRE(onOffTimer.GetTimerState() == OnOffTimer::TimerState::NO_TIMERS_ACTIVE);
  REQUIRE(onOffTimer.GetOnTimer().Finished());
  REQUIRE(onOffTimer.GetOffTimer().Finished());
  REQUIRE(onActionCalled);
  REQUIRE(not offActionCalled);

  // Stop can be called multiple times
  onActionCalled  = false;
  offActionCalled = false;
  onOffTimer.Stop();
  REQUIRE(onOffTimer.GetTimerState() == OnOffTimer::TimerState::NO_TIMERS_ACTIVE);
  REQUIRE(onOffTimer.GetOnTimer().Finished());
  REQUIRE(onOffTimer.GetOffTimer().Finished());
  REQUIRE(not onActionCalled);
  REQUIRE(not offActionCalled);
}

TEST_CASE("OnOffTimer with fails")
{
  auto goomTime                                       = GoomTime{};
  static constexpr auto NUM_ON_COUNT                  = 20U;
  static constexpr auto NUM_ON_COUNT_AFTER_FAILED_OFF = 4U;
  static constexpr auto NUM_OFF_COUNT                 = 10U;
  static constexpr auto NUM_OFF_COUNT_AFTER_FAILED_ON = 2U;
  static constexpr auto TIMER_COUNTS                  = OnOffTimer::TimerCounts{
                       .numOnCount               = NUM_ON_COUNT,
                       .numOnCountAfterFailedOff = NUM_ON_COUNT_AFTER_FAILED_OFF,
                       .numOffCount              = NUM_OFF_COUNT,
                       .numOffCountAfterFailedOn = NUM_OFF_COUNT_AFTER_FAILED_ON,
  };
  auto onOffTimer = OnOffTimer{goomTime, TIMER_COUNTS};

  bool onActionCalled = false;
  bool onActionReturn = true;
  const auto onAction = [&onActionCalled, &onActionReturn]()
  {
    onActionCalled = true;
    return onActionReturn;
  };
  bool offActionCalled = false;
  bool offActionReturn = true;
  const auto offAction = [&offActionCalled, &offActionReturn]()
  {
    offActionCalled = true;
    return offActionReturn;
  };

  onOffTimer.SetActions({.onAction = onAction, .offAction = offAction});

  REQUIRE(onOffTimer.GetTimerState() == OnOffTimer::TimerState::NO_TIMERS_ACTIVE);
  REQUIRE(not onActionCalled);
  REQUIRE(not offActionCalled);
  onOffTimer.StartOnTimer();
  REQUIRE(onOffTimer.GetTimerState() == OnOffTimer::TimerState::ON_TIMER_ACTIVE);
  REQUIRE(onActionCalled);
  REQUIRE(not offActionCalled);
  onActionCalled = false;
  UpdateTime(goomTime, NUM_ON_COUNT);
  REQUIRE(not onActionCalled);
  REQUIRE(not offActionCalled);
  onOffTimer.Update();
  REQUIRE(onOffTimer.GetTimerState() == OnOffTimer::TimerState::OFF_TIMER_ACTIVE);
  REQUIRE(not onActionCalled);
  REQUIRE(offActionCalled);

  // Run 'on' timer with failed 'off' action.
  onOffTimer.Reset();
  onOffTimer.SetTimerCounts(TIMER_COUNTS);
  REQUIRE(onOffTimer.GetTimerState() == OnOffTimer::TimerState::NO_TIMERS_ACTIVE);
  onActionCalled  = false;
  onActionReturn  = true;
  offActionCalled = false;
  offActionReturn = false;
  onOffTimer.StartOnTimer();
  REQUIRE(onOffTimer.GetTimerState() == OnOffTimer::TimerState::ON_TIMER_ACTIVE);
  REQUIRE(onActionCalled);
  REQUIRE(not offActionCalled);
  onActionCalled  = false;
  offActionCalled = false;
  UpdateTime(goomTime, NUM_ON_COUNT);
  onOffTimer.Update();
  REQUIRE(not onActionCalled);
  REQUIRE(offActionCalled);

  // Still running 'on' timer because 'off' action failed.
  REQUIRE(onOffTimer.GetTimerState() == OnOffTimer::TimerState::ON_TIMER_ACTIVE);
  onActionCalled  = false;
  onActionReturn  = true;
  offActionCalled = false;
  offActionReturn = true;
  UpdateTime(goomTime, NUM_ON_COUNT_AFTER_FAILED_OFF);
  onOffTimer.Update();
  REQUIRE(onOffTimer.GetTimerState() == OnOffTimer::TimerState::OFF_TIMER_ACTIVE);
  REQUIRE(not onActionCalled);
  REQUIRE(offActionCalled);

  // Should now have switched to 'off' timer.
  // But with failed 'on' action.
  onActionCalled  = false;
  onActionReturn  = false;
  offActionCalled = false;
  offActionReturn = true;
  UpdateTime(goomTime, NUM_OFF_COUNT);
  onOffTimer.Update();
  REQUIRE(onActionCalled);
  REQUIRE(not offActionCalled);

  // Still running 'off' timer because 'on' action failed.
  onActionCalled  = false;
  onActionReturn  = true;
  offActionCalled = false;
  offActionReturn = true;
  UpdateTime(goomTime, NUM_OFF_COUNT_AFTER_FAILED_ON);
  onOffTimer.Update();
  REQUIRE(onActionCalled);
  REQUIRE(not offActionCalled);
}

// NOLINTEND(readability-function-cognitive-complexity)
// NOLINTEND(bugprone-chained-comparison)

} // namespace GOOM::UNIT_TESTS

// NOLINTEND(cert-err58-cpp): Catch2 3.6.0 issue
