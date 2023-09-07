#pragma once

#include "after_effects/after_effects_states.h"
#include "after_effects/after_effects_types.h"
#include "after_effects/the_effects/rotation.h"
#include "filter_settings.h"
#include "filter_speed.h"
#include "goom/goom_config.h"
#include "goom/goom_types.h"
#include "goom/point2d.h"
#include "normalized_coords.h"
#include "utils/enum_utils.h"
#include "utils/math/goom_rand_base.h"

#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace GOOM
{
class Pixel;
class PluginInfo;
}

namespace GOOM::FILTER_FX
{

enum class ZoomFilterMode : UnderlyingEnumType
{
  AMULET_MODE = 0,
  CRYSTAL_BALL_MODE0,
  CRYSTAL_BALL_MODE1,
  DISTANCE_FIELD_MODE0,
  DISTANCE_FIELD_MODE1,
  DISTANCE_FIELD_MODE2,
  EXP_RECIPROCAL_MODE,
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
  WAVE_SQ_DIST_ANGLE_EFFECT_MODE0,
  WAVE_SQ_DIST_ANGLE_EFFECT_MODE1,
  WAVE_ATAN_ANGLE_EFFECT_MODE0,
  WAVE_ATAN_ANGLE_EFFECT_MODE1,
  Y_ONLY_MODE,
  _num // unused, and marks the enum end
};

class FilterSettingsService
{
public:
  struct ZoomFilterModeInfo
  {
    std::string_view name;
    std::shared_ptr<IZoomAdjustmentEffect> zoomAdjustmentEffect{};
    AFTER_EFFECTS::AfterEffectsStates::AfterEffectsProbabilities afterEffectsProbabilities;
  };
  using FilterModeEnumMap =
      UTILS::RuntimeEnumMap<ZoomFilterMode, FilterSettingsService::ZoomFilterModeInfo>;
  using CreateZoomAdjustmentEffectFunc =
      std::function<std::shared_ptr<IZoomAdjustmentEffect>(ZoomFilterMode filterMode,
                                                           const UTILS::MATH::IGoomRand& goomRand,
                                                           const std::string& resourcesDirectory)>;
  // TODO(glk) - Visual Studio doesn't like a trailing return type in above function definition.

  FilterSettingsService(const GOOM::PluginInfo& goomInfo,
                        const UTILS::MATH::IGoomRand& goomRand,
                        const std::string& resourcesDirectory,
                        const CreateZoomAdjustmentEffectFunc& createZoomAdjustmentEffect);
  FilterSettingsService(const FilterSettingsService&) noexcept = delete;
  FilterSettingsService(FilterSettingsService&&) noexcept      = delete;
  virtual ~FilterSettingsService() noexcept;
  auto operator=(const FilterSettingsService&) -> FilterSettingsService& = delete;
  auto operator=(FilterSettingsService&&) -> FilterSettingsService&      = delete;

  auto Start() -> void;
  auto NewCycle() -> void;

  auto NotifyUpdatedFilterEffectsSettings() -> void;
  [[nodiscard]] auto HasFilterModeChangedSinceLastUpdate() const -> bool;

  [[nodiscard]] auto GetCurrentFilterMode() const -> ZoomFilterMode;
  [[nodiscard]] auto GetCurrentFilterModeName() const -> const std::string_view&;
  [[nodiscard]] auto GetPreviousFilterModeName() const -> const std::string_view&;

  [[nodiscard]] auto GetFilterSettings() const -> const FilterSettings&;
  [[nodiscard]] auto GetFilterSettings() -> FilterSettings&;
  [[nodiscard]] auto GetROVitesse() const -> const Vitesse&;
  [[nodiscard]] auto GetRWVitesse() -> Vitesse&;

  auto SetNewRandomFilter() -> void;
  auto ResetRandomFilterMultiplierEffect() -> void;
  auto ResetRandomAfterEffects() -> void;
  auto ChangeMilieu() -> void;
  auto TurnOffRotation() -> void;
  auto MultiplyRotation(float factor) -> void;
  auto ToggleRotationDirection() -> void;

