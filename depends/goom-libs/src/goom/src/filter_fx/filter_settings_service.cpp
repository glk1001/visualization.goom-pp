#include "filter_settings_service.h"

#include "after_effects/after_effects_states.h"
#include "after_effects/after_effects_types.h"
#include "after_effects/the_effects/rotation.h"
#include "filter_consts.h"
#include "filter_effects/zoom_vector_effects.h"
#include "filter_settings.h"
#include "filter_speed.h"
#include "goom/goom_config.h"
#include "goom/math20.h"
#include "goom_plugin_info.h"
#include "normalized_coords.h"
#include "utils/enum_utils.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace GOOM::FILTER_FX
{

using AFTER_EFFECTS::AfterEffectsStates;
using AFTER_EFFECTS::AfterEffectsTypes;
using AFTER_EFFECTS::HypercosOverlayMode;
using AFTER_EFFECTS::RotationAdjustments;
using FILTER_EFFECTS::ZoomVectorEffects;
using UTILS::EnumMap;
using UTILS::GetFilledEnumMap;
using UTILS::NUM;
using UTILS::MATH::I_HALF;
using UTILS::MATH::I_QUARTER;
using UTILS::MATH::I_THREE_QUARTERS;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::U_HALF;
using UTILS::MATH::Weights;

namespace
{

// For debugging:

//constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::AMULET_MODE;
//constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::CRYSTAL_BALL_MODE0;
//constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::CRYSTAL_BALL_MODE1;
//constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::DISTANCE_FIELD_MODE0;
//constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::DISTANCE_FIELD_MODE1;
//constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::DISTANCE_FIELD_MODE2;
//constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::EXP_RECIPROCAL_MODE;
//constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::HYPERCOS_MODE0;
//constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::HYPERCOS_MODE1;
//constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::HYPERCOS_MODE2;
//constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::HYPERCOS_MODE3;
//constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::IMAGE_DISPLACEMENT_MODE;
constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::NORMAL_MODE;
//constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::SCRUNCH_MODE;
//constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::SPEEDWAY_MODE0;
//constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::SPEEDWAY_MODE1;
//constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::SPEEDWAY_MODE2;
//constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE0;
//constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE1;
//constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE0;
//constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE1;
//constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::WATER_MODE;
//constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::Y_ONLY_MODE;

//constexpr auto FORCED_AFTER_EFFECTS_TYPE = AfterEffectsTypes::HYPERCOS;
//constexpr auto FORCED_AFTER_EFFECTS_TYPE = AfterEffectsTypes::IMAGE_VELOCITY;
//constexpr auto FORCED_AFTER_EFFECTS_TYPE = AfterEffectsTypes::NOISE;
//constexpr auto FORCED_AFTER_EFFECTS_TYPE = AfterEffectsTypes::PLANES;
//constexpr auto FORCED_AFTER_EFFECTS_TYPE = AfterEffectsTypes::ROTATION;
//constexpr auto FORCED_AFTER_EFFECTS_TYPE = AfterEffectsTypes::TAN_EFFECT;
constexpr auto FORCED_AFTER_EFFECTS_TYPE = AfterEffectsTypes::XY_LERP_EFFECT;

// End debugging


constexpr auto PROB_HIGH = 0.9F;
constexpr auto PROB_HALF = 0.5F;
constexpr auto PROB_LOW  = 0.1F;
constexpr auto PROB_ZERO = 0.0F;

constexpr auto PROB_CRYSTAL_BALL_IN_MIDDLE = 0.8F;
constexpr auto PROB_WAVE_IN_MIDDLE         = 0.5F;
constexpr auto PROB_CHANGE_SPEED           = 0.5F;
constexpr auto PROB_REVERSE_SPEED          = 0.5F;

constexpr auto FILTER_MODE_NAMES = EnumMap<ZoomFilterMode, std::string_view>{{{
    {ZoomFilterMode::AMULET_MODE, "Amulet"},
    {ZoomFilterMode::CRYSTAL_BALL_MODE0, "Crystal Ball Mode 0"},
    {ZoomFilterMode::CRYSTAL_BALL_MODE1, "Crystal Ball Mode 1"},
    {ZoomFilterMode::DISTANCE_FIELD_MODE0, "Distance Field Mode 0"},
    {ZoomFilterMode::DISTANCE_FIELD_MODE1, "Distance Field Mode 1"},
    {ZoomFilterMode::DISTANCE_FIELD_MODE2, "Distance Field Mode 2"},
    {ZoomFilterMode::EXP_RECIPROCAL_MODE, "Exp Reciprocal"},
    {ZoomFilterMode::HYPERCOS_MODE0, "Hypercos Mode 0"},
    {ZoomFilterMode::HYPERCOS_MODE1, "Hypercos Mode 1"},
    {ZoomFilterMode::HYPERCOS_MODE2, "Hypercos Mode 2"},
    {ZoomFilterMode::HYPERCOS_MODE3, "Hypercos Mode 3"},
    {ZoomFilterMode::IMAGE_DISPLACEMENT_MODE, "Image Displacement"},
    {ZoomFilterMode::NORMAL_MODE, "Normal"},
    {ZoomFilterMode::SCRUNCH_MODE, "Scrunch"},
    {ZoomFilterMode::SPEEDWAY_MODE0, "Speedway Mode 0"},
    {ZoomFilterMode::SPEEDWAY_MODE1, "Speedway Mode 1"},
    {ZoomFilterMode::SPEEDWAY_MODE2, "Speedway Mode 2"},
    {ZoomFilterMode::WATER_MODE, "Water"},
    {ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE0, "Wave Sq Dist Mode 0"},
    {ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE1, "Wave Sq Dist Mode 1"},
    {ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE0, "Wave Atan Mode 0"},
    {ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE1, "Wave Atan Mode 1"},
    {ZoomFilterMode::Y_ONLY_MODE, "Y Only"},
}}};

using AfterEffectsProbs = EnumMap<AfterEffectsTypes, float>;

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

  using FiltMode   = ZoomFilterMode;
  using EffectType = AfterEffectsTypes;

  auto effectsProbs =
      GetFilledEnumMap<ZoomFilterMode, AfterEffectsProbs>(DEFAULT_AFTER_EFFECTS_PROBS);

  effectsProbs[FiltMode::AMULET_MODE][EffectType::ROTATION] = PROB_HIGH;

  effectsProbs[FiltMode::CRYSTAL_BALL_MODE0][EffectType::ROTATION] = PROB_HIGH;
  effectsProbs[FiltMode::CRYSTAL_BALL_MODE1][EffectType::ROTATION] = PROB_HIGH;

  effectsProbs[FiltMode::DISTANCE_FIELD_MODE0][EffectType::ROTATION] = PROB_HIGH;
  effectsProbs[FiltMode::DISTANCE_FIELD_MODE1][EffectType::ROTATION] = PROB_HIGH;
  effectsProbs[FiltMode::DISTANCE_FIELD_MODE2][EffectType::ROTATION] = PROB_HIGH;

  effectsProbs[FiltMode::EXP_RECIPROCAL_MODE][EffectType::ROTATION] = PROB_LOW;

  effectsProbs[FiltMode::HYPERCOS_MODE0][EffectType::ROTATION] = PROB_LOW;
  effectsProbs[FiltMode::HYPERCOS_MODE1][EffectType::ROTATION] = PROB_LOW;
  effectsProbs[FiltMode::HYPERCOS_MODE2][EffectType::ROTATION] = PROB_LOW;
  effectsProbs[FiltMode::HYPERCOS_MODE3][EffectType::ROTATION] = PROB_LOW;

  effectsProbs[FiltMode::IMAGE_DISPLACEMENT_MODE][EffectType::ROTATION] = PROB_ZERO;

  effectsProbs[FiltMode::NORMAL_MODE][EffectType::ROTATION] = PROB_ZERO;

  effectsProbs[FiltMode::SCRUNCH_MODE][EffectType::ROTATION] = PROB_HALF;

  effectsProbs[FiltMode::SPEEDWAY_MODE0][EffectType::ROTATION] = PROB_HALF;
  effectsProbs[FiltMode::SPEEDWAY_MODE1][EffectType::ROTATION] = PROB_HIGH;
  effectsProbs[FiltMode::SPEEDWAY_MODE2][EffectType::ROTATION] = PROB_HIGH;

  effectsProbs[FiltMode::WATER_MODE][EffectType::ROTATION] = PROB_ZERO;

  effectsProbs[FiltMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE0][EffectType::PLANES]   = WAVE0_PROB_PLANE;
  effectsProbs[FiltMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE0][EffectType::ROTATION] = PROB_HIGH;
  effectsProbs[FiltMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE1][EffectType::PLANES]   = WAVE1_PROB_PLANE;
  effectsProbs[FiltMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE1][EffectType::ROTATION] = PROB_HIGH;

  effectsProbs[FiltMode::WAVE_ATAN_ANGLE_EFFECT_MODE0][EffectType::PLANES]   = WAVE0_PROB_PLANE;
  effectsProbs[FiltMode::WAVE_ATAN_ANGLE_EFFECT_MODE0][EffectType::ROTATION] = PROB_HIGH;
  effectsProbs[FiltMode::WAVE_ATAN_ANGLE_EFFECT_MODE1][EffectType::PLANES]   = WAVE1_PROB_PLANE;
  effectsProbs[FiltMode::WAVE_ATAN_ANGLE_EFFECT_MODE1][EffectType::ROTATION] = PROB_HIGH;

  effectsProbs[FiltMode::Y_ONLY_MODE][EffectType::ROTATION] = PROB_HALF;

  return effectsProbs;
}

constexpr auto DEFAULT_AFTER_EFFECTS_STATES    = GetFilledEnumMap<AfterEffectsTypes, bool>(false);
constexpr auto DEFAULT_AFTER_EFFECTS_PROBS     = GetFilledEnumMap<AfterEffectsTypes, float>(0.0F);
constexpr auto ZERO_REPEAT_AFTER_EFFECTS_PROBS = GetFilledEnumMap<AfterEffectsTypes, float>(0.0F);
constexpr auto DEFAULT_REPEAT_AFTER_EFFECTS_PROBS = EnumMap<AfterEffectsTypes, float>{{{
    {AfterEffectsTypes::HYPERCOS, 0.5F},
    {AfterEffectsTypes::IMAGE_VELOCITY, 0.8F},
    {AfterEffectsTypes::NOISE, 0.0F},
    {AfterEffectsTypes::PLANES, 0.0F},
    {AfterEffectsTypes::ROTATION, 0.0F},
    {AfterEffectsTypes::TAN_EFFECT, 0.0F},
    {AfterEffectsTypes::XY_LERP_EFFECT, 0.1F},
}}};
constexpr auto DEFAULT_AFTER_EFFECTS_OFF_TIMES    = EnumMap<AfterEffectsTypes, uint32_t>{{{
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

// TODO(glk) - Can make this 'constexpr' with C++20.

auto GetWeightedFilterEvents(const UTILS::MATH::IGoomRand& goomRand)
    -> UTILS::MATH::ConditionalWeights<ZoomFilterMode>
{
  static constexpr auto AMULET_MODE_WEIGHT             = 10.0F;
  static constexpr auto CRYSTAL_BALL_MODE0_WEIGHT      = 04.0F;
  static constexpr auto CRYSTAL_BALL_MODE1_WEIGHT      = 02.0F;
  static constexpr auto DISTANCE_FIELD_MODE0_WEIGHT    = 03.0F;
  static constexpr auto DISTANCE_FIELD_MODE1_WEIGHT    = 03.0F;
  static constexpr auto DISTANCE_FIELD_MODE2_WEIGHT    = 02.0F;
  static constexpr auto EXP_RECIPROCAL_MODE_WEIGHT     = 10.0F;
  static constexpr auto HYPERCOS_MODE0_WEIGHT          = 08.0F;
  static constexpr auto HYPERCOS_MODE1_WEIGHT          = 04.0F;
  static constexpr auto HYPERCOS_MODE2_WEIGHT          = 02.0F;
  static constexpr auto HYPERCOS_MODE3_WEIGHT          = 01.0F;
  static constexpr auto IMAGE_DISPLACEMENT_MODE_WEIGHT = 05.0F;
  static constexpr auto NORMAL_MODE_WEIGHT             = 10.0F;
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

  // TODO(glk) - When we get to use C++20, replace the below 'inline consts' with 'constexpr'.
  static const auto s_CRYSTAL_BALL_MOD_E0_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {ZoomFilterMode::CRYSTAL_BALL_MODE0, 0.0F},
      {ZoomFilterMode::CRYSTAL_BALL_MODE1, 0.0F},
  };
  static const auto s_CRYSTAL_BALL_MOD_E1_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {ZoomFilterMode::CRYSTAL_BALL_MODE0, 0.0F},
      {ZoomFilterMode::CRYSTAL_BALL_MODE1, 0.0F},
  };
  static const auto s_NORMAL_MODE_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {         ZoomFilterMode::NORMAL_MODE, 1.0F},
      {ZoomFilterMode::DISTANCE_FIELD_MODE0, 2.0F},
      {ZoomFilterMode::DISTANCE_FIELD_MODE0, 2.0F},
      {ZoomFilterMode::DISTANCE_FIELD_MODE0, 2.0F},
  };
  static const auto s_HYPERCOS_MOD_E0_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {ZoomFilterMode::HYPERCOS_MODE0, 0.0F},
      {ZoomFilterMode::HYPERCOS_MODE1, 0.0F},
      {ZoomFilterMode::HYPERCOS_MODE2, 0.0F},
      {ZoomFilterMode::HYPERCOS_MODE3, 0.0F},
  };
  static const auto s_HYPERCOS_MOD_E1_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {ZoomFilterMode::HYPERCOS_MODE0, 1.0F}, // OK for mode0 to follow
      {ZoomFilterMode::HYPERCOS_MODE1, 0.0F},
      {ZoomFilterMode::HYPERCOS_MODE2, 0.0F},
      {ZoomFilterMode::HYPERCOS_MODE3, 0.0F},
  };
  static const auto s_HYPERCOS_MOD_E2_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {ZoomFilterMode::HYPERCOS_MODE0, 1.0F}, // OK for mode0 to follow
      {ZoomFilterMode::HYPERCOS_MODE1, 0.0F},
      {ZoomFilterMode::HYPERCOS_MODE2, 0.0F},
      {ZoomFilterMode::HYPERCOS_MODE3, 0.0F},
  };
  static const auto s_HYPERCOS_MOD_E3_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {ZoomFilterMode::HYPERCOS_MODE0, 1.0F}, // OK for mode0 to follow
      {ZoomFilterMode::HYPERCOS_MODE1, 0.0F},
      {ZoomFilterMode::HYPERCOS_MODE2, 0.0F},
      {ZoomFilterMode::HYPERCOS_MODE3, 0.0F},
  };
  static const auto s_SPEEDWAY_MOD_E0_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {ZoomFilterMode::SPEEDWAY_MODE0, 0.0F},
      {ZoomFilterMode::SPEEDWAY_MODE1, 0.0F},
      {ZoomFilterMode::SPEEDWAY_MODE2, 0.0F},
  };
  static const auto s_SPEEDWAY_MOD_E1_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {ZoomFilterMode::SPEEDWAY_MODE0, 0.0F},
      {ZoomFilterMode::SPEEDWAY_MODE1, 0.0F},
      {ZoomFilterMode::SPEEDWAY_MODE2, 0.0F},
  };
  static const auto s_SPEEDWAY_MOD_E2_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {ZoomFilterMode::SPEEDWAY_MODE0, 0.0F},
      {ZoomFilterMode::SPEEDWAY_MODE1, 0.0F},
      {ZoomFilterMode::SPEEDWAY_MODE2, 0.0F},
  };
  static const auto s_WAVE_SQ_DIST_MOD_E0_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE0, 0.0F},
      {ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE1, 0.0F},
  };
  static const auto s_WAVE_SQ_DIST_MOD_E1_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE0, 0.0F},
      {ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE1, 0.0F},
  };
  static const auto s_WAVE_ATAN_MOD_E0_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE0, 0.0F},
      {ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE1, 0.0F},
  };
  static const auto s_WAVE_ATAN_MOD_E1_MULTIPLIERS = std::map<ZoomFilterMode, float>{
      {ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE0, 0.0F},
      {ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE1, 0.0F},
  };

  return {
      goomRand,
      {
        {ZoomFilterMode::AMULET_MODE, AMULET_MODE_WEIGHT},
        {ZoomFilterMode::CRYSTAL_BALL_MODE0, CRYSTAL_BALL_MODE0_WEIGHT},
        {ZoomFilterMode::CRYSTAL_BALL_MODE1, CRYSTAL_BALL_MODE1_WEIGHT},
        {ZoomFilterMode::DISTANCE_FIELD_MODE0, DISTANCE_FIELD_MODE0_WEIGHT},
        {ZoomFilterMode::DISTANCE_FIELD_MODE1, DISTANCE_FIELD_MODE1_WEIGHT},
        {ZoomFilterMode::DISTANCE_FIELD_MODE2, DISTANCE_FIELD_MODE2_WEIGHT},
        {ZoomFilterMode::EXP_RECIPROCAL_MODE, EXP_RECIPROCAL_MODE_WEIGHT},
        {ZoomFilterMode::HYPERCOS_MODE0, HYPERCOS_MODE0_WEIGHT},
        {ZoomFilterMode::HYPERCOS_MODE1, HYPERCOS_MODE1_WEIGHT},
        {ZoomFilterMode::HYPERCOS_MODE2, HYPERCOS_MODE2_WEIGHT},
        {ZoomFilterMode::HYPERCOS_MODE3, HYPERCOS_MODE3_WEIGHT},
        {ZoomFilterMode::IMAGE_DISPLACEMENT_MODE, IMAGE_DISPLACEMENT_MODE_WEIGHT},
        {ZoomFilterMode::NORMAL_MODE, NORMAL_MODE_WEIGHT},
        {ZoomFilterMode::SCRUNCH_MODE, SCRUNCH_MODE_WEIGHT},
        {ZoomFilterMode::SPEEDWAY_MODE0, SPEEDWAY_MODE0_WEIGHT},
        {ZoomFilterMode::SPEEDWAY_MODE1, SPEEDWAY_MODE1_WEIGHT},
        {ZoomFilterMode::SPEEDWAY_MODE2, SPEEDWAY_MODE2_WEIGHT},
        {ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE0, WAVE_SQ_DIST_MODE0_WEIGHT},
        {ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE1, WAVE_SQ_DIST_MODE1_WEIGHT},
        {ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE0, WAVE_ATAN_MODE0_WEIGHT},
        {ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE1, WAVE_ATAN_MODE1_WEIGHT},
        {ZoomFilterMode::WATER_MODE, WATER_MODE_WEIGHT},
        {ZoomFilterMode::Y_ONLY_MODE, Y_ONLY_MODE_WEIGHT},
        },
      {
        {ZoomFilterMode::CRYSTAL_BALL_MODE0, s_CRYSTAL_BALL_MOD_E0_MULTIPLIERS},
        {ZoomFilterMode::CRYSTAL_BALL_MODE1, s_CRYSTAL_BALL_MOD_E1_MULTIPLIERS},
        {ZoomFilterMode::NORMAL_MODE, s_NORMAL_MODE_MULTIPLIERS},
        {ZoomFilterMode::HYPERCOS_MODE0, s_HYPERCOS_MOD_E0_MULTIPLIERS},
        {ZoomFilterMode::HYPERCOS_MODE1, s_HYPERCOS_MOD_E1_MULTIPLIERS},
        {ZoomFilterMode::HYPERCOS_MODE2, s_HYPERCOS_MOD_E2_MULTIPLIERS},
        {ZoomFilterMode::HYPERCOS_MODE3, s_HYPERCOS_MOD_E3_MULTIPLIERS},
        {ZoomFilterMode::SPEEDWAY_MODE0, s_SPEEDWAY_MOD_E0_MULTIPLIERS},
        {ZoomFilterMode::SPEEDWAY_MODE1, s_SPEEDWAY_MOD_E1_MULTIPLIERS},
        {ZoomFilterMode::SPEEDWAY_MODE2, s_SPEEDWAY_MOD_E2_MULTIPLIERS},
        {ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE0, s_WAVE_SQ_DIST_MOD_E0_MULTIPLIERS},
        {ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE1, s_WAVE_SQ_DIST_MOD_E1_MULTIPLIERS},
        {ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE0, s_WAVE_ATAN_MOD_E0_MULTIPLIERS},
        {ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE1, s_WAVE_ATAN_MOD_E1_MULTIPLIERS},
        }
  };
}

