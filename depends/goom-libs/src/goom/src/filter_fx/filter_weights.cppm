module;

#include <array>
#include <cstdint>
#include <map>
#include <vector>

export module Goom.FilterFx.FilterSettingsService:FilterWeights;

import Goom.FilterFx.AfterEffects.AfterEffectsStates;
import Goom.FilterFx.AfterEffects.AfterEffectsTypes;
import Goom.FilterFx.FilterConsts;
import Goom.FilterFx.FilterModes;
import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.AssertUtils;

using GOOM::FILTER_FX::GpuZoomFilterMode;
using GOOM::FILTER_FX::ZoomFilterMode;
using GOOM::FILTER_FX::AFTER_EFFECTS::AfterEffectsTypes;
using GOOM::FILTER_FX::AFTER_EFFECTS::HypercosOverlayMode;
using GOOM::UTILS::EnumMap;
using GOOM::UTILS::GetFilledEnumMap;
using GOOM::UTILS::MATH::ConditionalWeights;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::Weights;

using AfterEffectsProbs = EnumMap<AfterEffectsTypes, float>;

using enum GpuZoomFilterMode;
using enum ZoomFilterMode;

export namespace GOOM::FILTER_FX
{

// For debugging:

//inline constexpr auto FORCED_GPU_FILTER_MODE = GPU_AMULET_MODE;
//inline constexpr auto FORCED_GPU_FILTER_MODE = GPU_BEAUTIFUL_FIELD_MODE;
//inline constexpr auto FORCED_GPU_FILTER_MODE = GPU_NONE_MODE;
//inline constexpr auto FORCED_GPU_FILTER_MODE = GPU_REFLECTING_POOL_MODE;
inline constexpr auto FORCED_GPU_FILTER_MODE = GPU_UP_DOWN_MODE;
//inline constexpr auto FORCED_GPU_FILTER_MODE = GPU_VORTEX_MODE;
//inline constexpr auto FORCED_GPU_FILTER_MODE = GPU_WAVE_MODE;

//inline constexpr auto FORCED_FILTER_MODE = AMULET_MODE;
//inline constexpr auto FORCED_FILTER_MODE = COMPLEX_RATIONAL_MODE;
//inline constexpr auto FORCED_FILTER_MODE = CRYSTAL_BALL_MODE0;
//inline constexpr auto FORCED_FILTER_MODE = CRYSTAL_BALL_MODE1;
//inline constexpr auto FORCED_FILTER_MODE = DISTANCE_FIELD_MODE0;
//inline constexpr auto FORCED_FILTER_MODE = DISTANCE_FIELD_MODE1;
//inline constexpr auto FORCED_FILTER_MODE = DISTANCE_FIELD_MODE2;
//inline constexpr auto FORCED_FILTER_MODE = EXP_RECIPROCAL_MODE;
//inline constexpr auto FORCED_FILTER_MODE = FLOW_FIELD_MODE;
//inline constexpr auto FORCED_FILTER_MODE = HYPERCOS_MODE0;
//inline constexpr auto FORCED_FILTER_MODE = HYPERCOS_MODE1;
//inline constexpr auto FORCED_FILTER_MODE = HYPERCOS_MODE2;
//inline constexpr auto FORCED_FILTER_MODE = HYPERCOS_MODE3;
//inline constexpr auto FORCED_FILTER_MODE = IMAGE_DISPLACEMENT_MODE;
//inline constexpr auto FORCED_FILTER_MODE = JULIA_MODE;
//inline constexpr auto FORCED_FILTER_MODE = MOBIUS_MODE;
//inline constexpr auto FORCED_FILTER_MODE = NEWTON_MODE;
inline constexpr auto FORCED_FILTER_MODE = NORMAL_MODE;
//inline constexpr auto FORCED_FILTER_MODE = PERLIN_NOISE_MODE;
//inline constexpr auto FORCED_FILTER_MODE = SCRUNCH_MODE;
//inline constexpr auto FORCED_FILTER_MODE = SPEEDWAY_MODE0;
//inline constexpr auto FORCED_FILTER_MODE = SPEEDWAY_MODE1;
//inline constexpr auto FORCED_FILTER_MODE = SPEEDWAY_MODE2;
//inline constexpr auto FORCED_FILTER_MODE = WAVE_SQ_DIST_ANGLE_EFFECT_MODE0;
//inline constexpr auto FORCED_FILTER_MODE = WAVE_SQ_DIST_ANGLE_EFFECT_MODE1;
//inline constexpr auto FORCED_FILTER_MODE = WAVE_ATAN_ANGLE_EFFECT_MODE0;
//inline constexpr auto FORCED_FILTER_MODE = WAVE_ATAN_ANGLE_EFFECT_MODE1;
//inline constexpr auto FORCED_FILTER_MODE = WATER_MODE;
//inline constexpr auto FORCED_FILTER_MODE = Y_ONLY_MODE;

//inline constexpr auto FORCED_AFTER_EFFECTS_TYPE = AfterEffectsTypes::HYPERCOS;
//inline constexpr auto FORCED_AFTER_EFFECTS_TYPE = AfterEffectsTypes::IMAGE_VELOCITY;
//inline constexpr auto FORCED_AFTER_EFFECTS_TYPE = AfterEffectsTypes::NOISE;
//inline constexpr auto FORCED_AFTER_EFFECTS_TYPE = AfterEffectsTypes::PLANES;
//inline constexpr auto FORCED_AFTER_EFFECTS_TYPE = AfterEffectsTypes::ROTATION;
//inline constexpr auto FORCED_AFTER_EFFECTS_TYPE = AfterEffectsTypes::TAN_EFFECT;
inline constexpr auto FORCED_AFTER_EFFECTS_TYPE = AfterEffectsTypes::XY_LERP_EFFECT;

// End debugging

inline constexpr auto DEFAULT_AFTER_EFFECTS_STATES =
    GetFilledEnumMap<AfterEffectsTypes, bool>(false);

[[nodiscard]] constexpr auto GetAfterEffectsProbability(ZoomFilterMode filterMode)
    -> AfterEffectsProbs;

[[nodiscard]] constexpr auto GetRepeatAfterEffectsProbability() -> AfterEffectsProbs;

[[nodiscard]] constexpr auto GetAfterEffectsOffTime() -> EnumMap<AfterEffectsTypes, uint32_t>;

[[nodiscard]] auto GetWeightedFilterEvents(const GoomRand& goomRand)
    -> ConditionalWeights<ZoomFilterMode>;

[[nodiscard]] auto GetWeightedGpuFilterEvents(const GoomRand& goomRand)
    -> Weights<GpuZoomFilterMode>;

[[nodiscard]] constexpr auto GetHypercosWeights(ZoomFilterMode filterMode) noexcept
    -> std::vector<Weights<HypercosOverlayMode>::KeyValue>;

} // namespace GOOM::FILTER_FX