  static constexpr auto DEFAULT_TRAN_LERP_INCREMENT = 0.002F;
  static constexpr auto DEFAULT_SWITCH_MULT         = 0.03F;
  auto SetTransformBufferLerpIncrement(float value) -> void;
  auto SetDefaultTransformBufferLerpIncrement() -> void;
  auto MultiplyTransformBufferLerpIncrement(float factor) -> void;
  auto SetTransformBufferLerpToMaxLerp(float value) -> void;
  auto SetDefaultTransformBufferLerpToMaxLerp() -> void;
  [[nodiscard]] auto GetNextTransformBufferLerpFactor(float currentLerpFactor) const noexcept
      -> float;

protected:
  void SetFilterMode(ZoomFilterMode filterMode);
  [[nodiscard]] auto GetPluginInfo() const -> const PluginInfo&;
  [[nodiscard]] auto GetGoomRand() const -> const UTILS::MATH::IGoomRand&;
  virtual auto SetDefaultSettings() -> void;
  virtual auto SetRandomZoomMidpoint() -> void;
  virtual auto SetFilterModeRandomViewport() -> void;
  virtual auto SetFilterModeRandomEffects() -> void;
  virtual auto SetFilterModeAfterEffects() -> void;
  virtual auto SetRandomizedAfterEffects() -> void;
  virtual auto SetWaveModeAfterEffects() -> void;
  virtual auto UpdateFilterSettingsFromAfterEffects() -> void;

private:
  const PluginInfo* m_goomInfo;
  const UTILS::MATH::IGoomRand* m_goomRand;
  Point2dInt m_screenCentre;
  std::string m_resourcesDirectory;
  std::unique_ptr<AFTER_EFFECTS::AfterEffectsStates> m_randomizedAfterEffects;

  ZoomFilterMode m_filterMode             = ZoomFilterMode::NORMAL_MODE;
  ZoomFilterMode m_previousFilterMode     = ZoomFilterMode::NORMAL_MODE;
  ZoomFilterMode m_filterModeAtLastUpdate = ZoomFilterMode::NORMAL_MODE;
  auto SetRandomSettingsForNewFilterMode() -> void;

  FilterModeEnumMap m_filterModeData;

  static constexpr auto DEFAULT_ZOOM_MID_X                             = 16U;
  static constexpr auto DEFAULT_ZOOM_MID_Y                             = 1U;
  static constexpr auto DEFAULT_FILTER_VIEWPORT                        = Viewport{};
  static constexpr auto DEFAULT_MAX_ZOOM_ADJUSTMENT                    = 2.01F;
  static constexpr auto DEFAULT_BASE_ZOOM_ADJUSTMENT_FACTOR_MULTIPLIER = 1.0F;
  static constexpr auto DEFAULT_AFTER_EFFECTS_VELOCITY_CONTRIBUTION    = 0.5F;
  static constexpr auto MAX_MAX_ZOOM_ADJUSTMENT                        = 4.01F;
  static constexpr auto DEFAULT_MULTIPLIER_EFFECT_IS_ACTIVE            = false;
  static constexpr auto MIN_MULTIPLIER_EFFECT_FREQ                     = 0.5F;
  static constexpr auto MAX_MULTIPLIER_EFFECT_FREQ                     = 20.0F;
  static constexpr auto DEFAULT_MULTIPLIER_EFFECT_X_FREQ               = 1.0F;
  static constexpr auto DEFAULT_MULTIPLIER_EFFECT_Y_FREQ               = 1.0F;
  static constexpr auto MIN_MULTIPLIER_EFFECT_AMPLITUDE                = 0.0F;
  static constexpr auto MAX_MULTIPLIER_EFFECT_AMPLITUDE                = 0.03F;
  static constexpr auto DEFAULT_MULTIPLIER_EFFECT_X_AMPLITUDE          = 0.0F;
  static constexpr auto DEFAULT_MULTIPLIER_EFFECT_Y_AMPLITUDE          = 0.0F;
  static constexpr auto DEFAULT_LERP_ZOOM_ADJUSTMENT_TO_COORDS         = 0.5F;
  static constexpr auto PROB_ACTIVE_MULTIPLIER_EFFECT                  = 0.3F;
  static constexpr auto PROB_MULTIPLIER_EFFECT_FREQUENCIES_EQUAL       = 0.95F;
  static constexpr auto PROB_MULTIPLIER_EFFECT_AMPLITUDES_EQUAL        = 0.95F;
  FilterSettings m_filterSettings;
  UTILS::MATH::ConditionalWeights<ZoomFilterMode> m_weightedFilterEvents;
  [[nodiscard]] auto GetNewRandomMode() const -> ZoomFilterMode;
  [[nodiscard]] auto GetZoomAdjustmentEffect() -> std::shared_ptr<IZoomAdjustmentEffect>&;
  auto SetMaxZoomAdjustment() -> void;
  auto SetBaseZoomAdjustmentFactorMultiplier() noexcept -> void;
  auto SetAfterEffectsVelocityMultiplier() noexcept -> void;