// TODO(glk) - Can make this 'constexpr' with C++20.

[[nodiscard]] auto GetHypercosWeights(const ZoomFilterMode filterMode) noexcept
    -> std::vector<Weights<HypercosOverlayMode>::KeyValue>
{
  constexpr auto FORCED_HYPERCOS =
      USE_FORCED_AFTER_EFFECT and (FORCED_AFTER_EFFECTS_TYPE == AfterEffectsTypes::HYPERCOS);

  using Hyp         = HypercosOverlayMode;
  using ModeWeights = std::array<Weights<HypercosOverlayMode>::KeyValue, NUM<HypercosOverlayMode>>;

  constexpr auto AMULET_HYPERCOS_WEIGHTS = ModeWeights{
      {{Hyp::NONE, FORCED_HYPERCOS ? 0.0F : 20.0F},
       {Hyp::MODE0, 1.0F},
       {Hyp::MODE1, 5.0F},
       {Hyp::MODE2, 1.0F},
       {Hyp::MODE3, 1.0F}}
  };
  constexpr auto CRYSTAL_BALL0_HYPERCOS_WEIGHTS = ModeWeights{
      {{Hyp::NONE, FORCED_HYPERCOS ? 0.0F : 5.0F},
       {Hyp::MODE0, 10.0F},
       {Hyp::MODE1, 1.0F},
       {Hyp::MODE2, 1.0F},
       {Hyp::MODE3, 1.0F}}
  };
  constexpr auto CRYSTAL_BALL1_HYPERCOS_WEIGHTS = ModeWeights{
      {{Hyp::NONE, FORCED_HYPERCOS ? 0.0F : 5.0F},
       {Hyp::MODE0, 1.0F},
       {Hyp::MODE1, 99.0F},
       {Hyp::MODE2, 1.0F},
       {Hyp::MODE3, 1.0F}}
  };
  constexpr auto DISTANCE_FIELD_HYPERCOS_WEIGHTS = ModeWeights{
      {{Hyp::NONE, FORCED_HYPERCOS ? 0.0F : 5.0F},
       {Hyp::MODE0, 10.0F},
       {Hyp::MODE1, 1.0F},
       {Hyp::MODE2, 1.0F},
       {Hyp::MODE3, 1.0F}}
  };
  constexpr auto EXP_RECIPROCAL_HYPERCOS_WEIGHTS = ModeWeights{
      {{Hyp::NONE, FORCED_HYPERCOS ? 0.0F : 10.0F},
       {Hyp::MODE0, 5.0F},
       {Hyp::MODE1, 1.0F},
       {Hyp::MODE2, 1.0F},
       {Hyp::MODE3, 0.0F}}
  };
  constexpr auto HYPERCOS0_HYPERCOS_WEIGHTS = ModeWeights{
      {{Hyp::NONE, FORCED_HYPERCOS ? 0.0F : 1.0F},
       {Hyp::MODE0, 1.0F},
       {Hyp::MODE1, 2.0F},
       {Hyp::MODE2, 2.0F},
       {Hyp::MODE3, 2.0F}}
  };
  constexpr auto HYPERCOS1_HYPERCOS_WEIGHTS = ModeWeights{
      {{Hyp::NONE, FORCED_HYPERCOS ? 0.0F : 1.0F},
       {Hyp::MODE0, 2.0F},
       {Hyp::MODE1, 1.0F},
       {Hyp::MODE2, 2.0F},
       {Hyp::MODE3, 2.0F}}
  };
  constexpr auto HYPERCOS2_HYPERCOS_WEIGHTS = ModeWeights{
      {{Hyp::NONE, FORCED_HYPERCOS ? 0.0F : 1.0F},
       {Hyp::MODE0, 2.0F},
       {Hyp::MODE1, 2.0F},
       {Hyp::MODE2, 1.0F},
       {Hyp::MODE3, 2.0F}}
  };
  constexpr auto HYPERCOS3_HYPERCOS_WEIGHTS = ModeWeights{
      {{Hyp::NONE, FORCED_HYPERCOS ? 0.0F : 1.0F},
       {Hyp::MODE0, 2.0F},
       {Hyp::MODE1, 2.0F},
       {Hyp::MODE2, 2.0F},
       {Hyp::MODE3, 1.0F}}
  };
  constexpr auto IMAGE_DISPLACEMENT_HYPERCOS_WEIGHTS = ModeWeights{
      {{Hyp::NONE, FORCED_HYPERCOS ? 0.0F : 10.0F},
       {Hyp::MODE0, 1.0F},
       {Hyp::MODE1, 5.0F},
       {Hyp::MODE2, 1.0F},
       {Hyp::MODE3, 1.0F}}
  };
  constexpr auto NORMAL_HYPERCOS_WEIGHTS = ModeWeights{
      {{Hyp::NONE, FORCED_HYPERCOS ? 0.0F : 10.0F},
       {Hyp::MODE0, 5.0F},
       {Hyp::MODE1, 1.0F},
       {Hyp::MODE2, 1.0F},
       {Hyp::MODE3, 0.0F}}
  };
  constexpr auto SCRUNCH_HYPERCOS_WEIGHTS = ModeWeights{
      {{Hyp::NONE, FORCED_HYPERCOS ? 0.0F : 10.0F},
       {Hyp::MODE0, 1.0F},
       {Hyp::MODE1, 5.0F},
       {Hyp::MODE2, 1.0F},
       {Hyp::MODE3, 1.0F}}
  };
  constexpr auto SPEEDWAY_HYPERCOS_WEIGHTS = ModeWeights{
      {{Hyp::NONE, FORCED_HYPERCOS ? 0.0F : 10.0F},
       {Hyp::MODE0, 5.0F},
       {Hyp::MODE1, 1.0F},
       {Hyp::MODE2, 1.0F},
       {Hyp::MODE3, 1.0F}}
  };
  constexpr auto WATER_HYPERCOS_WEIGHTS = ModeWeights{
      {{Hyp::NONE, FORCED_HYPERCOS ? 0.0F : 10.0F},
       {Hyp::MODE0, 1.0F},
       {Hyp::MODE1, 5.0F},
       {Hyp::MODE2, 1.0F},
       {Hyp::MODE3, 1.0F}}
  };
  constexpr auto WAVE_SQ_DIST_MODE0_HYPERCOS_WEIGHTS = ModeWeights{
      {{Hyp::NONE, FORCED_HYPERCOS ? 0.0F : 10.0F},
       {Hyp::MODE0, 5.0F},
       {Hyp::MODE1, 1.0F},
       {Hyp::MODE2, 1.0F},
       {Hyp::MODE3, 1.0F}}
  };
  constexpr auto WAVE_SQ_DIST_MODE1_HYPERCOS_WEIGHTS = ModeWeights{
      {{Hyp::NONE, FORCED_HYPERCOS ? 0.0F : 10.0F},
       {Hyp::MODE0, 1.0F},
       {Hyp::MODE1, 5.0F},
       {Hyp::MODE2, 1.0F},
       {Hyp::MODE3, 1.0F}}
  };
  constexpr auto WAVE_ATAN_MODE0_HYPERCOS_WEIGHTS = ModeWeights{
      {{Hyp::NONE, FORCED_HYPERCOS ? 0.0F : 10.0F},
       {Hyp::MODE0, 5.0F},
       {Hyp::MODE1, 1.0F},
       {Hyp::MODE2, 1.0F},
       {Hyp::MODE3, 1.0F}}
  };
  constexpr auto WAVE_ATAN_MODE1_HYPERCOS_WEIGHTS = ModeWeights{
      {{Hyp::NONE, FORCED_HYPERCOS ? 0.0F : 10.0F},
       {Hyp::MODE0, 1.0F},
       {Hyp::MODE1, 5.0F},
       {Hyp::MODE2, 1.0F},
       {Hyp::MODE3, 1.0F}}
  };
  constexpr auto Y_ONLY_HYPERCOS_WEIGHTS = ModeWeights{
      {{Hyp::NONE, FORCED_HYPERCOS ? 0.0F : 10.0F},
       {Hyp::MODE0, 1.0F},
       {Hyp::MODE1, 5.0F},
       {Hyp::MODE2, 1.0F},
       {Hyp::MODE3, 1.0F}}
  };

  constexpr auto HYPERCOS_WEIGHTS = EnumMap<ZoomFilterMode, ModeWeights>{{{
      {ZoomFilterMode::AMULET_MODE, AMULET_HYPERCOS_WEIGHTS},
      {ZoomFilterMode::CRYSTAL_BALL_MODE0, CRYSTAL_BALL0_HYPERCOS_WEIGHTS},
      {ZoomFilterMode::CRYSTAL_BALL_MODE1, CRYSTAL_BALL1_HYPERCOS_WEIGHTS},
      {ZoomFilterMode::DISTANCE_FIELD_MODE0, DISTANCE_FIELD_HYPERCOS_WEIGHTS},
      {ZoomFilterMode::DISTANCE_FIELD_MODE1, DISTANCE_FIELD_HYPERCOS_WEIGHTS},
      {ZoomFilterMode::DISTANCE_FIELD_MODE2, DISTANCE_FIELD_HYPERCOS_WEIGHTS},
      {ZoomFilterMode::EXP_RECIPROCAL_MODE, EXP_RECIPROCAL_HYPERCOS_WEIGHTS},
      {ZoomFilterMode::HYPERCOS_MODE0, HYPERCOS0_HYPERCOS_WEIGHTS},
      {ZoomFilterMode::HYPERCOS_MODE1, HYPERCOS1_HYPERCOS_WEIGHTS},
      {ZoomFilterMode::HYPERCOS_MODE2, HYPERCOS2_HYPERCOS_WEIGHTS},
      {ZoomFilterMode::HYPERCOS_MODE3, HYPERCOS3_HYPERCOS_WEIGHTS},
      {ZoomFilterMode::IMAGE_DISPLACEMENT_MODE, IMAGE_DISPLACEMENT_HYPERCOS_WEIGHTS},
      {ZoomFilterMode::NORMAL_MODE, NORMAL_HYPERCOS_WEIGHTS},
      {ZoomFilterMode::SCRUNCH_MODE, SCRUNCH_HYPERCOS_WEIGHTS},
      {ZoomFilterMode::SPEEDWAY_MODE0, SPEEDWAY_HYPERCOS_WEIGHTS},
      {ZoomFilterMode::SPEEDWAY_MODE1, SPEEDWAY_HYPERCOS_WEIGHTS},
      {ZoomFilterMode::SPEEDWAY_MODE2, SPEEDWAY_HYPERCOS_WEIGHTS},
      {ZoomFilterMode::WATER_MODE, WATER_HYPERCOS_WEIGHTS},
      {ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE0, WAVE_SQ_DIST_MODE0_HYPERCOS_WEIGHTS},
      {ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE1, WAVE_SQ_DIST_MODE1_HYPERCOS_WEIGHTS},
      {ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE0, WAVE_ATAN_MODE0_HYPERCOS_WEIGHTS},
      {ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE1, WAVE_ATAN_MODE1_HYPERCOS_WEIGHTS},
      {ZoomFilterMode::Y_ONLY_MODE, Y_ONLY_HYPERCOS_WEIGHTS},
  }}};
  static_assert(HYPERCOS_WEIGHTS.size() == NUM<ZoomFilterMode>);

  return std::vector<Weights<HypercosOverlayMode>::KeyValue>{cbegin(HYPERCOS_WEIGHTS[filterMode]),
                                                             cend(HYPERCOS_WEIGHTS[filterMode])};
}

