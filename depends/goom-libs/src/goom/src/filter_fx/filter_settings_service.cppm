module;

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

export module Goom.FilterFx.FilterSettingsService;

import Goom.FilterFx.AfterEffects.TheEffects.Rotation;
import Goom.FilterFx.AfterEffects.AfterEffectsStates;
import Goom.FilterFx.AfterEffects.AfterEffectsTypes;
import Goom.FilterFx.FilterSettings;
import Goom.FilterFx.FilterSpeed;
import Goom.FilterFx.ZoomAdjustmentEffect;
import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;
import Goom.PluginInfo;

using GOOM::UTILS::NUM;
using GOOM::UTILS::RuntimeEnumMap;
using GOOM::UTILS::MATH::ConditionalWeights;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::NumberRange;
using GOOM::UTILS::MATH::Weights;

export namespace GOOM::FILTER_FX
{

enum class ZoomFilterMode : UnderlyingEnumType
{
  AMULET_MODE = 0,
  COMPLEX_RATIONAL_MODE,
  CRYSTAL_BALL_MODE0,
  CRYSTAL_BALL_MODE1,
  DISTANCE_FIELD_MODE0,
  DISTANCE_FIELD_MODE1,
  DISTANCE_FIELD_MODE2,
  EXP_RECIPROCAL_MODE,
  FLOW_FIELD_MODE,
  HYPERCOS_MODE0,
  HYPERCOS_MODE1,
  HYPERCOS_MODE2,
  HYPERCOS_MODE3,
  IMAGE_DISPLACEMENT_MODE,
  MOBIUS_MODE,
  NEWTON_MODE,
  NORMAL_MODE,
  PERLIN_NOISE_MODE,
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
};

class FilterSettingsService
{
public:
  struct ZoomFilterModeInfo
  {
    std::string_view name;
    std::shared_ptr<IZoomAdjustmentEffect> zoomAdjustmentEffect;
    AFTER_EFFECTS::AfterEffectsStates::AfterEffectsProbabilities afterEffectsProbabilities;
  };
  using FilterModeEnumMap =
      RuntimeEnumMap<ZoomFilterMode, FilterSettingsService::ZoomFilterModeInfo>;
  using CreateZoomAdjustmentEffectFunc = std::function<std::unique_ptr<IZoomAdjustmentEffect>(
      ZoomFilterMode filterMode, const GoomRand& goomRand, const std::string& resourcesDirectory)>;
  // TODO(glk) - Visual Studio doesn't like a trailing return type in above function definition.

  FilterSettingsService(const PluginInfo& goomInfo,
                        const GoomRand& goomRand,
                        const std::string& resourcesDirectory,
                        const CreateZoomAdjustmentEffectFunc& createZoomAdjustmentEffect);
  FilterSettingsService(const FilterSettingsService&) noexcept = delete;
  FilterSettingsService(FilterSettingsService&&) noexcept      = delete;
  virtual ~FilterSettingsService() noexcept;
  auto operator=(const FilterSettingsService&) -> FilterSettingsService& = delete;
  auto operator=(FilterSettingsService&&) -> FilterSettingsService&      = delete;

  auto Start() -> void;
  auto NewCycle() noexcept -> void;

  auto NotifyUpdatedFilterEffectsSettings() noexcept -> void;
  [[nodiscard]] auto HasFilterModeChangedSinceLastUpdate() const noexcept -> bool;

  [[nodiscard]] auto GetCurrentFilterMode() const noexcept -> ZoomFilterMode;
  [[nodiscard]] auto GetCurrentFilterModeName() const noexcept -> const std::string_view&;
  [[nodiscard]] auto GetPreviousFilterModeName() const noexcept -> const std::string_view&;

  [[nodiscard]] auto GetFilterSettings() const noexcept -> const FilterSettings&;
  [[nodiscard]] auto GetROVitesse() const noexcept -> const Vitesse&;
  [[nodiscard]] auto GetRWVitesse() noexcept -> Vitesse&;

  auto SetNewRandomFilter() -> void;
  auto ResetRandomFilterMultiplierEffect() -> void;
  auto ResetRandomAfterEffects() -> void;
  auto ChangeMilieu() -> void;
  auto TurnOffRotation() noexcept -> void;
  auto MultiplyRotation(float factor) noexcept -> void;
  auto ToggleRotationDirection() noexcept -> void;

