#pragma once

#include "filter_buffer_striper.h"
#include "filter_buffers.h"
#include "filter_settings.h"
#include "utils/name_value_pairs.h"
#include "zoom_vector.h"

#include <cstdint>
#include <memory>

namespace GOOM
{

class PluginInfo;

namespace UTILS
{
class Parallel;
}

namespace FILTER_FX
{

class FilterBuffersService
{
  using FilterBuffers = ZoomFilterBuffers<ZoomFilterBufferStriper>;

public:
  FilterBuffersService(UTILS::Parallel& parallel,
                       const PluginInfo& goomInfo,
                       const NormalizedCoordsConverter& normalizedCoordsConverter,
                       std::unique_ptr<IZoomVector> zoomVector) noexcept;

  auto Start() noexcept -> void;

  [[nodiscard]] auto GetCurrentFilterEffectsSettings() const noexcept
      -> const ZoomFilterEffectsSettings&;
  auto SetFilterEffectsSettings(const ZoomFilterEffectsSettings& filterEffectsSettings) noexcept
      -> void;
  auto SetFilterBufferSettings(const ZoomFilterBufferSettings& filterBufferSettings) noexcept
      -> void;

  [[nodiscard]] auto GetTranLerpFactor() const noexcept -> uint32_t;
  [[nodiscard]] auto GetFilterBuffers() noexcept -> FilterBuffers&;

  auto UpdateTranBuffers() noexcept -> void;

  struct TranLerpProperties
  {
    uint32_t tranLerpIncrement;
    float tranLerpToMaxSwitchMult;
  };
  auto UpdateTranLerpProperties(const TranLerpProperties& tranLerpProperties) noexcept -> void;

  [[nodiscard]] auto GetSourcePointInfo(size_t buffPos) const noexcept
      -> FILTER_BUFFERS::SourcePointInfo;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const noexcept
      -> UTILS::NameValuePairs;

private:
  std::unique_ptr<IZoomVector> m_zoomVector;
  FilterBuffers m_filterBuffers;

  ZoomFilterEffectsSettings m_currentFilterEffectsSettings{};
  ZoomFilterEffectsSettings m_nextFilterEffectsSettings{};
  bool m_pendingFilterEffectsSettings = false;

  auto UpdateFilterEffectsSettings() noexcept -> void;
  auto UpdateZoomVectorFilterEffectsSettings() noexcept -> void;
  [[nodiscard]] auto AreStartingFreshTranBuffers() const noexcept -> bool;
  auto StartFreshTranBuffers() noexcept -> void;
};

inline auto FilterBuffersService::GetCurrentFilterEffectsSettings() const noexcept
    -> const ZoomFilterEffectsSettings&
{
  return m_currentFilterEffectsSettings;
}

inline auto FilterBuffersService::GetSourcePointInfo(const size_t buffPos) const noexcept
    -> FILTER_BUFFERS::SourcePointInfo
{
  return m_filterBuffers.GetSourcePointInfo(buffPos);
}

inline auto FilterBuffersService::GetTranLerpFactor() const noexcept -> uint32_t
{
  return m_filterBuffers.GetTranLerpFactor();
}

inline auto FilterBuffersService::GetFilterBuffers() noexcept -> FilterBuffers&
{
  return m_filterBuffers;
}

} // namespace FILTER_FX
} // namespace GOOM