[[nodiscard]] auto GetFilterModeData(
    const IGoomRand& goomRand,
    const std::string& resourcesDirectory,
    const FilterSettingsService::CreateZoomAdjustmentEffectFunc& createZoomAdjustmentEffect)
    -> FilterSettingsService::FilterModeEnumMap
{
  static_assert(FILTER_MODE_NAMES.size() == NUM<ZoomFilterMode>);
  static_assert(GetEffectsProbabilities().size() == NUM<ZoomFilterMode>);

  auto filterModeVec = std::vector<FilterSettingsService::FilterModeEnumMap::KeyValue>{};

  for (auto i = 0U; i < NUM<ZoomFilterMode>; ++i)
  {
    const auto filterMode = static_cast<ZoomFilterMode>(i);

    filterModeVec.emplace_back(
        filterMode,
        FilterSettingsService::ZoomFilterModeInfo{
            FILTER_MODE_NAMES[filterMode],
            createZoomAdjustmentEffect(filterMode, goomRand, resourcesDirectory),
            {Weights<HypercosOverlayMode>{goomRand, GetHypercosWeights(filterMode)},
                                                        GetAfterEffectsProbability(filterMode)},
    });
  }

  return FilterSettingsService::FilterModeEnumMap::Make(std::move(filterModeVec));
}

} // namespace

