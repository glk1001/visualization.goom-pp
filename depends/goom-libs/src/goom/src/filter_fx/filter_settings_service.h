#pragma once

#include "filter_effects/extra_effects_states.h"
#include "filter_effects/rotation.h"
#include "filter_settings.h"
#include "speed_coefficients_effect.h"
#include "utils/math/goom_rand_base.h"
#include "utils/propagate_const.h"

#include <functional>
#include <map>
#include <memory>
#include <string_view>

namespace GOOM
{
class Pixel;
class PluginInfo;

namespace FILTER_FX
{

enum class ZoomFilterMode
{
  AMULET_MODE = 0,
  CRYSTAL_BALL_MODE0,
  CRYSTAL_BALL_MODE1,
  DISTANCE_FIELD_MODE,
  HYPERCOS_MODE0,
  HYPERCOS_MODE1,
  HYPERCOS_MODE2,
  HYPERCOS_MODE3,
  IMAGE_DISPLACEMENT_MODE,
  NORMAL_MODE,
  SCRUNCH_MODE,
  SPEEDWAY_MODE0,
  SPEEDWAY_MODE1,
  SPEEDWAY_MODE2,
  WATER_MODE,
  WAVE_MODE0,
  WAVE_MODE1,
  Y_ONLY_MODE,
  _num // unused and must be last
};

class FilterSettingsService
{
public:
  using CreateSpeedCoefficientsEffectFunc = std::function<std::shared_ptr<ISpeedCoefficientsEffect>(
      ZoomFilterMode filterMode,
      const UTILS::MATH::IGoomRand& goomRand,
      const std::string& resourcesDirectory)>;
  // TODO - Visual Studio doesn't like a trailing return type in above function definition.

  FilterSettingsService(const GOOM::PluginInfo& goomInfo,
                        const UTILS::MATH::IGoomRand& goomRand,
                        const std::string& resourcesDirectory,
                        const CreateSpeedCoefficientsEffectFunc& createSpeedCoefficientsEffect);
  FilterSettingsService(const FilterSettingsService&) noexcept = delete;
  FilterSettingsService(FilterSettingsService&&) noexcept = delete;
  virtual ~FilterSettingsService() noexcept;
  auto operator=(const FilterSettingsService&) -> FilterSettingsService& = delete;
  auto operator=(FilterSettingsService&&) -> FilterSettingsService& = delete;

  auto Start() -> void;
  auto NewCycle() -> void;

  auto NotifyUpdatedFilterEffectsSettings() -> void;
  [[nodiscard]] auto HaveEffectsSettingsChangedSinceLastUpdate() const -> bool;
  [[nodiscard]] auto HasFilterModeChangedSinceLastUpdate() const -> bool;

  [[nodiscard]] auto GetCurrentFilterMode() const -> ZoomFilterMode;
  [[nodiscard]] auto GetCurrentFilterModeName() const -> const std::string_view&;
  [[nodiscard]] auto GetPreviousFilterMode() const -> ZoomFilterMode;
  [[nodiscard]] auto GetPreviousFilterModeName() const -> const std::string_view&;

  [[nodiscard]] auto GetFilterSettings() const -> const ZoomFilterSettings&;
  [[nodiscard]] auto GetROVitesse() const -> const Vitesse&;
  [[nodiscard]] auto GetRWVitesse() -> Vitesse&;

  auto ChangeMilieu() -> void;
  auto ResetRandomExtraEffects() -> void;
  auto TurnOffRotation() -> void;
  auto MultiplyRotation(float factor) -> void;
  auto ToggleRotationDirection() -> void;

  auto SetNewRandomFilter() -> void;

  auto SetTranLerpIncrement(int32_t value) -> void;
  auto SetDefaultTranLerpIncrement() -> void;
  auto MultiplyTranLerpIncrement(int32_t factor) -> void;

