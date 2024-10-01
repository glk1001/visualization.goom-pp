module;

//#define DEBUG_GPU_FILTERS
#ifdef DEBUG_GPU_FILTERS
#include <print>
#endif

#include <map> // TODO(glk): Clang needs this otherwise link fails
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

module Goom.FilterFx.FilterSettingsService;

import :FilterWeights;
import Goom.FilterFx.AfterEffects.TheEffects.Rotation;
import Goom.FilterFx.AfterEffects.AfterEffectsStates;
import Goom.FilterFx.AfterEffects.AfterEffectsTypes;
import Goom.FilterFx.FilterEffects.ZoomVectorEffects;
import Goom.FilterFx.FilterUtils.GoomLerpData;
import Goom.FilterFx.FilterConsts;
import Goom.FilterFx.FilterModes;
import Goom.FilterFx.FilterSettings;
import Goom.FilterFx.FilterSpeed;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Lerper;
import Goom.Utils.Math.Misc;
import Goom.PluginInfo;

namespace GOOM::FILTER_FX
{

using AFTER_EFFECTS::AfterEffectsStates;
using AFTER_EFFECTS::AfterEffectsTypes;
using AFTER_EFFECTS::HypercosOverlayMode;
using AFTER_EFFECTS::RotationAdjustments;
using FILTER_EFFECTS::ZoomVectorEffects;
using FILTER_FX::NormalizedCoordsConverter;
using FILTER_UTILS::GoomLerpData;
using UTILS::NUM;
using UTILS::MATH::GoomRand;
using UTILS::MATH::I_HALF;
using UTILS::MATH::I_QUARTER;
using UTILS::MATH::I_THREE_QUARTERS;
using UTILS::MATH::Lerper;
using UTILS::MATH::NumberRange;
using UTILS::MATH::U_HALF;
using UTILS::MATH::UNIT_RANGE;
using UTILS::MATH::Weights;

using enum GpuZoomFilterMode;
using enum ZoomFilterMode;

namespace
{

constexpr auto PROB_CRYSTAL_BALL_IN_MIDDLE   = 0.8F;
constexpr auto PROB_EXP_RECIPROCAL_IN_MIDDLE = 0.6F;
constexpr auto PROB_FLOW_FIELD_IN_MIDDLE     = 0.4F;
constexpr auto PROB_WAVE_IN_MIDDLE           = 0.5F;
constexpr auto PROB_CHANGE_SPEED             = 0.5F;
constexpr auto PROB_REVERSE_SPEED            = 0.5F;

[[nodiscard]] auto GetFilterModeData(
    const GoomRand& goomRand,
    const std::string& resourcesDirectory,
    const FilterSettingsService::CreateZoomAdjustmentEffectFunc& createZoomAdjustmentEffect)
    -> FilterSettingsService::FilterModeEnumMap
{
  auto filterModeVec = std::vector<FilterSettingsService::FilterModeEnumMap::KeyValue>{};

  for (auto i = 0U; i < NUM<ZoomFilterMode>; ++i)
  {
    const auto filterMode = static_cast<ZoomFilterMode>(i);

    // clang-format off
    filterModeVec.emplace_back(
        filterMode,
        FilterSettingsService::ZoomFilterModeInfo{
            .name = GetFilterModeName(filterMode),
            .zoomAdjustmentEffect      = createZoomAdjustmentEffect(filterMode,
                                                                    goomRand,
                                                                    resourcesDirectory),
            .afterEffectsProbabilities = {
                .hypercosModeWeights = Weights<HypercosOverlayMode>{goomRand,
                                                                    GetHypercosWeights(filterMode)},
                .probabilities       = GetAfterEffectsProbability(filterMode)
            }
        }
    );
    // clang-format on
  }

  return FilterSettingsService::FilterModeEnumMap::Make(std::move(filterModeVec));
}

[[nodiscard]] auto GetGpuFilterModeData(
    const GoomRand& goomRand,
    const FilterSettingsService::CreateGpuZoomFilterEffectFunc& createGpuZoomFilterEffect)
    -> FilterSettingsService::GpuFilterModeEnumMap
{
  auto gpuFilterModeVec = std::vector<FilterSettingsService::GpuFilterModeEnumMap::KeyValue>{};

  for (auto i = 0U; i < NUM<GpuZoomFilterMode>; ++i)
  {
    const auto gpuFilterMode = static_cast<GpuZoomFilterMode>(i);

    gpuFilterModeVec.emplace_back(
        gpuFilterMode,
        FilterSettingsService::GpuZoomFilterModeInfo{
            .name                = GetGpuFilterModeName(gpuFilterMode),
            .gpuZoomFilterEffect = createGpuZoomFilterEffect(gpuFilterMode, goomRand)});
  }

  return FilterSettingsService::GpuFilterModeEnumMap::Make(std::move(gpuFilterModeVec));
}

} // namespace

auto FilterSettingsService::GetCentreZoomMidpoint() const noexcept -> Point2dFlt
{
  const auto centreCoords = m_normalizedCoordsConverter->OtherToNormalizedCoords(
      m_goomInfo->GetDimensions().GetCentrePoint());

  return centreCoords.GetFltCoords();
}

FilterSettingsService::FilterSettingsService(const PluginInfo& goomInfo,
                                             const GoomRand& goomRand,
                                             const std::string& resourcesDirectory,
                                             const NormalizedCoordsConverter&
                                                 normalizedCoordsConverter,
                                             const CreateZoomAdjustmentEffectFunc&
                                                 createZoomAdjustmentEffect,
                                             const CreateGpuZoomFilterEffectFunc&
                                                 createGpuZoomFilterEffect,
                                             const OkToChangeFilterSettings&
                                                   okToChangeFilterSettings,
                                             const OkToChangeGpuFilterSettings&
                                                   okToChangeGpuFilterSettings)
  : m_goomInfo{&goomInfo},
    m_goomRand{&goomRand},
    m_screenCentre{goomInfo.GetDimensions().GetCentrePoint()},
    m_resourcesDirectory{resourcesDirectory},
    m_normalizedCoordsConverter{&normalizedCoordsConverter},
    m_randomizedAfterEffects{
        goomInfo.GetTime(),
        goomRand,
        GetRepeatAfterEffectsProbability(),
        GetAfterEffectsOffTime()},
    m_filterModeData{GetFilterModeData(goomRand,
                                       m_resourcesDirectory,
                                       createZoomAdjustmentEffect)},
    m_weightedFilterEvents{GetWeightedFilterEvents(goomRand)},
    m_gpuFilterModeData{GetGpuFilterModeData(goomRand, createGpuZoomFilterEffect)},
    m_weightedGpuFilterEvents{GetWeightedGpuFilterEvents(goomRand)},
    m_filterSettings{
        .filterEffectsSettingsHaveChanged = false,
        .filterEffectsSettings = {
           .vitesse = Vitesse{},
           .maxZoomAdjustment = DEFAULT_MAX_ZOOM_ADJUSTMENT,
           .baseZoomAdjustmentFactorMultiplier = DEFAULT_BASE_ZOOM_ADJUSTMENT_FACTOR_MULTIPLIER,
           .afterEffectsVelocityMultiplier = DEFAULT_AFTER_EFFECTS_VELOCITY_CONTRIBUTION,
           .zoomAdjustmentEffect = nullptr,
           .okToChangeFilterSettings = okToChangeFilterSettings,
           .filterZoomMidpointHasChanged = false,
           .zoomMidpoint={.x = DEFAULT_ZOOM_MID_X, .y = DEFAULT_ZOOM_MID_Y},
           .filterMultiplierEffectsSettings = {
               .isActive = DEFAULT_MULTIPLIER_EFFECT_IS_ACTIVE,
               .xFreq = DEFAULT_MULTIPLIER_EFFECT_X_FREQ,
               .yFreq = DEFAULT_MULTIPLIER_EFFECT_Y_FREQ,
               .xAmplitude = DEFAULT_MULTIPLIER_EFFECT_X_AMPLITUDE,
               .yAmplitude = DEFAULT_MULTIPLIER_EFFECT_Y_AMPLITUDE,
               .lerpZoomAdjustmentToCoords = DEFAULT_LERP_ZOOM_ADJUSTMENT_TO_COORDS
           },
           .afterEffectsSettings = {
               .hypercosOverlayMode = HypercosOverlayMode::NONE,
               .isActive = DEFAULT_AFTER_EFFECTS_STATES,
               .rotationAdjustments = RotationAdjustments{},
            }
        },
        .gpuFilterEffectsSettingsHaveChanged = false,
        .gpuFilterEffectsSettings = {
            .gpuZoomFilterEffect = nullptr,
            .maxTimeToNextFilterModeChange = 1,
            .okToChangeGpuFilterSettings = okToChangeGpuFilterSettings,
            .gpuLerpFactor = {DEFAULT_NUM_GPU_LERP_FACTOR_STEPS,
                              0.0F, 1.0F,
                              Lerper<float>::LerperType::CONTINUOUS},
            .srceDestLerpFactor = {DEFAULT_NUM_GPU_SRCE_DEST_LERP_FACTOR_STEPS,
                              1.0F, 1.0F,
                              Lerper<float>::LerperType::SINGLE},
            .midpoint = {DEFAULT_NUM_GPU_MIDPOINT_LERP_STEPS,
                         GetCentreZoomMidpoint(),
                         GetCentreZoomMidpoint()},
        },
        .transformBufferLerpData = GoomLerpData{DEFAULT_TRAN_LERP_INCREMENT, true},
    },
    m_zoomMidpointWeights{
        goomRand,
        {
            {.key = ZoomMidpointEvents::BOTTOM_MID_POINT,  .weight = BOTTOM_MID_POINT_WEIGHT},
            {.key = ZoomMidpointEvents::TOP_MID_POINT,     .weight = TOP_MID_POINT_WEIGHT},
            {.key = ZoomMidpointEvents::LEFT_MID_POINT,    .weight = LEFT_MID_POINT_WEIGHT},
            {.key = ZoomMidpointEvents::RIGHT_MID_POINT,   .weight = RIGHT_MID_POINT_WEIGHT},
            {.key = ZoomMidpointEvents::CENTRE_MID_POINT,  .weight = CENTRE_MID_POINT_WEIGHT},
            {.key = ZoomMidpointEvents::BOTTOM_LEFT_QUARTER_MID_POINT,
                                                   .weight = BOTTOM_LEFT_QUARTER_MID_POINT_WEIGHT},
            {.key = ZoomMidpointEvents::TOP_LEFT_QUARTER_MID_POINT,
                                                   .weight = TOP_LEFT_QUARTER_MID_POINT_WEIGHT},
            {.key = ZoomMidpointEvents::BOTTOM_RIGHT_QUARTER_MID_POINT,
                                                   .weight = BOTTOM_RIGHT_QUARTER_MID_POINT_WEIGHT},
            {.key = ZoomMidpointEvents::TOP_RIGHT_QUARTER_MID_POINT,
                                                    .weight = TOP_RIGHT_QUARTER_MID_POINT_WEIGHT},
        }
    }
{
  static_assert(DEFAULT_MULTIPLIER_EFFECT_X_FREQ >= MULTIPLIER_EFFECT_FREQ_RANGE.min);
  static_assert(DEFAULT_MULTIPLIER_EFFECT_X_FREQ <= MULTIPLIER_EFFECT_FREQ_RANGE.max);
  static_assert(DEFAULT_MULTIPLIER_EFFECT_Y_FREQ >= MULTIPLIER_EFFECT_FREQ_RANGE.min);
  static_assert(DEFAULT_MULTIPLIER_EFFECT_Y_FREQ <= MULTIPLIER_EFFECT_FREQ_RANGE.max);
  static_assert(DEFAULT_MULTIPLIER_EFFECT_X_AMPLITUDE >= MULTIPLIER_EFFECT_AMPLITUDE_RANGE.min);
  static_assert(DEFAULT_MULTIPLIER_EFFECT_X_AMPLITUDE <= MULTIPLIER_EFFECT_AMPLITUDE_RANGE.max);
  static_assert(DEFAULT_MULTIPLIER_EFFECT_Y_AMPLITUDE >= MULTIPLIER_EFFECT_AMPLITUDE_RANGE.min);
  static_assert(DEFAULT_MULTIPLIER_EFFECT_Y_AMPLITUDE <= MULTIPLIER_EFFECT_AMPLITUDE_RANGE.max);
  static_assert(DEFAULT_LERP_ZOOM_ADJUSTMENT_TO_COORDS >= 0.0F);
  static_assert(DEFAULT_LERP_ZOOM_ADJUSTMENT_TO_COORDS <= 1.0F);
}

FilterSettingsService::~FilterSettingsService() noexcept = default;

auto FilterSettingsService::GetNewRandomFilterMode() const -> ZoomFilterMode
{
  if constexpr (USE_FORCED_FILTER_MODE)
  {
    return FORCED_FILTER_MODE;
  }
  return m_weightedFilterEvents.GetRandomWeighted(m_filterMode);
}

auto FilterSettingsService::GetNewRandomGpuFilterMode() const -> GpuZoomFilterMode
{
  if constexpr (USE_FORCED_GPU_FILTER_MODE)
  {
    return FORCED_GPU_FILTER_MODE;
  }

  if (m_gpuFilterMode == GPU_NONE_MODE)
  {
    // GPU_NONE_MODE is a special case - repeats allowed.
    return m_weightedGpuFilterEvents.GetRandomWeighted();
  }
  return m_weightedGpuFilterEvents.GetRandomWeighted(m_gpuFilterMode);
}

auto FilterSettingsService::Start() -> void
{
  [[maybe_unused]] const auto dontCare1 = SetNewRandomFilter();

  m_previousGpuFilterMode                                           = GPU_NONE_MODE;
  static constexpr auto APPROX_MAX_TIME_BETWEEN_FILTER_MODE_CHANGES = 300;
  [[maybe_unused]] const auto dontCare2 =
      SetNewRandomGpuFilter(APPROX_MAX_TIME_BETWEEN_FILTER_MODE_CHANGES);
}

auto FilterSettingsService::NewCycle() noexcept -> void
{
  m_filterSettings.transformBufferLerpData.Update();

  m_filterSettings.gpuFilterEffectsSettings.gpuLerpFactor.Increment();
  m_filterSettings.gpuFilterEffectsSettings.srceDestLerpFactor.Increment();
  m_filterSettings.gpuFilterEffectsSettings.midpoint.Increment();
}

auto FilterSettingsService::NotifyUpdatedFilterEffectsSettings() noexcept -> void
{
  m_filterSettings.filterEffectsSettingsHaveChanged = false;

  m_randomizedAfterEffects.CheckForPendingOffTimers();
}

auto FilterSettingsService::NotifyUpdatedGpuFilterEffectsSettings() noexcept -> void
{
  m_filterSettings.gpuFilterEffectsSettingsHaveChanged = false;
}

auto FilterSettingsService::SetDefaultFilterSettings() -> void
{
  m_filterSettings.filterEffectsSettings.zoomAdjustmentEffect =
      m_filterModeData[m_filterMode].zoomAdjustmentEffect;
  m_filterSettings.filterEffectsSettings.zoomMidpoint = m_screenCentre;
  m_filterSettings.filterEffectsSettings.vitesse.SetDefault();

  m_randomizedAfterEffects.SetDefaults();
}

auto FilterSettingsService::SetDefaultGpuFilterSettings() -> void
{
  m_filterSettings.gpuFilterEffectsSettings.gpuZoomFilterEffect =
      m_gpuFilterModeData[m_gpuFilterMode].gpuZoomFilterEffect;
}

auto FilterSettingsService::SetFilterModeRandomEffects() -> void
{
  m_filterSettings.filterEffectsSettings.zoomAdjustmentEffect->SetRandomParams();
}

auto FilterSettingsService::SetGpuFilterModeRandomEffects() -> void
{
  m_filterSettings.gpuFilterEffectsSettings.gpuZoomFilterEffect->SetRandomParams();
}

auto FilterSettingsService::SetFilterModeAfterEffects() -> void
{
  SetRandomizedAfterEffects();
  SetWaveModeAfterEffects();
}

auto FilterSettingsService::ResetRandomFilterMultiplierEffect() -> void
{
  auto& multiplierEffectsSettings =
      m_filterSettings.filterEffectsSettings.filterMultiplierEffectsSettings;

  if (not m_goomRand->ProbabilityOf<PROB_ACTIVE_MULTIPLIER_EFFECT>())
  {
    multiplierEffectsSettings.isActive = false;
  }
  else
  {
    multiplierEffectsSettings.isActive = true;

    multiplierEffectsSettings.xFreq = m_goomRand->GetRandInRange<MULTIPLIER_EFFECT_FREQ_RANGE>();
    if (m_goomRand->ProbabilityOf<PROB_MULTIPLIER_EFFECT_FREQUENCIES_EQUAL>())
    {
      multiplierEffectsSettings.yFreq = multiplierEffectsSettings.xFreq;
    }
    else
    {
      multiplierEffectsSettings.yFreq = m_goomRand->GetRandInRange<MULTIPLIER_EFFECT_FREQ_RANGE>();
    }

    multiplierEffectsSettings.xAmplitude =
        m_goomRand->GetRandInRange<MULTIPLIER_EFFECT_AMPLITUDE_RANGE>();
    if (m_goomRand->ProbabilityOf<PROB_MULTIPLIER_EFFECT_AMPLITUDES_EQUAL>())
    {
      multiplierEffectsSettings.yAmplitude = multiplierEffectsSettings.xAmplitude;
    }
    else
    {
      multiplierEffectsSettings.yAmplitude =
          m_goomRand->GetRandInRange<MULTIPLIER_EFFECT_AMPLITUDE_RANGE>();
    }
  }

  multiplierEffectsSettings.lerpZoomAdjustmentToCoords = m_goomRand->GetRandInRange<UNIT_RANGE>();
}

auto FilterSettingsService::ResetRandomAfterEffects() -> void
{
  const auto& modeInfo = m_filterModeData[m_filterMode];
  m_randomizedAfterEffects.ResetStandardStates(modeInfo.afterEffectsProbabilities);
  m_filterSettings.filterEffectsSettingsHaveChanged = true;
}

auto FilterSettingsService::SetRandomizedAfterEffects() -> void
{
  const auto& modeInfo = m_filterModeData[m_filterMode];

  m_randomizedAfterEffects.ResetAllStates(modeInfo.afterEffectsProbabilities);

  m_filterSettings.filterEffectsSettings.afterEffectsSettings.rotationAdjustments.SetMultiplyFactor(
      modeInfo.afterEffectsProbabilities.probabilities[AfterEffectsTypes::ROTATION],
      RotationAdjustments::AdjustmentType::AFTER_RANDOM);
}

auto FilterSettingsService::SetWaveModeAfterEffects() -> void
{
  if ((m_filterMode != WAVE_SQ_DIST_ANGLE_EFFECT_MODE0) and
      (m_filterMode != WAVE_SQ_DIST_ANGLE_EFFECT_MODE1) and
      (m_filterMode != WAVE_ATAN_ANGLE_EFFECT_MODE0) and
      (m_filterMode != WAVE_ATAN_ANGLE_EFFECT_MODE1))
  {
    return;
  }

  m_randomizedAfterEffects.TurnPlaneEffectOn();

  auto& filterEffectsSettings = m_filterSettings.filterEffectsSettings;
  filterEffectsSettings.vitesse.SetReverseVitesse(m_goomRand->ProbabilityOf<PROB_REVERSE_SPEED>());
  if (m_goomRand->ProbabilityOf<PROB_CHANGE_SPEED>())
  {
    filterEffectsSettings.vitesse.SetVitesse(
        U_HALF * (Vitesse::DEFAULT_SPEED + filterEffectsSettings.vitesse.GetVitesse()));
  }
}

auto FilterSettingsService::UpdateFilterSettingsFromAfterEffects() -> void
{
  m_filterSettings.filterEffectsSettingsHaveChanged = true;
  m_randomizedAfterEffects.UpdateAfterEffectsSettingsFromStates(
      m_filterSettings.filterEffectsSettings.afterEffectsSettings);
}

auto FilterSettingsService::SetBaseZoomAdjustmentFactorMultiplier() noexcept -> void
{
  if (static constexpr auto PROB_CALM_DOWN = 0.8F; m_goomRand->ProbabilityOf<PROB_CALM_DOWN>())
  {
    m_filterSettings.filterEffectsSettings.baseZoomAdjustmentFactorMultiplier = 1.0F;
    return;
  }

  // TODO(glk) Lerp between old and new?
  static constexpr auto MULTIPLIER_RANGE = NumberRange{0.1F, 5.0F};
  static_assert(ZoomVectorEffects::IsValidMultiplierRange(MULTIPLIER_RANGE));

  m_filterSettings.filterEffectsSettings.baseZoomAdjustmentFactorMultiplier =
      m_goomRand->GetRandInRange<MULTIPLIER_RANGE>();
}

auto FilterSettingsService::SetAfterEffectsVelocityMultiplier() noexcept -> void
{
  static constexpr auto CONTRIBUTION_RANGE = NumberRange{0.1F, 1.0F};

  m_filterSettings.filterEffectsSettings.afterEffectsVelocityMultiplier =
      m_goomRand->GetRandInRange<CONTRIBUTION_RANGE>();
}

auto FilterSettingsService::SetRandomZoomMidpoint() -> void
{
  if constexpr (ALL_AFTER_EFFECTS_TURNED_OFF)
  {
    m_filterSettings.filterEffectsSettings.filterZoomMidpointHasChanged =
        m_filterSettings.filterEffectsSettings.zoomMidpoint != m_screenCentre;
    m_filterSettings.filterEffectsSettings.zoomMidpoint = m_screenCentre;
#ifdef DEBUG_GPU_FILTERS
    std::println("SetRandomZoomMidpoint: after effects turned off: zoomMidpoint = ({}, {}).",
                 m_filterSettings.filterEffectsSettings.zoomMidpoint.x,
                 m_filterSettings.filterEffectsSettings.zoomMidpoint.y);
#endif
    return;
  }
  if (IsZoomMidpointInTheMiddle())
  {
    m_filterSettings.filterEffectsSettings.filterZoomMidpointHasChanged =
        m_filterSettings.filterEffectsSettings.zoomMidpoint != m_screenCentre;
    m_filterSettings.filterEffectsSettings.zoomMidpoint = m_screenCentre;
#ifdef DEBUG_GPU_FILTERS
    std::println("SetRandomZoomMidpoint: zoomMidpoint = ({}, {}).",
                 m_filterSettings.filterEffectsSettings.zoomMidpoint.x,
                 m_filterSettings.filterEffectsSettings.zoomMidpoint.y);
#endif
    return;
  }

  SetAnyRandomZoomMidpoint(IsAllowedEdgePoints(m_filterMode));

  UpdateGpuZoomMidpoint();
}

auto FilterSettingsService::UpdateGpuZoomMidpoint() -> void
{
  if (not m_filterSettings.filterEffectsSettings.filterZoomMidpointHasChanged)
  {
    return;
  }

#ifdef DEBUG_GPU_FILTERS
  std::println("  Updating new gpu midpoint...");
#endif
  const auto& currentMidpoint = m_filterSettings.gpuFilterEffectsSettings.midpoint();
  const auto newMidpoint =
      m_normalizedCoordsConverter
          ->OtherToNormalizedCoords(m_filterSettings.filterEffectsSettings.zoomMidpoint)
          .GetFltCoords();
  m_filterSettings.gpuFilterEffectsSettings.midpoint.ResetValues(currentMidpoint, newMidpoint);

#ifdef DEBUG_GPU_FILTERS
  std::println("  Old midpoint = ({}, {}), new midpoint = ({}, {}).",
               currentMidpoint.x,
               currentMidpoint.y,
               newMidpoint.x,
               newMidpoint.y);
#endif
}

auto FilterSettingsService::IsAllowedEdgePoints(const ZoomFilterMode filterMode) noexcept -> bool
{
  static const auto s_NO_EDGE_POINTS = std::unordered_set{
      EXP_RECIPROCAL_MODE,
      FLOW_FIELD_MODE,
      WAVE_SQ_DIST_ANGLE_EFFECT_MODE0,
      WAVE_SQ_DIST_ANGLE_EFFECT_MODE1,
      WAVE_ATAN_ANGLE_EFFECT_MODE0,
      WAVE_ATAN_ANGLE_EFFECT_MODE1,
  };

  return not s_NO_EDGE_POINTS.contains(filterMode);
}

auto FilterSettingsService::IsZoomMidpointInTheMiddle() const noexcept -> bool
{
  if ((m_filterMode == WATER_MODE) or (m_filterMode == AMULET_MODE))
  {
    return true;
  }

  if (((m_filterMode == CRYSTAL_BALL_MODE0) or (m_filterMode == CRYSTAL_BALL_MODE1)) and
      m_goomRand->ProbabilityOf<PROB_CRYSTAL_BALL_IN_MIDDLE>())
  {
    return true;
  }

  if ((m_filterMode == EXP_RECIPROCAL_MODE) and
      m_goomRand->ProbabilityOf<PROB_EXP_RECIPROCAL_IN_MIDDLE>())
  {
    return true;
  }

  if ((m_filterMode == FLOW_FIELD_MODE) and m_goomRand->ProbabilityOf<PROB_FLOW_FIELD_IN_MIDDLE>())
  {
    return true;
  }

  if (IsFilterModeAWaveMode() and m_goomRand->ProbabilityOf<PROB_WAVE_IN_MIDDLE>())
  {
    return true;
  }

  return false;
}

inline auto FilterSettingsService::IsFilterModeAWaveMode() const noexcept -> bool
{
  if (m_filterMode == WAVE_SQ_DIST_ANGLE_EFFECT_MODE0)
  {
    return true;
  }
  if (m_filterMode == WAVE_SQ_DIST_ANGLE_EFFECT_MODE1)
  {
    return true;
  }
  if (m_filterMode == WAVE_ATAN_ANGLE_EFFECT_MODE0)
  {
    return true;
  }
  if (m_filterMode == WAVE_ATAN_ANGLE_EFFECT_MODE1)
  {
    return true;
  }
  return false;
}

auto FilterSettingsService::GetWeightRandomMidPoint(const bool allowEdgePoints) const noexcept
    -> ZoomMidpointEvents
{
  if (allowEdgePoints)
  {
    return m_zoomMidpointWeights.GetRandomWeighted();
  }

  return m_zoomMidpointWeights.GetRandomWeightedUpTo(LAST_NON_EDGE_MIDPOINT);
}

inline auto FilterSettingsService::IsEdgeMidPoint(const ZoomMidpointEvents midPointEvent) noexcept
    -> bool
{
  static_assert(ZoomMidpointEvents::BOTTOM_MID_POINT >= LAST_EDGE_MIDPOINT);
  static_assert(ZoomMidpointEvents::TOP_MID_POINT >= LAST_EDGE_MIDPOINT);
  static_assert(ZoomMidpointEvents::RIGHT_MID_POINT >= LAST_EDGE_MIDPOINT);
  static_assert(ZoomMidpointEvents::LEFT_MID_POINT >= LAST_EDGE_MIDPOINT);

  return midPointEvent >= LAST_EDGE_MIDPOINT;
}

auto FilterSettingsService::SetAnyRandomZoomMidpoint(const bool allowEdgePoints) noexcept -> void
{
  static constexpr auto HEIGHT_MARGIN = 2;

  const auto oldZoomMidpoint = m_filterSettings.filterEffectsSettings.zoomMidpoint;

  switch (GetWeightRandomMidPoint(allowEdgePoints))
  {
    case ZoomMidpointEvents::BOTTOM_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = {
          .x = I_HALF * m_goomInfo->GetDimensions().GetIntWidth(),
          .y = m_goomInfo->GetDimensions().GetIntHeight() - HEIGHT_MARGIN};
      break;
    case ZoomMidpointEvents::TOP_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = {
          .x = I_HALF * m_goomInfo->GetDimensions().GetIntWidth(), .y = 1};
      break;
    case ZoomMidpointEvents::LEFT_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = {
          .x = 1, .y = I_HALF * m_goomInfo->GetDimensions().GetIntHeight()};
      break;
    case ZoomMidpointEvents::RIGHT_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = {
          .x = m_goomInfo->GetDimensions().GetIntWidth() - HEIGHT_MARGIN,
          .y = I_HALF * m_goomInfo->GetDimensions().GetIntHeight()};
      break;
    case ZoomMidpointEvents::CENTRE_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = m_screenCentre;
      break;
    case ZoomMidpointEvents::BOTTOM_LEFT_QUARTER_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = {
          .x = I_QUARTER * m_goomInfo->GetDimensions().GetIntWidth(),
          .y = I_THREE_QUARTERS * m_goomInfo->GetDimensions().GetIntHeight()};
      break;
    case ZoomMidpointEvents::TOP_LEFT_QUARTER_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = {
          .x = I_QUARTER * m_goomInfo->GetDimensions().GetIntWidth(),
          .y = I_QUARTER * m_goomInfo->GetDimensions().GetIntHeight()};
      break;
    case ZoomMidpointEvents::BOTTOM_RIGHT_QUARTER_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = {
          .x = I_THREE_QUARTERS * m_goomInfo->GetDimensions().GetIntWidth(),
          .y = I_THREE_QUARTERS * m_goomInfo->GetDimensions().GetIntHeight()};
      break;
    case ZoomMidpointEvents::TOP_RIGHT_QUARTER_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = {
          .x = I_THREE_QUARTERS * m_goomInfo->GetDimensions().GetIntWidth(),
          .y = I_QUARTER * m_goomInfo->GetDimensions().GetIntHeight()};
      break;
  }

  m_filterSettings.filterEffectsSettings.filterZoomMidpointHasChanged =
      oldZoomMidpoint != m_filterSettings.filterEffectsSettings.zoomMidpoint;

#ifdef DEBUG_GPU_FILTERS
  std::println("SetAnyRandomZoomMidpoint: zoomMidpoint = ({}, {}).",
               m_filterSettings.filterEffectsSettings.zoomMidpoint.x,
               m_filterSettings.filterEffectsSettings.zoomMidpoint.y);
#endif
}

} // namespace GOOM::FILTER_FX