FilterSettingsService::FilterSettingsService(const PluginInfo& goomInfo,
                                             const IGoomRand& goomRand,
                                             const std::string& resourcesDirectory,
                                             const CreateZoomAdjustmentEffectFunc&
                                                 createZoomAdjustmentEffect)
  : m_goomInfo{&goomInfo},
    m_goomRand{&goomRand},
    m_screenCentre{goomInfo.GetDimensions().GetCentrePoint()},
    m_resourcesDirectory{resourcesDirectory},
    m_randomizedAfterEffects{
        std::make_unique<AfterEffectsStates>(goomInfo.GetTime(),
                                             goomRand,
                                             GetRepeatAfterEffectsProbability(),
                                             GetAfterEffectsOffTime())},
    m_filterModeData{GetFilterModeData(goomRand,
                                       m_resourcesDirectory,
                                       createZoomAdjustmentEffect)},
    m_filterSettings{
        false,
        {
           Vitesse{},
           DEFAULT_MAX_ZOOM_ADJUSTMENT,
           DEFAULT_BASE_ZOOM_ADJUSTMENT_FACTOR_MULTIPLIER,
           DEFAULT_AFTER_EFFECTS_VELOCITY_CONTRIBUTION,
           nullptr,
           {DEFAULT_ZOOM_MID_X, DEFAULT_ZOOM_MID_Y},
           DEFAULT_FILTER_VIEWPORT,
           {
               DEFAULT_MULTIPLIER_EFFECT_IS_ACTIVE,
               DEFAULT_MULTIPLIER_EFFECT_X_FREQ,
               DEFAULT_MULTIPLIER_EFFECT_Y_FREQ,
               DEFAULT_MULTIPLIER_EFFECT_X_AMPLITUDE,
               DEFAULT_MULTIPLIER_EFFECT_Y_AMPLITUDE,
               DEFAULT_LERP_ZOOM_ADJUSTMENT_TO_COORDS
           },
           {
               HypercosOverlayMode::NONE,
               DEFAULT_AFTER_EFFECTS_STATES,
               RotationAdjustments{},
            }
        },
        {0.0F, DEFAULT_TRAN_LERP_INCREMENT, DEFAULT_SWITCH_MULT},
    },
    m_weightedFilterEvents{GetWeightedFilterEvents(goomRand)},
    m_zoomMidpointWeights{
      goomRand,
      {
          {ZoomMidpointEvents::BOTTOM_MID_POINT,            BOTTOM_MID_POINT_WEIGHT},
          {ZoomMidpointEvents::TOP_MID_POINT,               TOP_MID_POINT_WEIGHT},
          {ZoomMidpointEvents::LEFT_MID_POINT,              LEFT_MID_POINT_WEIGHT},
          {ZoomMidpointEvents::RIGHT_MID_POINT,             RIGHT_MID_POINT_WEIGHT},
          {ZoomMidpointEvents::CENTRE_MID_POINT,            CENTRE_MID_POINT_WEIGHT},
          {ZoomMidpointEvents::BOTTOM_LEFT_QUARTER_MID_POINT,
                                                            BOTTOM_LEFT_QUARTER_MID_POINT_WEIGHT},
          {ZoomMidpointEvents::TOP_LEFT_QUARTER_MID_POINT,  TOP_LEFT_QUARTER_MID_POINT_WEIGHT},
          {ZoomMidpointEvents::BOTTOM_RIGHT_QUARTER_MID_POINT,
                                                            BOTTOM_RIGHT_QUARTER_MID_POINT_WEIGHT},
          {ZoomMidpointEvents::TOP_RIGHT_QUARTER_MID_POINT, TOP_RIGHT_QUARTER_MID_POINT_WEIGHT},
      }
    }
{
  static_assert(DEFAULT_MULTIPLIER_EFFECT_X_FREQ >= MIN_MULTIPLIER_EFFECT_FREQ);
  static_assert(DEFAULT_MULTIPLIER_EFFECT_X_FREQ <= MAX_MULTIPLIER_EFFECT_FREQ);
  static_assert(DEFAULT_MULTIPLIER_EFFECT_Y_FREQ >= MIN_MULTIPLIER_EFFECT_FREQ);
  static_assert(DEFAULT_MULTIPLIER_EFFECT_Y_FREQ <= MAX_MULTIPLIER_EFFECT_FREQ);
  static_assert(DEFAULT_MULTIPLIER_EFFECT_X_AMPLITUDE >= MIN_MULTIPLIER_EFFECT_AMPLITUDE);
  static_assert(DEFAULT_MULTIPLIER_EFFECT_X_AMPLITUDE <= MAX_MULTIPLIER_EFFECT_AMPLITUDE);
  static_assert(DEFAULT_MULTIPLIER_EFFECT_Y_AMPLITUDE >= MIN_MULTIPLIER_EFFECT_AMPLITUDE);
  static_assert(DEFAULT_MULTIPLIER_EFFECT_Y_AMPLITUDE <= MAX_MULTIPLIER_EFFECT_AMPLITUDE);
  static_assert(DEFAULT_LERP_ZOOM_ADJUSTMENT_TO_COORDS >= 0.0F);
  static_assert(DEFAULT_LERP_ZOOM_ADJUSTMENT_TO_COORDS <= 1.0F);
}

