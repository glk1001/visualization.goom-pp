module;

#include "goom/debug_with_println.h"

#undef NO_LOGGING
#include "goom/goom_logger.h"

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

export module Goom.FilterFx.FilterSettingsService;

import Goom.FilterFx.AfterEffects.TheEffects.Rotation;
import Goom.FilterFx.AfterEffects.AfterEffectsStates;
import Goom.FilterFx.AfterEffects.AfterEffectsTypes;
import Goom.FilterFx.FilterEffects.ZoomAdjustmentEffect;
import Goom.FilterFx.GpuFilterEffects.GpuZoomFilterEffect;
import Goom.FilterFx.FilterConsts;
import Goom.FilterFx.FilterModes;
import Goom.FilterFx.FilterSettings;
import Goom.FilterFx.FilterSpeed;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.DebuggingLogger;
import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;
import Goom.PluginInfo;

using GOOM::FILTER_FX::NormalizedCoordsConverter;
using GOOM::FILTER_FX::FILTER_EFFECTS::IZoomAdjustmentEffect;
using GOOM::FILTER_FX::GPU_FILTER_EFFECTS::IGpuZoomFilterEffect;
using GOOM::UTILS::NUM;
using GOOM::UTILS::RuntimeEnumMap;
using GOOM::UTILS::MATH::ConditionalWeights;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::NumberRange;
using GOOM::UTILS::MATH::Weights;

export namespace GOOM::FILTER_FX
{

class FilterSettingsService
{
public:
  static constexpr auto MIN_ZOOM_ADJUSTMENT         = -4.01F;
  static constexpr auto MAX_MAX_ZOOM_ADJUSTMENT     = 4.01F;
  static constexpr auto DEFAULT_MAX_ZOOM_ADJUSTMENT = 2.01F;

  struct ZoomFilterModeInfo
  {
    std::string_view name;
    std::shared_ptr<IZoomAdjustmentEffect> zoomAdjustmentEffect;
    AFTER_EFFECTS::AfterEffectsStates::AfterEffectsProbabilities afterEffectsProbabilities;
  };
  using FilterModeEnumMap              = RuntimeEnumMap<ZoomFilterMode, ZoomFilterModeInfo>;
  using CreateZoomAdjustmentEffectFunc = std::function<std::unique_ptr<IZoomAdjustmentEffect>(
      ZoomFilterMode filterMode, const GoomRand& goomRand, const std::string& resourcesDirectory)>;
  // TODO(glk) - Visual Studio doesn't like a trailing return type in above function definition.
  struct GpuZoomFilterModeInfo
  {
    std::string_view name;
    std::shared_ptr<IGpuZoomFilterEffect> gpuZoomFilterEffect;
  };
  using GpuFilterModeEnumMap          = RuntimeEnumMap<GpuZoomFilterMode, GpuZoomFilterModeInfo>;
  using CreateGpuZoomFilterEffectFunc = std::function<std::unique_ptr<IGpuZoomFilterEffect>(
      GpuZoomFilterMode gpuFilterMode, const GoomRand& goomRand)>;
  using OkToChangeFilterSettings      = std::function<bool()>;
  using OkToChangeGpuFilterSettings   = std::function<bool()>;

  FilterSettingsService(const PluginInfo& goomInfo,
                        const GoomRand& goomRand,
                        const std::string& resourcesDirectory,
                        const NormalizedCoordsConverter& normalizedCoordsConverter,
                        const CreateZoomAdjustmentEffectFunc& createZoomAdjustmentEffect,
                        const CreateGpuZoomFilterEffectFunc& createGpuZoomFilterEffect,
                        const OkToChangeFilterSettings& okToChangeFilterSettings,
                        const OkToChangeGpuFilterSettings& okToChangeGpuFilterSettings);
  FilterSettingsService(const FilterSettingsService&) noexcept = delete;
  FilterSettingsService(FilterSettingsService&&) noexcept      = delete;
  virtual ~FilterSettingsService() noexcept;
  auto operator=(const FilterSettingsService&) -> FilterSettingsService& = delete;
  auto operator=(FilterSettingsService&&) -> FilterSettingsService&      = delete;