namespace GOOM::FILTER_FX
{

using UTILS::NUM;

static constexpr auto GPU_NONE_MODE_WEIGHT       = 60.0F;
static constexpr auto GPU_AMULET_MODE_WEIGHT     = 10.0F;
static constexpr auto GPU_WAVE_WEIGHT            = 10.0F;
static constexpr auto GPU_VORTEX_WEIGHT          = 10.0F;
static constexpr auto GPU_REFLECTING_POOL_WEIGHT = 10.0F;
static constexpr auto GPU_BEAUTIFUL_FIELD_WEIGHT = 10.0F;
static constexpr auto GPU_UP_DOWN_WEIGHT         = 10.0F;

static constexpr auto AMULET_MODE_WEIGHT             = 10.0F;
static constexpr auto COMPLEX_RATIONAL_MODE_WEIGHT   = 10.0F;
static constexpr auto CRYSTAL_BALL_MODE0_WEIGHT      = 04.0F;
static constexpr auto CRYSTAL_BALL_MODE1_WEIGHT      = 02.0F;
static constexpr auto DISTANCE_FIELD_MODE0_WEIGHT    = 03.0F;
static constexpr auto DISTANCE_FIELD_MODE1_WEIGHT    = 03.0F;
static constexpr auto DISTANCE_FIELD_MODE2_WEIGHT    = 02.0F;
static constexpr auto EXP_RECIPROCAL_MODE_WEIGHT     = 10.0F;
static constexpr auto FLOW_FIELD_MODE_WEIGHT         = 10.0F;
static constexpr auto HYPERCOS_MODE0_WEIGHT          = 08.0F;
static constexpr auto HYPERCOS_MODE1_WEIGHT          = 04.0F;
static constexpr auto HYPERCOS_MODE2_WEIGHT          = 02.0F;
static constexpr auto HYPERCOS_MODE3_WEIGHT          = 01.0F;
static constexpr auto IMAGE_DISPLACEMENT_MODE_WEIGHT = 05.0F;
static constexpr auto JULIA_MODE_WEIGHT              = 10.0F;
static constexpr auto MOBIUS_MODE_WEIGHT             = 10.0F;
static constexpr auto NEWTON_MODE_WEIGHT             = 10.0F;
static constexpr auto NORMAL_MODE_WEIGHT             = 10.0F;
static constexpr auto PERLIN_NOISE_MODE_WEIGHT       = 10.0F;
static constexpr auto SCRUNCH_MODE_WEIGHT            = 06.0F;
static constexpr auto SPEEDWAY_MODE0_WEIGHT          = 02.0F;
static constexpr auto SPEEDWAY_MODE1_WEIGHT          = 01.0F;
static constexpr auto SPEEDWAY_MODE2_WEIGHT          = 05.0F;
static constexpr auto WAVE_SQ_DIST_MODE0_WEIGHT      = 05.0F;
static constexpr auto WAVE_SQ_DIST_MODE1_WEIGHT      = 04.0F;
static constexpr auto WAVE_ATAN_MODE0_WEIGHT         = 05.0F;
static constexpr auto WAVE_ATAN_MODE1_WEIGHT         = 04.0F;
static constexpr auto WATER_MODE_WEIGHT              = 00.0F;
static constexpr auto Y_ONLY_MODE_WEIGHT             = 05.0F;

static constexpr auto PROB_HIGH = 0.9F;
static constexpr auto PROB_HALF = 0.5F;
static constexpr auto PROB_LOW  = 0.1F;
static constexpr auto PROB_ZERO = 0.0F;

constexpr auto GetEffectsProbabilities() noexcept -> EnumMap<ZoomFilterMode, AfterEffectsProbs>
{
  constexpr auto DEFAULT_AFTER_EFFECTS_PROBS = AfterEffectsProbs{{{
      {AfterEffectsTypes::HYPERCOS, 0.9F},
      {AfterEffectsTypes::IMAGE_VELOCITY, 0.1F},
      {AfterEffectsTypes::NOISE, 0.1F},
      {AfterEffectsTypes::PLANES, 0.7F},
      {AfterEffectsTypes::ROTATION, 0.0F},
      {AfterEffectsTypes::TAN_EFFECT, 0.2F},
      {AfterEffectsTypes::XY_LERP_EFFECT, 0.2F},
  }}};
  static_assert(DEFAULT_AFTER_EFFECTS_PROBS.size() == NUM<AfterEffectsTypes>);

  constexpr auto WAVE0_PROB_PLANE = 0.8F;
  constexpr auto WAVE1_PROB_PLANE = 0.8F;

  using EffectType = AfterEffectsTypes;

  auto effectsProbs =
      GetFilledEnumMap<ZoomFilterMode, AfterEffectsProbs>(DEFAULT_AFTER_EFFECTS_PROBS);

  effectsProbs[AMULET_MODE][EffectType::ROTATION] = PROB_HIGH;

  effectsProbs[COMPLEX_RATIONAL_MODE][EffectType::ROTATION] = PROB_HIGH;

  effectsProbs[CRYSTAL_BALL_MODE0][EffectType::ROTATION] = PROB_HIGH;
  effectsProbs[CRYSTAL_BALL_MODE1][EffectType::ROTATION] = PROB_HIGH;

  effectsProbs[DISTANCE_FIELD_MODE0][EffectType::ROTATION] = PROB_HIGH;
  effectsProbs[DISTANCE_FIELD_MODE1][EffectType::ROTATION] = PROB_HIGH;
  effectsProbs[DISTANCE_FIELD_MODE2][EffectType::ROTATION] = PROB_HIGH;

  effectsProbs[EXP_RECIPROCAL_MODE][EffectType::ROTATION] = PROB_HIGH;

  effectsProbs[FLOW_FIELD_MODE][EffectType::ROTATION] = PROB_HIGH;

  effectsProbs[HYPERCOS_MODE0][EffectType::ROTATION] = PROB_LOW;
  effectsProbs[HYPERCOS_MODE1][EffectType::ROTATION] = PROB_LOW;
  effectsProbs[HYPERCOS_MODE2][EffectType::ROTATION] = PROB_LOW;
  effectsProbs[HYPERCOS_MODE3][EffectType::ROTATION] = PROB_LOW;

  effectsProbs[IMAGE_DISPLACEMENT_MODE][EffectType::ROTATION] = PROB_ZERO;

  effectsProbs[JULIA_MODE][EffectType::ROTATION] = PROB_HIGH;

  effectsProbs[MOBIUS_MODE][EffectType::ROTATION] = PROB_HIGH;

  effectsProbs[NEWTON_MODE][EffectType::ROTATION] = PROB_HIGH;

  effectsProbs[NORMAL_MODE][EffectType::ROTATION] = PROB_ZERO;

  effectsProbs[PERLIN_NOISE_MODE][EffectType::ROTATION] = PROB_HALF;

  effectsProbs[SCRUNCH_MODE][EffectType::ROTATION] = PROB_HALF;

  effectsProbs[SPEEDWAY_MODE0][EffectType::ROTATION] = PROB_HALF;
  effectsProbs[SPEEDWAY_MODE1][EffectType::ROTATION] = PROB_HIGH;
  effectsProbs[SPEEDWAY_MODE2][EffectType::ROTATION] = PROB_HIGH;

  effectsProbs[WATER_MODE][EffectType::ROTATION] = PROB_ZERO;

  effectsProbs[WAVE_SQ_DIST_ANGLE_EFFECT_MODE0][EffectType::PLANES]   = WAVE0_PROB_PLANE;
  effectsProbs[WAVE_SQ_DIST_ANGLE_EFFECT_MODE0][EffectType::ROTATION] = PROB_HIGH;
  effectsProbs[WAVE_SQ_DIST_ANGLE_EFFECT_MODE1][EffectType::PLANES]   = WAVE1_PROB_PLANE;
  effectsProbs[WAVE_SQ_DIST_ANGLE_EFFECT_MODE1][EffectType::ROTATION] = PROB_HIGH;

  effectsProbs[WAVE_ATAN_ANGLE_EFFECT_MODE0][EffectType::PLANES]   = WAVE0_PROB_PLANE;
  effectsProbs[WAVE_ATAN_ANGLE_EFFECT_MODE0][EffectType::ROTATION] = PROB_HIGH;
  effectsProbs[WAVE_ATAN_ANGLE_EFFECT_MODE1][EffectType::PLANES]   = WAVE1_PROB_PLANE;
  effectsProbs[WAVE_ATAN_ANGLE_EFFECT_MODE1][EffectType::ROTATION] = PROB_HIGH;

  effectsProbs[Y_ONLY_MODE][EffectType::ROTATION] = PROB_HALF;

  return effectsProbs;
}

static constexpr auto DEFAULT_AFTER_EFFECTS_PROBS =
    GetFilledEnumMap<AfterEffectsTypes, float>(0.0F);
static constexpr auto ZERO_REPEAT_AFTER_EFFECTS_PROBS =
    GetFilledEnumMap<AfterEffectsTypes, float>(0.0F);
static constexpr auto DEFAULT_REPEAT_AFTER_EFFECTS_PROBS = EnumMap<AfterEffectsTypes, float>{{{
    {AfterEffectsTypes::HYPERCOS, 0.5F},
    {AfterEffectsTypes::IMAGE_VELOCITY, 0.8F},
    {AfterEffectsTypes::NOISE, 0.0F},
    {AfterEffectsTypes::PLANES, 0.0F},
    {AfterEffectsTypes::ROTATION, 0.0F},
    {AfterEffectsTypes::TAN_EFFECT, 0.0F},
    {AfterEffectsTypes::XY_LERP_EFFECT, 0.1F},
}}};
static constexpr auto DEFAULT_AFTER_EFFECTS_OFF_TIMES    = EnumMap<AfterEffectsTypes, uint32_t>{{{
    {AfterEffectsTypes::HYPERCOS, 100U},
    {AfterEffectsTypes::IMAGE_VELOCITY, 100U},
    {AfterEffectsTypes::NOISE, 100U},
    {AfterEffectsTypes::PLANES, 100U},
    {AfterEffectsTypes::ROTATION, 0U},
    {AfterEffectsTypes::TAN_EFFECT, 100U},
    {AfterEffectsTypes::XY_LERP_EFFECT, 100U},
}}};

[[nodiscard]] constexpr auto GetAfterEffectsProbability(const ZoomFilterMode filterMode)
    -> AfterEffectsProbs
{
  if constexpr (USE_FORCED_AFTER_EFFECT)
  {
    auto forcedProbabilities                       = DEFAULT_AFTER_EFFECTS_PROBS;
    forcedProbabilities[FORCED_AFTER_EFFECTS_TYPE] = 1.0F;
    return forcedProbabilities;
  }

  static_assert(GetEffectsProbabilities().size() == NUM<ZoomFilterMode>);

  return GetEffectsProbabilities()[filterMode];
}

[[nodiscard]] constexpr auto GetRepeatAfterEffectsProbability() -> AfterEffectsProbs
{
  if constexpr (USE_FORCED_AFTER_EFFECT)
  {
    auto forcedRepeatProbabilities                       = ZERO_REPEAT_AFTER_EFFECTS_PROBS;
    forcedRepeatProbabilities[FORCED_AFTER_EFFECTS_TYPE] = 1.0F;
    return forcedRepeatProbabilities;
  }

  return DEFAULT_REPEAT_AFTER_EFFECTS_PROBS;
}

[[nodiscard]] constexpr auto GetAfterEffectsOffTime() -> EnumMap<AfterEffectsTypes, uint32_t>
{
  if constexpr (USE_FORCED_AFTER_EFFECT)
  {
    auto forcedOffTimes                       = DEFAULT_AFTER_EFFECTS_OFF_TIMES;
    forcedOffTimes[FORCED_AFTER_EFFECTS_TYPE] = 0;
    return forcedOffTimes;
  }

  return DEFAULT_AFTER_EFFECTS_OFF_TIMES;
}

[[nodiscard]] auto GetWeightedFilterEvents(const GoomRand& goomRand)
    -> ConditionalWeights<ZoomFilterMode>
{
  static const auto s_CRYSTAL_BALL_MOD_E0_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {CRYSTAL_BALL_MODE0, 0.0F},
      {CRYSTAL_BALL_MODE1, 0.0F},
  };
  static const auto s_CRYSTAL_BALL_MOD_E1_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {CRYSTAL_BALL_MODE0, 0.0F},
      {CRYSTAL_BALL_MODE1, 0.0F},
  };
  static const auto s_NORMAL_MODE_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {         NORMAL_MODE, 1.0F},
      {DISTANCE_FIELD_MODE0, 2.0F},
      {DISTANCE_FIELD_MODE0, 2.0F},
      {DISTANCE_FIELD_MODE0, 2.0F},
  };
  static const auto s_HYPERCOS_MOD_E0_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {HYPERCOS_MODE0, 0.0F},
      {HYPERCOS_MODE1, 0.0F},
      {HYPERCOS_MODE2, 0.0F},
      {HYPERCOS_MODE3, 0.0F},
  };
  static const auto s_HYPERCOS_MOD_E1_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {HYPERCOS_MODE0, 1.0F}, // OK for mode0 to follow
      {HYPERCOS_MODE1, 0.0F},
      {HYPERCOS_MODE2, 0.0F},
      {HYPERCOS_MODE3, 0.0F},
  };
  static const auto s_HYPERCOS_MOD_E2_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {HYPERCOS_MODE0, 1.0F}, // OK for mode0 to follow
      {HYPERCOS_MODE1, 0.0F},
      {HYPERCOS_MODE2, 0.0F},
      {HYPERCOS_MODE3, 0.0F},
  };
  static const auto s_HYPERCOS_MOD_E3_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {HYPERCOS_MODE0, 1.0F}, // OK for mode0 to follow
      {HYPERCOS_MODE1, 0.0F},
      {HYPERCOS_MODE2, 0.0F},
      {HYPERCOS_MODE3, 0.0F},
  };
  static const auto s_SPEEDWAY_MOD_E0_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {SPEEDWAY_MODE0, 0.0F},
      {SPEEDWAY_MODE1, 0.0F},
      {SPEEDWAY_MODE2, 0.0F},
  };
  static const auto s_SPEEDWAY_MOD_E1_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {SPEEDWAY_MODE0, 0.0F},
      {SPEEDWAY_MODE1, 0.0F},
      {SPEEDWAY_MODE2, 0.0F},
  };
  static const auto s_SPEEDWAY_MOD_E2_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {SPEEDWAY_MODE0, 0.0F},
      {SPEEDWAY_MODE1, 0.0F},
      {SPEEDWAY_MODE2, 0.0F},
  };
  static const auto s_WAVE_SQ_DIST_MOD_E0_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {WAVE_SQ_DIST_ANGLE_EFFECT_MODE0, 0.0F},
      {WAVE_SQ_DIST_ANGLE_EFFECT_MODE1, 0.0F},
  };
  static const auto s_WAVE_SQ_DIST_MOD_E1_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {WAVE_SQ_DIST_ANGLE_EFFECT_MODE0, 0.0F},
      {WAVE_SQ_DIST_ANGLE_EFFECT_MODE1, 0.0F},
  };
  static const auto s_WAVE_ATAN_MOD_E0_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {WAVE_ATAN_ANGLE_EFFECT_MODE0, 0.0F},
      {WAVE_ATAN_ANGLE_EFFECT_MODE1, 0.0F},
  };
  static const auto s_WAVE_ATAN_MOD_E1_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {WAVE_ATAN_ANGLE_EFFECT_MODE0, 0.0F},
      {WAVE_ATAN_ANGLE_EFFECT_MODE1, 0.0F},
  };

  auto filterWeights = ConditionalWeights<ZoomFilterMode>{
      goomRand,
      {
        {.key = AMULET_MODE, .weight = AMULET_MODE_WEIGHT},
        {.key = COMPLEX_RATIONAL_MODE, .weight = COMPLEX_RATIONAL_MODE_WEIGHT},
        {.key = CRYSTAL_BALL_MODE0, .weight = CRYSTAL_BALL_MODE0_WEIGHT},
        {.key = CRYSTAL_BALL_MODE1, .weight = CRYSTAL_BALL_MODE1_WEIGHT},
        {.key = DISTANCE_FIELD_MODE0, .weight = DISTANCE_FIELD_MODE0_WEIGHT},
        {.key = DISTANCE_FIELD_MODE1, .weight = DISTANCE_FIELD_MODE1_WEIGHT},
        {.key = DISTANCE_FIELD_MODE2, .weight = DISTANCE_FIELD_MODE2_WEIGHT},
        {.key = EXP_RECIPROCAL_MODE, .weight = EXP_RECIPROCAL_MODE_WEIGHT},
        {.key = FLOW_FIELD_MODE, .weight = FLOW_FIELD_MODE_WEIGHT},
        {.key = HYPERCOS_MODE0, .weight = HYPERCOS_MODE0_WEIGHT},
        {.key = HYPERCOS_MODE1, .weight = HYPERCOS_MODE1_WEIGHT},
        {.key = HYPERCOS_MODE2, .weight = HYPERCOS_MODE2_WEIGHT},
        {.key = HYPERCOS_MODE3, .weight = HYPERCOS_MODE3_WEIGHT},
        {.key = IMAGE_DISPLACEMENT_MODE, .weight = IMAGE_DISPLACEMENT_MODE_WEIGHT},
        {.key = JULIA_MODE, .weight = JULIA_MODE_WEIGHT},
        {.key = MOBIUS_MODE, .weight = MOBIUS_MODE_WEIGHT},
        {.key = NEWTON_MODE, .weight = NEWTON_MODE_WEIGHT},
        {.key = NORMAL_MODE, .weight = NORMAL_MODE_WEIGHT},
        {.key = PERLIN_NOISE_MODE, .weight = PERLIN_NOISE_MODE_WEIGHT},
        {.key = SCRUNCH_MODE, .weight = SCRUNCH_MODE_WEIGHT},
        {.key = SPEEDWAY_MODE0, .weight = SPEEDWAY_MODE0_WEIGHT},
        {.key = SPEEDWAY_MODE1, .weight = SPEEDWAY_MODE1_WEIGHT},
        {.key = SPEEDWAY_MODE2, .weight = SPEEDWAY_MODE2_WEIGHT},
        {.key = WAVE_SQ_DIST_ANGLE_EFFECT_MODE0, .weight = WAVE_SQ_DIST_MODE0_WEIGHT},
        {.key = WAVE_SQ_DIST_ANGLE_EFFECT_MODE1, .weight = WAVE_SQ_DIST_MODE1_WEIGHT},
        {.key = WAVE_ATAN_ANGLE_EFFECT_MODE0, .weight = WAVE_ATAN_MODE0_WEIGHT},
        {.key = WAVE_ATAN_ANGLE_EFFECT_MODE1, .weight = WAVE_ATAN_MODE1_WEIGHT},
        {.key = WATER_MODE, .weight = WATER_MODE_WEIGHT},
        {.key = Y_ONLY_MODE, .weight = Y_ONLY_MODE_WEIGHT},
        },
      {
        {.key = CRYSTAL_BALL_MODE0, .weightMultipliers = s_CRYSTAL_BALL_MOD_E0_MULTIPLIERS},
        {.key = CRYSTAL_BALL_MODE1, .weightMultipliers = s_CRYSTAL_BALL_MOD_E1_MULTIPLIERS},
        {.key = NORMAL_MODE, .weightMultipliers = s_NORMAL_MODE_MULTIPLIERS},
        {.key = HYPERCOS_MODE0, .weightMultipliers = s_HYPERCOS_MOD_E0_MULTIPLIERS},
        {.key = HYPERCOS_MODE1, .weightMultipliers = s_HYPERCOS_MOD_E1_MULTIPLIERS},
        {.key = HYPERCOS_MODE2, .weightMultipliers = s_HYPERCOS_MOD_E2_MULTIPLIERS},
        {.key = HYPERCOS_MODE3, .weightMultipliers = s_HYPERCOS_MOD_E3_MULTIPLIERS},
        {.key = SPEEDWAY_MODE0, .weightMultipliers = s_SPEEDWAY_MOD_E0_MULTIPLIERS},
        {.key = SPEEDWAY_MODE1, .weightMultipliers = s_SPEEDWAY_MOD_E1_MULTIPLIERS},
        {.key = SPEEDWAY_MODE2, .weightMultipliers = s_SPEEDWAY_MOD_E2_MULTIPLIERS},
        {.key               = WAVE_SQ_DIST_ANGLE_EFFECT_MODE0,
           .weightMultipliers = s_WAVE_SQ_DIST_MOD_E0_MULTIPLIERS},
        {.key               = WAVE_SQ_DIST_ANGLE_EFFECT_MODE1,
           .weightMultipliers = s_WAVE_SQ_DIST_MOD_E1_MULTIPLIERS},
        {.key               = WAVE_ATAN_ANGLE_EFFECT_MODE0,
           .weightMultipliers = s_WAVE_ATAN_MOD_E0_MULTIPLIERS},
        {.key               = WAVE_ATAN_ANGLE_EFFECT_MODE1,
           .weightMultipliers = s_WAVE_ATAN_MOD_E1_MULTIPLIERS},
        }
  };

  Ensures(filterWeights.GetNumSetWeights() == NUM<ZoomFilterMode>);

  return filterWeights;
}