FilterSettingsService::~FilterSettingsService() noexcept = default;

auto FilterSettingsService::GetNewRandomMode() const -> ZoomFilterMode
{
  if constexpr (USE_FORCED_FILTER_MODE)
  {
    return FORCED_FILTER_MODE;
  }
  return m_weightedFilterEvents.GetRandomWeighted(m_filterMode);
}

auto FilterSettingsService::Start() -> void
{
  SetNewRandomFilter();
}

inline auto FilterSettingsService::GetZoomAdjustmentEffect()
    -> std::shared_ptr<IZoomAdjustmentEffect>&
{
  return m_filterModeData[m_filterMode].zoomAdjustmentEffect;
}

auto FilterSettingsService::NewCycle() -> void
{
  m_filterModeAtLastUpdate = m_filterMode;
  m_filterSettings.transformBufferLerpData.Update();
}

auto FilterSettingsService::NotifyUpdatedFilterEffectsSettings() -> void
{
  m_filterSettings.filterEffectsSettingsHaveChanged = false;

  m_randomizedAfterEffects->CheckForPendingOffTimers();
}

auto FilterSettingsService::SetDefaultSettings() -> void
{
  m_filterSettings.filterEffectsSettings.zoomAdjustmentEffect = GetZoomAdjustmentEffect();
  m_filterSettings.filterEffectsSettings.zoomMidpoint         = m_screenCentre;
  m_filterSettings.filterEffectsSettings.filterViewport       = Viewport{};
  m_filterSettings.filterEffectsSettings.vitesse.SetDefault();

  m_randomizedAfterEffects->SetDefaults();
}

