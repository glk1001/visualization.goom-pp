#ifndef VISUALIZATION_GOOM_FILTER_CONTROL_H
#define VISUALIZATION_GOOM_FILTER_CONTROL_H

#include "filter_buffers_service.h"
#include "filter_settings.h"
#include "filter_speed_coefficients_effect.h"
#include "filter_zoom_colors.h"
#include "filter_zoom_vector.h"
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
} // namespace UTILS

namespace FILTERS
{

class FilterControl
{
public:
  class FilterEvents;

  FilterControl(UTILS::Parallel& parallel,
                const std::shared_ptr<const GOOM::PluginInfo>& goomInfo,
                const std::string& resourcesDirectory) noexcept;

  auto GetZoomFilterBuffersService() -> std::unique_ptr<ZoomFilterBuffersService>;
  auto GetZoomFilterColors() -> std::unique_ptr<ZoomFilterColors>;

  void Start();

  void NotifyUpdatedFilterSettings();
  auto HaveSettingsChangedSinceLastUpdate() const -> bool;

  [[nodiscard]] auto GetFilterSettings() const -> const ZoomFilterSettings&;
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

private:
  enum class ZoomFilterMode
  {
    _NULL = -1,
    AMULET_MODE = 0,
    CRYSTAL_BALL_MODE0,
    CRYSTAL_BALL_MODE1,
    HYPERCOS_MODE0,
    HYPERCOS_MODE1,
    HYPERCOS_MODE2,
    HYPERCOS_MODE3,
    IMAGE_DISPLACEMENT_MODE,
    NORMAL_MODE,
    SCRUNCH_MODE,
    SPEEDWAY_MODE,
    WATER_MODE,
    WAVE_MODE0,
    WAVE_MODE1,
    Y_ONLY_MODE,
    _NUM // unused and must be last
  };

  ZoomFilterMode m_zoomFilterMode = ZoomFilterMode::NORMAL_MODE;

  void SetRandomFilterSettings(ZoomFilterMode mode);
  void SetDefaultFilterSettings(ZoomFilterMode mode);

  static const UTILS::Weights<ZoomFilterMode> WEIGHTED_FILTER_EVENTS;
  UTILS::Parallel& m_parallel;
  const std::shared_ptr<const PluginInfo> m_goomInfo;
  const V2dInt m_midScreenPoint;
  const std::string m_resourcesDirectory;
  ZoomFilterSettings m_filterSettings{};
  spimpl::unique_impl_ptr<FilterEvents> m_filterEvents;

  [[nodiscard]] auto GetNewRandomMode() const -> ZoomFilterMode;

  bool m_settingsHaveChanged = false;

  std::vector<std::shared_ptr<SpeedCoefficientsEffect>> m_speedCoefficientsEffect;
  [[nodiscard]] auto GetSpeedCoefficientsEffect() -> std::shared_ptr<SpeedCoefficientsEffect>&;
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

inline auto FilterControl::GetFilterSettings() const -> const ZoomFilterSettings&
{
  return m_filterSettings;
}

inline auto FilterControl::HaveSettingsChangedSinceLastUpdate() const -> bool
{
  return m_settingsHaveChanged;
}

inline auto FilterControl::GetROVitesseSetting() -> const Vitesse&
{
  return m_filterSettings.vitesse;
}

inline auto FilterControl::GetRWVitesseSetting() -> Vitesse&
{
  m_settingsHaveChanged = true;
  return m_filterSettings.vitesse;
}

inline void FilterControl::ChangeMilieu()
{
  m_settingsHaveChanged = true;
  SetMiddlePoints();
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
  m_zoomFilterMode = mode;
  SetDefaultSettings();
}

inline void FilterControl::SetNoisifySetting(const bool value)
{
  m_settingsHaveChanged = true;
  m_filterSettings.noisify = value;
}

inline void FilterControl::SetNoiseFactorSetting(const float value)
{
  m_settingsHaveChanged = true;
  m_filterSettings.noiseFactor = value;
}

inline void FilterControl::ReduceNoiseFactor()
{
  if (!GetFilterSettings().noisify)
  {
    return;
  }
  constexpr float REDUCING_FACTOR = 0.94F;
  const float reducedNoiseFactor = m_filterSettings.noiseFactor * REDUCING_FACTOR;
  SetNoiseFactorSetting(reducedNoiseFactor);
}

inline void FilterControl::SetBlockyWavySetting(const bool value)
{
  m_settingsHaveChanged = true;
  m_filterSettings.blockyWavy = value;
}

inline void FilterControl::SetRotateSetting(const float value)
{
  m_settingsHaveChanged = true;
  m_filterSettings.rotateSpeed = value;
}

inline void FilterControl::MultiplyRotateSetting(const float factor)
{
  m_settingsHaveChanged = true;
  m_filterSettings.rotateSpeed *= factor;
}

inline void FilterControl::ToggleRotateSetting()
{
  m_settingsHaveChanged = true;
  m_filterSettings.rotateSpeed = -m_filterSettings.rotateSpeed;
}

} // namespace FILTERS
} // namespace GOOM

#endif //VISUALIZATION_GOOM_FILTER_CONTROL_H