[[nodiscard]] auto GetWeightedGpuFilterEvents(const GoomRand& goomRand)
    -> Weights<GpuZoomFilterMode>
{
  auto filterGpuWeights = Weights<GpuZoomFilterMode>{
      goomRand,
      {
        {.key = GPU_NONE_MODE, .weight = GPU_NONE_MODE_WEIGHT},
        {.key = GPU_AMULET_MODE, .weight = GPU_AMULET_MODE_WEIGHT},
        {.key = GPU_WAVE_MODE, .weight = GPU_WAVE_WEIGHT},
        {.key = GPU_VORTEX_MODE, .weight = GPU_VORTEX_WEIGHT},
        {.key = GPU_REFLECTING_POOL_MODE, .weight = GPU_REFLECTING_POOL_WEIGHT},
        {.key = GPU_BEAUTIFUL_FIELD_MODE, .weight = GPU_BEAUTIFUL_FIELD_WEIGHT},
        {.key = GPU_UP_DOWN_MODE, .weight = GPU_UP_DOWN_WEIGHT},
        },
  };

  Ensures(filterGpuWeights.GetNumSetWeights() == NUM<GpuZoomFilterMode>);

  return filterGpuWeights;
}

// NOLINTBEGIN(readability-function-cognitive-complexity)