auto FilterSettingsService::SetFilterModeRandomViewport() -> void
{
  m_filterSettings.filterEffectsSettings.filterViewport =
      m_filterModeData[m_filterMode].zoomAdjustmentEffect->GetZoomAdjustmentViewport();
}

auto FilterSettingsService::SetFilterModeRandomEffects() -> void
{
  m_filterSettings.filterEffectsSettings.zoomAdjustmentEffect->SetRandomParams();
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

  if (not m_goomRand->ProbabilityOf(PROB_ACTIVE_MULTIPLIER_EFFECT))
  {
    multiplierEffectsSettings.isActive = false;
  }
  else
  {
    multiplierEffectsSettings.isActive = true;

    multiplierEffectsSettings.xFreq =
        m_goomRand->GetRandInRange(MIN_MULTIPLIER_EFFECT_FREQ, MAX_MULTIPLIER_EFFECT_FREQ);
    if (m_goomRand->ProbabilityOf(PROB_MULTIPLIER_EFFECT_FREQUENCIES_EQUAL))
    {
      multiplierEffectsSettings.yFreq = multiplierEffectsSettings.xFreq;
    }
    else
    {
      multiplierEffectsSettings.yFreq =
          m_goomRand->GetRandInRange(MIN_MULTIPLIER_EFFECT_FREQ, MAX_MULTIPLIER_EFFECT_FREQ);
    }

    multiplierEffectsSettings.xAmplitude = m_goomRand->GetRandInRange(
        MIN_MULTIPLIER_EFFECT_AMPLITUDE, MAX_MULTIPLIER_EFFECT_AMPLITUDE);
    if (m_goomRand->ProbabilityOf(PROB_MULTIPLIER_EFFECT_AMPLITUDES_EQUAL))
    {
      multiplierEffectsSettings.yAmplitude = multiplierEffectsSettings.xAmplitude;
    }
    else
    {
      multiplierEffectsSettings.yAmplitude = m_goomRand->GetRandInRange(
          MIN_MULTIPLIER_EFFECT_AMPLITUDE, MAX_MULTIPLIER_EFFECT_AMPLITUDE);
    }
  }

  multiplierEffectsSettings.lerpZoomAdjustmentToCoords = m_goomRand->GetRandInRange(0.0F, 1.0F);
}