  auto SetTranLerpToMaxSwitchMult(float value) -> void;
  auto SetTranLerpToMaxDefaultSwitchMult() -> void;

protected:
  void SetFilterMode(ZoomFilterMode filterMode);
  [[nodiscard]] auto GetFilterSettings() -> ZoomFilterSettings&;
  [[nodiscard]] auto GetPluginInfo() const -> const PluginInfo&;
  [[nodiscard]] auto GetGoomRand() const -> const UTILS::MATH::IGoomRand&;
  virtual auto SetDefaultSettings() -> void;
  virtual auto SetRandomZoomMidpoint() -> void;
  virtual auto SetFilterModeExtraEffects() -> void;
  virtual auto SetRandomizedExtraEffects() -> void;
  virtual auto SetWaveModeExtraEffects() -> void;
  virtual auto UpdateFilterSettingsFromExtraEffects() -> void;

private:
  ZoomFilterMode m_filterMode = ZoomFilterMode::NORMAL_MODE;
  ZoomFilterMode m_previousFilterMode = ZoomFilterMode::NORMAL_MODE;
  ZoomFilterMode m_filterModeAtLastUpdate = ZoomFilterMode::NORMAL_MODE;

  auto SetRandomSettingsForNewFilterMode() -> void;

  const PluginInfo& m_goomInfo;
  const UTILS::MATH::IGoomRand& m_goomRand;
  const Point2dInt m_screenMidpoint;
  const std::string m_resourcesDirectory;
  std::experimental::propagate_const<std::unique_ptr<FILTER_EFFECTS::ExtraEffectsStates>>
      m_randomizedExtraEffects;

  struct ZoomFilterModeInfo
  {
    const std::string_view name;
    std::shared_ptr<ISpeedCoefficientsEffect> speedCoefficientsEffect{};
    const FILTER_EFFECTS::ExtraEffectsProbabilities extraEffectsProbabilities;
    const UTILS::MATH::Weights<HypercosOverlay> hypercosWeights;
  };
  std::map<ZoomFilterMode, ZoomFilterModeInfo> m_filterModeData;
  [[nodiscard]] static auto GetFilterModeData(
      const UTILS::MATH::IGoomRand& goomRand,
      const std::string& resourcesDirectory,
      const CreateSpeedCoefficientsEffectFunc& createSpeedCoefficientsEffect)
      -> std::map<ZoomFilterMode, ZoomFilterModeInfo>;

  static constexpr uint32_t DEFAULT_ZOOM_MID_X = 16;
  static constexpr uint32_t DEFAULT_ZOOM_MID_Y = 1;
  static constexpr int DEFAULT_TRAN_LERP_INCREMENT = 0x7f;
  static constexpr float DEFAULT_SWITCH_MULT = 29.0F / 30.0F;
  static constexpr float DEFAULT_MAX_SPEED_COEFF = 2.01F;
  static constexpr float MAX_MAX_SPEED_COEFF = 4.01F;
  ZoomFilterSettings m_filterSettings;
  const UTILS::MATH::ConditionalWeights<ZoomFilterMode> m_weightedFilterEvents;

  bool m_filterEffectsSettingsHaveChanged = false;

  [[nodiscard]] auto GetNewRandomMode() const -> ZoomFilterMode;
  [[nodiscard]] auto GetSpeedCoefficientsEffect() -> std::shared_ptr<ISpeedCoefficientsEffect>&;