[[nodiscard]] constexpr auto GetHypercosWeights(const ZoomFilterMode filterMode) noexcept
    -> std::vector<Weights<HypercosOverlayMode>::KeyValue>
{
  constexpr auto FORCED_HYPERCOS =
      USE_FORCED_AFTER_EFFECT and (FORCED_AFTER_EFFECTS_TYPE == AfterEffectsTypes::HYPERCOS);

  using Hyp         = HypercosOverlayMode;
  using ModeWeights = std::array<Weights<HypercosOverlayMode>::KeyValue, NUM<HypercosOverlayMode>>;

  constexpr auto AMULET_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 20.0F},
       {.key = Hyp::MODE0, .weight = 1.0F},
       {.key = Hyp::MODE1, .weight = 5.0F},
       {.key = Hyp::MODE2, .weight = 1.0F},
       {.key = Hyp::MODE3, .weight = 1.0F}}
  };
  constexpr auto COMPLEX_RATIONAL_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 10.0F},
       {.key = Hyp::MODE0, .weight = 5.0F},
       {.key = Hyp::MODE1, .weight = 1.0F},
       {.key = Hyp::MODE2, .weight = 1.0F},
       {.key = Hyp::MODE3, .weight = 0.0F}}
  };
  constexpr auto CRYSTAL_BALL0_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 5.0F},
       {.key = Hyp::MODE0, .weight = 10.0F},
       {.key = Hyp::MODE1, .weight = 1.0F},
       {.key = Hyp::MODE2, .weight = 1.0F},
       {.key = Hyp::MODE3, .weight = 1.0F}}
  };
  constexpr auto CRYSTAL_BALL1_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 5.0F},
       {.key = Hyp::MODE0, .weight = 1.0F},
       {.key = Hyp::MODE1, .weight = 99.0F},
       {.key = Hyp::MODE2, .weight = 1.0F},
       {.key = Hyp::MODE3, .weight = 1.0F}}
  };
  constexpr auto DISTANCE_FIELD_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 5.0F},
       {.key = Hyp::MODE0, .weight = 10.0F},
       {.key = Hyp::MODE1, .weight = 1.0F},
       {.key = Hyp::MODE2, .weight = 1.0F},
       {.key = Hyp::MODE3, .weight = 1.0F}}
  };
  constexpr auto EXP_RECIPROCAL_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 10.0F},
       {.key = Hyp::MODE0, .weight = 5.0F},
       {.key = Hyp::MODE1, .weight = 1.0F},
       {.key = Hyp::MODE2, .weight = 1.0F},
       {.key = Hyp::MODE3, .weight = 0.0F}}
  };
  constexpr auto FLOW_FIELD_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 20.0F},
       {.key = Hyp::MODE0, .weight = 1.0F},
       {.key = Hyp::MODE1, .weight = 5.0F},
       {.key = Hyp::MODE2, .weight = 1.0F},
       {.key = Hyp::MODE3, .weight = 1.0F}}
  };
  constexpr auto HYPERCOS0_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 1.0F},
       {.key = Hyp::MODE0, .weight = 1.0F},
       {.key = Hyp::MODE1, .weight = 2.0F},
       {.key = Hyp::MODE2, .weight = 2.0F},
       {.key = Hyp::MODE3, .weight = 2.0F}}
  };
  constexpr auto HYPERCOS1_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 1.0F},
       {.key = Hyp::MODE0, .weight = 2.0F},
       {.key = Hyp::MODE1, .weight = 1.0F},
       {.key = Hyp::MODE2, .weight = 2.0F},
       {.key = Hyp::MODE3, .weight = 2.0F}}
  };
  constexpr auto HYPERCOS2_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 1.0F},
       {.key = Hyp::MODE0, .weight = 2.0F},
       {.key = Hyp::MODE1, .weight = 2.0F},
       {.key = Hyp::MODE2, .weight = 1.0F},
       {.key = Hyp::MODE3, .weight = 2.0F}}
  };
  constexpr auto HYPERCOS3_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 1.0F},
       {.key = Hyp::MODE0, .weight = 2.0F},
       {.key = Hyp::MODE1, .weight = 2.0F},
       {.key = Hyp::MODE2, .weight = 2.0F},
       {.key = Hyp::MODE3, .weight = 1.0F}}
  };
  constexpr auto IMAGE_DISPLACEMENT_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 10.0F},
       {.key = Hyp::MODE0, .weight = 1.0F},
       {.key = Hyp::MODE1, .weight = 5.0F},
       {.key = Hyp::MODE2, .weight = 1.0F},
       {.key = Hyp::MODE3, .weight = 1.0F}}
  };
  constexpr auto JULIA_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 10.0F},
       {.key = Hyp::MODE0, .weight = 5.0F},
       {.key = Hyp::MODE1, .weight = 1.0F},
       {.key = Hyp::MODE2, .weight = 1.0F},
       {.key = Hyp::MODE3, .weight = 0.0F}}
  };
  constexpr auto MOBIUS_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 10.0F},
       {.key = Hyp::MODE0, .weight = 5.0F},
       {.key = Hyp::MODE1, .weight = 1.0F},
       {.key = Hyp::MODE2, .weight = 1.0F},
       {.key = Hyp::MODE3, .weight = 0.0F}}
  };
  constexpr auto NEWTON_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 10.0F},
       {.key = Hyp::MODE0, .weight = 5.0F},
       {.key = Hyp::MODE1, .weight = 1.0F},
       {.key = Hyp::MODE2, .weight = 1.0F},
       {.key = Hyp::MODE3, .weight = 0.0F}}
  };
  constexpr auto NORMAL_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 10.0F},
       {.key = Hyp::MODE0, .weight = 5.0F},
       {.key = Hyp::MODE1, .weight = 1.0F},
       {.key = Hyp::MODE2, .weight = 1.0F},
       {.key = Hyp::MODE3, .weight = 0.0F}}
  };
  constexpr auto PERLIN_NOISE_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 10.0F},
       {.key = Hyp::MODE0, .weight = 5.0F},
       {.key = Hyp::MODE1, .weight = 1.0F},
       {.key = Hyp::MODE2, .weight = 1.0F},
       {.key = Hyp::MODE3, .weight = 0.0F}}
  };
  constexpr auto SCRUNCH_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 10.0F},
       {.key = Hyp::MODE0, .weight = 1.0F},
       {.key = Hyp::MODE1, .weight = 5.0F},
       {.key = Hyp::MODE2, .weight = 1.0F},
       {.key = Hyp::MODE3, .weight = 1.0F}}
  };
  constexpr auto SPEEDWAY_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 10.0F},
       {.key = Hyp::MODE0, .weight = 5.0F},
       {.key = Hyp::MODE1, .weight = 1.0F},
       {.key = Hyp::MODE2, .weight = 1.0F},
       {.key = Hyp::MODE3, .weight = 1.0F}}
  };
  constexpr auto WATER_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 10.0F},
       {.key = Hyp::MODE0, .weight = 1.0F},
       {.key = Hyp::MODE1, .weight = 5.0F},
       {.key = Hyp::MODE2, .weight = 1.0F},
       {.key = Hyp::MODE3, .weight = 1.0F}}
  };
  constexpr auto WAVE_SQ_DIST_MODE0_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 10.0F},
       {.key = Hyp::MODE0, .weight = 5.0F},
       {.key = Hyp::MODE1, .weight = 1.0F},
       {.key = Hyp::MODE2, .weight = 1.0F},
       {.key = Hyp::MODE3, .weight = 1.0F}}
  };
  constexpr auto WAVE_SQ_DIST_MODE1_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 10.0F},
       {.key = Hyp::MODE0, .weight = 1.0F},
       {.key = Hyp::MODE1, .weight = 5.0F},
       {.key = Hyp::MODE2, .weight = 1.0F},
       {.key = Hyp::MODE3, .weight = 1.0F}}
  };
  constexpr auto WAVE_ATAN_MODE0_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 10.0F},
       {.key = Hyp::MODE0, .weight = 5.0F},
       {.key = Hyp::MODE1, .weight = 1.0F},
       {.key = Hyp::MODE2, .weight = 1.0F},
       {.key = Hyp::MODE3, .weight = 1.0F}}
  };
  constexpr auto WAVE_ATAN_MODE1_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 10.0F},
       {.key = Hyp::MODE0, .weight = 1.0F},
       {.key = Hyp::MODE1, .weight = 5.0F},
       {.key = Hyp::MODE2, .weight = 1.0F},
       {.key = Hyp::MODE3, .weight = 1.0F}}
  };
  constexpr auto Y_ONLY_HYPERCOS_WEIGHTS = ModeWeights{
      {{.key = Hyp::NONE, .weight = FORCED_HYPERCOS ? 0.0F : 10.0F},
       {.key = Hyp::MODE0, .weight = 1.0F},
       {.key = Hyp::MODE1, .weight = 5.0F},
       {.key = Hyp::MODE2, .weight = 1.0F},
       {.key = Hyp::MODE3, .weight = 1.0F}}
  };

  constexpr auto HYPERCOS_WEIGHTS = EnumMap<ZoomFilterMode, ModeWeights>{{{
      {AMULET_MODE, AMULET_HYPERCOS_WEIGHTS},
      {COMPLEX_RATIONAL_MODE, COMPLEX_RATIONAL_HYPERCOS_WEIGHTS},
      {CRYSTAL_BALL_MODE0, CRYSTAL_BALL0_HYPERCOS_WEIGHTS},
      {CRYSTAL_BALL_MODE1, CRYSTAL_BALL1_HYPERCOS_WEIGHTS},
      {DISTANCE_FIELD_MODE0, DISTANCE_FIELD_HYPERCOS_WEIGHTS},
      {DISTANCE_FIELD_MODE1, DISTANCE_FIELD_HYPERCOS_WEIGHTS},
      {DISTANCE_FIELD_MODE2, DISTANCE_FIELD_HYPERCOS_WEIGHTS},
      {EXP_RECIPROCAL_MODE, EXP_RECIPROCAL_HYPERCOS_WEIGHTS},
      {FLOW_FIELD_MODE, FLOW_FIELD_HYPERCOS_WEIGHTS},
      {HYPERCOS_MODE0, HYPERCOS0_HYPERCOS_WEIGHTS},
      {HYPERCOS_MODE1, HYPERCOS1_HYPERCOS_WEIGHTS},
      {HYPERCOS_MODE2, HYPERCOS2_HYPERCOS_WEIGHTS},
      {HYPERCOS_MODE3, HYPERCOS3_HYPERCOS_WEIGHTS},
      {IMAGE_DISPLACEMENT_MODE, IMAGE_DISPLACEMENT_HYPERCOS_WEIGHTS},
      {JULIA_MODE, JULIA_HYPERCOS_WEIGHTS},
      {MOBIUS_MODE, MOBIUS_HYPERCOS_WEIGHTS},
      {NEWTON_MODE, NEWTON_HYPERCOS_WEIGHTS},
      {NORMAL_MODE, NORMAL_HYPERCOS_WEIGHTS},
      {PERLIN_NOISE_MODE, PERLIN_NOISE_HYPERCOS_WEIGHTS},
      {SCRUNCH_MODE, SCRUNCH_HYPERCOS_WEIGHTS},
      {SPEEDWAY_MODE0, SPEEDWAY_HYPERCOS_WEIGHTS},
      {SPEEDWAY_MODE1, SPEEDWAY_HYPERCOS_WEIGHTS},
      {SPEEDWAY_MODE2, SPEEDWAY_HYPERCOS_WEIGHTS},
      {WATER_MODE, WATER_HYPERCOS_WEIGHTS},
      {WAVE_SQ_DIST_ANGLE_EFFECT_MODE0, WAVE_SQ_DIST_MODE0_HYPERCOS_WEIGHTS},
      {WAVE_SQ_DIST_ANGLE_EFFECT_MODE1, WAVE_SQ_DIST_MODE1_HYPERCOS_WEIGHTS},
      {WAVE_ATAN_ANGLE_EFFECT_MODE0, WAVE_ATAN_MODE0_HYPERCOS_WEIGHTS},
      {WAVE_ATAN_ANGLE_EFFECT_MODE1, WAVE_ATAN_MODE1_HYPERCOS_WEIGHTS},
      {Y_ONLY_MODE, Y_ONLY_HYPERCOS_WEIGHTS},
  }}};
  static_assert(HYPERCOS_WEIGHTS.size() == NUM<ZoomFilterMode>);

  return std::vector<Weights<HypercosOverlayMode>::KeyValue>{cbegin(HYPERCOS_WEIGHTS[filterMode]),
                                                             cend(HYPERCOS_WEIGHTS[filterMode])};
}

// NOLINTEND(readability-function-cognitive-complexity)

} // namespace GOOM::FILTER_FX
