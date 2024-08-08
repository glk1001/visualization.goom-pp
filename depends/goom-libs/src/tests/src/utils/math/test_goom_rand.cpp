// NOLINTBEGIN(cert-err58-cpp): Catch2 3.6.0 issue

#include <algorithm>
#include <array>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <cstdint>
#include <format>
#include <functional>
#include <map>

import Goom.Tests.Utils.Math.RandHelper;
import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.GoomTypes;

namespace GOOM::UNIT_TESTS
{

using Catch::Approx;
using GOOM::UnderlyingEnumType;
using UTILS::EnumToString;
using UTILS::NUM;
using UTILS::MATH::ConditionalWeights;
using UTILS::MATH::GetWeightedSample;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;
using UTILS::MATH::Weights;

namespace
{

enum class Events : UnderlyingEnumType
{
  EVENT1,
  EVENT2,
  EVENT3,
  EVENT4,
};
using EventCounts             = std::array<uint32_t, NUM<Events>>;
constexpr auto PREVIOUS_EVENT = Events::EVENT3;

constexpr size_t NUM_LOOPS     = 10000000;
constexpr double DBL_NUM_LOOPS = NUM_LOOPS;

template<typename E>
[[nodiscard]] auto GetWeightedCounts(const size_t numLoops,
                                     const std::function<E()>& getRandomWeighted) -> EventCounts
{
  EventCounts eventCounts{};
  for (auto i = 0U; i < numLoops; ++i)
  {
    const auto event = getRandomWeighted();
    ++eventCounts.at(static_cast<size_t>(event));
  }

  return eventCounts;
}

[[nodiscard]] auto GetConditionalWeightedCounts(
    const size_t numLoops, const ConditionalWeights<Events>& weights) -> EventCounts
{
  EventCounts eventCounts{};
  for (auto i = 0U; i < numLoops; ++i)
  {
    const auto event = weights.GetRandomWeighted(PREVIOUS_EVENT);
    ++eventCounts.at(static_cast<size_t>(event));
  }

  return eventCounts;
}

constexpr auto GOOM_RAND = GoomRand{};

template<NumberRange numberRange> // NOLINT(readability-identifier-naming)
auto GetRangeCountResults(const size_t numLoops) -> CountResults<decltype(numberRange.min)>
{
  const auto randInRange = [](const auto n0, const auto nRange)
  { return GOOM_RAND.GetRandInRange(NumberRange<decltype(numberRange.min)>{n0, n0 + nRange - 1}); };

  return GetCountResults(numLoops, numberRange.min, numberRange.rangePlus1, randInRange);
}

} // namespace

// NOLINTBEGIN(bugprone-chained-comparison): Catch2 needs to fix this.
// NOLINTBEGIN(readability-function-cognitive-complexity)

TEST_CASE("NumberRange")
{
  static constexpr auto N_MIN1 = 999U;
  static constexpr auto N_MAX1 = 10001U;
  static constexpr auto RANGE  = NumberRange{N_MIN1, N_MAX1};

  STATIC_REQUIRE(RANGE.min == N_MIN1);
  STATIC_REQUIRE(RANGE.max == N_MAX1);
  STATIC_REQUIRE(RANGE.range == N_MAX1 - N_MIN1);
}

TEST_CASE("NumberRange min max get random")
{
  // After a big enough loop, a good random distribution should have
  // covered the entire range: nMin <= n <= nMax
  static constexpr auto NUM_RANGE_LOOPS                  = 100'000U;
  static constexpr auto ACCEPTABLE_OUT_OF_UNIFORM_MARGIN = 300U;

  static constexpr auto N_MIN1   = 999U;
  static constexpr auto N_MAX1   = 10001U;
  static constexpr auto N_RANGE1 = NumberRange{N_MIN1, N_MAX1};
  const auto countsResults1      = GetRangeCountResults<N_RANGE1>(NUM_RANGE_LOOPS);
  REQUIRE(countsResults1.min == N_MIN1);
  REQUIRE(countsResults1.max == N_MAX1);
  REQUIRE(countsResults1.numCounts == (N_RANGE1.range + 1));
  UNSCOPED_INFO(std::format(
      "minCount = {}, minCountAt = {}", countsResults1.minCount, countsResults1.minCountAt));
  UNSCOPED_INFO(std::format(
      "maxCount = {}, maxCountAt = {}", countsResults1.maxCount, countsResults1.maxCountAt));
  REQUIRE(countsResults1.maxCount - countsResults1.minCount < ACCEPTABLE_OUT_OF_UNIFORM_MARGIN);

  static constexpr auto N_MIN2   = 0U;
  static constexpr auto N_MAX2   = 120U;
  static constexpr auto N_RANGE2 = NumberRange{N_MIN2, N_MAX2};
  const auto countsResults2      = GetRangeCountResults<N_RANGE2>(NUM_RANGE_LOOPS);
  REQUIRE(countsResults2.min == N_MIN2);
  REQUIRE(countsResults2.max == N_MAX2);
  REQUIRE(countsResults2.numCounts == (N_RANGE2.range + 1));
  UNSCOPED_INFO(std::format(
      "minCount = {}, minCountAt = {}", countsResults2.minCount, countsResults2.minCountAt));
  UNSCOPED_INFO(std::format(
      "maxCount = {}, maxCountAt = {}", countsResults2.maxCount, countsResults2.maxCountAt));
  REQUIRE(countsResults2.maxCount - countsResults2.minCount < ACCEPTABLE_OUT_OF_UNIFORM_MARGIN);

  REQUIRE(5U == GOOM_RAND.GetRandInRange(NumberRange{5U, 5U}));
}

TEST_CASE("Shuffle")
{
  static constexpr auto TEST_VEC = std::array{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  auto vec = TEST_VEC;
  GOOM_RAND.Shuffle(vec);
  REQUIRE(vec != TEST_VEC);
  std::ranges::sort(vec);
  REQUIRE(vec == TEST_VEC);
}

TEST_CASE("Weighted Events")
{
  static constexpr auto EVENT1_WEIGHT = 05.0F;
  static constexpr auto EVENT2_WEIGHT = 02.0F;
  static constexpr auto EVENT3_WEIGHT = 10.0F;
  static constexpr auto EVENT4_WEIGHT = 06.0F;
  const auto weightPairs              = Weights<Events>::EventWeightPairs{
                   {.key = Events::EVENT1, .weight = EVENT1_WEIGHT},
                   {.key = Events::EVENT2, .weight = EVENT2_WEIGHT},
                   {.key = Events::EVENT3, .weight = EVENT3_WEIGHT},
                   {.key = Events::EVENT4, .weight = EVENT4_WEIGHT},
  };
  const auto weightedEvents = Weights<Events>{GOOM_RAND, weightPairs};
  static constexpr auto EXPECTED_SUM =
      EVENT1_WEIGHT + EVENT2_WEIGHT + EVENT3_WEIGHT + EVENT4_WEIGHT;

  SECTION("Unconditional weights")
  {
    const auto sumOfWeights = static_cast<double>(weightedEvents.GetSumOfWeights());
    REQUIRE(sumOfWeights == Approx(EXPECTED_SUM));

    const auto getRandomWeighted = [&weightedEvents]()
    { return weightedEvents.GetRandomWeighted(); };
    const auto eventCounts = GetWeightedCounts<Events>(NUM_LOOPS, getRandomWeighted);

    for (auto i = 0U; i < NUM<Events>; ++i)
    {
      const auto fEventCount = static_cast<double>(eventCounts.at(i));
      const auto fEventWeight =
          static_cast<double>(weightedEvents.GetWeight(static_cast<Events>(i)));

      const auto countFraction = fEventCount / DBL_NUM_LOOPS;
      const auto eventFraction = fEventWeight / sumOfWeights;

      UNSCOPED_INFO(std::format(
          "i = {}, countFraction = {}, eventFraction = {}", i, countFraction, eventFraction));
      static constexpr auto CLOSE_ENOUGH = 0.005;
      REQUIRE(countFraction == Approx(eventFraction).epsilon(CLOSE_ENOUGH));
    }
  }

  SECTION("Weighted Sample")
  {
    static constexpr auto SAMPLE_EVENT1_WEIGHT = 05.0F;
    static constexpr auto SAMPLE_EVENT2_WEIGHT = 02.0F;
    static constexpr auto SAMPLE_EVENT3_WEIGHT = 10.0F;
    static constexpr auto SAMPLE_EVENT4_WEIGHT = 06.0F;
    const auto weightedSamplePairs             = Weights<Events>::EventWeightPairs{
                    {.key = Events::EVENT1, .weight = SAMPLE_EVENT1_WEIGHT},
                    {.key = Events::EVENT2, .weight = SAMPLE_EVENT2_WEIGHT},
                    {.key = Events::EVENT3, .weight = SAMPLE_EVENT3_WEIGHT},
                    {.key = Events::EVENT4, .weight = SAMPLE_EVENT4_WEIGHT},
    };
    const auto weightedSampleEvents = Weights<Events>{GOOM_RAND, weightedSamplePairs};
    const auto sumOfWeights         = static_cast<double>(weightedSampleEvents.GetSumOfWeights());

    static constexpr auto SAMPLE_SIZE = NUM<Events>;
    auto eventsSample                 = std::vector<Events>(SAMPLE_SIZE);

    // TODO(glk): Not sure how to test positions 1,2,3. '0' seems OK but
    //            the others seem quite difficult.
    auto weightedEventPositions0 = EventCounts{0};
    for (auto i = 0U; i < NUM_LOOPS; ++i)
    {
      GetWeightedSample(weightedSampleEvents, SAMPLE_SIZE, eventsSample);
      REQUIRE(eventsSample.size() == SAMPLE_SIZE);
      ++weightedEventPositions0.at(static_cast<size_t>(eventsSample.at(0)));
    }

    const auto& weights = weightedSampleEvents.GetWeightArray();

    for (auto i = 0U; i < NUM<Events>; ++i)
    {
      const auto fEventCount  = static_cast<double>(weightedEventPositions0.at(i));
      const auto fEventWeight = static_cast<double>(weights.at(i));

      const auto countFraction = fEventCount / DBL_NUM_LOOPS;
      const auto eventFraction = fEventWeight / sumOfWeights;

      UNSCOPED_INFO(std::format("i = {}, fEventWeight = {}, sumOfWeights = {}, eventFraction = {}",
                                i,
                                fEventWeight,
                                sumOfWeights,
                                eventFraction));
      UNSCOPED_INFO(std::format("i = {}, fEventCount = {}, DBL_NUM_LOOPS = {}, countFraction = {}",
                                i,
                                fEventCount,
                                DBL_NUM_LOOPS,
                                countFraction));
      static constexpr auto CLOSE_ENOUGH = 0.005;
      REQUIRE(countFraction == Approx(eventFraction).epsilon(CLOSE_ENOUGH));
    }
  }

  SECTION("Conditional weights")
  {
    static const auto s_EVENT3_WEIGHT_MULTIPLIERS = std::map<Events, float>{
        {Events::EVENT1, 1.0F},
        {Events::EVENT2, 0.0F},
        {Events::EVENT3, 2.0F},
        {Events::EVENT4, 1.0F},
    };
    static const auto s_WEIGHT_MULTIPLIERS = ConditionalWeights<Events>::EventWeightMultiplierPairs{
        {.key = PREVIOUS_EVENT, .weightMultipliers = s_EVENT3_WEIGHT_MULTIPLIERS}
    };
    const auto conditionalWeightedEvents =
        ConditionalWeights<Events>{GOOM_RAND, weightPairs, s_WEIGHT_MULTIPLIERS};
    const auto conditionalSumOfWeights =
        static_cast<double>(conditionalWeightedEvents.GetSumOfWeights(PREVIOUS_EVENT));
    static constexpr auto EXPECTED_SUM_FOR_GIVEN = 5.0 + (2.0 * 10.0) + 6.0;
    REQUIRE(conditionalSumOfWeights == Approx(EXPECTED_SUM_FOR_GIVEN));

    const auto conditionalEventCounts =
        GetConditionalWeightedCounts(NUM_LOOPS, conditionalWeightedEvents);

    for (auto i = 0U; i < NUM<Events>; ++i)
    {
      const auto fConditionalEventCount   = static_cast<double>(conditionalEventCounts.at(i));
      const auto conditionalCountFraction = fConditionalEventCount / DBL_NUM_LOOPS;

      UNSCOPED_INFO(std::format("i:{}, fConditionalEventCount = {}", i, fConditionalEventCount));
      UNSCOPED_INFO(std::format("i:{}, NUM_LOOPS = {}", i, NUM_LOOPS));
      UNSCOPED_INFO(
          std::format("i:{}, conditionalCountFraction = {}", i, conditionalCountFraction));

      const auto fConditionalEventWeight  = static_cast<double>(conditionalWeightedEvents.GetWeight(
          {.previousEvent = PREVIOUS_EVENT, .event = static_cast<Events>(i)}));
      const auto conditionalEventFraction = fConditionalEventWeight / conditionalSumOfWeights;
      UNSCOPED_INFO(std::format("i:{}, fConditionalEventWeight({}) = {}",
                                i,
                                EnumToString(PREVIOUS_EVENT),
                                fConditionalEventWeight));
      UNSCOPED_INFO(std::format("i:{}, conditionalSumOfWeights({}) = {}",
                                i,
                                EnumToString(PREVIOUS_EVENT),
                                conditionalSumOfWeights));
      UNSCOPED_INFO(
          std::format("i:{}, conditionalEventFraction = {}", i, conditionalEventFraction));

      static constexpr auto CLOSE_ENOUGH = 0.005;
      REQUIRE(conditionalCountFraction == Approx(conditionalEventFraction).epsilon(CLOSE_ENOUGH));
    }
  }

  SECTION("DisallowEventsSameAsGiven = true")
  {
    const auto conditionalWeightedEvents = ConditionalWeights<Events>{GOOM_RAND, weightPairs, true};

    const auto conditionalEventCounts =
        GetConditionalWeightedCounts(NUM_LOOPS, conditionalWeightedEvents);

    REQUIRE(conditionalEventCounts.at(static_cast<size_t>(PREVIOUS_EVENT)) == 0);
  }
}
// NOLINTEND(readability-function-cognitive-complexity)

// NOLINTBEGIN(readability-function-cognitive-complexity)
TEST_CASE("Weighted Events Corner Cases")
{
  const auto weightedEvents = Weights<Events>{
      GOOM_RAND,
      {
        {.key = Events::EVENT1, .weight = 0.0F},
        {.key = Events::EVENT2, .weight = 1.0F},
        {.key = Events::EVENT3, .weight = 0.0F},
        {.key = Events::EVENT4, .weight = 0.0F},
        }
  };

  SECTION("Unconditional weights")
  {
    const auto sumOfWeights = static_cast<double>(weightedEvents.GetSumOfWeights());
    REQUIRE(sumOfWeights == Approx(1.0F));

    const auto getRandomWeighted = [&weightedEvents]()
    { return weightedEvents.GetRandomWeighted(); };
    const auto eventCounts = GetWeightedCounts<Events>(NUM_LOOPS, getRandomWeighted);

    for (auto i = 0U; i < NUM<Events>; ++i)
    {
      if (static_cast<Events>(i) != Events::EVENT2)
      {
        REQUIRE(eventCounts.at(i) == 0);
      }
      else
      {
        REQUIRE(eventCounts.at(i) == NUM_LOOPS);
      }
    }
  }
}
// NOLINTEND(readability-function-cognitive-complexity)
// NOLINTEND(bugprone-chained-comparison)

/*** Catch2 can't catch 'assert' calls.
TEST_CASE("Weighted Events Exceptions")
{
  const GoomRand goomRand{};

  REQUIRE_THROWS_WITH(Weights<Events>(goomRand, {}).GetNumElements() > 0,
                      "Sum of weights is zero.");

  const typename Weights<Events>::EventWeightPairs weightPairs1 = {
      {Events::EVENT1, 0.0F},
      {Events::EVENT2, 0.0F},
      {Events::EVENT3, 0.0F},
      {Events::EVENT4, 0.0F},
  };
  REQUIRE_THROWS_WITH(Weights<Events>(goomRand, weightPairs1).GetNumElements() > 0,
                      "Sum of weights is zero.");

  const typename Weights<Events>::EventWeightPairs weightPairs2 = {
      {Events::EVENT1, 0.0F},
      {Events::EVENT3, 0.0F},
  };
  REQUIRE_THROWS_WITH(Weights<Events>(goomRand, weightPairs2).GetNumElements() > 0,
                      "Sum of weights is zero.");
}

TEST_CASE("Conditional Weighted Events Exceptions")
{
  const GoomRand goomRand{};

  const typename Weights<Events>::EventWeightPairs weightPairs1 = {
      {Events::EVENT1, 05.0F},
      {Events::EVENT2, 02.0F},
      {Events::EVENT3, 10.0F},
      {Events::EVENT4, 06.0F},
  };
  const std::map<Events, float> event3WeightMultipliers1 = {
      {Events::EVENT1, 0.0F},
      {Events::EVENT2, 0.0F},
      {Events::EVENT3, 0.0F},
      {Events::EVENT4, 0.0F},
  };
  static constexpr Events GIVEN_EVENT = Events::EVENT3;
  const ConditionalWeights<Events>::EventWeightMultiplierPairs weightMultipliers1 = {
      {GIVEN_EVENT, event3WeightMultipliers1}
  };

  REQUIRE_THROWS_WITH(ConditionalWeights<Events>(goomRand, weightPairs1, weightMultipliers1)
                              .GetSumOfWeights(GIVEN_EVENT) > 0,
                      "Sum of weights is zero.");

  const typename Weights<Events>::EventWeightPairs weightPairs2 = {
      {Events::EVENT1, 05.0F},
      {Events::EVENT3, 10.0F},
  };
  const std::map<Events, float> event3WeightMultipliers2 = {
      {Events::EVENT1, 0.0F},
      {Events::EVENT2, 1.0F},
      {Events::EVENT3, 0.0F},
      {Events::EVENT4, 1.0F},
  };
  const ConditionalWeights<Events>::EventWeightMultiplierPairs weightMultipliers2 = {
      {GIVEN_EVENT, event3WeightMultipliers2}
  };

  REQUIRE_THROWS_WITH(ConditionalWeights<Events>(goomRand, weightPairs2, weightMultipliers2)
                              .GetSumOfWeights(GIVEN_EVENT) == 0.0F,
                      "Sum of weights is zero.");
}
***/

} // namespace GOOM::UNIT_TESTS

// NOLINTEND(cert-err58-cpp): Catch2 3.6.0 issue