  enum class ZoomMidpointEvents
  {
    BOTTOM_MID_POINT,
    RIGHT_MID_POINT,
    LEFT_MID_POINT,
    CENTRE_MID_POINT,
    TOP_LEFT_QUARTER_MID_POINT,
    BOTTOM_RIGHT_QUARTER_MID_POINT,
    _num // unused and must be last
  };
  const UTILS::MATH::Weights<ZoomMidpointEvents> m_zoomMidpointWeights;
  [[nodiscard]] auto IsZoomMidpointInTheMiddle() const -> bool;
  auto SetAnyRandomZoomMidpoint(bool allowEdgePoints) -> void;
  [[nodiscard]] auto GetWeightRandomMidPoint(bool allowEdgePoints) const -> ZoomMidpointEvents;
  [[nodiscard]] static auto IsEdgeMidPoint(ZoomMidpointEvents midPointEvent) -> bool;
  auto SetMaxSpeedCoeff() -> void;
};

inline auto FilterSettingsService::GetFilterSettings() const -> const ZoomFilterSettings&
{
  return m_filterSettings;
}

inline auto FilterSettingsService::GetCurrentFilterMode() const -> ZoomFilterMode
{
  return m_filterMode;
}

inline auto FilterSettingsService::GetCurrentFilterModeName() const -> const std::string_view&
{
  return m_filterModeData.at(m_filterMode).name;
}

inline auto FilterSettingsService::GetPreviousFilterMode() const -> ZoomFilterMode
{
  return m_previousFilterMode;
}

inline auto FilterSettingsService::GetPreviousFilterModeName() const -> const std::string_view&
{
  return m_filterModeData.at(m_previousFilterMode).name;
}

inline auto FilterSettingsService::GetFilterSettings() -> ZoomFilterSettings&
{
  return m_filterSettings;
}

inline auto FilterSettingsService::GetPluginInfo() const -> const PluginInfo&
{
  return m_goomInfo;
}

inline auto FilterSettingsService::GetGoomRand() const -> const UTILS::MATH::IGoomRand&
{
  return m_goomRand;
}

inline auto FilterSettingsService::HaveEffectsSettingsChangedSinceLastUpdate() const -> bool
{
  return m_filterEffectsSettingsHaveChanged;
}

inline auto FilterSettingsService::HasFilterModeChangedSinceLastUpdate() const -> bool
{
  return m_filterModeAtLastUpdate != m_filterMode;
}

inline auto FilterSettingsService::GetROVitesse() const -> const Vitesse&
{
  return m_filterSettings.filterEffectsSettings.vitesse;
}

inline auto FilterSettingsService::GetRWVitesse() -> Vitesse&
{
  m_filterEffectsSettingsHaveChanged = true;
  return m_filterSettings.filterEffectsSettings.vitesse;
}

inline auto FilterSettingsService::ChangeMilieu() -> void
{
  m_filterEffectsSettingsHaveChanged = true;
  SetMaxSpeedCoeff();
  SetRandomZoomMidpoint();
}

inline auto FilterSettingsService::SetFilterMode(const ZoomFilterMode filterMode) -> void
{
  m_filterEffectsSettingsHaveChanged = true;

  m_previousFilterMode = m_filterMode;
  m_filterMode = filterMode;

  SetRandomSettingsForNewFilterMode();
}

inline auto FilterSettingsService::SetNewRandomFilter() -> void
{
  m_filterEffectsSettingsHaveChanged = true;

  m_previousFilterMode = m_filterMode;
  m_filterMode = GetNewRandomMode();

  SetRandomSettingsForNewFilterMode();
}

inline auto FilterSettingsService::TurnOffRotation() -> void
{
  if (!m_filterSettings.filterEffectsSettings.rotationEffect)
  {
    return;
  }
  m_filterEffectsSettingsHaveChanged = true;
  m_filterSettings.filterEffectsSettings.rotationEffect = false;
}

inline auto FilterSettingsService::MultiplyRotation(const float factor) -> void
{
  if (!m_filterSettings.filterEffectsSettings.rotationEffect)
  {
    return;
  }
  m_filterEffectsSettingsHaveChanged = true;
  m_filterSettings.filterEffectsSettings.rotationAdjustments.SetMultiplyFactor(
      factor, FILTER_EFFECTS::RotationAdjustments::AdjustmentType::INSTEAD_OF_RANDOM);
}

inline auto FilterSettingsService::ToggleRotationDirection() -> void
{
  if (!m_filterSettings.filterEffectsSettings.rotationEffect)
  {
    return;
  }

  m_filterEffectsSettingsHaveChanged = true;
  m_filterSettings.filterEffectsSettings.rotationAdjustments.Toggle(
      FILTER_EFFECTS::RotationAdjustments::AdjustmentType::INSTEAD_OF_RANDOM);
}

inline auto FilterSettingsService::SetTranLerpIncrement(const int32_t value) -> void
{
  m_filterSettings.filterBufferSettings.tranLerpIncrement = value;
}

inline auto FilterSettingsService::SetDefaultTranLerpIncrement() -> void
{
  SetTranLerpIncrement(DEFAULT_TRAN_LERP_INCREMENT);
}

inline auto FilterSettingsService::MultiplyTranLerpIncrement(const int32_t factor) -> void
{
  m_filterSettings.filterBufferSettings.tranLerpIncrement *= factor;
}

inline auto FilterSettingsService::SetTranLerpToMaxSwitchMult(const float value) -> void
{
  m_filterSettings.filterBufferSettings.tranLerpToMaxSwitchMult = value;
}

inline auto FilterSettingsService::SetTranLerpToMaxDefaultSwitchMult() -> void
{
  SetTranLerpToMaxSwitchMult(DEFAULT_SWITCH_MULT);
}

} // namespace FILTER_FX
} // namespace GOOM