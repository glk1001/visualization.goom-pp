#ifndef VISUALIZATION_GOOM_FILTER_CONTROL_H
#define VISUALIZATION_GOOM_FILTER_CONTROL_H

#include "filter_buffers_service.h"
#include "filter_speed_coefficients_effect.h"
#include "filter_zoom_colors.h"
#include "filter_zoom_vector.h"
#include "goom/filter_data.h"
#include "goomutils/spimpl.h"

#include <memory>
#include <string>
#include <vector>

namespace GOOM
{

class PluginInfo;

namespace UTILS
{
class Parallel;
}; // namespace UTILS

namespace FILTERS
{

class FilterControl
{
public:
  class FilterEvents;

  FilterControl(UTILS::Parallel& p,
                const std::shared_ptr<const GOOM::PluginInfo>& goomInfo,
                const std::string& resourcesDirectory) noexcept;

  auto GetZoomFilterBuffersService() -> ZoomFilterBuffersService&;
  auto GetZoomFilterColors() -> ZoomFilterColors&;

  void Start();

  void UpdateFilterSettings();
  auto HaveSettingsChangedSinceLastUpdate() const -> bool;

  [[nodiscard]] auto GetFilterSettings() const -> const ZoomFilterData&;
  [[nodiscard]] auto GetROVitesseSetting() -> const Vitesse&;
  [[nodiscard]] auto GetRWVitesseSetting() -> Vitesse&;

  void ChangeMilieu();
  void SetMiddlePoints();
  void SetNoisifySetting(bool value);
  void SetNoiseFactorSetting(float value);
  void ReduceNoiseFactor();
  void SetBlockyWavySetting(bool value);
  void SetRotateSetting(float value);
  void MultiplyRotateSetting(float factor);
  void ToggleRotateSetting();

  void SetRandomFilterSettings();
  void SetRandomFilterSettings(ZoomFilterMode mode);
  void SetDefaultFilterSettings(ZoomFilterMode mode);

private:
  static const UTILS::Weights<ZoomFilterMode> WEIGHTED_FILTER_EVENTS;
  const std::shared_ptr<const PluginInfo> m_goomInfo;
  const V2dInt m_midScreenPoint;
  ZoomFilterBuffersService m_zoomFilterBuffersService;
  ZoomFilterColors m_filterColors{};
  ZoomFilterData m_filterData{};
  spimpl::unique_impl_ptr<FilterEvents> m_filterEvents;

  [[nodiscard]] auto GetNewRandomMode() const -> ZoomFilterMode;

  bool m_settingsHaveChanged = false;

  std::vector<std::shared_ptr<SpeedCoefficientsEffect>> m_speedCoefficientsEffect;
  [[nodiscard]] auto GetSpeedCoefficientsEffect(ZoomFilterMode mode)
      -> std::shared_ptr<SpeedCoefficientsEffect>&;
  [[nodiscard]] static auto MakeSpeedCoefficientsEffects(const std::string& resourcesDirectory)
      -> std::vector<std::shared_ptr<SpeedCoefficientsEffect>>;
  [[nodiscard]] static auto MakeSpeedCoefficientsEffect(ZoomFilterMode mode,
                                                        const std::string& resourcesDirectory)
      -> std::shared_ptr<SpeedCoefficientsEffect>;

  void SetDefaultSettings();
  void SetFilterModeSettings();
  void SetAmuletModeSettings();
  void SetCrystalBall0ModeSettings();
  void SetCrystalBall1ModeSettings();
  void SetHypercosMode0Settings();
  void SetHypercosMode1Settings();
  void SetHypercosMode2Settings();
  void SetHypercosMode3Settings();
  void SetImageDisplacementModeSettings();
  void SetNormalModeSettings();
  void SetScrunchModeSettings();
  void SetSpeedwayModeSettings();
  void SetWaterModeSettings();
  void SetWaveMode0Settings();
  void SetWaveMode1Settings();
  void SetWaveModeSettings();
  void SetYOnlyModeSettings();

  void SetRandomMiddlePoints();
  void SetRotate(float rotateProbability);
};

inline auto FilterControl::GetFilterSettings() const -> const ZoomFilterData&
{
  return m_filterData;
}

inline auto FilterControl::HaveSettingsChangedSinceLastUpdate() const -> bool
{
  return m_settingsHaveChanged;
}

inline auto FilterControl::GetROVitesseSetting() -> const Vitesse&
{
  return m_filterData.vitesse;
}

inline auto FilterControl::GetRWVitesseSetting() -> Vitesse&
{
  m_settingsHaveChanged = true;
  return m_filterData.vitesse;
}

inline void FilterControl::ChangeMilieu()
{
  m_settingsHaveChanged = true;
  SetMiddlePoints();
  m_zoomFilterBuffersService.UpdatePlaneEffects();
}

inline void FilterControl::SetMiddlePoints()
{
  m_settingsHaveChanged = true;
  SetRandomMiddlePoints();
}

inline void FilterControl::SetRandomFilterSettings()
{
  m_settingsHaveChanged = true;
  SetRandomFilterSettings(GetNewRandomMode());
}

inline void FilterControl::SetDefaultFilterSettings(const ZoomFilterMode mode)
{
  m_settingsHaveChanged = true;
  m_filterData.SetMode(mode);
  SetDefaultSettings();
}

inline void FilterControl::SetNoisifySetting(const bool value)
{
  m_settingsHaveChanged = true;
  m_filterData.noisify = value;
}

inline void FilterControl::SetNoiseFactorSetting(const float value)
{
  m_settingsHaveChanged = true;
  m_filterData.noiseFactor = value;
}

inline void FilterControl::ReduceNoiseFactor()
{
  if (!GetFilterSettings().noisify)
  {
    return;
  }
  constexpr float REDUCING_FACTOR = 0.94F;
  const float reducedNoiseFactor = m_filterData.noiseFactor * REDUCING_FACTOR;
  SetNoiseFactorSetting(reducedNoiseFactor);
}

inline void FilterControl::SetBlockyWavySetting(const bool value)
{
  m_settingsHaveChanged = true;
  m_filterData.blockyWavy = value;
}

inline void FilterControl::SetRotateSetting(const float value)
{
  m_settingsHaveChanged = true;
  m_filterData.rotateSpeed = value;
}

inline void FilterControl::MultiplyRotateSetting(const float factor)
{
  m_settingsHaveChanged = true;
  m_filterData.rotateSpeed *= factor;
}

inline void FilterControl::ToggleRotateSetting()
{
  m_settingsHaveChanged = true;
  m_filterData.rotateSpeed = -m_filterData.rotateSpeed;
}

} // namespace FILTERS
} // namespace GOOM

#endif //VISUALIZATION_GOOM_FILTER_CONTROL_H
