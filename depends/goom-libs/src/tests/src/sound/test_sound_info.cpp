// NOLINTBEGIN(cert-err58-cpp): Catch2 3.6.0 issue

#include <algorithm>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <span>
#include <vector>

import Goom.Control.GoomSoundEvents;
import Goom.Utils.Math.Misc;
import Goom.Utils.GoomTime;
import Goom.Lib.SoundInfo;

namespace GOOM::UNIT_TESTS
{

using Catch::Approx;
using CONTROL::GoomSoundEvents;
using UTILS::GoomTime;
using UTILS::MATH::SMALL_FLOAT;

namespace
{

constexpr auto NUM_SAMPLE_CHANNELS = AudioSamples::NUM_AUDIO_SAMPLES;

constexpr auto X_MIN0          = -0.9F;
constexpr auto X_MAX0          = +0.1F;
constexpr auto X_MIN1          = +0.2F;
constexpr auto X_MAX1          = +0.9F;
constexpr auto EXPECTED_X_MIN0 = X_MIN0;
constexpr auto EXPECTED_X_MAX0 = X_MAX0;
constexpr auto EXPECTED_X_MIN1 = X_MIN1;
constexpr auto EXPECTED_X_MAX1 = X_MAX1;

[[nodiscard]] auto GetAudioData() -> std::vector<float>
{
  auto audioData =
      std::vector<float>(AudioSamples::NUM_AUDIO_SAMPLES * AudioSamples::AUDIO_SAMPLE_LEN);

  auto x     = X_MIN0;
  auto xStep = (X_MAX0 - X_MIN0) / static_cast<float>(AudioSamples::AUDIO_SAMPLE_LEN - 1);
  for (auto i = 0U; i < audioData.size(); i += 2)
  {
    audioData.at(i) = x;
    x += xStep;
  }

  x     = X_MIN1;
  xStep = (X_MAX1 - X_MIN1) / static_cast<float>(AudioSamples::AUDIO_SAMPLE_LEN - 1);
  for (auto i = 1U; i < audioData.size(); i += 2)
  {
    audioData.at(i) = x;
    x += xStep;
  }

  return audioData;
}

} // namespace

// NOLINTBEGIN(bugprone-chained-comparison): Catch2 needs to fix this.
// NOLINTBEGIN(readability-function-cognitive-complexity)
TEST_CASE("Test AudioSamples MinMax")
{
  const auto audioData    = GetAudioData();
  const auto audioSamples = AudioSamples{NUM_SAMPLE_CHANNELS, std::span<const float>(audioData)};

  REQUIRE(AudioSamples::NUM_AUDIO_SAMPLES == 2);
  REQUIRE(AudioSamples::AUDIO_SAMPLE_LEN == 512);
  REQUIRE(audioSamples.GetNumDistinctChannels() == NUM_SAMPLE_CHANNELS);

  REQUIRE(audioSamples.GetSampleMinMax(0).minVal ==
          Approx(AudioSamples::GetPositiveValue(EXPECTED_X_MIN0)));
  REQUIRE(audioSamples.GetSampleMinMax(0).maxVal ==
          Approx(AudioSamples::GetPositiveValue(EXPECTED_X_MAX0)));

  REQUIRE(audioSamples.GetSampleMinMax(1).minVal ==
          Approx(AudioSamples::GetPositiveValue(EXPECTED_X_MIN1)));
  REQUIRE(audioSamples.GetSampleMinMax(1).maxVal ==
          Approx(AudioSamples::GetPositiveValue(EXPECTED_X_MAX1)));
}

TEST_CASE("Test AudioSamples Arrays")
{
  const auto audioData    = GetAudioData();
  const auto audioSamples = AudioSamples{NUM_SAMPLE_CHANNELS, std::span<const float>(audioData)};

  REQUIRE(audioData.at(0) == Approx(X_MIN0));

  REQUIRE(audioSamples.GetSample(0).at(0) ==
          Approx(AudioSamples::GetPositiveValue(EXPECTED_X_MIN0)));

  REQUIRE(audioData.at(audioData.size() - 2) == Approx(X_MAX0).margin(SMALL_FLOAT));

  REQUIRE(audioSamples.GetSample(0).at(AudioSamples::AUDIO_SAMPLE_LEN - 1) ==
          Approx(AudioSamples::GetPositiveValue(EXPECTED_X_MAX0)));

  REQUIRE(audioData.at(1) == Approx(X_MIN1));

  REQUIRE(audioSamples.GetSample(1).at(0) ==
          Approx(AudioSamples::GetPositiveValue(EXPECTED_X_MIN1)));

  REQUIRE(audioData.at(audioData.size() - 1) == Approx(X_MAX1));

  REQUIRE(audioSamples.GetSample(1).at(AudioSamples::AUDIO_SAMPLE_LEN - 1) ==
          Approx(AudioSamples::GetPositiveValue(EXPECTED_X_MAX1)));
}

TEST_CASE("Test SoundInfo ProcessSample Defaults")
{
  auto goomTime        = GoomTime{};
  auto soundInfo       = SoundInfo{};
  auto goomSoundEvents = GoomSoundEvents{goomTime, soundInfo};

  REQUIRE(soundInfo.GetVolume() == Approx(0.0F));
  REQUIRE(goomSoundEvents.GetTimeSinceLastGoom() == 0);
  REQUIRE(goomSoundEvents.GetTimeSinceLastBigGoom() == 0);
  REQUIRE(goomSoundEvents.GetTotalGoomsInCurrentCycle() == 0);

  const auto audioData    = GetAudioData();
  const auto audioSamples = AudioSamples{NUM_SAMPLE_CHANNELS, std::span<const float>(audioData)};

  soundInfo.ProcessSample(audioSamples);
  goomSoundEvents.Update();

  REQUIRE(soundInfo.GetVolume() == Approx(AudioSamples::GetPositiveValue(X_MAX1)));
  REQUIRE(goomSoundEvents.GetTimeSinceLastGoom() == 1);
  REQUIRE(goomSoundEvents.GetTimeSinceLastBigGoom() == 1);
  REQUIRE(goomSoundEvents.GetTotalGoomsInCurrentCycle() == 0);
}

TEST_CASE("Test SoundInfo Volume")
{
  auto soundInfo    = SoundInfo{};
  auto audioData    = GetAudioData();
  auto audioSamples = std::make_unique<AudioSamples>(NUM_SAMPLE_CHANNELS, audioData);

  // First update - defaults
  static constexpr auto ALL_TIMES_MAX = AudioSamples::GetPositiveValue(EXPECTED_X_MAX1);
  static constexpr auto ALL_TIMES_MIN = AudioSamples::GetPositiveValue(EXPECTED_X_MIN0);
  soundInfo.ProcessSample(*audioSamples);
  REQUIRE(soundInfo.GetVolume() == Approx(ALL_TIMES_MAX));
  REQUIRE(soundInfo.GetAllTimesMaxVolume() == Approx(ALL_TIMES_MAX));
  REQUIRE(soundInfo.GetAllTimesMinVolume() == Approx(ALL_TIMES_MIN));

  // Second update - flat line, new volume
  static constexpr auto FLAT_VOL            = 0.1F;
  static constexpr auto EXPECTED_NEW_VOLUME = AudioSamples::GetPositiveValue(FLAT_VOL);

  std::ranges::fill(audioData, FLAT_VOL);
  audioSamples = std::make_unique<AudioSamples>(NUM_SAMPLE_CHANNELS, audioData);
  soundInfo.ProcessSample(*audioSamples);
  REQUIRE(soundInfo.GetVolume() == Approx(EXPECTED_NEW_VOLUME));
  REQUIRE(soundInfo.GetAllTimesMaxVolume() == Approx(ALL_TIMES_MAX));
  REQUIRE(soundInfo.GetAllTimesMinVolume() == Approx(ALL_TIMES_MIN));

  // Third update - new max volume
  static constexpr auto NEW_MAX_VOL        = X_MAX1 + 0.02F;
  static constexpr auto INDEX_WITH_MAX_VOL = 10;
  audioData.at(INDEX_WITH_MAX_VOL)         = NEW_MAX_VOL;
  audioSamples = std::make_unique<AudioSamples>(NUM_SAMPLE_CHANNELS, audioData);
  soundInfo.ProcessSample(*audioSamples);
  REQUIRE(soundInfo.GetVolume() == Approx(AudioSamples::GetPositiveValue(NEW_MAX_VOL)));
  REQUIRE(soundInfo.GetAllTimesMaxVolume() == Approx(AudioSamples::GetPositiveValue(NEW_MAX_VOL)));
  REQUIRE(soundInfo.GetAllTimesMinVolume() == Approx(ALL_TIMES_MIN));

  // Fourth update - negative sound values
  static constexpr auto NEGATIVE_VOL = -0.2F;
  std::ranges::fill(audioData, NEGATIVE_VOL);
  audioSamples = std::make_unique<AudioSamples>(NUM_SAMPLE_CHANNELS, audioData);
  soundInfo.ProcessSample(*audioSamples);
  REQUIRE(soundInfo.GetVolume() == Approx(AudioSamples::GetPositiveValue(NEGATIVE_VOL)));
  REQUIRE(soundInfo.GetAllTimesMaxVolume() == Approx(AudioSamples::GetPositiveValue(NEW_MAX_VOL)));
  REQUIRE(soundInfo.GetAllTimesMinVolume() == Approx(ALL_TIMES_MIN));
}
// NOLINTEND(readability-function-cognitive-complexity)
// NOLINTEND(bugprone-chained-comparison)

} // namespace GOOM::UNIT_TESTS

// NOLINTEND(cert-err58-cpp): Catch2 3.6.0 issue
