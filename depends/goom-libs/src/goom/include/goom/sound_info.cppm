module;

#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <span>

export module Goom.Lib.SoundInfo;

import Goom.Lib.AssertUtils;

export namespace GOOM
{

class AudioSamples
{
public:
  static constexpr size_t NUM_AUDIO_SAMPLES = 2;
  static constexpr size_t AUDIO_SAMPLE_LEN  = 512;
  [[nodiscard]] static constexpr auto GetPositiveValue(float audioValue) -> float;

  // AudioSample object: numSampleChannels = 1 or 2.
  //   If numSampleChannels = 1, then the first AUDIO_SAMPLE_LEN values of 'rawAudioData'
  //   are used for the two channels.
  //   If numSampleChannels = 2, then the 'rawAudioData' must interleave the two channels,
  //   one after the other. So 'rawAudioData[0]' is channel 0, 'rawAudioData[1]' is
  //   channel 1, 'rawAudioData[2]' is channel 0, 'rawAudioData[3]' is channel 1, etc.
  AudioSamples(size_t numSampleChannels, std::span<const float> rawAudioData);

  [[nodiscard]] auto GetNumDistinctChannels() const -> size_t;

  struct MinMaxValues
  {
    float minVal;
    float maxVal;
  };
  using SampleArray = std::array<float, AUDIO_SAMPLE_LEN>;
  [[nodiscard]] auto GetSample(size_t channelIndex) const -> const SampleArray&;
  [[nodiscard]] auto GetSampleMinMax(size_t channelIndex) const -> const MinMaxValues&;
  [[nodiscard]] auto GetSampleOverallMinMax() const -> const MinMaxValues&;

private:
  size_t m_numDistinctChannels;
  std::array<SampleArray, NUM_AUDIO_SAMPLES> m_sampleArrays;
  std::array<MinMaxValues, NUM_AUDIO_SAMPLES> m_minMaxSampleValues{
      GetMinMaxSampleValues(m_sampleArrays)};
  static_assert(2 == NUM_AUDIO_SAMPLES);
  MinMaxValues m_overallMinMaxSampleValues{
      .minVal = std::min(m_minMaxSampleValues[0].minVal, m_minMaxSampleValues[1].minVal),
      .maxVal = std::max(m_minMaxSampleValues[0].maxVal, m_minMaxSampleValues[1].maxVal)};
  [[nodiscard]] static auto GetSampleArrays(std::span<const float> rawAudioData)
      -> std::array<SampleArray, NUM_AUDIO_SAMPLES>;
  [[nodiscard]] static auto GetMinMaxSampleValues(
      const std::array<SampleArray, NUM_AUDIO_SAMPLES>& sampleArrays)
      -> std::array<MinMaxValues, NUM_AUDIO_SAMPLES>;
};

class SoundInfo
{
public:
  SoundInfo() noexcept = default;

  void ProcessSample(const AudioSamples& samples);

  // Volume of the sound [0..1]
  [[nodiscard]] auto GetVolume() const -> float;

  // Speed of the sound [0..1]
  [[nodiscard]] auto GetSpeed() const -> float;
  static constexpr auto SPEED_MIDPOINT = 0.5F;

  // Acceleration of the sound [0..1]
  [[nodiscard]] auto GetAcceleration() const -> float;
  static constexpr auto ACCELERATION_MIDPOINT = 0.5F;

  [[nodiscard]] auto GetAllTimesMaxVolume() const -> float;
  [[nodiscard]] auto GetAllTimesMinVolume() const -> float;

private:
  float m_volume       = 0.0F;
  float m_acceleration = 0.0F;
  float m_speed        = 0.0F;

  float m_allTimesMaxVolume = std::numeric_limits<float>::min();
  float m_allTimesMinVolume = std::numeric_limits<float>::max();

  void UpdateVolume(const AudioSamples& samples);
  void UpdateSpeed(float prevVolume);
  void UpdateAcceleration(float prevSpeed);
};

} // namespace GOOM

namespace GOOM
{

constexpr auto AudioSamples::GetPositiveValue(const float audioValue) -> float
{
  constexpr auto MIN_AUDIO_VALUE = -1.0F;
  constexpr auto MAX_AUDIO_VALUE = +1.0F;
  Expects(MIN_AUDIO_VALUE <= audioValue);
  Expects(audioValue <= MAX_AUDIO_VALUE);

  return (1.0F + audioValue) / (MAX_AUDIO_VALUE - MIN_AUDIO_VALUE);
}

inline auto AudioSamples::GetNumDistinctChannels() const -> size_t
{
  return m_numDistinctChannels;
}

inline auto AudioSamples::GetSample(const size_t channelIndex) const -> const SampleArray&
{
  return m_sampleArrays.at(channelIndex);
}

inline auto AudioSamples::GetSampleMinMax(const size_t channelIndex) const -> const MinMaxValues&
{
  return m_minMaxSampleValues.at(channelIndex);
}

inline auto AudioSamples::GetSampleOverallMinMax() const -> const MinMaxValues&
{
  return m_overallMinMaxSampleValues;
}

inline auto SoundInfo::GetVolume() const -> float
{
  return m_volume;
}

inline auto SoundInfo::GetSpeed() const -> float
{
  return m_speed;
}

inline auto SoundInfo::GetAcceleration() const -> float
{
  return m_acceleration;
}

inline auto SoundInfo::GetAllTimesMaxVolume() const -> float
{
  return m_allTimesMaxVolume;
}

inline auto SoundInfo::GetAllTimesMinVolume() const -> float
{
  return m_allTimesMinVolume;
}

} // namespace GOOM
