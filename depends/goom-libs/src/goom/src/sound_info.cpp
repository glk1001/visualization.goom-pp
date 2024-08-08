module;

#include <algorithm>
#include <array>
#include <cstddef>
#include <span>

module Goom.Lib.SoundInfo;

namespace GOOM
{

AudioSamples::AudioSamples(const size_t numSampleChannels,
                           const std::span<const float> rawAudioData)
  : m_numDistinctChannels{numSampleChannels}, m_sampleArrays{GetSampleArrays(rawAudioData)}
{
  Expects((0 < numSampleChannels) && (numSampleChannels <= NUM_AUDIO_SAMPLES));
}

auto AudioSamples::GetSampleArrays(const std::span<const float> rawAudioData)
    -> std::array<SampleArray, NUM_AUDIO_SAMPLES>
{
  Expects((NUM_AUDIO_SAMPLES * AUDIO_SAMPLE_LEN) == rawAudioData.size());

  auto sampleArrays = std::array<SampleArray, NUM_AUDIO_SAMPLES>{};

  auto fpos = 0U;
  for (size_t j = 0; j < AUDIO_SAMPLE_LEN; ++j)
  {
    for (auto i = 0U; i < NUM_AUDIO_SAMPLES; ++i)
    {
      sampleArrays.at(i).at(j) = GetPositiveValue(rawAudioData[fpos]);
      ++fpos;
    }
  }

  return sampleArrays;
}

auto AudioSamples::GetMinMaxSampleValues(
    const std::array<SampleArray, NUM_AUDIO_SAMPLES>& sampleArrays)
    -> std::array<MinMaxValues, NUM_AUDIO_SAMPLES>
{
  auto minMaxSampleValues = std::array<MinMaxValues, NUM_AUDIO_SAMPLES>{};

  for (auto i = 0U; i < NUM_AUDIO_SAMPLES; ++i)
  {
    const auto& sampleArray         = sampleArrays.at(i);
    const auto& [minVal, maxVal]    = std::ranges::minmax_element(sampleArray);
    minMaxSampleValues.at(i).minVal = *minVal;
    minMaxSampleValues.at(i).maxVal = *maxVal;
  }

  return minMaxSampleValues;
}

void SoundInfo::ProcessSample(const AudioSamples& samples)
{
  const auto prevVolume = m_volume;
  UpdateVolume(samples);

  const auto prevSpeed = m_speed;
  UpdateSpeed(prevVolume);
  UpdateAcceleration(prevSpeed);
}

inline void SoundInfo::UpdateVolume(const AudioSamples& samples)
{
  m_volume = samples.GetSampleOverallMinMax().maxVal;

  m_allTimesMaxVolume = std::max(m_volume, m_allTimesMaxVolume);
  m_allTimesMinVolume = std::min(samples.GetSampleOverallMinMax().minVal, m_allTimesMinVolume);

  Ensures((0.0F <= m_volume) and (m_volume <= 1.0F));
}

inline void SoundInfo::UpdateSpeed(const float prevVolume)
{
  m_speed = AudioSamples::GetPositiveValue(m_volume - prevVolume);
  Ensures((0.0F <= m_speed) and (m_speed <= 1.0F));
}

inline void SoundInfo::UpdateAcceleration(const float prevSpeed)
{
  m_acceleration = AudioSamples::GetPositiveValue(m_speed - prevSpeed);
  Ensures((0.0F <= m_acceleration) and (m_acceleration <= 1.0F));
}

} // namespace GOOM