  static constexpr auto DEFAULT_TRAN_LERP_INCREMENT = 0.002F;
  auto ResetTransformBufferLerpData() noexcept -> void;
  auto SetTransformBufferLerpIncrement(float value) noexcept -> void;
  auto SetDefaultTransformBufferLerpIncrement() noexcept -> void;
  auto MultiplyTransformBufferLerpIncrement(float factor) noexcept -> void;
  auto SetTransformBufferLerpToEnd() noexcept -> void;

protected:
  [[nodiscard]] auto GetFilterSettings() noexcept -> FilterSettings&;
  auto SetFilterMode(ZoomFilterMode filterMode) noexcept -> void;
  [[nodiscard]] auto GetPluginInfo() const noexcept -> const PluginInfo&;
  [[nodiscard]] auto GetGoomRand() const noexcept -> const GoomRand&;
  virtual auto SetDefaultSettings() -> void;
  virtual auto SetRandomZoomMidpoint() -> void;
  virtual auto SetFilterModeRandomEffects() -> void;
  virtual auto SetFilterModeAfterEffects() -> void;
  virtual auto SetRandomizedAfterEffects() -> void;
  virtual auto SetWaveModeAfterEffects() -> void;
  virtual auto UpdateFilterSettingsFromAfterEffects() -> void;

private:
  const PluginInfo* m_goomInfo;
  const GoomRand* m_goomRand;
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
  static constexpr auto DEFAULT_MAX_ZOOM_ADJUSTMENT                    = 2.01F;
  static constexpr auto DEFAULT_BASE_ZOOM_ADJUSTMENT_FACTOR_MULTIPLIER = 1.0F;
  static constexpr auto DEFAULT_AFTER_EFFECTS_VELOCITY_CONTRIBUTION    = 0.5F;
  static constexpr auto MAX_MAX_ZOOM_ADJUSTMENT                        = 4.01F;
  static constexpr auto DEFAULT_MULTIPLIER_EFFECT_IS_ACTIVE            = false;
  static constexpr auto MULTIPLIER_EFFECT_FREQ_RANGE                   = NumberRange{0.5F, 20.0F};
  static constexpr auto DEFAULT_MULTIPLIER_EFFECT_X_FREQ               = 1.0F;
  static constexpr auto DEFAULT_MULTIPLIER_EFFECT_Y_FREQ               = 1.0F;
  static constexpr auto MULTIPLIER_EFFECT_AMPLITUDE_RANGE              = NumberRange{0.0F, 0.03F};
  static constexpr auto DEFAULT_MULTIPLIER_EFFECT_X_AMPLITUDE          = 0.0F;
  static constexpr auto DEFAULT_MULTIPLIER_EFFECT_Y_AMPLITUDE          = 0.0F;
  static constexpr auto DEFAULT_LERP_ZOOM_ADJUSTMENT_TO_COORDS         = 0.5F;
  static constexpr auto PROB_ACTIVE_MULTIPLIER_EFFECT                  = 0.3F;
  static constexpr auto PROB_MULTIPLIER_EFFECT_FREQUENCIES_EQUAL       = 0.95F;
  static constexpr auto PROB_MULTIPLIER_EFFECT_AMPLITUDES_EQUAL        = 0.95F;
  FilterSettings m_filterSettings;
  ConditionalWeights<ZoomFilterMode> m_weightedFilterEvents;
  [[nodiscard]] auto GetNewRandomMode() const -> ZoomFilterMode;
  [[nodiscard]] auto GetZoomAdjustmentEffect() -> std::shared_ptr<IZoomAdjustmentEffect>&;
  auto SetMaxZoomAdjustment() -> void;
  auto SetBaseZoomAdjustmentFactorMultiplier() noexcept -> void;
  auto SetAfterEffectsVelocityMultiplier() noexcept -> void;

  enum class ZoomMidpointEvents : UnderlyingEnumType
  {
    CENTRE_MID_POINT,
    BOTTOM_LEFT_QUARTER_MID_POINT,
    TOP_LEFT_QUARTER_MID_POINT,
    BOTTOM_RIGHT_QUARTER_MID_POINT,
    TOP_RIGHT_QUARTER_MID_POINT,
    // NOTE: Put the 'edge' points here, last!
    BOTTOM_MID_POINT,
    TOP_MID_POINT,
    LEFT_MID_POINT,
    RIGHT_MID_POINT,
  };
  static constexpr auto LAST_EDGE_MIDPOINT =
      static_cast<ZoomMidpointEvents>(NUM<ZoomMidpointEvents> - 4);
  static constexpr auto LAST_NON_EDGE_MIDPOINT =
      static_cast<ZoomMidpointEvents>(std::to_underlying(LAST_EDGE_MIDPOINT) - 1);
  static constexpr auto BOTTOM_MID_POINT_WEIGHT               = 03.0F;
  static constexpr auto TOP_MID_POINT_WEIGHT                  = 03.0F;
  static constexpr auto LEFT_MID_POINT_WEIGHT                 = 02.0F;
  static constexpr auto RIGHT_MID_POINT_WEIGHT                = 02.0F;
  static constexpr auto CENTRE_MID_POINT_WEIGHT               = 18.0F;
  static constexpr auto TOP_LEFT_QUARTER_MID_POINT_WEIGHT     = 10.0F;
  static constexpr auto TOP_RIGHT_QUARTER_MID_POINT_WEIGHT    = 10.0F;
  static constexpr auto BOTTOM_LEFT_QUARTER_MID_POINT_WEIGHT  = 10.0F;
  static constexpr auto BOTTOM_RIGHT_QUARTER_MID_POINT_WEIGHT = 10.0F;
  Weights<ZoomMidpointEvents> m_zoomMidpointWeights;
  [[nodiscard]] auto IsZoomMidpointInTheMiddle() const noexcept -> bool;
  [[nodiscard]] auto IsFilterModeAWaveMode() const noexcept -> bool;
  [[nodiscard]] auto IsAllowedEdgePoints(ZoomFilterMode filterMode) const noexcept -> bool;
  auto SetAnyRandomZoomMidpoint(bool allowEdgePoints) noexcept -> void;
  [[nodiscard]] auto GetWeightRandomMidPoint(bool allowEdgePoints) const noexcept
      -> ZoomMidpointEvents;
  [[nodiscard]] static auto IsEdgeMidPoint(ZoomMidpointEvents midPointEvent) noexcept -> bool;
};

auto GetFilterModeName(ZoomFilterMode filterMode) noexcept -> std::string_view;

} // namespace GOOM::FILTER_FX

