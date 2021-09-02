#include "filter_buffers_service.h"

#include "filter_buffers.h"
#include "filter_data.h"
#include "filter_normalized_coords.h"
#include "goom_plugin_info.h"
#include "goom_zoom_vector.h"
#include "goomutils/goomrand.h"
#include "goomutils/logging_control.h"
//#undef NO_LOGGING
#include "goomutils/logging.h"
#include "goomutils/mathutils.h"
#include "goomutils/parallel_utils.h"
#undef NDEBUG
#include <cassert>
#include <cstdint>
#include <string>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif


using FILTERS::IZoomVector;
using FILTERS::NormalizedCoords;
using FILTERS::ZoomFilterBuffers;
using UTILS::floats_equal;
using UTILS::GetRandInRange;
using UTILS::Logging;
using UTILS::Parallel;

constexpr float MAX_MAX_SPEED_COEFF = +4.01F;

ZoomFilterBuffersService::ZoomFilterBuffersService(
    Parallel& p,
    const std::shared_ptr<const PluginInfo>& goomInfo,
    std::unique_ptr<IZoomVector> zoomVector) noexcept
  : m_screenWidth{goomInfo->GetScreenInfo().width},
    m_zoomVector{std::move(zoomVector)},
    m_filterBuffers{p, goomInfo, [this](const NormalizedCoords& normalizedCoords) {
                      return m_zoomVector->GetZoomPoint(normalizedCoords);
                    }}
{
}

void ZoomFilterBuffersService::Start()
{
  m_currentFilterSettings = m_nextFilterSettings;

  UpdateFilterSettings();

  m_filterBuffers.Start();
}

void ZoomFilterBuffersService::SetFilterSettings(const ZoomFilterData& filterSettings)
{
  m_nextFilterSettings = filterSettings;
  m_pendingFilterSettings = true;
}

void ZoomFilterBuffersService::SetSpeedCoefficientsEffect(
    const std::shared_ptr<const SpeedCoefficientsEffect> val)
{
  m_nextSpeedCoefficientsEffect = val;
  m_pendingFilterSettings = true;
}

void ZoomFilterBuffersService::UpdatePlaneEffects()
{
  m_pendingPlaneEffects = true;
  m_pendingFilterSettings = true;
}

inline void ZoomFilterBuffersService::UpdateFilterSettings()
{
  m_zoomVector->SetFilterSettings(m_currentFilterSettings);
  m_zoomVector->SetSpeedCoefficientsEffect(m_nextSpeedCoefficientsEffect);
  if (m_pendingPlaneEffects)
  {
    m_zoomVector->SetRandomPlaneEffects(m_currentFilterSettings.zoomMidPoint, m_screenWidth);
    m_pendingPlaneEffects = false;
  }
  // TODO Random calc should not be here
  m_zoomVector->SetMaxSpeedCoeff(GetRandInRange(0.5F, 1.0F) * MAX_MAX_SPEED_COEFF);

  m_filterBuffers.SetBuffMidPoint(m_currentFilterSettings.zoomMidPoint);
  m_filterBuffers.NotifyFilterSettingsHaveChanged();
}

void ZoomFilterBuffersService::UpdateTranBuffers()
{
  m_filterBuffers.UpdateTranBuffers();

  if (AreStartingFreshTranBuffers())
  {
    StartFreshTranBuffers();
  }
}

inline auto ZoomFilterBuffersService::AreStartingFreshTranBuffers() const -> bool
{
  return m_filterBuffers.GetTranBuffersState() ==
         ZoomFilterBuffers::TranBuffersState::START_FRESH_TRAN_BUFFERS;
}

void ZoomFilterBuffersService::StartFreshTranBuffers()
{
  // Don't start making new stripes until filter settings change.
  if (!m_pendingFilterSettings)
  {
    return;
  }

  m_currentFilterSettings = m_nextFilterSettings;

  UpdateFilterSettings();

  m_pendingFilterSettings = false;
}

void ZoomFilterBuffersService::UpdateTranLerpFactor(const int32_t switchIncr,
                                                    const float switchMult)
{
  int32_t tranLerpFactor = m_filterBuffers.GetTranLerpFactor();

  if (switchIncr != 0)
  {
    tranLerpFactor =
        stdnew::clamp(tranLerpFactor + switchIncr, 0, ZoomFilterBuffers::GetMaxTranLerpFactor());
  }

  if (!floats_equal(switchMult, 1.0F))
  {
    tranLerpFactor = static_cast<int32_t>(
        stdnew::lerp(static_cast<float>(ZoomFilterBuffers::GetMaxTranLerpFactor()),
                     static_cast<float>(tranLerpFactor), switchMult));
  }

  m_filterBuffers.SetTranLerpFactor(tranLerpFactor);
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