auto FilterSettingsService::ResetRandomAfterEffects() -> void
{
  const auto& modeInfo = m_filterModeData[m_filterMode];
  m_randomizedAfterEffects->ResetStandardStates(modeInfo.afterEffectsProbabilities);
  m_filterSettings.filterEffectsSettingsHaveChanged = true;
}

auto FilterSettingsService::SetRandomizedAfterEffects() -> void
{
  const auto& modeInfo = m_filterModeData[m_filterMode];

  m_randomizedAfterEffects->ResetAllStates(modeInfo.afterEffectsProbabilities);

  m_filterSettings.filterEffectsSettings.afterEffectsSettings.rotationAdjustments.SetMultiplyFactor(
      modeInfo.afterEffectsProbabilities.probabilities[AfterEffectsTypes::ROTATION],
      RotationAdjustments::AdjustmentType::AFTER_RANDOM);
}

auto FilterSettingsService::SetWaveModeAfterEffects() -> void
{
  if ((m_filterMode != ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE0) and
      (m_filterMode != ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE1) and
      (m_filterMode != ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE0) and
      (m_filterMode != ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE1))
  {
    return;
  }

  m_randomizedAfterEffects->TurnPlaneEffectOn();

  auto& filterEffectsSettings = m_filterSettings.filterEffectsSettings;
  filterEffectsSettings.vitesse.SetReverseVitesse(m_goomRand->ProbabilityOf(PROB_REVERSE_SPEED));
  if (m_goomRand->ProbabilityOf(PROB_CHANGE_SPEED))
  {
    filterEffectsSettings.vitesse.SetVitesse(
        U_HALF * (Vitesse::DEFAULT_SPEED + filterEffectsSettings.vitesse.GetVitesse()));
  }
}

auto FilterSettingsService::UpdateFilterSettingsFromAfterEffects() -> void
{
  m_filterSettings.filterEffectsSettingsHaveChanged = true;
  m_randomizedAfterEffects->UpdateAfterEffectsSettingsFromStates(
      m_filterSettings.filterEffectsSettings.afterEffectsSettings);
}