  auto Start() -> void;
  auto NewCycle() noexcept -> void;

  auto NotifyUpdatedFilterEffectsSettings() noexcept -> void;
  auto NotifyUpdatedGpuFilterEffectsSettings() noexcept -> void;

  [[nodiscard]] auto GetCurrentFilterMode() const noexcept -> ZoomFilterMode;
  [[nodiscard]] auto GetCurrentFilterModeName() const noexcept -> const std::string_view&;
  [[nodiscard]] auto GetPreviousFilterModeName() const noexcept -> const std::string_view&;

  [[nodiscard]] auto GetCurrentGpuFilterMode() const noexcept -> GpuZoomFilterMode;
  [[nodiscard]] auto GetCurrentGPUFilterModeName() const noexcept -> const std::string_view&;
  [[nodiscard]] auto GetPreviousGPUFilterModeName() const noexcept -> const std::string_view&;

  [[nodiscard]] auto GetFilterSettings() const noexcept -> const FilterSettings&;
  [[nodiscard]] auto GetROVitesse() const noexcept -> const Vitesse&;

  // The following methods will change filter settings.
  [[nodiscard]] auto GetRWVitesse() noexcept -> Vitesse&;
  [[nodiscard]] auto SetNewRandomFilter() -> bool;
  [[nodiscard]] auto SetNewRandomGpuFilter(int32_t maxTimeToNextFilterModeChange) -> bool;
  [[nodiscard]] auto ChangeMilieu() -> bool;
  [[nodiscard]] auto TurnOffRotation() noexcept -> bool;
  [[nodiscard]] auto MultiplyRotation(float factor) noexcept -> bool;
  [[nodiscard]] auto ToggleRotationDirection() noexcept -> bool;

  auto SetRandomwTextureWrapType() noexcept -> void;

  static constexpr auto DEFAULT_TRAN_LERP_INCREMENT = 0.002F;
  auto ResetTransformBufferLerpData() noexcept -> void;
  auto SetTransformBufferLerpIncrement(float value) noexcept -> void;
  auto SetDefaultTransformBufferLerpIncrement() noexcept -> void;
  auto MultiplyTransformBufferLerpIncrement(float factor) noexcept -> void;
  auto SetTransformBufferLerpToEnd() noexcept -> void;

  static constexpr auto DEFAULT_NUM_GPU_SRCE_DEST_LERP_FACTOR_STEPS = 100U;
  static constexpr auto DEFAULT_NUM_GPU_MIDPOINT_LERP_STEPS         = 500U;
  auto SetDefaultGpuLerpIncrement() noexcept -> void;
  auto ResetGpuLerpFactorUpABit() noexcept -> void;
  auto ResetGpuLerpFactorDownABit() noexcept -> void;
  auto SpeedUpGpuLerpFactorABit() noexcept -> void;
  auto SlowDownGpuLerpFactorABit() noexcept -> void;

protected:
  [[nodiscard]] auto GetFilterSettings() noexcept -> FilterSettings&;
  auto SetFilterMode(ZoomFilterMode filterMode) noexcept -> void;
  auto SetGpuFilterMode(GpuZoomFilterMode gpuFilterMode) noexcept -> void;
  [[nodiscard]] auto GetPluginInfo() const noexcept -> const PluginInfo&;
  [[nodiscard]] auto GetGoomRand() const noexcept -> const GoomRand&;
  virtual auto SetDefaultFilterSettings() -> void;
  virtual auto SetDefaultGpuFilterSettings() -> void;
  virtual auto SetRandomZoomMidpoint() -> void;
  virtual auto SetFilterModeRandomEffects() -> void;
  virtual auto SetGpuFilterModeRandomEffects() -> void;
  virtual auto SetFilterModeAfterEffects() -> void;
  virtual auto SetRandomizedAfterEffects() -> void;
  virtual auto SetWaveModeAfterEffects() -> void;
  virtual auto UpdateFilterSettingsFromAfterEffects() -> void;

private:
  const PluginInfo* m_goomInfo;
  const GoomRand* m_goomRand;
  Point2dInt m_screenCentre;
  std::string m_resourcesDirectory;
  const NormalizedCoordsConverter* m_normalizedCoordsConverter;
  AFTER_EFFECTS::AfterEffectsStates m_randomizedAfterEffects;
  Weights<TextureWrapType> m_weightedTextureWrapTypes;

