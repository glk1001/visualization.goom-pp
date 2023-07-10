//#undef NO_LOGGING

#include "filter_buffers_service.h"

#include "filter_buffer_striper.h"
#include "filter_buffers.h"
#include "filter_settings.h"
#include "goom_config.h"
#include "goom_logger.h"
#include "goom_plugin_info.h"
#include "normalized_coords.h"
#include "utils/math/misc.h"
#include "utils/parallel_utils.h"
#include "zoom_vector.h"

namespace GOOM::FILTER_FX
{

using UTILS::NameValuePairs;
using UTILS::Parallel;
using UTILS::MATH::FloatsEqual;

FilterBuffersService::FilterBuffersService(
    Parallel& parallel,
    const PluginInfo& goomInfo,
    const NormalizedCoordsConverter& normalizedCoordsConverter,
    std::unique_ptr<IZoomVector> zoomVector) noexcept
  : m_zoomVector{std::move(zoomVector)},
    m_filterBuffers{std::make_unique<ZoomFilterBufferStriper>(
        parallel,
        goomInfo,
        normalizedCoordsConverter,
        [this](const NormalizedCoords& normalizedCoords,
               const NormalizedCoords& normalizedFilterViewportCoords)
        { return m_zoomVector->GetZoomInPoint(normalizedCoords, normalizedFilterViewportCoords); })}
{
}

auto FilterBuffersService::SetFilterTransformBufferSettings(
    const FilterTransformBufferSettings& filterTransformBufferSettings) noexcept -> void
{
  m_filterBuffers.SetFilterViewport(filterTransformBufferSettings.viewport);

  UpdateTransformBufferLerpData({filterTransformBufferSettings.lerpData.lerpIncrement,
                                 filterTransformBufferSettings.lerpData.lerpToMaxLerp});
}

auto FilterBuffersService::SetFilterEffectsSettings(
    const FilterEffectsSettings& filterEffectsSettings) noexcept -> void
{
  m_nextFilterEffectsSettings    = filterEffectsSettings;
  m_pendingFilterEffectsSettings = true;
}

auto FilterBuffersService::GetNameValueParams(const std::string& paramGroup) const noexcept
    -> NameValuePairs
{
  return m_zoomVector->GetNameValueParams(paramGroup);
}

auto FilterBuffersService::Start() noexcept -> void
{
  m_currentFilterEffectsSettings = m_nextFilterEffectsSettings;
  Expects(m_currentFilterEffectsSettings.zoomInCoefficientsEffect != nullptr);

  UpdateFilterEffectsSettings();

  m_filterBuffers.Start();
}

inline auto FilterBuffersService::UpdateFilterEffectsSettings() noexcept -> void
{
  UpdateZoomVectorFilterEffectsSettings();

  m_filterBuffers.SetTransformBufferMidpoint(m_currentFilterEffectsSettings.zoomMidpoint);
  m_filterBuffers.NotifyFilterSettingsHaveChanged();
}

inline auto FilterBuffersService::UpdateZoomVectorFilterEffectsSettings() noexcept -> void
{
  m_zoomVector->SetFilterEffectsSettings(m_currentFilterEffectsSettings);

  m_currentFilterEffectsSettings.afterEffectsSettings.rotationAdjustments.Reset();
}

auto FilterBuffersService::UpdateTransformBuffer() noexcept -> void
{
  m_filterBuffers.UpdateTransformBuffer();

  if (IsStartingFreshTransformBuffer())
  {
    StartFreshTransformBuffer();
  }
}

inline auto FilterBuffersService::IsStartingFreshTransformBuffer() const noexcept -> bool
{
  return m_filterBuffers.GetTransformBufferState() ==
         FilterBuffers::TransformBufferState::START_FRESH_TRANSFORM_BUFFER;
}

auto FilterBuffersService::StartFreshTransformBuffer() noexcept -> void
{
  // Don't start making new stripes until filter settings change.
  if (not m_pendingFilterEffectsSettings)
  {
    return;
  }

  m_currentFilterEffectsSettings = m_nextFilterEffectsSettings;

  UpdateFilterEffectsSettings();

  m_pendingFilterEffectsSettings = false;
}

inline auto FilterBuffersService::UpdateTransformBufferLerpData(
    const TransformBufferLerpData& transformBufferLerpData) noexcept -> void
{
  auto tranLerpFactor = m_filterBuffers.GetTransformBufferLerpFactor();

  if (transformBufferLerpData.lerpIncrement != 0U)
  {
    tranLerpFactor = std::min(tranLerpFactor + transformBufferLerpData.lerpIncrement,
                              FilterBuffers::MAX_TRAN_LERP_VALUE);
  }

  if (not FloatsEqual(transformBufferLerpData.lerpToMaxLerp, 1.0F))
  {
    tranLerpFactor = STD20::lerp(
        FilterBuffers::MAX_TRAN_LERP_VALUE, tranLerpFactor, transformBufferLerpData.lerpToMaxLerp);
  }

  m_filterBuffers.SetTransformBufferLerpFactor(tranLerpFactor);
}

} // namespace GOOM::FILTER_FX