auto FilterSettingsService::SetBaseZoomAdjustmentFactorMultiplier() noexcept -> void
{
  if (static constexpr auto PROB_CALM_DOWN = 0.8F; m_goomRand->ProbabilityOf(PROB_CALM_DOWN))
  {
    m_filterSettings.filterEffectsSettings.baseZoomAdjustmentFactorMultiplier = 1.0F;
    return;
  }

  // TODO(glk) Lerp between old and new?
  static constexpr auto MULTIPLIER_RANGE = IGoomRand::NumberRange<float>{0.1F, 5.0F};
  static_assert(ZoomVectorEffects::IsValidMultiplierRange(MULTIPLIER_RANGE));

  m_filterSettings.filterEffectsSettings.baseZoomAdjustmentFactorMultiplier =
      m_goomRand->GetRandInRange(MULTIPLIER_RANGE);
}

auto FilterSettingsService::SetAfterEffectsVelocityMultiplier() noexcept -> void
{
  static constexpr auto CONTRIBUTION_RANGE = IGoomRand::NumberRange<float>{0.1F, 1.0F};

  m_filterSettings.filterEffectsSettings.afterEffectsVelocityMultiplier =
      m_goomRand->GetRandInRange(CONTRIBUTION_RANGE);
}

auto FilterSettingsService::SetRandomZoomMidpoint() -> void
{
  if constexpr (ALL_AFTER_EFFECTS_TURNED_OFF)
  {
    m_filterSettings.filterEffectsSettings.zoomMidpoint = m_screenCentre;
    return;
  }
  if (IsZoomMidpointInTheMiddle())
  {
    m_filterSettings.filterEffectsSettings.zoomMidpoint = m_screenCentre;
    return;
  }

  const auto allowEdgePoints = (m_filterMode != ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE0) and
                               (m_filterMode != ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE1) and
                               (m_filterMode != ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE0) and
                               (m_filterMode != ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE1);
  SetAnyRandomZoomMidpoint(allowEdgePoints);
}

auto FilterSettingsService::IsZoomMidpointInTheMiddle() const -> bool
{
  if ((m_filterMode == ZoomFilterMode::WATER_MODE) or (m_filterMode == ZoomFilterMode::AMULET_MODE))
  {
    return true;
  }

  if (((m_filterMode == ZoomFilterMode::CRYSTAL_BALL_MODE0) or
       (m_filterMode == ZoomFilterMode::CRYSTAL_BALL_MODE1)) and
      m_goomRand->ProbabilityOf(PROB_CRYSTAL_BALL_IN_MIDDLE))
  {
    return true;
  }

  if (IsFilterModeAWaveMode() and m_goomRand->ProbabilityOf(PROB_WAVE_IN_MIDDLE))
  {
    return true;
  }

  return false;
}

inline auto FilterSettingsService::IsFilterModeAWaveMode() const -> bool
{
  if (m_filterMode == ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE0)
  {
    return true;
  }
  if (m_filterMode == ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE1)
  {
    return true;
  }
  if (m_filterMode == ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE0)
  {
    return true;
  }
  if (m_filterMode == ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE1)
  {
    return true;
  }
  return false;
}

auto FilterSettingsService::GetWeightRandomMidPoint(const bool allowEdgePoints) const
    -> ZoomMidpointEvents
{
  auto midPointEvent = m_zoomMidpointWeights.GetRandomWeighted();

  if (allowEdgePoints)
  {
    return midPointEvent;
  }

  while (IsEdgeMidPoint(midPointEvent))
  {
    midPointEvent = m_zoomMidpointWeights.GetRandomWeighted();
  }
  return midPointEvent;
}

inline auto FilterSettingsService::IsEdgeMidPoint(const ZoomMidpointEvents midPointEvent) -> bool
{
  return (midPointEvent == ZoomMidpointEvents::BOTTOM_MID_POINT) or
         (midPointEvent == ZoomMidpointEvents::TOP_MID_POINT) or
         (midPointEvent == ZoomMidpointEvents::RIGHT_MID_POINT) or
         (midPointEvent == ZoomMidpointEvents::LEFT_MID_POINT);
}

auto FilterSettingsService::SetAnyRandomZoomMidpoint(const bool allowEdgePoints) -> void
{
  static constexpr auto HEIGHT_MARGIN = 2;

  switch (GetWeightRandomMidPoint(allowEdgePoints))
  {
    case ZoomMidpointEvents::BOTTOM_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = {
          I_HALF * m_goomInfo->GetDimensions().GetIntWidth(),
          m_goomInfo->GetDimensions().GetIntHeight() - HEIGHT_MARGIN};
      break;
    case ZoomMidpointEvents::TOP_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = {
          I_HALF * m_goomInfo->GetDimensions().GetIntWidth(), 1};
      break;
    case ZoomMidpointEvents::LEFT_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = {
          1, I_HALF * m_goomInfo->GetDimensions().GetIntHeight()};
      break;
    case ZoomMidpointEvents::RIGHT_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = {
          m_goomInfo->GetDimensions().GetIntWidth() - HEIGHT_MARGIN,
          I_HALF * m_goomInfo->GetDimensions().GetIntHeight()};
      break;
    case ZoomMidpointEvents::CENTRE_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = m_screenCentre;
      break;
    case ZoomMidpointEvents::BOTTOM_LEFT_QUARTER_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = {
          I_QUARTER * m_goomInfo->GetDimensions().GetIntWidth(),
          I_THREE_QUARTERS * m_goomInfo->GetDimensions().GetIntHeight()};
      break;
    case ZoomMidpointEvents::TOP_LEFT_QUARTER_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = {
          I_QUARTER * m_goomInfo->GetDimensions().GetIntWidth(),
          I_QUARTER * m_goomInfo->GetDimensions().GetIntHeight()};
      break;
    case ZoomMidpointEvents::BOTTOM_RIGHT_QUARTER_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = {
          I_THREE_QUARTERS * m_goomInfo->GetDimensions().GetIntWidth(),
          I_THREE_QUARTERS * m_goomInfo->GetDimensions().GetIntHeight()};
      break;
    case ZoomMidpointEvents::TOP_RIGHT_QUARTER_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = {
          I_THREE_QUARTERS * m_goomInfo->GetDimensions().GetIntWidth(),
          I_QUARTER * m_goomInfo->GetDimensions().GetIntHeight()};
      break;
    default:
      FailFast();
  }
}

} // namespace GOOM::FILTER_FX
