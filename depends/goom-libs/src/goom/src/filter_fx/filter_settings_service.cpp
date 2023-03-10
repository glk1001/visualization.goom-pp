#include "filter_settings_service.h"

#include "after_effects/after_effects_states.h"
#include "filter_consts.h"
#include "filter_effects/zoom_vector_effects.h"
#include "filter_settings.h"
#include "goom_config.h"
#include "goom_plugin_info.h"
#include "utils/enum_utils.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"

#include <array>
#include <stdexcept>
#include <vector>

namespace GOOM::FILTER_FX
{

using AFTER_EFFECTS::AfterEffectsStates;
using AFTER_EFFECTS::AfterEffectsTypes;
using AFTER_EFFECTS::HypercosOverlay;
using AFTER_EFFECTS::RotationAdjustments;
using FILTER_EFFECTS::ZoomVectorEffects;
using UTILS::EnumMap;
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

//constexpr auto FORCED_AFTER_EFFECTS_TYPE = AfterEffectsTypes::BLOCK_WAVY;
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

constexpr auto AMULET_MODE_WEIGHT             = 10.0F;
constexpr auto CRYSTAL_BALL_MODE0_WEIGHT      = 04.0F;
constexpr auto CRYSTAL_BALL_MODE1_WEIGHT      = 02.0F;
constexpr auto DISTANCE_FIELD_MODE0_WEIGHT    = 03.0F;
constexpr auto DISTANCE_FIELD_MODE1_WEIGHT    = 03.0F;
constexpr auto DISTANCE_FIELD_MODE2_WEIGHT    = 02.0F;
constexpr auto EXP_RECIPROCAL_MODE_WEIGHT     = 10.0F;
constexpr auto HYPERCOS_MODE0_WEIGHT          = 08.0F;
constexpr auto HYPERCOS_MODE1_WEIGHT          = 04.0F;
constexpr auto HYPERCOS_MODE2_WEIGHT          = 02.0F;
constexpr auto HYPERCOS_MODE3_WEIGHT          = 01.0F;
constexpr auto IMAGE_DISPLACEMENT_MODE_WEIGHT = 05.0F;
constexpr auto NORMAL_MODE_WEIGHT             = 10.0F;
constexpr auto SCRUNCH_MODE_WEIGHT            = 06.0F;
constexpr auto SPEEDWAY_MODE0_WEIGHT          = 02.0F;
constexpr auto SPEEDWAY_MODE1_WEIGHT          = 01.0F;
constexpr auto SPEEDWAY_MODE2_WEIGHT          = 05.0F;
constexpr auto WAVE_SQ_DIST_MODE0_WEIGHT      = 05.0F;
constexpr auto WAVE_SQ_DIST_MODE1_WEIGHT      = 04.0F;
constexpr auto WAVE_ATAN_MODE0_WEIGHT         = 05.0F;
constexpr auto WAVE_ATAN_MODE1_WEIGHT         = 04.0F;
constexpr auto WATER_MODE_WEIGHT              = 00.0F;
constexpr auto Y_ONLY_MODE_WEIGHT             = 05.0F;

constexpr auto BOTTOM_MID_POINT_WEIGHT               = 03.0F;
constexpr auto TOP_MID_POINT_WEIGHT                  = 03.0F;
constexpr auto LEFT_MID_POINT_WEIGHT                 = 02.0F;
constexpr auto RIGHT_MID_POINT_WEIGHT                = 02.0F;
constexpr auto CENTRE_MID_POINT_WEIGHT               = 18.0F;
constexpr auto TOP_LEFT_QUARTER_MID_POINT_WEIGHT     = 10.0F;
constexpr auto TOP_RIGHT_QUARTER_MID_POINT_WEIGHT    = 10.0F;
constexpr auto BOTTOM_LEFT_QUARTER_MID_POINT_WEIGHT  = 10.0F;
constexpr auto BOTTOM_RIGHT_QUARTER_MID_POINT_WEIGHT = 10.0F;

// TODO(glk) - When we get to use C++20, replace the below 'inline consts' with 'constexpr'.
// NOLINTBEGIN(cert-err58-cpp): Will be fixed with C++20 and 'constexpr'.
inline const auto CRYSTAL_BALL_MODE0_MULTIPLIERS = std::map<ZoomFilterMode, float>{
    {ZoomFilterMode::CRYSTAL_BALL_MODE0, 0.0F},
    {ZoomFilterMode::CRYSTAL_BALL_MODE1, 0.0F},
};
inline const auto CRYSTAL_BALL_MODE1_MULTIPLIERS = std::map<ZoomFilterMode, float>{
    {ZoomFilterMode::CRYSTAL_BALL_MODE0, 0.0F},
    {ZoomFilterMode::CRYSTAL_BALL_MODE1, 0.0F},
};
inline const auto NORMAL_MODE_MULTIPLIERS = std::map<ZoomFilterMode, float>{
    {         ZoomFilterMode::NORMAL_MODE, 1.0F},
    {ZoomFilterMode::DISTANCE_FIELD_MODE0, 2.0F},
    {ZoomFilterMode::DISTANCE_FIELD_MODE0, 2.0F},
    {ZoomFilterMode::DISTANCE_FIELD_MODE0, 2.0F},
};
inline const auto HYPERCOS_MODE0_MULTIPLIERS = std::map<ZoomFilterMode, float>{
    {ZoomFilterMode::HYPERCOS_MODE0, 0.0F},
    {ZoomFilterMode::HYPERCOS_MODE1, 0.0F},
    {ZoomFilterMode::HYPERCOS_MODE2, 0.0F},
    {ZoomFilterMode::HYPERCOS_MODE3, 0.0F},
};
inline const auto HYPERCOS_MODE1_MULTIPLIERS = std::map<ZoomFilterMode, float>{
    {ZoomFilterMode::HYPERCOS_MODE0, 1.0F}, // OK for mode0 to follow
    {ZoomFilterMode::HYPERCOS_MODE1, 0.0F},
    {ZoomFilterMode::HYPERCOS_MODE2, 0.0F},
    {ZoomFilterMode::HYPERCOS_MODE3, 0.0F},
};
inline const auto HYPERCOS_MODE2_MULTIPLIERS = std::map<ZoomFilterMode, float>{
    {ZoomFilterMode::HYPERCOS_MODE0, 1.0F}, // OK for mode0 to follow
    {ZoomFilterMode::HYPERCOS_MODE1, 0.0F},
    {ZoomFilterMode::HYPERCOS_MODE2, 0.0F},
    {ZoomFilterMode::HYPERCOS_MODE3, 0.0F},
};
inline const auto HYPERCOS_MODE3_MULTIPLIERS = std::map<ZoomFilterMode, float>{
    {ZoomFilterMode::HYPERCOS_MODE0, 1.0F}, // OK for mode0 to follow
    {ZoomFilterMode::HYPERCOS_MODE1, 0.0F},
    {ZoomFilterMode::HYPERCOS_MODE2, 0.0F},
    {ZoomFilterMode::HYPERCOS_MODE3, 0.0F},
};
inline const auto SPEEDWAY_MODE0_MULTIPLIERS = std::map<ZoomFilterMode, float>{
    {ZoomFilterMode::SPEEDWAY_MODE0, 0.0F},
    {ZoomFilterMode::SPEEDWAY_MODE1, 0.0F},
    {ZoomFilterMode::SPEEDWAY_MODE2, 0.0F},
};
inline const auto SPEEDWAY_MODE1_MULTIPLIERS = std::map<ZoomFilterMode, float>{
    {ZoomFilterMode::SPEEDWAY_MODE0, 0.0F},
    {ZoomFilterMode::SPEEDWAY_MODE1, 0.0F},
    {ZoomFilterMode::SPEEDWAY_MODE2, 0.0F},
};
inline const auto SPEEDWAY_MODE2_MULTIPLIERS = std::map<ZoomFilterMode, float>{
    {ZoomFilterMode::SPEEDWAY_MODE0, 0.0F},
    {ZoomFilterMode::SPEEDWAY_MODE1, 0.0F},
    {ZoomFilterMode::SPEEDWAY_MODE2, 0.0F},
};
inline const auto WAVE_SQ_DIST_MODE0_MULTIPLIERS = std::map<ZoomFilterMode, float>{
    {ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE0, 0.0F},
    {ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE1, 0.0F},
};
inline const auto WAVE_SQ_DIST_MODE1_MULTIPLIERS = std::map<ZoomFilterMode, float>{
    {ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE0, 0.0F},
    {ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE1, 0.0F},
};
inline const auto WAVE_ATAN_MODE0_MULTIPLIERS = std::map<ZoomFilterMode, float>{
    {ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE0, 0.0F},
    {ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE1, 0.0F},
};
inline const auto WAVE_ATAN_MODE1_MULTIPLIERS = std::map<ZoomFilterMode, float>{
    {ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE0, 0.0F},
    {ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE1, 0.0F},
};
// NOLINTEND(cert-err58-cpp): Will be fixed with C++20 and 'constexpr'.

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

constexpr auto AMULET_PROB_ROTATE             = PROB_HIGH;
constexpr auto CRYSTAL_BALL0_PROB_ROTATE      = PROB_HIGH;
constexpr auto CRYSTAL_BALL1_PROB_ROTATE      = PROB_HIGH;
constexpr auto DISTANCE_FIELD0_PROB_ROTATE    = PROB_HIGH;
constexpr auto DISTANCE_FIELD1_PROB_ROTATE    = PROB_HIGH;
constexpr auto DISTANCE_FIELD2_PROB_ROTATE    = PROB_HIGH;
constexpr auto EXP_RECIPROCAL_PROB_ROTATE     = PROB_LOW;
constexpr auto HYPERCOS0_PROB_ROTATE          = PROB_LOW;
constexpr auto HYPERCOS1_PROB_ROTATE          = PROB_LOW;
constexpr auto HYPERCOS2_PROB_ROTATE          = PROB_LOW;
constexpr auto HYPERCOS3_PROB_ROTATE          = PROB_LOW;
constexpr auto IMAGE_DISPLACEMENT_PROB_ROTATE = PROB_ZERO;
constexpr auto NORMAL_PROB_ROTATE             = PROB_ZERO;
constexpr auto SCRUNCH_PROB_ROTATE            = PROB_HALF;
constexpr auto SPEEDWAY0_PROB_ROTATE          = PROB_HALF;
constexpr auto SPEEDWAY1_PROB_ROTATE          = PROB_HIGH;
constexpr auto SPEEDWAY2_PROB_ROTATE          = PROB_HIGH;
constexpr auto WATER_PROB_ROTATE              = PROB_ZERO;
constexpr auto WAVE0_PROB_ROTATE              = PROB_HIGH;
constexpr auto WAVE1_PROB_ROTATE              = PROB_HIGH;
constexpr auto Y_ONLY_PROB_ROTATE             = PROB_HALF;

constexpr auto DEFAULT_PROB_BLOCKY_WAVY_EFFECT    = 0.3F;
constexpr auto DEFAULT_PROB_HYPERCOS_EFFECT       = 0.9F;
constexpr auto DEFAULT_PROB_IMAGE_VELOCITY_EFFECT = 0.1F;
constexpr auto DEFAULT_PROB_NOISE_EFFECT          = 0.1F;
constexpr auto DEFAULT_PROB_PLANE_EFFECT          = 0.7F;
constexpr auto DEFAULT_PROB_TAN_EFFECT            = 0.2F;
constexpr auto DEFAULT_PROB_XY_LERP_EFFECT        = 0.2F;

constexpr auto WAVE0_PROB_PLANE_EFFECT = 0.8F;
constexpr auto WAVE1_PROB_PLANE_EFFECT = 0.8F;

// clang-format off
constexpr auto EFFECTS_PROBABILITIES = EnumMap<ZoomFilterMode,
                                                      EnumMap<AfterEffectsTypes, float>>{{{
    { ZoomFilterMode::AMULET_MODE,
      EnumMap<AfterEffectsTypes, float>{{{
          {AfterEffectsTypes::BLOCK_WAVY, DEFAULT_PROB_BLOCKY_WAVY_EFFECT},
          {AfterEffectsTypes::HYPERCOS, DEFAULT_PROB_HYPERCOS_EFFECT},
          {AfterEffectsTypes::IMAGE_VELOCITY, DEFAULT_PROB_IMAGE_VELOCITY_EFFECT},
          {AfterEffectsTypes::NOISE, DEFAULT_PROB_NOISE_EFFECT},
          {AfterEffectsTypes::PLANES, DEFAULT_PROB_PLANE_EFFECT},
          {AfterEffectsTypes::ROTATION, AMULET_PROB_ROTATE},
          {AfterEffectsTypes::TAN_EFFECT, DEFAULT_PROB_TAN_EFFECT},
          {AfterEffectsTypes::XY_LERP_EFFECT, DEFAULT_PROB_XY_LERP_EFFECT},
      }}}
    },
    { ZoomFilterMode::CRYSTAL_BALL_MODE0,
      EnumMap<AfterEffectsTypes, float>{{{
          {AfterEffectsTypes::BLOCK_WAVY, DEFAULT_PROB_BLOCKY_WAVY_EFFECT},
          {AfterEffectsTypes::HYPERCOS, DEFAULT_PROB_HYPERCOS_EFFECT},
          {AfterEffectsTypes::IMAGE_VELOCITY, DEFAULT_PROB_IMAGE_VELOCITY_EFFECT},
          {AfterEffectsTypes::NOISE, DEFAULT_PROB_NOISE_EFFECT},
          {AfterEffectsTypes::PLANES, DEFAULT_PROB_PLANE_EFFECT},
          {AfterEffectsTypes::ROTATION, CRYSTAL_BALL0_PROB_ROTATE},
          {AfterEffectsTypes::TAN_EFFECT, DEFAULT_PROB_TAN_EFFECT},
          {AfterEffectsTypes::XY_LERP_EFFECT, DEFAULT_PROB_XY_LERP_EFFECT},
      }}}
    },
    { ZoomFilterMode::CRYSTAL_BALL_MODE1,
      EnumMap<AfterEffectsTypes, float>{{{
          {AfterEffectsTypes::BLOCK_WAVY, DEFAULT_PROB_BLOCKY_WAVY_EFFECT},
          {AfterEffectsTypes::HYPERCOS, DEFAULT_PROB_HYPERCOS_EFFECT},
          {AfterEffectsTypes::IMAGE_VELOCITY, DEFAULT_PROB_IMAGE_VELOCITY_EFFECT},
          {AfterEffectsTypes::NOISE, DEFAULT_PROB_NOISE_EFFECT},
          {AfterEffectsTypes::PLANES, DEFAULT_PROB_PLANE_EFFECT},
          {AfterEffectsTypes::ROTATION, CRYSTAL_BALL1_PROB_ROTATE},
          {AfterEffectsTypes::TAN_EFFECT, DEFAULT_PROB_TAN_EFFECT},
          {AfterEffectsTypes::XY_LERP_EFFECT, DEFAULT_PROB_XY_LERP_EFFECT},
      }}}
    },
    { ZoomFilterMode::DISTANCE_FIELD_MODE0,
      EnumMap<AfterEffectsTypes, float>{{{
          {AfterEffectsTypes::BLOCK_WAVY, DEFAULT_PROB_BLOCKY_WAVY_EFFECT},
          {AfterEffectsTypes::HYPERCOS, DEFAULT_PROB_HYPERCOS_EFFECT},
          {AfterEffectsTypes::IMAGE_VELOCITY, DEFAULT_PROB_IMAGE_VELOCITY_EFFECT},
          {AfterEffectsTypes::NOISE, DEFAULT_PROB_NOISE_EFFECT},
          {AfterEffectsTypes::PLANES, DEFAULT_PROB_PLANE_EFFECT},
          {AfterEffectsTypes::ROTATION, DISTANCE_FIELD0_PROB_ROTATE},
          {AfterEffectsTypes::TAN_EFFECT, DEFAULT_PROB_TAN_EFFECT},
          {AfterEffectsTypes::XY_LERP_EFFECT, DEFAULT_PROB_XY_LERP_EFFECT},
      }}}
    },
    { ZoomFilterMode::DISTANCE_FIELD_MODE1,
      EnumMap<AfterEffectsTypes, float>{{{
          {AfterEffectsTypes::BLOCK_WAVY, DEFAULT_PROB_BLOCKY_WAVY_EFFECT},
          {AfterEffectsTypes::HYPERCOS, DEFAULT_PROB_HYPERCOS_EFFECT},
          {AfterEffectsTypes::IMAGE_VELOCITY, DEFAULT_PROB_IMAGE_VELOCITY_EFFECT},
          {AfterEffectsTypes::NOISE, DEFAULT_PROB_NOISE_EFFECT},
          {AfterEffectsTypes::PLANES, DEFAULT_PROB_PLANE_EFFECT},
          {AfterEffectsTypes::ROTATION, DISTANCE_FIELD1_PROB_ROTATE},
          {AfterEffectsTypes::TAN_EFFECT, DEFAULT_PROB_TAN_EFFECT},
          {AfterEffectsTypes::XY_LERP_EFFECT, DEFAULT_PROB_XY_LERP_EFFECT},
      }}}
    },
    { ZoomFilterMode::DISTANCE_FIELD_MODE2,
      EnumMap<AfterEffectsTypes, float>{{{
          {AfterEffectsTypes::BLOCK_WAVY, DEFAULT_PROB_BLOCKY_WAVY_EFFECT},
          {AfterEffectsTypes::HYPERCOS, DEFAULT_PROB_HYPERCOS_EFFECT},
          {AfterEffectsTypes::IMAGE_VELOCITY, DEFAULT_PROB_IMAGE_VELOCITY_EFFECT},
          {AfterEffectsTypes::NOISE, DEFAULT_PROB_NOISE_EFFECT},
          {AfterEffectsTypes::PLANES, DEFAULT_PROB_PLANE_EFFECT},
          {AfterEffectsTypes::ROTATION, DISTANCE_FIELD2_PROB_ROTATE},
          {AfterEffectsTypes::TAN_EFFECT, DEFAULT_PROB_TAN_EFFECT},
          {AfterEffectsTypes::XY_LERP_EFFECT, DEFAULT_PROB_XY_LERP_EFFECT},
      }}}
    },
    { ZoomFilterMode::EXP_RECIPROCAL_MODE,
      EnumMap<AfterEffectsTypes, float>{{{
          {AfterEffectsTypes::BLOCK_WAVY, DEFAULT_PROB_BLOCKY_WAVY_EFFECT},
          {AfterEffectsTypes::HYPERCOS, DEFAULT_PROB_HYPERCOS_EFFECT},
          {AfterEffectsTypes::IMAGE_VELOCITY, DEFAULT_PROB_IMAGE_VELOCITY_EFFECT},
          {AfterEffectsTypes::NOISE, DEFAULT_PROB_NOISE_EFFECT},
          {AfterEffectsTypes::PLANES, DEFAULT_PROB_PLANE_EFFECT},
          {AfterEffectsTypes::ROTATION, EXP_RECIPROCAL_PROB_ROTATE},
          {AfterEffectsTypes::TAN_EFFECT, DEFAULT_PROB_TAN_EFFECT},
          {AfterEffectsTypes::XY_LERP_EFFECT, DEFAULT_PROB_XY_LERP_EFFECT},
      }}}
    },
    { ZoomFilterMode::HYPERCOS_MODE0,
      EnumMap<AfterEffectsTypes, float>{{{
          {AfterEffectsTypes::BLOCK_WAVY, DEFAULT_PROB_BLOCKY_WAVY_EFFECT},
          {AfterEffectsTypes::HYPERCOS, DEFAULT_PROB_HYPERCOS_EFFECT},
          {AfterEffectsTypes::IMAGE_VELOCITY, DEFAULT_PROB_IMAGE_VELOCITY_EFFECT},
          {AfterEffectsTypes::NOISE, DEFAULT_PROB_NOISE_EFFECT},
          {AfterEffectsTypes::PLANES, DEFAULT_PROB_PLANE_EFFECT},
          {AfterEffectsTypes::ROTATION, HYPERCOS0_PROB_ROTATE},
          {AfterEffectsTypes::TAN_EFFECT, DEFAULT_PROB_TAN_EFFECT},
          {AfterEffectsTypes::XY_LERP_EFFECT, DEFAULT_PROB_XY_LERP_EFFECT},
      }}}
    },
    { ZoomFilterMode::HYPERCOS_MODE1,
      EnumMap<AfterEffectsTypes, float>{{{
          {AfterEffectsTypes::BLOCK_WAVY, DEFAULT_PROB_BLOCKY_WAVY_EFFECT},
          {AfterEffectsTypes::HYPERCOS, DEFAULT_PROB_HYPERCOS_EFFECT},
          {AfterEffectsTypes::IMAGE_VELOCITY, DEFAULT_PROB_IMAGE_VELOCITY_EFFECT},
          {AfterEffectsTypes::NOISE, DEFAULT_PROB_NOISE_EFFECT},
          {AfterEffectsTypes::PLANES, DEFAULT_PROB_PLANE_EFFECT},
          {AfterEffectsTypes::ROTATION, HYPERCOS1_PROB_ROTATE},
          {AfterEffectsTypes::TAN_EFFECT, DEFAULT_PROB_TAN_EFFECT},
          {AfterEffectsTypes::XY_LERP_EFFECT, DEFAULT_PROB_XY_LERP_EFFECT},
      }}}
    },
    { ZoomFilterMode::HYPERCOS_MODE2,
      EnumMap<AfterEffectsTypes, float>{{{
          {AfterEffectsTypes::BLOCK_WAVY, DEFAULT_PROB_BLOCKY_WAVY_EFFECT},
          {AfterEffectsTypes::HYPERCOS, DEFAULT_PROB_HYPERCOS_EFFECT},
          {AfterEffectsTypes::IMAGE_VELOCITY, DEFAULT_PROB_IMAGE_VELOCITY_EFFECT},
          {AfterEffectsTypes::NOISE, DEFAULT_PROB_NOISE_EFFECT},
          {AfterEffectsTypes::PLANES, DEFAULT_PROB_PLANE_EFFECT},
          {AfterEffectsTypes::ROTATION, HYPERCOS2_PROB_ROTATE},
          {AfterEffectsTypes::TAN_EFFECT, DEFAULT_PROB_TAN_EFFECT},
          {AfterEffectsTypes::XY_LERP_EFFECT, DEFAULT_PROB_XY_LERP_EFFECT},
      }}}
    },
    { ZoomFilterMode::HYPERCOS_MODE3,
      EnumMap<AfterEffectsTypes, float>{{{
          {AfterEffectsTypes::BLOCK_WAVY, DEFAULT_PROB_BLOCKY_WAVY_EFFECT},
          {AfterEffectsTypes::HYPERCOS, DEFAULT_PROB_HYPERCOS_EFFECT},
          {AfterEffectsTypes::IMAGE_VELOCITY, DEFAULT_PROB_IMAGE_VELOCITY_EFFECT},
          {AfterEffectsTypes::NOISE, DEFAULT_PROB_NOISE_EFFECT},
          {AfterEffectsTypes::PLANES, DEFAULT_PROB_PLANE_EFFECT},
          {AfterEffectsTypes::ROTATION, HYPERCOS3_PROB_ROTATE},
          {AfterEffectsTypes::TAN_EFFECT, DEFAULT_PROB_TAN_EFFECT},
          {AfterEffectsTypes::XY_LERP_EFFECT, DEFAULT_PROB_XY_LERP_EFFECT},
      }}}
    },
    { ZoomFilterMode::IMAGE_DISPLACEMENT_MODE,
      EnumMap<AfterEffectsTypes, float>{{{
          {AfterEffectsTypes::BLOCK_WAVY, DEFAULT_PROB_BLOCKY_WAVY_EFFECT},
          {AfterEffectsTypes::HYPERCOS, DEFAULT_PROB_HYPERCOS_EFFECT},
          {AfterEffectsTypes::IMAGE_VELOCITY, DEFAULT_PROB_IMAGE_VELOCITY_EFFECT},
          {AfterEffectsTypes::NOISE, DEFAULT_PROB_NOISE_EFFECT},
          {AfterEffectsTypes::PLANES, DEFAULT_PROB_PLANE_EFFECT},
          {AfterEffectsTypes::ROTATION, IMAGE_DISPLACEMENT_PROB_ROTATE},
          {AfterEffectsTypes::TAN_EFFECT, DEFAULT_PROB_TAN_EFFECT},
          {AfterEffectsTypes::XY_LERP_EFFECT, DEFAULT_PROB_XY_LERP_EFFECT},
      }}}
    },
    { ZoomFilterMode::NORMAL_MODE,
      EnumMap<AfterEffectsTypes, float>{{{
          {AfterEffectsTypes::BLOCK_WAVY, DEFAULT_PROB_BLOCKY_WAVY_EFFECT},
          {AfterEffectsTypes::HYPERCOS, DEFAULT_PROB_HYPERCOS_EFFECT},
          {AfterEffectsTypes::IMAGE_VELOCITY, DEFAULT_PROB_IMAGE_VELOCITY_EFFECT},
          {AfterEffectsTypes::NOISE, DEFAULT_PROB_NOISE_EFFECT},
          {AfterEffectsTypes::PLANES, DEFAULT_PROB_PLANE_EFFECT},
          {AfterEffectsTypes::ROTATION, NORMAL_PROB_ROTATE},
          {AfterEffectsTypes::TAN_EFFECT, DEFAULT_PROB_TAN_EFFECT},
          {AfterEffectsTypes::XY_LERP_EFFECT, DEFAULT_PROB_XY_LERP_EFFECT},
      }}}
    },
    { ZoomFilterMode::SCRUNCH_MODE,
      EnumMap<AfterEffectsTypes, float>{{{
          {AfterEffectsTypes::BLOCK_WAVY, DEFAULT_PROB_BLOCKY_WAVY_EFFECT},
          {AfterEffectsTypes::HYPERCOS, DEFAULT_PROB_HYPERCOS_EFFECT},
          {AfterEffectsTypes::IMAGE_VELOCITY, DEFAULT_PROB_IMAGE_VELOCITY_EFFECT},
          {AfterEffectsTypes::NOISE, DEFAULT_PROB_NOISE_EFFECT},
          {AfterEffectsTypes::PLANES, DEFAULT_PROB_PLANE_EFFECT},
          {AfterEffectsTypes::ROTATION, SCRUNCH_PROB_ROTATE},
          {AfterEffectsTypes::TAN_EFFECT, DEFAULT_PROB_TAN_EFFECT},
          {AfterEffectsTypes::XY_LERP_EFFECT, DEFAULT_PROB_XY_LERP_EFFECT},
      }}}
    },
    { ZoomFilterMode::SPEEDWAY_MODE0,
      EnumMap<AfterEffectsTypes, float>{{{
          {AfterEffectsTypes::BLOCK_WAVY, DEFAULT_PROB_BLOCKY_WAVY_EFFECT},
          {AfterEffectsTypes::HYPERCOS, DEFAULT_PROB_HYPERCOS_EFFECT},
          {AfterEffectsTypes::IMAGE_VELOCITY, DEFAULT_PROB_IMAGE_VELOCITY_EFFECT},
          {AfterEffectsTypes::NOISE, DEFAULT_PROB_NOISE_EFFECT},
          {AfterEffectsTypes::PLANES, DEFAULT_PROB_PLANE_EFFECT},
          {AfterEffectsTypes::ROTATION, SPEEDWAY0_PROB_ROTATE},
          {AfterEffectsTypes::TAN_EFFECT, DEFAULT_PROB_TAN_EFFECT},
          {AfterEffectsTypes::XY_LERP_EFFECT, DEFAULT_PROB_XY_LERP_EFFECT},
      }}}
    },
    { ZoomFilterMode::SPEEDWAY_MODE1,
      EnumMap<AfterEffectsTypes, float>{{{
          {AfterEffectsTypes::BLOCK_WAVY, DEFAULT_PROB_BLOCKY_WAVY_EFFECT},
          {AfterEffectsTypes::HYPERCOS, DEFAULT_PROB_HYPERCOS_EFFECT},
          {AfterEffectsTypes::IMAGE_VELOCITY, DEFAULT_PROB_IMAGE_VELOCITY_EFFECT},
          {AfterEffectsTypes::NOISE, DEFAULT_PROB_NOISE_EFFECT},
          {AfterEffectsTypes::PLANES, DEFAULT_PROB_PLANE_EFFECT},
          {AfterEffectsTypes::ROTATION, SPEEDWAY1_PROB_ROTATE},
          {AfterEffectsTypes::TAN_EFFECT, DEFAULT_PROB_TAN_EFFECT},
          {AfterEffectsTypes::XY_LERP_EFFECT, DEFAULT_PROB_XY_LERP_EFFECT},
      }}}
    },
    { ZoomFilterMode::SPEEDWAY_MODE2,
      EnumMap<AfterEffectsTypes, float>{{{
          {AfterEffectsTypes::BLOCK_WAVY, DEFAULT_PROB_BLOCKY_WAVY_EFFECT},
          {AfterEffectsTypes::HYPERCOS, DEFAULT_PROB_HYPERCOS_EFFECT},
          {AfterEffectsTypes::IMAGE_VELOCITY, DEFAULT_PROB_IMAGE_VELOCITY_EFFECT},
          {AfterEffectsTypes::NOISE, DEFAULT_PROB_NOISE_EFFECT},
          {AfterEffectsTypes::PLANES, DEFAULT_PROB_PLANE_EFFECT},
          {AfterEffectsTypes::ROTATION, SPEEDWAY2_PROB_ROTATE},
          {AfterEffectsTypes::TAN_EFFECT, DEFAULT_PROB_TAN_EFFECT},
          {AfterEffectsTypes::XY_LERP_EFFECT, DEFAULT_PROB_XY_LERP_EFFECT},
      }}}
    },
    { ZoomFilterMode::WATER_MODE,
      EnumMap<AfterEffectsTypes, float>{{{
          {AfterEffectsTypes::BLOCK_WAVY, DEFAULT_PROB_BLOCKY_WAVY_EFFECT},
          {AfterEffectsTypes::HYPERCOS, DEFAULT_PROB_HYPERCOS_EFFECT},
          {AfterEffectsTypes::IMAGE_VELOCITY, DEFAULT_PROB_IMAGE_VELOCITY_EFFECT},
          {AfterEffectsTypes::NOISE, DEFAULT_PROB_NOISE_EFFECT},
          {AfterEffectsTypes::PLANES, DEFAULT_PROB_PLANE_EFFECT},
          {AfterEffectsTypes::ROTATION, WATER_PROB_ROTATE},
          {AfterEffectsTypes::TAN_EFFECT, DEFAULT_PROB_TAN_EFFECT},
          {AfterEffectsTypes::XY_LERP_EFFECT, DEFAULT_PROB_XY_LERP_EFFECT},
      }}}
    },
    { ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE0,
      EnumMap<AfterEffectsTypes, float>{{{
          {AfterEffectsTypes::BLOCK_WAVY, DEFAULT_PROB_BLOCKY_WAVY_EFFECT},
          {AfterEffectsTypes::HYPERCOS, DEFAULT_PROB_HYPERCOS_EFFECT},
          {AfterEffectsTypes::IMAGE_VELOCITY, DEFAULT_PROB_IMAGE_VELOCITY_EFFECT},
          {AfterEffectsTypes::NOISE, DEFAULT_PROB_NOISE_EFFECT},
          {AfterEffectsTypes::PLANES, WAVE0_PROB_PLANE_EFFECT},
          {AfterEffectsTypes::ROTATION, WAVE0_PROB_ROTATE},
          {AfterEffectsTypes::TAN_EFFECT, DEFAULT_PROB_TAN_EFFECT},
          {AfterEffectsTypes::XY_LERP_EFFECT, DEFAULT_PROB_XY_LERP_EFFECT},
      }}}
    },
    { ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE1,
      EnumMap<AfterEffectsTypes, float>{{{
          {AfterEffectsTypes::BLOCK_WAVY, DEFAULT_PROB_BLOCKY_WAVY_EFFECT},
          {AfterEffectsTypes::HYPERCOS, DEFAULT_PROB_HYPERCOS_EFFECT},
          {AfterEffectsTypes::IMAGE_VELOCITY, DEFAULT_PROB_IMAGE_VELOCITY_EFFECT},
          {AfterEffectsTypes::NOISE, DEFAULT_PROB_NOISE_EFFECT},
          {AfterEffectsTypes::PLANES, WAVE1_PROB_PLANE_EFFECT},
          {AfterEffectsTypes::ROTATION, WAVE1_PROB_ROTATE},
          {AfterEffectsTypes::TAN_EFFECT, DEFAULT_PROB_TAN_EFFECT},
          {AfterEffectsTypes::XY_LERP_EFFECT, DEFAULT_PROB_XY_LERP_EFFECT},
      }}}
    },
    { ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE0,
      EnumMap<AfterEffectsTypes, float>{{{
          {AfterEffectsTypes::BLOCK_WAVY, DEFAULT_PROB_BLOCKY_WAVY_EFFECT},
          {AfterEffectsTypes::HYPERCOS, DEFAULT_PROB_HYPERCOS_EFFECT},
          {AfterEffectsTypes::IMAGE_VELOCITY, DEFAULT_PROB_IMAGE_VELOCITY_EFFECT},
          {AfterEffectsTypes::NOISE, DEFAULT_PROB_NOISE_EFFECT},
          {AfterEffectsTypes::PLANES, WAVE0_PROB_PLANE_EFFECT},
          {AfterEffectsTypes::ROTATION, WAVE0_PROB_ROTATE},
          {AfterEffectsTypes::TAN_EFFECT, DEFAULT_PROB_TAN_EFFECT},
          {AfterEffectsTypes::XY_LERP_EFFECT, DEFAULT_PROB_XY_LERP_EFFECT},
      }}}
    },
    { ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE1,
      EnumMap<AfterEffectsTypes, float>{{{
          {AfterEffectsTypes::BLOCK_WAVY, DEFAULT_PROB_BLOCKY_WAVY_EFFECT},
          {AfterEffectsTypes::HYPERCOS, DEFAULT_PROB_HYPERCOS_EFFECT},
          {AfterEffectsTypes::IMAGE_VELOCITY, DEFAULT_PROB_IMAGE_VELOCITY_EFFECT},
          {AfterEffectsTypes::NOISE, DEFAULT_PROB_NOISE_EFFECT},
          {AfterEffectsTypes::PLANES, WAVE1_PROB_PLANE_EFFECT},
          {AfterEffectsTypes::ROTATION, WAVE1_PROB_ROTATE},
          {AfterEffectsTypes::TAN_EFFECT, DEFAULT_PROB_TAN_EFFECT},
          {AfterEffectsTypes::XY_LERP_EFFECT, DEFAULT_PROB_XY_LERP_EFFECT},
      }}}
    },
    { ZoomFilterMode::Y_ONLY_MODE,
      EnumMap<AfterEffectsTypes, float>{{{
          {AfterEffectsTypes::BLOCK_WAVY, DEFAULT_PROB_BLOCKY_WAVY_EFFECT},
          {AfterEffectsTypes::HYPERCOS, DEFAULT_PROB_HYPERCOS_EFFECT},
          {AfterEffectsTypes::IMAGE_VELOCITY, DEFAULT_PROB_IMAGE_VELOCITY_EFFECT},
          {AfterEffectsTypes::NOISE, DEFAULT_PROB_NOISE_EFFECT},
          {AfterEffectsTypes::PLANES, DEFAULT_PROB_PLANE_EFFECT},
          {AfterEffectsTypes::ROTATION, Y_ONLY_PROB_ROTATE},
          {AfterEffectsTypes::TAN_EFFECT, DEFAULT_PROB_TAN_EFFECT},
          {AfterEffectsTypes::XY_LERP_EFFECT, DEFAULT_PROB_XY_LERP_EFFECT},
      }}}
    },
}}};
// clang-format on

constexpr auto DEFAULT_AFTER_EFFECTS_STATES = EnumMap<AfterEffectsTypes, bool>{{{
    {AfterEffectsTypes::BLOCK_WAVY, false},
    {AfterEffectsTypes::HYPERCOS, false},
    {AfterEffectsTypes::IMAGE_VELOCITY, false},
    {AfterEffectsTypes::NOISE, false},
    {AfterEffectsTypes::PLANES, false},
    {AfterEffectsTypes::ROTATION, false},
    {AfterEffectsTypes::TAN_EFFECT, false},
    {AfterEffectsTypes::XY_LERP_EFFECT, false},
}}};

constexpr auto DEFAULT_AFTER_EFFECTS_PROBABILITIES = EnumMap<AfterEffectsTypes, float>{{{
    {AfterEffectsTypes::BLOCK_WAVY, 0.0F},
    {AfterEffectsTypes::HYPERCOS, 0.0F},
    {AfterEffectsTypes::IMAGE_VELOCITY, 0.0F},
    {AfterEffectsTypes::NOISE, 0.0F},
    {AfterEffectsTypes::PLANES, 0.0F},
    {AfterEffectsTypes::ROTATION, 0.0F},
    {AfterEffectsTypes::TAN_EFFECT, 0.0F},
    {AfterEffectsTypes::XY_LERP_EFFECT, 0.0F},
}}};

constexpr auto DEFAULT_REPEAT_AFTER_EFFECTS_PROBABILITIES = EnumMap<AfterEffectsTypes, float>{{{
    {AfterEffectsTypes::BLOCK_WAVY, 0.9F},
    {AfterEffectsTypes::HYPERCOS, 0.5F},
    {AfterEffectsTypes::IMAGE_VELOCITY, 0.8F},
    {AfterEffectsTypes::NOISE, 0.0F},
    {AfterEffectsTypes::PLANES, 0.0F},
    {AfterEffectsTypes::ROTATION, 0.0F},
    {AfterEffectsTypes::TAN_EFFECT, 0.0F},
    {AfterEffectsTypes::XY_LERP_EFFECT, 0.1F},
}}};

constexpr auto ZERO_REPEAT_AFTER_EFFECTS_PROBABILITIES = EnumMap<AfterEffectsTypes, float>{{{
    {AfterEffectsTypes::BLOCK_WAVY, 0.0F},
    {AfterEffectsTypes::HYPERCOS, 0.0F},
    {AfterEffectsTypes::IMAGE_VELOCITY, 0.0F},
    {AfterEffectsTypes::NOISE, 0.0F},
    {AfterEffectsTypes::PLANES, 0.0F},
    {AfterEffectsTypes::ROTATION, 0.0F},
    {AfterEffectsTypes::TAN_EFFECT, 0.0F},
    {AfterEffectsTypes::XY_LERP_EFFECT, 0.0F},
}}};

constexpr auto DEFAULT_AFTER_EFFECTS_OFF_TIMES = EnumMap<AfterEffectsTypes, uint32_t>{{{
    {AfterEffectsTypes::BLOCK_WAVY, 100U},
    {AfterEffectsTypes::HYPERCOS, 100U},
    {AfterEffectsTypes::IMAGE_VELOCITY, 100U},
    {AfterEffectsTypes::NOISE, 100U},
    {AfterEffectsTypes::PLANES, 100U},
    {AfterEffectsTypes::ROTATION, 0U},
    {AfterEffectsTypes::TAN_EFFECT, 100U},
    {AfterEffectsTypes::XY_LERP_EFFECT, 100U},
}}};

[[nodiscard]] constexpr auto GetAfterEffectsProbability(const ZoomFilterMode filterMode)
    -> EnumMap<AfterEffectsTypes, float>
{
  if constexpr (USE_FORCED_AFTER_EFFECT)
  {
    auto forcedProbabilities                       = DEFAULT_AFTER_EFFECTS_PROBABILITIES;
    forcedProbabilities[FORCED_AFTER_EFFECTS_TYPE] = 1.0F;
    return forcedProbabilities;
  }

  return EFFECTS_PROBABILITIES[filterMode];
}

[[nodiscard]] constexpr auto GetRepeatAfterEffectsProbability() -> EnumMap<AfterEffectsTypes, float>
{
  if constexpr (USE_FORCED_AFTER_EFFECT)
  {
    auto forcedRepeatProbabilities                       = ZERO_REPEAT_AFTER_EFFECTS_PROBABILITIES;
    forcedRepeatProbabilities[FORCED_AFTER_EFFECTS_TYPE] = 1.0F;
    return forcedRepeatProbabilities;
  }

  return DEFAULT_REPEAT_AFTER_EFFECTS_PROBABILITIES;
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

[[nodiscard]] auto GetHypercosWeights(const ZoomFilterMode filterMode) noexcept
    -> std::vector<Weights<HypercosOverlay>::KeyValue>
{
  constexpr auto FORCED_HYPERCOS =
      USE_FORCED_AFTER_EFFECT and (FORCED_AFTER_EFFECTS_TYPE == AfterEffectsTypes::HYPERCOS);

  using Hyp         = HypercosOverlay;
  using ModeWeights = std::array<Weights<HypercosOverlay>::KeyValue, NUM<HypercosOverlay>>;

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
  Expects(HYPERCOS_WEIGHTS.size() == NUM<ZoomFilterMode>);

  return std::vector<Weights<HypercosOverlay>::KeyValue>{cbegin(HYPERCOS_WEIGHTS[filterMode]),
                                                         cend(HYPERCOS_WEIGHTS[filterMode])};
}

} // namespace

auto FilterSettingsService::GetFilterModeData(
    const IGoomRand& goomRand,
    const std::string& resourcesDirectory,
    const CreateZoomInCoefficientsEffectFunc& createZoomInCoefficientsEffect) -> FilterModeEnumMap
{
  Expects(FILTER_MODE_NAMES.size() == NUM<ZoomFilterMode>);
  Expects(EFFECTS_PROBABILITIES.size() == NUM<ZoomFilterMode>);

  auto filterModeVec = std::vector<FilterModeEnumMap::KeyValue>{};

  for (auto i = 0U; i < NUM<ZoomFilterMode>; ++i)
  {
    const auto filterMode = static_cast<ZoomFilterMode>(i);

    filterModeVec.emplace_back(
        filterMode,
        ZoomFilterModeInfo{
            FILTER_MODE_NAMES[filterMode],
            createZoomInCoefficientsEffect(filterMode, goomRand, resourcesDirectory),
            {Weights<HypercosOverlay>{goomRand, GetHypercosWeights(filterMode)},
                                                        GetAfterEffectsProbability(filterMode)},
    });
  }

  return FilterModeEnumMap::Make(std::move(filterModeVec));
}

FilterSettingsService::FilterSettingsService(const PluginInfo& goomInfo,
                                             const IGoomRand& goomRand,
                                             const std::string& resourcesDirectory,
                                             const CreateZoomInCoefficientsEffectFunc&
                                                 createZoomInCoefficientsEffect)
  : m_goomInfo{&goomInfo},
    m_goomRand{&goomRand},
    m_screenCentre{m_goomInfo->GetDimensions().GetCentrePoint()},
    m_resourcesDirectory{resourcesDirectory},
    m_randomizedAfterEffects{
        std::make_unique<AfterEffectsStates>(*m_goomRand,
                                             GetRepeatAfterEffectsProbability(),
                                             GetAfterEffectsOffTime())},
    m_filterModeData{GetFilterModeData(*m_goomRand,
                                       m_resourcesDirectory,
                                       createZoomInCoefficientsEffect)},
    m_filterSettings{
        false,
        {
           Vitesse{},
           DEFAULT_MAX_ZOOM_IN_COEFF,
           DEFAULT_BASE_ZOOM_IN_COEFF_FACTOR_MULTIPLIER,
           nullptr,
           {DEFAULT_ZOOM_MID_X, DEFAULT_ZOOM_MID_Y},
           {
               HypercosOverlay::NONE,
               DEFAULT_AFTER_EFFECTS_STATES,
               RotationAdjustments{},
            }
        },
        {DEFAULT_TRAN_LERP_INCREMENT, DEFAULT_SWITCH_MULT, DEFAULT_FILTER_VIEWPORT},
    },
    m_weightedFilterEvents{
        *m_goomRand,
        {
            {ZoomFilterMode::AMULET_MODE,                     AMULET_MODE_WEIGHT},
            {ZoomFilterMode::CRYSTAL_BALL_MODE0,              CRYSTAL_BALL_MODE0_WEIGHT},
            {ZoomFilterMode::CRYSTAL_BALL_MODE1,              CRYSTAL_BALL_MODE1_WEIGHT},
            {ZoomFilterMode::DISTANCE_FIELD_MODE0,            DISTANCE_FIELD_MODE0_WEIGHT},
            {ZoomFilterMode::DISTANCE_FIELD_MODE1,            DISTANCE_FIELD_MODE1_WEIGHT},
            {ZoomFilterMode::DISTANCE_FIELD_MODE2,            DISTANCE_FIELD_MODE2_WEIGHT},
            {ZoomFilterMode::EXP_RECIPROCAL_MODE,             EXP_RECIPROCAL_MODE_WEIGHT},
            {ZoomFilterMode::HYPERCOS_MODE0,                  HYPERCOS_MODE0_WEIGHT},
            {ZoomFilterMode::HYPERCOS_MODE1,                  HYPERCOS_MODE1_WEIGHT},
            {ZoomFilterMode::HYPERCOS_MODE2,                  HYPERCOS_MODE2_WEIGHT},
            {ZoomFilterMode::HYPERCOS_MODE3,                  HYPERCOS_MODE3_WEIGHT},
            {ZoomFilterMode::IMAGE_DISPLACEMENT_MODE,         IMAGE_DISPLACEMENT_MODE_WEIGHT},
            {ZoomFilterMode::NORMAL_MODE,                     NORMAL_MODE_WEIGHT},
            {ZoomFilterMode::SCRUNCH_MODE,                    SCRUNCH_MODE_WEIGHT},
            {ZoomFilterMode::SPEEDWAY_MODE0,                  SPEEDWAY_MODE0_WEIGHT},
            {ZoomFilterMode::SPEEDWAY_MODE1,                  SPEEDWAY_MODE1_WEIGHT},
            {ZoomFilterMode::SPEEDWAY_MODE2,                  SPEEDWAY_MODE2_WEIGHT},
            {ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE0, WAVE_SQ_DIST_MODE0_WEIGHT},
            {ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE1, WAVE_SQ_DIST_MODE1_WEIGHT},
            {ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE0,    WAVE_ATAN_MODE0_WEIGHT},
            {ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE1,    WAVE_ATAN_MODE1_WEIGHT},
            {ZoomFilterMode::WATER_MODE,                      WATER_MODE_WEIGHT},
            {ZoomFilterMode::Y_ONLY_MODE,                     Y_ONLY_MODE_WEIGHT},
        },
        {
            {ZoomFilterMode::CRYSTAL_BALL_MODE0,              CRYSTAL_BALL_MODE0_MULTIPLIERS},
            {ZoomFilterMode::CRYSTAL_BALL_MODE1,              CRYSTAL_BALL_MODE1_MULTIPLIERS},
            {ZoomFilterMode::NORMAL_MODE,                     NORMAL_MODE_MULTIPLIERS},
            {ZoomFilterMode::HYPERCOS_MODE0,                  HYPERCOS_MODE0_MULTIPLIERS},
            {ZoomFilterMode::HYPERCOS_MODE1,                  HYPERCOS_MODE1_MULTIPLIERS},
            {ZoomFilterMode::HYPERCOS_MODE2,                  HYPERCOS_MODE2_MULTIPLIERS},
            {ZoomFilterMode::HYPERCOS_MODE3,                  HYPERCOS_MODE3_MULTIPLIERS},
            {ZoomFilterMode::SPEEDWAY_MODE0,                  SPEEDWAY_MODE0_MULTIPLIERS},
            {ZoomFilterMode::SPEEDWAY_MODE1,                  SPEEDWAY_MODE1_MULTIPLIERS},
            {ZoomFilterMode::SPEEDWAY_MODE2,                  SPEEDWAY_MODE2_MULTIPLIERS},
            {ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE0, WAVE_SQ_DIST_MODE0_MULTIPLIERS},
            {ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE1, WAVE_SQ_DIST_MODE1_MULTIPLIERS},
            {ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE0,    WAVE_ATAN_MODE0_MULTIPLIERS},
            {ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE1,    WAVE_ATAN_MODE1_MULTIPLIERS},
        },
    },
    m_zoomMidpointWeights{
      *m_goomRand,
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

inline auto FilterSettingsService::GetZoomInCoefficientsEffect()
    -> std::shared_ptr<IZoomInCoefficientsEffect>&
{
  return m_filterModeData[m_filterMode].zoomInCoefficientsEffect;
}

auto FilterSettingsService::NewCycle() -> void
{
  m_randomizedAfterEffects->UpdateTimers();
}

auto FilterSettingsService::NotifyUpdatedFilterEffectsSettings() -> void
{
  m_filterSettings.filterEffectsSettingsHaveChanged = false;

  m_filterModeAtLastUpdate = m_filterMode;
  m_randomizedAfterEffects->CheckForPendingOffTimers();
}

auto FilterSettingsService::SetDefaultSettings() -> void
{
  m_filterSettings.filterEffectsSettings.zoomInCoefficientsEffect = GetZoomInCoefficientsEffect();
  m_filterSettings.filterEffectsSettings.zoomMidpoint             = m_screenCentre;
  m_filterSettings.filterBufferSettings.filterEffectViewport      = Viewport{};
  m_filterSettings.filterEffectsSettings.vitesse.SetDefault();

  m_randomizedAfterEffects->SetDefaults();
}

auto FilterSettingsService::SetFilterModeRandomViewport() -> void
{
  m_filterSettings.filterBufferSettings.filterEffectViewport =
      m_filterModeData[m_filterMode].zoomInCoefficientsEffect->GetZoomInCoefficientsViewport();
}

auto FilterSettingsService::SetFilterModeRandomEffects() -> void
{
  m_filterSettings.filterEffectsSettings.zoomInCoefficientsEffect->SetRandomParams();
}

auto FilterSettingsService::SetFilterModeExtraEffects() -> void
{
  SetRandomizedExtraEffects();
  SetWaveModeExtraEffects();
}

auto FilterSettingsService::ResetRandomExtraEffects() -> void
{
  const auto& modeInfo = m_filterModeData[m_filterMode];
  m_randomizedAfterEffects->ResetStandardStates(modeInfo.afterEffectsProbabilities);
  m_filterSettings.filterEffectsSettingsHaveChanged = true;
}

auto FilterSettingsService::SetRandomizedExtraEffects() -> void
{
  const auto& modeInfo = m_filterModeData[m_filterMode];

  m_randomizedAfterEffects->ResetAllStates(modeInfo.afterEffectsProbabilities);

  m_filterSettings.filterEffectsSettings.afterEffectsSettings.rotationAdjustments.SetMultiplyFactor(
      modeInfo.afterEffectsProbabilities.probabilities[AfterEffectsTypes::ROTATION],
      RotationAdjustments::AdjustmentType::AFTER_RANDOM);
}

auto FilterSettingsService::SetWaveModeExtraEffects() -> void
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

auto FilterSettingsService::UpdateFilterSettingsFromExtraEffects() -> void
{
  m_filterSettings.filterEffectsSettingsHaveChanged = true;
  m_randomizedAfterEffects->UpdateFilterSettingsFromStates(
      m_filterSettings.filterEffectsSettings.afterEffectsSettings);
}

auto FilterSettingsService::SetBaseZoomInCoeffFactorMultiplier() noexcept -> void
{
  if (static constexpr auto PROB_CALM_DOWN = 0.8F; m_goomRand->ProbabilityOf(PROB_CALM_DOWN))
  {
    m_filterSettings.filterEffectsSettings.baseZoomInCoeffFactorMultiplier = 1.0F;
    return;
  }

  // TODO(glk) Lerp between old and new?
  static constexpr auto MULTIPLIER_RANGE = IGoomRand::NumberRange<float>{0.1F, 5.0F};

  static_assert(
      ZoomVectorEffects::MIN_ALLOWED_BASE_ZOOM_IN_COEFF <=
      ZoomVectorEffects::GetBaseZoomInCoeff(
          MULTIPLIER_RANGE.min * ZoomVectorEffects::RAW_BASE_ZOOM_IN_COEFF_FACTOR, -1.0F));
  static_assert(
      ZoomVectorEffects::MIN_ALLOWED_BASE_ZOOM_IN_COEFF <=
      ZoomVectorEffects::GetBaseZoomInCoeff(
          MULTIPLIER_RANGE.max * ZoomVectorEffects::RAW_BASE_ZOOM_IN_COEFF_FACTOR, -1.0F));
  static_assert(
      ZoomVectorEffects::MAX_ALLOWED_BASE_ZOOM_IN_COEFF >=
      ZoomVectorEffects::GetBaseZoomInCoeff(
          MULTIPLIER_RANGE.min * ZoomVectorEffects::RAW_BASE_ZOOM_IN_COEFF_FACTOR, +1.0F));
  static_assert(
      ZoomVectorEffects::MAX_ALLOWED_BASE_ZOOM_IN_COEFF >=
      ZoomVectorEffects::GetBaseZoomInCoeff(
          MULTIPLIER_RANGE.max * ZoomVectorEffects::RAW_BASE_ZOOM_IN_COEFF_FACTOR, +1.0F));

  m_filterSettings.filterEffectsSettings.baseZoomInCoeffFactorMultiplier =
      m_goomRand->GetRandInRange(MULTIPLIER_RANGE);
}

auto FilterSettingsService::SetRandomZoomMidpoint() -> void
{
  if (ALL_AFTER_EFFECTS_TURNED_OFF or IsZoomMidpointInTheMiddle())
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
  if ((m_filterMode == ZoomFilterMode::WATER_MODE) || (m_filterMode == ZoomFilterMode::AMULET_MODE))
  {
    return true;
  }

  if (((m_filterMode == ZoomFilterMode::CRYSTAL_BALL_MODE0) ||
       (m_filterMode == ZoomFilterMode::CRYSTAL_BALL_MODE1)) &&
      m_goomRand->ProbabilityOf(PROB_CRYSTAL_BALL_IN_MIDDLE))
  {
    return true;
  }

  if (((m_filterMode == ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE0) or
       (m_filterMode == ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE1) or
       (m_filterMode == ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE0) or
       (m_filterMode == ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE1)) and
      m_goomRand->ProbabilityOf(PROB_WAVE_IN_MIDDLE))
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
  return (midPointEvent == ZoomMidpointEvents::BOTTOM_MID_POINT) ||
         (midPointEvent == ZoomMidpointEvents::TOP_MID_POINT) ||
         (midPointEvent == ZoomMidpointEvents::RIGHT_MID_POINT) ||
         (midPointEvent == ZoomMidpointEvents::LEFT_MID_POINT);
}

auto FilterSettingsService::SetAnyRandomZoomMidpoint(const bool allowEdgePoints) -> void
{
  switch (GetWeightRandomMidPoint(allowEdgePoints))
  {
    case ZoomMidpointEvents::BOTTOM_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = {
          I_HALF * m_goomInfo->GetDimensions().GetIntWidth(),
          m_goomInfo->GetDimensions().GetIntHeight() - 2};
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
          m_goomInfo->GetDimensions().GetIntWidth() - 2,
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
