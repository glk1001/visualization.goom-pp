module;

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <thread>

namespace GOOM
{
class GoomLogger;
}

export module Goom.GoomVisualization;

import Goom.Lib.GoomControl;
import Goom.Lib.GoomTypes;
import Goom.Lib.SoundInfo;
export import Goom.GoomVisualization.GlUtils;
export import :DisplacementFilter;
export import :GlRenderTypes;
export import :Scene;
export import :SlotProducerConsumer;

export namespace GOOM::VIS
{

class GoomVisualization
{
public:
  GoomVisualization(GoomLogger& goomLogger,
                    const std::string& resourcesDir,
                    uint32_t consumeWaitForProducerMs,
                    const std::string& shaderDir,
                    const TextureBufferDimensions& textureBufferDimensions);
  GoomVisualization(GoomLogger& goomLogger,
                    const std::string& resourcesDir,
                    uint32_t consumeWaitForProducerMs,
                    const TextureBufferDimensions& textureBufferDimensions,
                    std::unique_ptr<OPENGL::DisplacementFilter>&& glScene);

  static auto SetRandomSeed(uint64_t seed) noexcept -> void;
  auto SetWindowDimensions(const WindowDimensions& windowDimensions) noexcept -> void;
  auto SetShowSongTitle(ShowSongTitleType showMusicTitleType) -> void;
  auto SetShowGoomState(bool value) -> void;
  auto SetDumpDirectory(const std::string& dumpDirectory) -> void;
  auto SetBrightnessAdjust(float value) -> void;

  auto Start(int numChannels) -> void;
  auto StartThread() -> void;
  auto Stop() -> void;

  auto AddAudioSample(std::span<const float> audioSample) -> bool;

  struct TrackInfo
  {
    std::string title;
    std::string artist;
    std::string albumArtist;
    std::string genre;
    uint32_t duration = 0U;
  };
  auto UpdateTrack(const TrackInfo& track) -> void;

  [[nodiscard]] auto GetScene() noexcept -> OPENGL::DisplacementFilter&;
  [[nodiscard]] auto GetNumAudioSamples() const noexcept -> uint32_t;

private:
  bool m_started = false;
  GoomLogger* m_goomLogger;
  uint32_t m_consumeWaitForProducerMs;
  auto InitConstructor() noexcept -> void;

  std::unique_ptr<OPENGL::DisplacementFilter> m_glScene;
  auto InitSceneFrameData() -> void;

  GoomControl m_goomControl;
  auto InitGoomControl() noexcept -> void;

  SlotProducerConsumer<AudioSamples> m_slotProducerConsumer;
  SlotProducerIsDriving<AudioSamples> m_slotProducerIsDriving;
  std::thread m_slotProducerConsumerThread;
  auto ProduceItem(size_t slot, const AudioSamples& audioSamples) noexcept -> void;
  auto ConsumeItem(size_t slot) noexcept -> void;
  uint32_t m_numberOfDroppedAudioSamples = 0U;
  auto LogProducerConsumerSummary() -> void;
  double m_totalProductionTimeInMs = 0.0;
  uint64_t m_numItemsProduced      = 0U;

  size_t m_numChannels       = 0;
  size_t m_audioSampleLen    = 0;
  uint32_t m_numAudioSamples = 0U;
  auto InitAudioValues(int32_t numChannels) noexcept -> void;
};

} // namespace GOOM::VIS

namespace GOOM::VIS
{

inline auto GoomVisualization::GetScene() noexcept -> OPENGL::DisplacementFilter&
{
  return *m_glScene;
}

inline auto GoomVisualization::GetNumAudioSamples() const noexcept -> uint32_t
{
  return m_numAudioSamples;
}

} // namespace GOOM::VIS