namespace GOOM::FILTER_FX
{

inline auto FilterSettingsService::GetFilterSettings() const noexcept -> const FilterSettings&
{
  return m_filterSettings;
}

inline auto FilterSettingsService::GetFilterSettings() noexcept -> FilterSettings&
{
  return m_filterSettings;
}

inline auto FilterSettingsService::GetCurrentFilterMode() const noexcept -> ZoomFilterMode
{
  return m_filterMode;
}

inline auto FilterSettingsService::GetCurrentFilterModeName() const noexcept
    -> const std::string_view&
{
  return m_filterModeData[m_filterMode].name;
}

inline auto FilterSettingsService::GetPreviousFilterModeName() const noexcept
    -> const std::string_view&
{
  return m_filterModeData[m_previousFilterMode].name;
}

inline auto FilterSettingsService::GetPluginInfo() const noexcept -> const PluginInfo&
{
  return *m_goomInfo;
}

inline auto FilterSettingsService::GetGoomRand() const noexcept -> const GoomRand&
{
  return *m_goomRand;
}

inline auto FilterSettingsService::HasFilterModeChangedSinceLastUpdate() const noexcept -> bool
{
  return m_filterModeAtLastUpdate != m_filterMode;
}

inline auto FilterSettingsService::GetROVitesse() const noexcept -> const Vitesse&
{
  return m_filterSettings.filterEffectsSettings.vitesse;
}

inline auto FilterSettingsService::GetRWVitesse() noexcept -> Vitesse&
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
  static constexpr auto SPEED_FACTOR_RANGE = NumberRange{0.5F, 1.0F};
  m_filterSettings.filterEffectsSettings.maxZoomAdjustment =
      m_goomRand->GetRandInRange<SPEED_FACTOR_RANGE>() * MAX_MAX_ZOOM_ADJUSTMENT;
}

inline auto FilterSettingsService::SetFilterMode(const ZoomFilterMode filterMode) noexcept -> void
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
  ResetRandomFilterMultiplierEffect();
  SetFilterModeAfterEffects();
  UpdateFilterSettingsFromAfterEffects();
}

inline auto FilterSettingsService::TurnOffRotation() noexcept -> void
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

inline auto FilterSettingsService::MultiplyRotation(const float factor) noexcept -> void
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

inline auto FilterSettingsService::ToggleRotationDirection() noexcept -> void
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

inline auto FilterSettingsService::ResetTransformBufferLerpData() noexcept -> void
{
  m_filterSettings.transformBufferLerpData.Reset();
}

inline auto FilterSettingsService::SetTransformBufferLerpIncrement(const float value) noexcept
    -> void
{
  Expects(value >= 0.0F);
  m_filterSettings.transformBufferLerpData.SetIncrement(value);
}

inline auto FilterSettingsService::SetDefaultTransformBufferLerpIncrement() noexcept -> void
{
  SetTransformBufferLerpIncrement(DEFAULT_TRAN_LERP_INCREMENT);
}

inline auto FilterSettingsService::MultiplyTransformBufferLerpIncrement(const float factor) noexcept
    -> void
{
  m_filterSettings.transformBufferLerpData.SetIncrement(
      m_filterSettings.transformBufferLerpData.GetIncrement() * factor);
}

inline auto FilterSettingsService::SetTransformBufferLerpToEnd() noexcept -> void
{
  m_filterSettings.transformBufferLerpData.SetLerpToEnd();
}

} // namespace GOOM::FILTER_FX