  ZoomFilterMode m_filterMode         = ZoomFilterMode::NORMAL_MODE;
  ZoomFilterMode m_previousFilterMode = ZoomFilterMode::NORMAL_MODE;
  FilterModeEnumMap m_filterModeData;
  ConditionalWeights<ZoomFilterMode> m_weightedFilterEvents;

  GpuZoomFilterMode m_gpuFilterMode         = GpuZoomFilterMode::GPU_NONE_MODE;
  GpuZoomFilterMode m_previousGpuFilterMode = GpuZoomFilterMode::GPU_NONE_MODE;
  GpuFilterModeEnumMap m_gpuFilterModeData;
  Weights<GpuZoomFilterMode> m_weightedGpuFilterEvents;

  FilterSettings m_filterSettings;
  [[nodiscard]] auto GetCentreZoomMidpoint() const noexcept -> Point2dFlt;
  auto UpdateGpuZoomMidpoint() -> void;

  [[nodiscard]] auto CanChangeFilterSettings() const noexcept -> bool;
  [[nodiscard]] auto CanChangeGpuFilterSettings() const noexcept -> bool;
  auto SetRandomSettingsForNewFilterMode() -> void;
  auto SetRandomSettingsForNewGpuFilterMode() -> void;

  static constexpr auto DEFAULT_ZOOM_MID_X                             = 16U;
  static constexpr auto DEFAULT_ZOOM_MID_Y                             = 1U;
  static constexpr auto DEFAULT_BASE_ZOOM_ADJUSTMENT_FACTOR_MULTIPLIER = 1.0F;
  static constexpr auto DEFAULT_AFTER_EFFECTS_VELOCITY_CONTRIBUTION    = 0.5F;
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
  [[nodiscard]] auto GetNewRandomFilterMode() const -> ZoomFilterMode;
  [[nodiscard]] auto GetNewRandomGpuFilterMode() const -> GpuZoomFilterMode;
  auto ResetRandomFilterMultiplierEffect() -> void;
  auto ResetRandomAfterEffects() -> void;
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
  [[nodiscard]] static auto IsAllowedEdgePoints(ZoomFilterMode filterMode) noexcept -> bool;
  auto SetAnyRandomZoomMidpoint(bool allowEdgePoints) noexcept -> void;
  [[nodiscard]] auto GetWeightRandomMidPoint(bool allowEdgePoints) const noexcept
      -> ZoomMidpointEvents;
  [[nodiscard]] static auto IsEdgeMidPoint(ZoomMidpointEvents midPointEvent) noexcept -> bool;
};

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

inline auto FilterSettingsService::GetCurrentGPUFilterModeName() const noexcept
    -> const std::string_view&
{
  return m_gpuFilterModeData[m_gpuFilterMode].name;
}
inline auto FilterSettingsService::GetPreviousGPUFilterModeName() const noexcept
    -> const std::string_view&
{
  return m_gpuFilterModeData[m_previousGpuFilterMode].name;
}

inline auto FilterSettingsService::GetCurrentGpuFilterMode() const noexcept -> GpuZoomFilterMode
{
  return m_gpuFilterMode;
}

inline auto FilterSettingsService::GetPluginInfo() const noexcept -> const PluginInfo&
{
  return *m_goomInfo;
}

inline auto FilterSettingsService::GetGoomRand() const noexcept -> const GoomRand&
{
  return *m_goomRand;
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

inline auto FilterSettingsService::CanChangeFilterSettings() const noexcept -> bool
{
  return m_filterSettings.filterEffectsSettings.okToChangeFilterSettings();
}

inline auto FilterSettingsService::CanChangeGpuFilterSettings() const noexcept -> bool
{
  if (not m_filterSettings.gpuFilterEffectsSettings.okToChangeGpuFilterSettings())
  {
#ifdef DEBUG_WITH_PRINTLN
    std::println("Not ok to change gpu filter settings.");
#endif

    return false;
  }

  return true;
}

inline auto FilterSettingsService::ChangeMilieu() -> bool
{
  if (not CanChangeFilterSettings())
  {
    return false;
  }

  m_filterSettings.filterEffectsSettingsHaveChanged = true;
  SetMaxZoomAdjustment();
  ResetRandomFilterMultiplierEffect();
  SetBaseZoomAdjustmentFactorMultiplier();
  SetAfterEffectsVelocityMultiplier();
  SetRandomZoomMidpoint(); // NOTE: Gpu is OK with midpoint change anytime
  ResetRandomAfterEffects();

  return true;
}

inline auto FilterSettingsService::SetMaxZoomAdjustment() -> void
{
  static constexpr auto SPEED_FACTOR_RANGE = NumberRange{0.5F, 1.0F};
  m_filterSettings.filterEffectsSettings.maxZoomAdjustment =
      m_goomRand->GetRandInRange<SPEED_FACTOR_RANGE>() * MAX_MAX_ZOOM_ADJUSTMENT;
}

inline auto FilterSettingsService::SetNewRandomFilter() -> bool
{
  if (not CanChangeFilterSettings())
  {
    return false;
  }

  m_filterSettings.filterEffectsSettingsHaveChanged = true;
  m_previousFilterMode                              = m_filterMode;
  m_filterMode                                      = GetNewRandomFilterMode();

  SetRandomSettingsForNewFilterMode();

  return true;
}

inline auto FilterSettingsService::SetNewRandomGpuFilter(
    const int32_t maxTimeToNextFilterModeChange) -> bool
{
#ifdef DEBUG_WITH_PRINTLN
  std::println("Check if can set new gpu filter...");
#endif

  if (not CanChangeGpuFilterSettings())
  {
#ifdef DEBUG_WITH_PRINTLN
    std::println("  Cannot set new gpu filter.");
#endif

    return false;
  }

  Expects(maxTimeToNextFilterModeChange > 0);

  m_filterSettings.gpuFilterEffectsSettingsHaveChanged = true;
  m_previousGpuFilterMode                              = m_gpuFilterMode;
  m_gpuFilterMode                                      = GetNewRandomGpuFilterMode();

#ifdef DEBUG_WITH_PRINTLN
  std::println("  Set new gpu filter to {} (prev = {}).",
               UTILS::EnumToString(m_gpuFilterMode),
               UTILS::EnumToString(m_previousGpuFilterMode));
#endif

  auto& gpuFilterEffectsSettings = m_filterSettings.gpuFilterEffectsSettings;

  if constexpr (USE_FORCED_GPU_FILTER_MODE)
  {
    SetDefaultGpuFilterSettings();
    if (m_gpuFilterMode != m_previousGpuFilterMode)
    {
      gpuFilterEffectsSettings.maxTimeToNextFilterModeChange = maxTimeToNextFilterModeChange;
      SetRandomSettingsForNewGpuFilterMode();

#ifdef DEBUG_WITH_PRINTLN
      std::println("  Set new filter params for '{}'. Max filter time = {}.",
                   UTILS::EnumToString(m_gpuFilterMode),
                   maxTimeToNextFilterModeChange);
#endif
    }
  }
  else
  {
    if ((m_gpuFilterMode == m_previousGpuFilterMode) and
        (m_gpuFilterMode != GpuZoomFilterMode::GPU_NONE_MODE))
    {

#ifdef DEBUG_WITH_PRINTLN
      std::println("  WARN: Wrong weighted filter returned. Should not be same as previous: '{}'.",
                   UTILS::EnumToString(m_gpuFilterMode));
#endif
      LogWarn(UTILS::GetGoomLogger(),
              "Wrong weighted filter returned. Should not be same as previous: '{}'.",
              UTILS::EnumToString(m_gpuFilterMode));
    }

    gpuFilterEffectsSettings.maxTimeToNextFilterModeChange = maxTimeToNextFilterModeChange;
    if (m_gpuFilterMode != GpuZoomFilterMode::GPU_NONE_MODE)
    {
      gpuFilterEffectsSettings.srceDestLerpFactor.ResetValues(0.0F, 1.0F);
    }
    SetRandomSettingsForNewGpuFilterMode();

#ifdef DEBUG_WITH_PRINTLN
    std::println("  Set new filter params for '{}'. Max filter time = {}.",
                 UTILS::EnumToString(m_gpuFilterMode),
                 maxTimeToNextFilterModeChange);
#endif
  }

#ifdef DEBUG_WITH_PRINTLN
  std::println("  m_filterSettings.gpuFilterEffectsSettingsHaveChanged = {}.",
               m_filterSettings.gpuFilterEffectsSettingsHaveChanged);
#endif

  return true;
}

inline auto FilterSettingsService::SetRandomSettingsForNewFilterMode() -> void
{
  SetDefaultFilterSettings();
  SetRandomZoomMidpoint();
  SetFilterModeRandomEffects();
  ResetRandomFilterMultiplierEffect();
  SetFilterModeAfterEffects();
  UpdateFilterSettingsFromAfterEffects();
}

inline auto FilterSettingsService::SetRandomSettingsForNewGpuFilterMode() -> void
{
  SetDefaultGpuFilterSettings();
  SetGpuFilterModeRandomEffects();
}

inline auto FilterSettingsService::TurnOffRotation() noexcept -> bool
{
  if (not CanChangeFilterSettings())
  {
    return false;
  }

  if (not m_filterSettings.filterEffectsSettings.afterEffectsSettings
              .isActive[AFTER_EFFECTS::AfterEffectsTypes::ROTATION])
  {
    return true;
  }
  m_filterSettings.filterEffectsSettingsHaveChanged = true;
  m_filterSettings.filterEffectsSettings.afterEffectsSettings
      .isActive[AFTER_EFFECTS::AfterEffectsTypes::ROTATION] = false;

  return true;
}

inline auto FilterSettingsService::MultiplyRotation(const float factor) noexcept -> bool
{
  if (not CanChangeFilterSettings())
  {
    return false;
  }

  if (not m_filterSettings.filterEffectsSettings.afterEffectsSettings
              .isActive[AFTER_EFFECTS::AfterEffectsTypes::ROTATION])
  {
    return true;
  }
  m_filterSettings.filterEffectsSettingsHaveChanged = true;
  m_filterSettings.filterEffectsSettings.afterEffectsSettings.rotationAdjustments.SetMultiplyFactor(
      factor, AFTER_EFFECTS::RotationAdjustments::AdjustmentType::INSTEAD_OF_RANDOM);

  return true;
}

inline auto FilterSettingsService::ToggleRotationDirection() noexcept -> bool
{
  if (not CanChangeFilterSettings())
  {
    return false;
  }

  if (not m_filterSettings.filterEffectsSettings.afterEffectsSettings
              .isActive[AFTER_EFFECTS::AfterEffectsTypes::ROTATION])
  {
    return true;
  }

  m_filterSettings.filterEffectsSettingsHaveChanged = true;
  m_filterSettings.filterEffectsSettings.afterEffectsSettings.rotationAdjustments.Toggle(
      AFTER_EFFECTS::RotationAdjustments::AdjustmentType::INSTEAD_OF_RANDOM);

  return true;
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

inline auto FilterSettingsService::ResetGpuLerpFactorUpABit() noexcept -> void
{
#ifdef DEBUG_WITH_PRINTLN
  const auto oldGpuLerpFactor = m_filterSettings.gpuFilterEffectsSettings.gpuLerpFactor();
#endif

  m_filterSettings.gpuFilterEffectsSettings.gpuLerpFactor.GoUpABit();

#ifdef DEBUG_WITH_PRINTLN
  std::println("ResetGpuLerpDataUpABit: oldGpuLerpFactor = {:.2f}, gpuLerpFactor = {:.2f}.",
               oldGpuLerpFactor,
               m_filterSettings.gpuFilterEffectsSettings.gpuLerpFactor());
#endif
}

inline auto FilterSettingsService::ResetGpuLerpFactorDownABit() noexcept -> void
{
#ifdef DEBUG_WITH_PRINTLN
  const auto oldGpuLerpFactor = m_filterSettings.gpuFilterEffectsSettings.gpuLerpFactor();
#endif

  m_filterSettings.gpuFilterEffectsSettings.gpuLerpFactor.GoDownABit();

#ifdef DEBUG_WITH_PRINTLN
  std::println("ResetGpuLerpDataDownABit: oldGpuLerpFactor = {:.2f}, gpuLerpFactor = {:.2f}.",
               oldGpuLerpFactor,
               m_filterSettings.gpuFilterEffectsSettings.gpuLerpFactor());
#endif
}

inline auto FilterSettingsService::SetDefaultGpuLerpIncrement() noexcept -> void
{
  m_filterSettings.gpuFilterEffectsSettings.gpuLerpFactor.ResetNumStepsToDefault();

#ifdef DEBUG_WITH_PRINTLN
  std::println("SetDefaultGpuLerpIncrement: gpuLerpFactor = {}.",
               m_filterSettings.gpuFilterEffectsSettings.gpuLerpFactor());
#endif
}

inline auto FilterSettingsService::SpeedUpGpuLerpFactorABit() noexcept -> void
{
#ifdef DEBUG_WITH_PRINTLN
  const auto oldNumSteps = m_filterSettings.gpuFilterEffectsSettings.gpuLerpFactor.GetNumSteps();
#endif

  m_filterSettings.gpuFilterEffectsSettings.gpuLerpFactor.SpeedUpABit();

#ifdef DEBUG_WITH_PRINTLN
  std::println("SpeedUpGpuLerpABit: gpuLerpFactor = {}, oldNumSteps = {}, newNumSteps = {}.",
               m_filterSettings.gpuFilterEffectsSettings.gpuLerpFactor(),
               oldNumSteps,
               m_filterSettings.gpuFilterEffectsSettings.gpuLerpFactor.GetNumSteps());
#endif
}

inline auto FilterSettingsService::SlowDownGpuLerpFactorABit() noexcept -> void
{
#ifdef DEBUG_WITH_PRINTLN
  const auto oldNumSteps = m_filterSettings.gpuFilterEffectsSettings.gpuLerpFactor.GetNumSteps();
#endif

  m_filterSettings.gpuFilterEffectsSettings.gpuLerpFactor.SlowDownABit();

#ifdef DEBUG_WITH_PRINTLN
  std::println("SlowDownGpuLerpABit: gpuLerpFactor = {}, oldNumSteps = {}, newNumSteps = {}.",
               m_filterSettings.gpuFilterEffectsSettings.gpuLerpFactor(),
               oldNumSteps,
               m_filterSettings.gpuFilterEffectsSettings.gpuLerpFactor.GetNumSteps());
#endif
}

} // namespace GOOM::FILTER_FX