  enum class ZoomMidpointEvents : UnderlyingEnumType
  {
    BOTTOM_MID_POINT,
    TOP_MID_POINT,
    LEFT_MID_POINT,
    RIGHT_MID_POINT,
    CENTRE_MID_POINT,
    BOTTOM_LEFT_QUARTER_MID_POINT,
    TOP_LEFT_QUARTER_MID_POINT,
    BOTTOM_RIGHT_QUARTER_MID_POINT,
    TOP_RIGHT_QUARTER_MID_POINT,
    _num // unused, and marks the enum end
  };
  static constexpr auto BOTTOM_MID_POINT_WEIGHT               = 03.0F;
  static constexpr auto TOP_MID_POINT_WEIGHT                  = 03.0F;
  static constexpr auto LEFT_MID_POINT_WEIGHT                 = 02.0F;
  static constexpr auto RIGHT_MID_POINT_WEIGHT                = 02.0F;
  static constexpr auto CENTRE_MID_POINT_WEIGHT               = 18.0F;
  static constexpr auto TOP_LEFT_QUARTER_MID_POINT_WEIGHT     = 10.0F;
  static constexpr auto TOP_RIGHT_QUARTER_MID_POINT_WEIGHT    = 10.0F;
  static constexpr auto BOTTOM_LEFT_QUARTER_MID_POINT_WEIGHT  = 10.0F;
  static constexpr auto BOTTOM_RIGHT_QUARTER_MID_POINT_WEIGHT = 10.0F;
  UTILS::MATH::Weights<ZoomMidpointEvents> m_zoomMidpointWeights;
  [[nodiscard]] auto IsZoomMidpointInTheMiddle() const -> bool;
  [[nodiscard]] auto IsFilterModeAWaveMode() const -> bool;
  auto SetAnyRandomZoomMidpoint(bool allowEdgePoints) -> void;
  [[nodiscard]] auto GetWeightRandomMidPoint(bool allowEdgePoints) const -> ZoomMidpointEvents;
  [[nodiscard]] static auto IsEdgeMidPoint(ZoomMidpointEvents midPointEvent) -> bool;
};

inline auto FilterSettingsService::GetFilterSettings() const -> const FilterSettings&
{
  return m_filterSettings;
}

inline auto FilterSettingsService::GetFilterSettings() -> FilterSettings&
{
  return m_filterSettings;
}

inline auto FilterSettingsService::GetCurrentFilterMode() const -> ZoomFilterMode
{
  return m_filterMode;
}

inline auto FilterSettingsService::GetCurrentFilterModeName() const -> const std::string_view&
{
  return m_filterModeData[m_filterMode].name;
}

inline auto FilterSettingsService::GetPreviousFilterModeName() const -> const std::string_view&
{
  return m_filterModeData[m_previousFilterMode].name;
}

inline auto FilterSettingsService::GetPluginInfo() const -> const PluginInfo&
{
  return *m_goomInfo;
}

inline auto FilterSettingsService::GetGoomRand() const -> const UTILS::MATH::IGoomRand&
{
  return *m_goomRand;
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
  m_filterSettings.filterEffectsSettingsHaveChanged = true;
  return m_filterSettings.filterEffectsSettings.vitesse;
}

inline auto FilterSettingsService::ChangeMilieu() -> void
{
  m_filterSettings.filterEffectsSettingsHaveChanged = true;
  SetMaxZoomAdjustment();
  ResetRandomFilterMultiplierEffect();
  SetBaseZoomAdjustmentFactorMultiplier();
  SetAfterEffectsVelocityMultiplier();
  SetRandomZoomMidpoint();
}

inline auto FilterSettingsService::SetMaxZoomAdjustment() -> void
{
  static constexpr auto MIN_SPEED_FACTOR = 0.5F;
  static constexpr auto MAX_SPEED_FACTOR = 1.0F;
  m_filterSettings.filterEffectsSettings.maxZoomAdjustment =
      m_goomRand->GetRandInRange(MIN_SPEED_FACTOR, MAX_SPEED_FACTOR) * MAX_MAX_ZOOM_ADJUSTMENT;
}

inline auto FilterSettingsService::SetFilterMode(const ZoomFilterMode filterMode) -> void
{
  m_filterSettings.filterEffectsSettingsHaveChanged = true;

  m_previousFilterMode = m_filterMode;
  m_filterMode         = filterMode;

  SetRandomSettingsForNewFilterMode();
}

inline auto FilterSettingsService::SetNewRandomFilter() -> void
{
  m_filterSettings.filterEffectsSettingsHaveChanged = true;

  m_previousFilterMode = m_filterMode;
  m_filterMode         = GetNewRandomMode();

  SetRandomSettingsForNewFilterMode();
}

inline auto FilterSettingsService::SetRandomSettingsForNewFilterMode() -> void
{
  SetDefaultSettings();
  SetRandomZoomMidpoint();
  SetFilterModeRandomEffects();
  SetFilterModeRandomViewport();
  ResetRandomFilterMultiplierEffect();
  SetFilterModeAfterEffects();
  UpdateFilterSettingsFromAfterEffects();
}

inline auto FilterSettingsService::TurnOffRotation() -> void
{
  if (not m_filterSettings.filterEffectsSettings.afterEffectsSettings
              .isActive[AFTER_EFFECTS::AfterEffectsTypes::ROTATION])
  {
    return;
  }
  m_filterSettings.filterEffectsSettingsHaveChanged = true;
  m_filterSettings.filterEffectsSettings.afterEffectsSettings
      .isActive[AFTER_EFFECTS::AfterEffectsTypes::ROTATION] = false;
}

inline auto FilterSettingsService::MultiplyRotation(const float factor) -> void
{
  if (not m_filterSettings.filterEffectsSettings.afterEffectsSettings
              .isActive[AFTER_EFFECTS::AfterEffectsTypes::ROTATION])
  {
    return;
  }
  m_filterSettings.filterEffectsSettingsHaveChanged = true;
  m_filterSettings.filterEffectsSettings.afterEffectsSettings.rotationAdjustments.SetMultiplyFactor(
      factor, AFTER_EFFECTS::RotationAdjustments::AdjustmentType::INSTEAD_OF_RANDOM);
}

inline auto FilterSettingsService::ToggleRotationDirection() -> void
{
  if (not m_filterSettings.filterEffectsSettings.afterEffectsSettings
              .isActive[AFTER_EFFECTS::AfterEffectsTypes::ROTATION])
  {
    return;
  }

  m_filterSettings.filterEffectsSettingsHaveChanged = true;
  m_filterSettings.filterEffectsSettings.afterEffectsSettings.rotationAdjustments.Toggle(
      AFTER_EFFECTS::RotationAdjustments::AdjustmentType::INSTEAD_OF_RANDOM);
}

inline auto FilterSettingsService::SetTransformBufferLerpIncrement(const float value) -> void
{
  Expects(value >= 0.0F);
  m_filterSettings.transformBufferLerpData.lerpIncrement = value;
}

inline auto FilterSettingsService::SetDefaultTransformBufferLerpIncrement() -> void
{
  SetTransformBufferLerpIncrement(DEFAULT_TRAN_LERP_INCREMENT);
}

inline auto FilterSettingsService::MultiplyTransformBufferLerpIncrement(const float factor) -> void
{
  m_filterSettings.transformBufferLerpData.lerpIncrement *= factor;
}

inline auto FilterSettingsService::SetTransformBufferLerpToMaxLerp(const float value) -> void
{
  Expects(value >= 0.0F);
  Expects(value <= 1.0F);
  m_filterSettings.transformBufferLerpData.lerpToMaxLerp = value;
}

inline auto FilterSettingsService::SetDefaultTransformBufferLerpToMaxLerp() -> void
{
  SetTransformBufferLerpToMaxLerp(DEFAULT_SWITCH_MULT);
}

} // namespace GOOM::FILTER_FX
