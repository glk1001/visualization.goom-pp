module;

#undef NO_LOGGING

#include "goom/goom_logger.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <thread>
#include <utility>

module Goom.GoomVisualization;

import Goom.GoomVisualization.BuildTime;
import Goom.Lib.AssertUtils;
import Goom.Lib.CompilerVersions;
import Goom.Lib.GoomControl;
import Goom.Lib.GoomTypes;
import Goom.Lib.GoomUtils;
import Goom.Lib.SoundInfo;
import :DisplacementFilter;
import :GlRenderTypes;
import :SlotProducerConsumer;

namespace GOOM::VIS
{

using OPENGL::DisplacementFilter;

namespace
{

[[nodiscard]] auto GetGoomVisualizationBuildTime() -> std::string
{
  return ::GetBuildTime();
}

#ifdef SAVE_AUDIO_BUFFERS
[[nodiscard]] auto GetAudioBuffersSaveDir()
{
  static constexpr auto* AUDIO_BUFFERS_DIR_PREFIX = "audio_buffers";

  const auto kodiGoomDataDir = kodi::vfs::TranslateSpecialProtocol(GOOM_ADDON_DATA_DIR);

  return kodiGoomDataDir + PATH_SEP + AUDIO_BUFFERS_DIR_PREFIX;
}
#endif

constexpr auto GOOM_BUFFER_PRODUCER_CONSUMER = "Goom";
constexpr auto MAX_BUFFER_QUEUE_LEN          = DisplacementFilter::NUM_PBOS;
constexpr auto MAX_AUDIO_DATA_QUEUE_LEN      = 100U;

} // namespace

GoomVisualization::GoomVisualization(GoomLogger& goomLogger,
                                     const std::string& resourcesDir,
                                     const uint32_t consumeWaitForProducerMs,
                                     const std::string& shaderDir,
                                     const TextureBufferDimensions& textureBufferDimensions)
  : m_goomLogger{&goomLogger},
    m_consumeWaitForProducerMs{consumeWaitForProducerMs},
    m_glScene{
        std::make_unique<DisplacementFilter>(*m_goomLogger, shaderDir, textureBufferDimensions)},
    m_goomControl{
        Dimensions{textureBufferDimensions.width, textureBufferDimensions.height},
        resourcesDir,
        *m_goomLogger},
    m_slotProducerConsumer{*m_goomLogger,
                           MAX_BUFFER_QUEUE_LEN,
                           GOOM_BUFFER_PRODUCER_CONSUMER,
                           MAX_AUDIO_DATA_QUEUE_LEN},
    m_slotProducerIsDriving{m_slotProducerConsumer}
{
  InitConstructor();
}

GoomVisualization::GoomVisualization(GoomLogger& goomLogger,
                                     const std::string& resourcesDir,
                                     const uint32_t consumeWaitForProducerMs,
                                     const TextureBufferDimensions& textureBufferDimensions,
                                     std::unique_ptr<DisplacementFilter>&& glScene)
  : m_goomLogger{&goomLogger},
    m_consumeWaitForProducerMs{consumeWaitForProducerMs},
    m_glScene{std::move(glScene)},
    m_goomControl{
        Dimensions{textureBufferDimensions.width, textureBufferDimensions.height},
        resourcesDir,
        *m_goomLogger},
    m_slotProducerConsumer{*m_goomLogger,
                           MAX_BUFFER_QUEUE_LEN,
                           GOOM_BUFFER_PRODUCER_CONSUMER,
                           MAX_AUDIO_DATA_QUEUE_LEN},
    m_slotProducerIsDriving{m_slotProducerConsumer}
{
  InitConstructor();
}

auto GoomVisualization::InitConstructor() noexcept -> void
{
  m_slotProducerConsumer.SetProduceItemFunc(
      [this](const size_t slot, const AudioSamples& audioSamples)
      { ProduceItem(slot, audioSamples); });
  m_slotProducerConsumer.SetConsumeItemFunc([this](const size_t slot) { ConsumeItem(slot); });

  auto requestNextDataFrame = [this]()
  { return m_slotProducerConsumer.ConsumeWithoutRelease(m_consumeWaitForProducerMs); };
  m_glScene->SetRequestNextFrameDataFunc(requestNextDataFrame);

  auto releaseCurrentFrameData = [this](const size_t slot)
  { m_slotProducerConsumer.ReleaseAfterConsume(slot); };
  m_glScene->SetReleaseCurrentFrameDataFunc(releaseCurrentFrameData);

  LogDebug(*m_goomLogger, "Created Goom visualizationGoom object.");
}

auto GoomVisualization::SetRandomSeed(const uint64_t seed) noexcept -> void
{
  SetRandSeed(seed);
}

auto GoomVisualization::SetWindowDimensions(const WindowDimensions& windowDimensions) noexcept
    -> void
{
  m_glScene->Resize(windowDimensions);
}

auto GoomVisualization::SetShowSongTitle(const ShowSongTitleType showMusicTitleType) -> void
{
  m_goomControl.SetShowSongTitle(showMusicTitleType);
}

auto GoomVisualization::SetShowGoomState(const bool value) -> void
{
  m_goomControl.SetShowGoomState(value);
}

auto GoomVisualization::SetDumpDirectory(const std::string& dumpDirectory) -> void
{
  m_goomControl.SetDumpDirectory(dumpDirectory);
}

auto GoomVisualization::SetBrightnessAdjust(const float value) -> void
{
  m_glScene->SetBrightnessAdjust(value);
}

auto GoomVisualization::Start(const int numChannels) -> void
{
  Expects(not m_started);

  LogInfo(*m_goomLogger, "Starting visualization.");

  LogInfo(*m_goomLogger, "Goom Vis: Build Time     : {}.", GetGoomVisualizationBuildTime());
  LogInfo(*m_goomLogger, "Goom: Version            : {}.", GetGoomLibVersionInfo());
  LogInfo(*m_goomLogger, "Goom: Compiler           : {}.", GetCompilerVersion());
  LogInfo(*m_goomLogger, "Goom Library: Compiler   : {}.", GetGoomLibCompilerVersion());
  LogInfo(*m_goomLogger, "Goom Library: Build Time : {}.", GetGoomLibBuildTime());
  LogInfo(*m_goomLogger, "Random seed              : {}.", GetRandSeed());
  LogInfo(*m_goomLogger, "Num pool threads         : {}.", m_goomControl.GetNumPoolThreads());
  LogInfo(*m_goomLogger,
          "Texture width, height    : {}, {}.",
          m_glScene->GetWidth(),
          m_glScene->GetHeight());
  LogInfo(*m_goomLogger,
          "Frame width, height      : {}, {}.",
          m_glScene->GetFramebufferWidth(),
          m_glScene->GetFramebufferHeight());
  LogInfo(*m_goomLogger, "Shader Dir               : '{}'.", m_glScene->GetShaderDir());
  LogInfo(*m_goomLogger, "Brightness Adjust        : {:.2f}.", m_glScene->GetBrightnessAdjust());
  LogInfo(*m_goomLogger, "Dump directory           : {}.", m_goomControl.GetDumpDirectory());

  InitAudioValues(numChannels);
  InitSceneFrameData();
  InitGoomControl();

  m_goomControl.Start();
  m_slotProducerConsumer.Start();

  m_totalProductionTimeInMs = 0.0;
  m_numItemsProduced        = 0U;

  m_started = true;
}

auto GoomVisualization::StartThread() -> void
{
  Expects(m_started);

  LogInfo(*m_goomLogger, "Slot producer consumer thread starting.");
  m_slotProducerConsumerThread =
      std::thread{&SlotProducerIsDriving<AudioSamples>::ProducerThread, &m_slotProducerIsDriving};
}

auto GoomVisualization::Stop() -> void
{
  Expects(m_started);

  m_started = false;

  LogInfo(*m_goomLogger, "Goom visualization stopping...");

  LogInfo(*m_goomLogger, "Slot producer consumer thread stopping...");
  m_slotProducerConsumer.Stop();
  m_slotProducerConsumerThread.join();
  LogInfo(*m_goomLogger, "Slot producer consumer thread stopped.");

  m_goomControl.Finish();

  m_glScene->DestroyScene();

  LogInfo(*m_goomLogger, "Goom visualization stopped.");

  LogProducerConsumerSummary();
}

auto GoomVisualization::LogProducerConsumerSummary() -> void
{
  LogInfo(*m_goomLogger, "Number of items produced: {}.", m_numItemsProduced);
  LogInfo(*m_goomLogger,
          "Number of consume requests: {}.",
          m_slotProducerConsumer.GetConsumeRequests());
  LogInfo(*m_goomLogger,
          "Average produce item time = {:.1f}ms.",
          m_totalProductionTimeInMs / static_cast<double>(m_numItemsProduced));
  LogInfo(*m_goomLogger, "Number of dropped audio samples: {}.", m_numberOfDroppedAudioSamples);
  const auto percentNumConsumerGaveUpWaiting =
      m_slotProducerConsumer.GetConsumeRequests() == 0
          ? 0U
          : static_cast<uint32_t>(
                100.0F *
                static_cast<float>(m_slotProducerConsumer.GetNumTimesConsumerGaveUpWaiting()) /
                static_cast<float>(m_slotProducerConsumer.GetConsumeRequests()));
  LogInfo(*m_goomLogger,
          "Number of times consumer gave up waiting: {} ({}%).",
          m_slotProducerConsumer.GetNumTimesConsumerGaveUpWaiting(),
          percentNumConsumerGaveUpWaiting);
}

auto GoomVisualization::InitAudioValues(int32_t numChannels) noexcept -> void
{
  m_numChannels    = static_cast<size_t>(numChannels);
  m_audioSampleLen = m_numChannels * AudioSamples::AUDIO_SAMPLE_LEN;
}

auto GoomVisualization::InitSceneFrameData() -> void
{
  m_glScene->InitScene();
}

auto GoomVisualization::InitGoomControl() noexcept -> void
{
  m_goomControl.SetFrameData(m_glScene->GetFrameData(0));
}

auto GoomVisualization::AddAudioSample(const std::span<const float> audioSample) -> bool
{
  Expects(m_started);
  Expects(m_audioSampleLen == audioSample.size());

#ifdef DEBUG_LOGGING
  // LogInfo(*m_goomLogger, "Moving audio sample to producer.");
#endif
  if (not m_slotProducerConsumer.AddResource(AudioSamples{m_numChannels, audioSample}))
  {
    ++m_numberOfDroppedAudioSamples;
#ifdef DEBUG_LOGGING
    // LogWarn(*m_goomLogger, "### Resource queue full - skipping this audio sample.");
#endif
    return false;
  }

  ++m_numAudioSamples;

  return true;
}

auto GoomVisualization::UpdateTrack(const TrackInfo& track) -> void
{
  const auto artist          = not track.artist.empty() ? track.artist : track.albumArtist;
  const auto currentSongName = artist.empty() ? track.title : ((artist + " - ") + track.title);

  LogInfo(*m_goomLogger, "Current Title = '{}'", currentSongName);
  LogInfo(*m_goomLogger, "Genre = '{}', Duration = {}", track.genre, track.duration);

  m_goomControl.SetSongInfo(
      {.title = currentSongName, .genre = track.genre, .duration = track.duration});

#ifdef SAVE_AUDIO_BUFFERS
  m_audioBufferWriter = GetAudioBufferWriter(track.title);
#endif
}

auto GoomVisualization::ConsumeItem(const size_t slot) noexcept -> void
{
#ifdef DEBUG_LOGGING
  LogInfo(*m_goomLogger, std::format("Consumer consuming slot {}.", slot));
#endif

  m_glScene->UpdateFrameData(slot);

#ifdef DEBUG_LOGGING
  LogInfo(*m_goomLogger, std::format("Consumer consumed slot {}.", slot));
#endif
}

auto GoomVisualization::ProduceItem(const size_t slot, const AudioSamples& audioSamples) noexcept
    -> void
{
#ifdef DEBUG_LOGGING
  LogInfo(*m_goomLogger, std::format("Producer producing slot {}.", slot));
#endif

  ++m_numItemsProduced;
  const auto startTime = std::chrono::system_clock::now();

  auto& frameData = m_glScene->GetFrameData(slot);

  m_goomControl.SetFrameData(frameData);
  m_goomControl.UpdateGoomBuffers(audioSamples);

#ifdef GOOM_RUNNER_TOO_FAST_BUG
  static constexpr auto SLOWDOWN_MS = 10U;
  std::this_thread::sleep_for(std::chrono::milliseconds(SLOWDOWN_MS));
#endif

  const auto duration = std::chrono::system_clock::now() - startTime;
  m_totalProductionTimeInMs +=
      static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());

#ifdef DEBUG_LOGGING
  LogInfo(*m_goomLogger, std::format("Producer produced slot {}.", slot));
#endif
}

} // namespace GOOM::VIS
