// NOLINTBEGIN(cert-err58-cpp): Catch2 3.6.0 issue

#include <catch2/catch_approx.hpp>
#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <vector>

import Goom.Utils.Math.IncrementedValues;
import Goom.Utils.Math.TValues;
import Goom.Utils.Math.Misc;

namespace GOOM::UNIT_TESTS
{

using Catch::Approx;
using UTILS::MATH::IncrementedValue;
using UTILS::MATH::SMALL_FLOAT;
using UTILS::MATH::TValue;

static constexpr auto TWO = 2.0F;

// NOLINTBEGIN(bugprone-chained-comparison): Catch2 needs to fix this.
// NOLINTBEGIN(misc-const-correctness)
// NOLINTBEGIN(readability-function-cognitive-complexity)
TEST_CASE("TValue SINGLE_CYCLE")
{
  static constexpr auto NUM_STEPS = 10U;
  auto tValue                     = TValue{
                          {.stepType = TValue::StepType::SINGLE_CYCLE, .numSteps = NUM_STEPS}
  };

  static constexpr auto STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(tValue.GetStepSize() == Approx(STEP_SIZE));

  // Zero steps
  REQUIRE(tValue() == Approx(TValue::MIN_T_VALUE));
  REQUIRE(tValue.IsInsideBoundary());

  // One step
  tValue.Increment();
  REQUIRE(tValue() == Approx(STEP_SIZE));
  REQUIRE(tValue.IsInsideBoundary());

  while (not tValue.IsStopped())
  {
    tValue.Increment();
  }
  REQUIRE(tValue() >= TValue::MAX_T_VALUE);
  REQUIRE(tValue.IsInsideBoundary());

  // Reset
  static constexpr auto TEST_RESET = 0.2F;
  tValue.Reset(TEST_RESET);
  REQUIRE(tValue() == Approx(TEST_RESET));
  REQUIRE(tValue.IsInsideBoundary());

  tValue.Reset(TValue::MIN_T_VALUE);
  REQUIRE(tValue() == Approx(TValue::MIN_T_VALUE));
  REQUIRE(tValue.IsInsideBoundary());

  tValue.Reset(TValue::MAX_T_VALUE);
  REQUIRE(tValue() == Approx(TValue::MAX_T_VALUE));
  REQUIRE(tValue.IsInsideBoundary());
}

TEST_CASE("TValue CONTINUOUS_REPEATABLE")
{
  static constexpr auto NUM_STEPS = 10U;
  auto tValue                     = TValue{
                          {.stepType = TValue::StepType::CONTINUOUS_REPEATABLE, .numSteps = NUM_STEPS}
  };

  static constexpr auto STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(tValue.GetStepSize() == Approx(STEP_SIZE));

  // Zero steps
  REQUIRE(tValue() == Approx(TValue::MIN_T_VALUE));
  REQUIRE(tValue.IsInsideBoundary());

  // One step
  tValue.Increment();
  REQUIRE(tValue() == Approx(STEP_SIZE));
  REQUIRE(tValue.IsInsideBoundary());

  // Step till MAX_T_VALUE
  for (auto i = 1U; i < NUM_STEPS; ++i)
  {
    tValue.Increment();
  }
  REQUIRE(tValue() == Approx(TValue::MAX_T_VALUE));
  REQUIRE(tValue.HasJustHitEndBoundary());

  // Should go back to start
  tValue.Increment();
  REQUIRE(tValue() == Approx(TValue::MIN_T_VALUE));
  REQUIRE(tValue.HasJustHitStartBoundary());

  // Reset
  static constexpr auto TEST_RESET = 0.2F;
  tValue.Reset(TEST_RESET);
  REQUIRE(tValue() == Approx(TEST_RESET));
  REQUIRE(tValue.IsInsideBoundary());

  tValue.Reset(TValue::MIN_T_VALUE);
  REQUIRE(tValue() == Approx(TValue::MIN_T_VALUE));
  REQUIRE(tValue.IsInsideBoundary());

  tValue.Reset(TValue::MAX_T_VALUE);
  REQUIRE(tValue() == Approx(TValue::MAX_T_VALUE));
  REQUIRE(tValue.HasJustHitEndBoundary());
}

TEST_CASE("TValue CONTINUOUS_REVERSIBLE")
{
  static constexpr auto NUM_STEPS = 10U;
  auto tValue                     = TValue{
                          {.stepType = TValue::StepType::CONTINUOUS_REVERSIBLE, .numSteps = NUM_STEPS}
  };

  static constexpr auto STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(tValue.GetStepSize() == Approx(STEP_SIZE));

  // Zero steps
  REQUIRE(tValue() == Approx(TValue::MIN_T_VALUE));
  REQUIRE(tValue.IsInsideBoundary());

  // One step
  tValue.Increment();
  REQUIRE(tValue() == Approx(STEP_SIZE));
  REQUIRE(tValue.IsInsideBoundary());

  // Step till MAX_T_VALUE
  for (auto i = 1U; i < NUM_STEPS; ++i)
  {
    tValue.Increment();
  }
  REQUIRE(tValue() == Approx(TValue::MAX_T_VALUE));
  REQUIRE(tValue.HasJustHitEndBoundary());

  // Back down
  tValue.Increment();
  REQUIRE(tValue() == Approx(TValue::MAX_T_VALUE - STEP_SIZE));
  REQUIRE(tValue.IsInsideBoundary());

  // Keep going down
  for (auto i = 1U; i < NUM_STEPS; ++i)
  {
    tValue.Increment();
  }
  REQUIRE(tValue() == Approx(TValue::MIN_T_VALUE));
  REQUIRE(tValue.HasJustHitStartBoundary());

  // Back up
  tValue.Increment();
  REQUIRE(tValue() == Approx(STEP_SIZE));
  REQUIRE(tValue.IsInsideBoundary());

  // Reset
  static constexpr auto TEST_RESET = 0.2F;
  tValue.Reset(TEST_RESET);
  REQUIRE(tValue() == Approx(TEST_RESET));
  REQUIRE(tValue.IsInsideBoundary());
  tValue.Increment();
  REQUIRE(tValue() == Approx(TEST_RESET + STEP_SIZE));

  tValue.Reset(TValue::MIN_T_VALUE);
  REQUIRE(tValue() == Approx(TValue::MIN_T_VALUE));
  REQUIRE(tValue.HasJustHitStartBoundary());
  tValue.Increment();
  REQUIRE(tValue() == Approx(TValue::MIN_T_VALUE + STEP_SIZE));

  tValue.Reset(TValue::MAX_T_VALUE);
  REQUIRE(tValue() == Approx(TValue::MAX_T_VALUE));
  REQUIRE(tValue.HasJustHitEndBoundary());
  tValue.Increment();
  REQUIRE(tValue() == Approx(TValue::MAX_T_VALUE - STEP_SIZE));
  REQUIRE(tValue.IsInsideBoundary());
}

namespace
{

void GotoUpToValue(const float t, TValue& tValue)
{
  while (tValue() < t)
  {
    tValue.Increment();
  }
}

void GotoDownToValue(const float t, TValue& tValue)
{
  while (tValue() > t)
  {
    tValue.Increment();
  }
}

void CheckIsDelayedThenGoPastDelay(TValue& tValue, const uint32_t delayTime, const float delayT)
{
  for (auto i = 0U; i < delayTime; ++i)
  {
    INFO("i: " << i << ", delayTime = " << delayTime << ", delayT = " << delayT);

    REQUIRE(tValue.IsDelayed());

    if (i == (delayTime - 1))
    {
      REQUIRE(tValue.DelayJustFinishing());
    }

    tValue.Increment();
    REQUIRE(tValue() == Approx(delayT));
  }
}

} // namespace

TEST_CASE("TValue CONTINUOUS_REPEATABLE with delay")
{
  static constexpr auto NUM_STEPS    = 10U;
  static constexpr auto T_DELAY_TIME = 6U;
  static constexpr auto MID_DELAY_T  = 0.5F;
  auto tValue                        = TValue{
                             {.stepType = TValue::StepType::CONTINUOUS_REPEATABLE, .numSteps = NUM_STEPS},
                             {{.t0 = TValue::MIN_T_VALUE, .delayTime = T_DELAY_TIME},
                              {.t0 = MID_DELAY_T, .delayTime = T_DELAY_TIME},
                              {.t0 = TValue::MAX_T_VALUE, .delayTime = T_DELAY_TIME}}
  };
  REQUIRE((NUM_STEPS % 2) == 0);
  REQUIRE((T_DELAY_TIME % 2) == 0);

  static constexpr auto STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(tValue.GetStepSize() == Approx(STEP_SIZE));

  // Zero steps
  REQUIRE(tValue() == Approx(TValue::MIN_T_VALUE));
  REQUIRE(tValue.IsInsideBoundary());

  // Should be delayed here
  tValue.Increment();
  CheckIsDelayedThenGoPastDelay(tValue, T_DELAY_TIME - 1, TValue::MIN_T_VALUE);
  REQUIRE(not tValue.IsDelayed());
  REQUIRE(tValue.IsInsideBoundary());

  GotoUpToValue(MID_DELAY_T, tValue);
  REQUIRE(tValue() == Approx(MID_DELAY_T));
  REQUIRE(tValue.IsInsideBoundary());

  // Should be delayed here
  tValue.Increment();
  CheckIsDelayedThenGoPastDelay(tValue, T_DELAY_TIME - 1, MID_DELAY_T);
  REQUIRE(not tValue.IsDelayed());
  REQUIRE(tValue.IsInsideBoundary());

  // Normal incrementing up
  auto val = MID_DELAY_T;
  for (auto i = 0U; i < NUM_STEPS / 2; ++i)
  {
    tValue.Increment();
    val += STEP_SIZE;
    INFO("Step = " << i << " val = " << val);
    REQUIRE(tValue() == Approx(val));
  }
  REQUIRE(tValue() == Approx(TValue::MAX_T_VALUE));
  REQUIRE(tValue.HasJustHitEndBoundary());

  // Should be delayed here
  tValue.Increment();
  CheckIsDelayedThenGoPastDelay(tValue, T_DELAY_TIME - 1, TValue::MAX_T_VALUE);
  REQUIRE(not tValue.IsDelayed());
  REQUIRE(tValue.HasJustHitEndBoundary());

  // Back to the start
  tValue.Increment();
  REQUIRE(tValue() == Approx(TValue::MIN_T_VALUE));
  REQUIRE(tValue.HasJustHitStartBoundary());
}

TEST_CASE("TValue CONTINUOUS_REVERSIBLE with delay")
{
  static constexpr auto NUM_STEPS    = 10U;
  static constexpr auto T_DELAY_TIME = 6U;
  static constexpr auto MID_DELAY_T  = 0.5F;
  auto tValue                        = TValue{
                             {.stepType = TValue::StepType::CONTINUOUS_REVERSIBLE, .numSteps = NUM_STEPS},
                             {{.t0 = TValue::MIN_T_VALUE, .delayTime = T_DELAY_TIME},
                              {.t0 = MID_DELAY_T, .delayTime = T_DELAY_TIME},
                              {.t0 = TValue::MAX_T_VALUE, .delayTime = T_DELAY_TIME}}
  };
  REQUIRE((NUM_STEPS % 2) == 0);
  REQUIRE((T_DELAY_TIME % 2) == 0);
  REQUIRE(tValue() == Approx(TValue::MIN_T_VALUE));
  REQUIRE(tValue.IsInsideBoundary());

  static constexpr auto STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(tValue.GetStepSize() == Approx(STEP_SIZE));
  REQUIRE(tValue.GetCurrentStep() > 0.0F);

  // Should be delayed here
  tValue.Increment();
  CheckIsDelayedThenGoPastDelay(tValue, T_DELAY_TIME - 1, TValue::MIN_T_VALUE);
  REQUIRE(not tValue.IsDelayed());
  REQUIRE(tValue.IsInsideBoundary());

  GotoUpToValue(MID_DELAY_T, tValue);
  REQUIRE(tValue() == Approx(MID_DELAY_T));
  REQUIRE(tValue.IsInsideBoundary());

  // Should be delayed here
  tValue.Increment();
  CheckIsDelayedThenGoPastDelay(tValue, T_DELAY_TIME - 1, MID_DELAY_T);
  REQUIRE(not tValue.IsDelayed());

  // Normal incrementing
  auto val = MID_DELAY_T;
  for (auto i = 0U; i < NUM_STEPS / 2; ++i)
  {
    tValue.Increment();
    val += STEP_SIZE;
    REQUIRE(tValue() == Approx(val));
  }
  REQUIRE(tValue() == Approx(TValue::MAX_T_VALUE));
  REQUIRE(tValue.HasJustHitEndBoundary());

  // Should be delayed here
  tValue.Increment();
  CheckIsDelayedThenGoPastDelay(tValue, T_DELAY_TIME - 1, TValue::MAX_T_VALUE);
  REQUIRE(not tValue.IsDelayed());

  // Back down
  tValue.Increment();
  REQUIRE(tValue.GetCurrentStep() < 0.0F);
  REQUIRE(tValue() == Approx(TValue::MAX_T_VALUE - STEP_SIZE));
  REQUIRE(tValue.IsInsideBoundary());

  GotoDownToValue(MID_DELAY_T, tValue);
  REQUIRE(tValue() == Approx(MID_DELAY_T));
  REQUIRE(tValue.IsInsideBoundary());

  // Should be delayed here
  tValue.Increment();
  CheckIsDelayedThenGoPastDelay(tValue, T_DELAY_TIME - 1, MID_DELAY_T);
  REQUIRE(not tValue.IsDelayed());
  REQUIRE(tValue.IsInsideBoundary());

  // Normal incrementing - going down
  val = MID_DELAY_T;
  for (auto i = 0U; i < NUM_STEPS / 2; ++i)
  {
    tValue.Increment();
    val -= STEP_SIZE;
    REQUIRE(tValue() == Approx(val).margin(SMALL_FLOAT));
  }
  REQUIRE(tValue() == Approx(TValue::MIN_T_VALUE));
  REQUIRE(tValue.GetCurrentStep() < 0.0F);
  REQUIRE(tValue.HasJustHitStartBoundary());

  // Should be delayed here
  tValue.Increment();
  REQUIRE(tValue() == Approx(TValue::MIN_T_VALUE));
  REQUIRE(tValue.HasJustHitStartBoundary());
  REQUIRE(tValue.GetCurrentStep() < 0.0F);
  CheckIsDelayedThenGoPastDelay(tValue, T_DELAY_TIME - 1, TValue::MIN_T_VALUE);
  REQUIRE(not tValue.IsDelayed());
  REQUIRE(tValue.HasJustHitStartBoundary());

  // Normal incrementing - going back up
  tValue.Increment();
  REQUIRE(tValue.GetCurrentStep() > 0.0F);
  REQUIRE(tValue() == Approx(STEP_SIZE));
  REQUIRE(tValue.IsInsideBoundary());
}

TEST_CASE("IncrementedValue SINGLE_CYCLE")
{
  static constexpr auto MIN_VALUE = -5.0F;
  static constexpr auto MAX_VALUE = +15.0F;
  static constexpr auto NUM_STEPS = 10U;
  auto value =
      IncrementedValue<float>{MIN_VALUE, MAX_VALUE, TValue::StepType::SINGLE_CYCLE, NUM_STEPS};

  static constexpr auto STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(value.GetT().GetStepSize() == Approx(STEP_SIZE));

  // Zero steps
  REQUIRE(value() == Approx(MIN_VALUE));
  REQUIRE(value.GetT().IsInsideBoundary());

  // One step
  value.Increment();
  REQUIRE(value() == Approx(MIN_VALUE + (STEP_SIZE * (MAX_VALUE - MIN_VALUE))));
  REQUIRE(value.GetT().IsInsideBoundary());

  // Peek
  const auto nextValue = value.PeekNext();
  REQUIRE(nextValue == Approx(MIN_VALUE + ((TWO * STEP_SIZE) * (MAX_VALUE - MIN_VALUE))));
  REQUIRE(value() == Approx(MIN_VALUE + (STEP_SIZE * (MAX_VALUE - MIN_VALUE))));
  REQUIRE(value.GetT().IsInsideBoundary());

  for (auto i = 1U; i < NUM_STEPS; ++i)
  {
    value.Increment();
  }
  REQUIRE(value() == Approx(MAX_VALUE).margin(SMALL_FLOAT));
  REQUIRE(value.GetT().IsInsideBoundary());
}

TEST_CASE("IncrementedValue CONTINUOUS_REPEATABLE")
{
  static constexpr auto MIN_VALUE = -5.0F;
  static constexpr auto MAX_VALUE = +15.0F;
  static constexpr auto NUM_STEPS = 10U;
  auto value                      = IncrementedValue<float>{
      MIN_VALUE, MAX_VALUE, TValue::StepType::CONTINUOUS_REPEATABLE, NUM_STEPS};

  static constexpr auto STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(value.GetT().GetStepSize() == Approx(STEP_SIZE));

  // Zero steps
  REQUIRE(value() == Approx(MIN_VALUE));
  REQUIRE(value.GetT().IsInsideBoundary());

  // One step
  value.Increment();
  REQUIRE(value() == Approx(MIN_VALUE + (STEP_SIZE * (MAX_VALUE - MIN_VALUE))));
  REQUIRE(value.GetT().IsInsideBoundary());

  // Peek
  const auto nextValue = value.PeekNext();
  REQUIRE(nextValue == Approx(MIN_VALUE + ((TWO * STEP_SIZE) * (MAX_VALUE - MIN_VALUE))));
  REQUIRE(value() == Approx(MIN_VALUE + (STEP_SIZE * (MAX_VALUE - MIN_VALUE))));
  REQUIRE(value.GetT().IsInsideBoundary());

  // Step till MAX_VALUE
  for (auto i = 1U; i < NUM_STEPS; ++i)
  {
    value.Increment();
  }
  REQUIRE(value() == Approx(MAX_VALUE));
  REQUIRE(value.GetT().HasJustHitEndBoundary());

  // Should go back to start
  value.Increment();
  REQUIRE(value() == Approx(MIN_VALUE));
  REQUIRE(value.GetT().HasJustHitStartBoundary());
}

TEST_CASE("IncrementedValue CONTINUOUS_REVERSIBLE")
{
  static constexpr auto MIN_VALUE       = -5.0F;
  static constexpr auto MAX_VALUE       = +15.0F;
  static constexpr auto INBETWEEN_VALUE = +10.0F;
  static constexpr auto NUM_STEPS       = 10U;
  auto value                            = IncrementedValue<float>{
      MIN_VALUE, MAX_VALUE, TValue::StepType::CONTINUOUS_REVERSIBLE, NUM_STEPS};

  static constexpr auto STEP_SIZE = 1.0F / static_cast<float>(NUM_STEPS);
  REQUIRE(value.GetT().GetStepSize() == Approx(STEP_SIZE));

  // Zero steps
  REQUIRE(value.GetT()() == Approx(TValue::MIN_T_VALUE));
  REQUIRE(value() == Approx(MIN_VALUE));
  REQUIRE(value.GetT().IsInsideBoundary());

  // One step
  value.Increment();
  REQUIRE(value.GetT()() == Approx(STEP_SIZE));
  REQUIRE(value() == Approx(MIN_VALUE + (STEP_SIZE * (MAX_VALUE - MIN_VALUE))));
  REQUIRE(value.GetT().IsInsideBoundary());

  // Peek
  const auto nextValue = value.PeekNext();
  REQUIRE(nextValue == Approx(MIN_VALUE + ((TWO * STEP_SIZE) * (MAX_VALUE - MIN_VALUE))));
  REQUIRE(value() == Approx(MIN_VALUE + (STEP_SIZE * (MAX_VALUE - MIN_VALUE))));
  REQUIRE(value.GetT().IsInsideBoundary());

  // Step till MAX_VALUE
  UNSCOPED_INFO("value.GetT()() = " << value.GetT()());
  for (auto i = 1U; i < NUM_STEPS; ++i)
  {
    value.Increment();
    UNSCOPED_INFO("value.GetT()() = " << value.GetT()());
  }
  UNSCOPED_INFO("value.GetT()() = " << value.GetT()());
  REQUIRE(value.GetT().HasJustHitEndBoundary());
  REQUIRE(value.GetT()() == Approx(TValue::MAX_T_VALUE));
  REQUIRE(value() == Approx(MAX_VALUE));

  // Back down
  value.Increment();
  REQUIRE(value() ==
          Approx(MIN_VALUE + ((TValue::MAX_T_VALUE - STEP_SIZE) * (MAX_VALUE - MIN_VALUE))));
  REQUIRE(value.GetT().IsInsideBoundary());

  // Keep going down
  for (auto i = 1U; i < NUM_STEPS; ++i)
  {
    value.Increment();
  }
  REQUIRE(value.GetT().HasJustHitStartBoundary());
  REQUIRE(value() == Approx(MIN_VALUE).margin(SMALL_FLOAT));

  // Back up
  value.Increment();
  REQUIRE(value.GetT().IsInsideBoundary());
  REQUIRE(value() == Approx(MIN_VALUE + (STEP_SIZE * (MAX_VALUE - MIN_VALUE))));

  // Reset value
  value.ResetCurrentValue(INBETWEEN_VALUE);
  REQUIRE(value() == Approx(INBETWEEN_VALUE));
  REQUIRE(value.GetT().IsInsideBoundary());

  value.ResetCurrentValue(MAX_VALUE + 1.0F);
  REQUIRE(value() == Approx(MAX_VALUE));
  REQUIRE(value.GetT().HasJustHitEndBoundary());

  value.ResetCurrentValue(INBETWEEN_VALUE);
  REQUIRE(value() == Approx(INBETWEEN_VALUE));
  REQUIRE(value.GetT().IsInsideBoundary());

  value.SetValues(MIN_VALUE + 1.0F, MAX_VALUE - 1.0F);
  REQUIRE(value() == Approx(INBETWEEN_VALUE));
  REQUIRE(value.GetT().IsInsideBoundary());

  value.SetValues(INBETWEEN_VALUE + 1.0F, MAX_VALUE);
  REQUIRE(value() == Approx(INBETWEEN_VALUE + 1.0F));
  REQUIRE(value.GetT().HasJustHitStartBoundary());
}
// NOLINTEND(readability-function-cognitive-complexity)
// NOLINTEND(misc-const-correctness)
// NOLINTEND(bugprone-chained-comparison)

} // namespace GOOM::UNIT_TESTS

// NOLINTEND(cert-err58-cpp): Catch2 3.6.0 issue
