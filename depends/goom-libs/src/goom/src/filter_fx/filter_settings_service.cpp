#include "filter_settings_service.h"

#include "after_effects/after_effects_states.h"
#include "filter_consts.h"
#include "filter_settings.h"
#include "goom_config.h"
#include "goom_plugin_info.h"
#include "utils/enum_utils.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"

#include <stdexcept>

namespace GOOM::FILTER_FX
{

using AFTER_EFFECTS::AfterEffectsProbabilities;
using AFTER_EFFECTS::AfterEffectsStates;
using AFTER_EFFECTS::HypercosOverlay;
using AFTER_EFFECTS::RotationAdjustments;
using UTILS::EnumMap;
using UTILS::NUM;
using UTILS::MATH::I_HALF;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::U_HALF;
using UTILS::MATH::U_QUARTER;
using UTILS::MATH::U_THREE_QUARTERS;
using UTILS::MATH::Weights;


// For debugging:
static constexpr auto USE_FORCED_FILTER_MODE = ALL_FILTER_EFFECTS_TURNED_OFF;
//static constexpr auto USE_FORCED_FILTER_MODE = true;

//static constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::AMULET_MODE;
//static constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::CRYSTAL_BALL_MODE0;
//static constexpr auto FORCED_FILTER_MODE ZoomFilterMode::CRYSTAL_BALL_MODE1;
//static constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::DISTANCE_FIELD_MODE;
//static constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::HYPERCOS_MODE0;
//static constexpr auto FORCED_FILTER_MODE ZoomFilterMode::HYPERCOS_MODE1;
//static constexpr auto FORCED_FILTER_MODE ZoomFilterMode::HYPERCOS_MODE2;
//static constexpr auto FORCED_FILTER_MODE ZoomFilterMode::HYPERCOS_MODE3;
//static constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::IMAGE_DISPLACEMENT_MODE;
//static constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::NORMAL_MODE;
//static constexpr auto FORCED_FILTER_MODE ZoomFilterMode::SCRUNCH_MODE;
//static constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::SPEEDWAY_MODE0;
//static constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::SPEEDWAY_MODE1;
static constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::SPEEDWAY_MODE2;
//static constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::WAVE_MODE0;
//static constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::WAVE_MODE1;
//static constexpr auto FORCED_FILTER_MODE ZoomFilterMode::WATER_MODE;
//static constexpr auto FORCED_FILTER_MODE = ZoomFilterMode::Y_ONLY_MODE;
// End debugging


static constexpr auto PROB_HIGH = 0.9F;
static constexpr auto PROB_HALF = 0.5F;
static constexpr auto PROB_LOW  = 0.1F;
static constexpr auto PROB_ZERO = 0.0F;

static constexpr auto PROB_CRYSTAL_BALL_IN_MIDDLE = 0.8F;
static constexpr auto PROB_WAVE_IN_MIDDLE         = 0.5F;
static constexpr auto PROB_CHANGE_SPEED           = 0.5F;
static constexpr auto PROB_REVERSE_SPEED          = 0.5F;

static constexpr auto AMULET_MODE_WEIGHT             = 10.0F;
static constexpr auto CRYSTAL_BALL_MODE0_WEIGHT      = 04.0F;
static constexpr auto CRYSTAL_BALL_MODE1_WEIGHT      = 02.0F;
static constexpr auto DISTANCE_FIELD_MODE_WEIGHT     = 08.0F;
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
static constexpr auto WAVE_MODE0_WEIGHT              = 05.0F;
static constexpr auto WAVE_MODE1_WEIGHT              = 04.0F;
static constexpr auto WATER_MODE_WEIGHT              = 00.0F;
static constexpr auto Y_ONLY_MODE_WEIGHT             = 04.0F;

static constexpr auto BOTTOM_MID_POINT_WEIGHT               = 03.0F;
static constexpr auto RIGHT_MID_POINT_WEIGHT                = 02.0F;
static constexpr auto LEFT_MID_POINT_WEIGHT                 = 02.0F;
static constexpr auto CENTRE_MID_POINT_WEIGHT               = 18.0F;
static constexpr auto TOP_LEFT_QUARTER_MID_POINT_WEIGHT     = 10.0F;
static constexpr auto BOTTOM_RIGHT_QUARTER_MID_POINT_WEIGHT = 10.0F;

// TODO - When we get to use C++20, then replace the below 'inline consts' with 'static constexpr'.

inline const auto CRYSTAL_BALL_MODE0_MULTIPLIERS = std::map<ZoomFilterMode, float>{
    {ZoomFilterMode::CRYSTAL_BALL_MODE0, 0.0F},
    {ZoomFilterMode::CRYSTAL_BALL_MODE1, 0.0F},
};
inline const auto CRYSTAL_BALL_MODE1_MULTIPLIERS = std::map<ZoomFilterMode, float>{
    {ZoomFilterMode::CRYSTAL_BALL_MODE0, 0.0F},
    {ZoomFilterMode::CRYSTAL_BALL_MODE1, 0.0F},
};
inline const auto NORMAL_MODE_MULTIPLIERS = std::map<ZoomFilterMode, float>{
    {        ZoomFilterMode::NORMAL_MODE, 1.0F},
    {ZoomFilterMode::DISTANCE_FIELD_MODE, 2.0F},
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
inline const auto WAVE_MODE0_MULTIPLIERS = std::map<ZoomFilterMode, float>{
    {ZoomFilterMode::WAVE_MODE0, 0.0F},
    {ZoomFilterMode::WAVE_MODE1, 0.0F},
};
inline const auto WAVE_MODE1_MULTIPLIERS = std::map<ZoomFilterMode, float>{
    {ZoomFilterMode::WAVE_MODE0, 0.0F},
    {ZoomFilterMode::WAVE_MODE1, 0.0F},
};

static constexpr auto FILTER_MODE_NAMES = EnumMap<ZoomFilterMode, std::string_view>{{{
    {ZoomFilterMode::AMULET_MODE, "Amulet"},
    {ZoomFilterMode::CRYSTAL_BALL_MODE0, "Crystal Ball Mode 0"},
    {ZoomFilterMode::CRYSTAL_BALL_MODE1, "Crystal Ball Mode 1"},
    {ZoomFilterMode::DISTANCE_FIELD_MODE, "Distance Field"},
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
    {ZoomFilterMode::WAVE_MODE0, "Wave Mode 0"},
    {ZoomFilterMode::WAVE_MODE1, "Wave Mode 1"},
    {ZoomFilterMode::Y_ONLY_MODE, "Y Only"},
}}};

static constexpr auto AMULET_PROB_ROTATE             = PROB_HIGH;
static constexpr auto CRYSTAL_BALL0_PROB_ROTATE      = PROB_HIGH;
static constexpr auto CRYSTAL_BALL1_PROB_ROTATE      = PROB_HIGH;
static constexpr auto DISTANCE_FIELD_PROB_ROTATE     = PROB_HIGH;
static constexpr auto HYPERCOS0_PROB_ROTATE          = PROB_LOW;
static constexpr auto HYPERCOS1_PROB_ROTATE          = PROB_LOW;
static constexpr auto HYPERCOS2_PROB_ROTATE          = PROB_LOW;
static constexpr auto HYPERCOS3_PROB_ROTATE          = PROB_LOW;
static constexpr auto IMAGE_DISPLACEMENT_PROB_ROTATE = PROB_ZERO;
static constexpr auto NORMAL_PROB_ROTATE             = PROB_ZERO;
static constexpr auto SCRUNCH_PROB_ROTATE            = PROB_HALF;
static constexpr auto SPEEDWAY0_PROB_ROTATE          = PROB_HALF;
static constexpr auto SPEEDWAY1_PROB_ROTATE          = PROB_HIGH;
static constexpr auto SPEEDWAY2_PROB_ROTATE          = PROB_HIGH;
static constexpr auto WATER_PROB_ROTATE              = PROB_ZERO;
static constexpr auto WAVE0_PROB_ROTATE              = PROB_HIGH;
static constexpr auto WAVE1_PROB_ROTATE              = PROB_HIGH;
static constexpr auto Y_ONLY_PROB_ROTATE             = PROB_HALF;

static constexpr auto DEFAULT_PROB_BLOCKY_WAVY_EFFECT    = 0.3F;
static constexpr auto DEFAULT_PROB_IMAGE_VELOCITY_EFFECT = 0.1F;
static constexpr auto DEFAULT_PROB_NOISE_EFFECT          = 0.1F;
static constexpr auto DEFAULT_PROB_PLANE_EFFECT          = 0.8F;
static constexpr auto DEFAULT_PROB_TAN_EFFECT            = 0.2F;

static constexpr auto WAVE0_PROB_PLANE_EFFECT = 1.0F;
static constexpr auto WAVE1_PROB_PLANE_EFFECT = 1.0F;

// clang-format off
static constexpr auto EFFECTS_PROBABILITIES = EnumMap<ZoomFilterMode,
                                                      AfterEffectsProbabilities::Probabilities>{{{
    { ZoomFilterMode::AMULET_MODE,
        {
            DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
            DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
            DEFAULT_PROB_NOISE_EFFECT,
            DEFAULT_PROB_PLANE_EFFECT,
            AMULET_PROB_ROTATE,
            DEFAULT_PROB_TAN_EFFECT
        }
    },
    { ZoomFilterMode::CRYSTAL_BALL_MODE0,
        {
            DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
            DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
            DEFAULT_PROB_NOISE_EFFECT,
            DEFAULT_PROB_PLANE_EFFECT,
            CRYSTAL_BALL0_PROB_ROTATE,
            DEFAULT_PROB_TAN_EFFECT
        }
    },
    { ZoomFilterMode::CRYSTAL_BALL_MODE1,
        {
            DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
            DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
            DEFAULT_PROB_NOISE_EFFECT,
            DEFAULT_PROB_PLANE_EFFECT,
            CRYSTAL_BALL1_PROB_ROTATE,
            DEFAULT_PROB_TAN_EFFECT
        }
    },
    { ZoomFilterMode::DISTANCE_FIELD_MODE,
        {
            DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
            DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
            DEFAULT_PROB_NOISE_EFFECT,
            DEFAULT_PROB_PLANE_EFFECT,
            DISTANCE_FIELD_PROB_ROTATE,
            DEFAULT_PROB_TAN_EFFECT
        }
    },
    { ZoomFilterMode::HYPERCOS_MODE0,
        {
            DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
            DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
            DEFAULT_PROB_NOISE_EFFECT,
            DEFAULT_PROB_PLANE_EFFECT,
            HYPERCOS0_PROB_ROTATE,
            DEFAULT_PROB_TAN_EFFECT
        }
    },
    { ZoomFilterMode::HYPERCOS_MODE1,
        {
            DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
            DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
            DEFAULT_PROB_NOISE_EFFECT,
            DEFAULT_PROB_PLANE_EFFECT,
            HYPERCOS1_PROB_ROTATE,
            DEFAULT_PROB_TAN_EFFECT
        }
    },
    { ZoomFilterMode::HYPERCOS_MODE2,
        {
            DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
            DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
            DEFAULT_PROB_NOISE_EFFECT,
            DEFAULT_PROB_PLANE_EFFECT,
            HYPERCOS2_PROB_ROTATE,
            DEFAULT_PROB_TAN_EFFECT
        }
    },
    { ZoomFilterMode::HYPERCOS_MODE3,
        {
            DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
            DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
            DEFAULT_PROB_NOISE_EFFECT,
            DEFAULT_PROB_PLANE_EFFECT,
            HYPERCOS3_PROB_ROTATE,
            DEFAULT_PROB_TAN_EFFECT
        }
    },
    { ZoomFilterMode::IMAGE_DISPLACEMENT_MODE,
        {
            DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
            DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
            DEFAULT_PROB_NOISE_EFFECT,
            DEFAULT_PROB_PLANE_EFFECT,
            IMAGE_DISPLACEMENT_PROB_ROTATE,
            DEFAULT_PROB_TAN_EFFECT
        }
    },
    { ZoomFilterMode::NORMAL_MODE,
        {
            DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
            DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
            DEFAULT_PROB_NOISE_EFFECT,
            DEFAULT_PROB_PLANE_EFFECT,
            NORMAL_PROB_ROTATE,
            DEFAULT_PROB_TAN_EFFECT
        }
    },
    { ZoomFilterMode::SCRUNCH_MODE,
        {
            DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
            DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
            DEFAULT_PROB_NOISE_EFFECT,
            DEFAULT_PROB_PLANE_EFFECT,
            SCRUNCH_PROB_ROTATE,
            DEFAULT_PROB_TAN_EFFECT
        }
    },
    { ZoomFilterMode::SPEEDWAY_MODE0,
        {
            DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
            DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
            DEFAULT_PROB_NOISE_EFFECT,
            DEFAULT_PROB_PLANE_EFFECT,
            SPEEDWAY0_PROB_ROTATE,
            DEFAULT_PROB_TAN_EFFECT
        }
    },
    { ZoomFilterMode::SPEEDWAY_MODE1,
        {
            DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
            DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
            DEFAULT_PROB_NOISE_EFFECT,
            DEFAULT_PROB_PLANE_EFFECT,
            SPEEDWAY1_PROB_ROTATE,
            DEFAULT_PROB_TAN_EFFECT
        }
    },
    { ZoomFilterMode::SPEEDWAY_MODE2,
        {
          DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
          DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
          DEFAULT_PROB_NOISE_EFFECT,
          DEFAULT_PROB_PLANE_EFFECT,
          SPEEDWAY2_PROB_ROTATE,
          DEFAULT_PROB_TAN_EFFECT
        }
    },
    { ZoomFilterMode::WATER_MODE,
        {
            DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
            DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
            DEFAULT_PROB_NOISE_EFFECT,
            DEFAULT_PROB_PLANE_EFFECT,
            WATER_PROB_ROTATE,
            DEFAULT_PROB_TAN_EFFECT
        }
    },
    { ZoomFilterMode::WAVE_MODE0,
        {
            DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
            DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
            DEFAULT_PROB_NOISE_EFFECT,
            WAVE0_PROB_PLANE_EFFECT,
            WAVE0_PROB_ROTATE,
            DEFAULT_PROB_TAN_EFFECT
        }
    },
    { ZoomFilterMode::WAVE_MODE1,
        {
            DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
            DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
            DEFAULT_PROB_NOISE_EFFECT,
            WAVE1_PROB_PLANE_EFFECT,
            WAVE1_PROB_ROTATE,
            DEFAULT_PROB_TAN_EFFECT
        }
    },
    { ZoomFilterMode::Y_ONLY_MODE,
        {
            DEFAULT_PROB_BLOCKY_WAVY_EFFECT,
            DEFAULT_PROB_IMAGE_VELOCITY_EFFECT,
            DEFAULT_PROB_NOISE_EFFECT,
            DEFAULT_PROB_PLANE_EFFECT,
            Y_ONLY_PROB_ROTATE,
            DEFAULT_PROB_TAN_EFFECT
        }
    },
}}};
// clang-format on

using Hyp         = HypercosOverlay;
using ModeWeights = std::vector<std::pair<HypercosOverlay, float>>;

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"
#endif
// clang-format off
inline const auto HYPERCOS_WEIGHTS = EnumMap<ZoomFilterMode, ModeWeights>{{{
  { ZoomFilterMode::AMULET_MODE,
    {{ {Hyp::NONE, 20.0F}, {Hyp::MODE0,  1.0F}, {Hyp::MODE1,  5.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::CRYSTAL_BALL_MODE0,
    {{ {Hyp::NONE,  5.0F}, {Hyp::MODE0, 10.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::CRYSTAL_BALL_MODE1,
    {{ {Hyp::NONE,  5.0F}, {Hyp::MODE0,  1.0F}, {Hyp::MODE1, 99.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::DISTANCE_FIELD_MODE,
    {{ {Hyp::NONE,  5.0F}, {Hyp::MODE0, 10.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::HYPERCOS_MODE0,
    {{ {Hyp::NONE,  0.0F}, {Hyp::MODE0,  1.0F}, {Hyp::MODE1,  0.0F}, {Hyp::MODE2,  0.0F}, {Hyp::MODE3,  0.0F} }}
  },
  { ZoomFilterMode::HYPERCOS_MODE1,
    {{ {Hyp::NONE,  0.0F}, {Hyp::MODE0,  0.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  0.0F}, {Hyp::MODE3,  0.0F} }}
  },
  { ZoomFilterMode::HYPERCOS_MODE2,
    {{ {Hyp::NONE,  0.0F}, {Hyp::MODE0,  0.0F}, {Hyp::MODE1,  0.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  0.0F} }}
  },
  { ZoomFilterMode::HYPERCOS_MODE3,
    {{ {Hyp::NONE,  0.0F}, {Hyp::MODE0,  0.0F}, {Hyp::MODE1,  0.0F}, {Hyp::MODE2,  0.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::IMAGE_DISPLACEMENT_MODE,
    {{ {Hyp::NONE, 99.0F}, {Hyp::MODE0,  1.0F}, {Hyp::MODE1,  5.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::NORMAL_MODE,
    {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  5.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  0.0F} }}
  },
  { ZoomFilterMode::SCRUNCH_MODE,
    {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  1.0F}, {Hyp::MODE1,  5.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::SPEEDWAY_MODE0,
    {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  5.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::SPEEDWAY_MODE1,
    {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  5.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::SPEEDWAY_MODE2,
    {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  5.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::WATER_MODE,
    {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  1.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::WAVE_MODE0,
    {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  5.0F}, {Hyp::MODE1,  1.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::WAVE_MODE1,
    {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  1.0F}, {Hyp::MODE1,  5.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
  { ZoomFilterMode::Y_ONLY_MODE,
    {{ {Hyp::NONE, 10.0F}, {Hyp::MODE0,  1.0F}, {Hyp::MODE1,  5.0F}, {Hyp::MODE2,  1.0F}, {Hyp::MODE3,  1.0F} }}
  },
}}};
// clang-format on
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

auto FilterSettingsService::GetFilterModeData(
    const IGoomRand& goomRand,
    const std::string& resourcesDirectory,
    const CreateSpeedCoefficientsEffectFunc& createSpeedCoefficientsEffect) -> FilterModeEnumMap
{
  Expects(FILTER_MODE_NAMES.size() == NUM<ZoomFilterMode>);
  Expects(EFFECTS_PROBABILITIES.size() == NUM<ZoomFilterMode>);
  Expects(HYPERCOS_WEIGHTS.size() == NUM<ZoomFilterMode>);

  auto filterModeVec = std::vector<FilterModeEnumMap::KeyValue>{};

  for (auto i = 0U; i < NUM<ZoomFilterMode>; ++i)
  {
    const auto filterMode = static_cast<ZoomFilterMode>(i);

    filterModeVec.emplace_back(
        filterMode,
        ZoomFilterModeInfo{
            FILTER_MODE_NAMES[filterMode],
            createSpeedCoefficientsEffect(filterMode, goomRand, resourcesDirectory),
            {Weights<Hyp>{goomRand, HYPERCOS_WEIGHTS[filterMode]},
                                                        EFFECTS_PROBABILITIES[filterMode]},
    });
  }

  return FilterModeEnumMap::Make(std::move(filterModeVec));
}

FilterSettingsService::FilterSettingsService(const PluginInfo& goomInfo,
                                             const IGoomRand& goomRand,
                                             const std::string& resourcesDirectory,
                                             const CreateSpeedCoefficientsEffectFunc&
                                                 createSpeedCoefficientsEffect)
  : m_goomInfo{goomInfo},
    m_goomRand{goomRand},
    m_screenMidpoint{U_HALF * m_goomInfo.GetScreenWidth(),
                     U_HALF * m_goomInfo.GetScreenHeight()},
    m_resourcesDirectory{resourcesDirectory},
    m_randomizedAfterEffects{std::make_unique<AfterEffectsStates>(m_goomRand)},
    m_filterModeData{GetFilterModeData(m_goomRand,
                                       m_resourcesDirectory,
                                       createSpeedCoefficientsEffect)},
    m_filterSettings{
        false,
        {
           Vitesse{},
           DEFAULT_MAX_SPEED_COEFF,
           nullptr,
           {DEFAULT_ZOOM_MID_X, DEFAULT_ZOOM_MID_Y},
           {
               false,
               HypercosOverlay::NONE,
               false,
               false,
               false,
               false,
               false,
               RotationAdjustments{},
            }
        },
        {DEFAULT_TRAN_LERP_INCREMENT, DEFAULT_SWITCH_MULT},
    },
    m_weightedFilterEvents{
        m_goomRand,
        {
            {ZoomFilterMode::AMULET_MODE,             AMULET_MODE_WEIGHT},
            {ZoomFilterMode::CRYSTAL_BALL_MODE0,      CRYSTAL_BALL_MODE0_WEIGHT},
            {ZoomFilterMode::CRYSTAL_BALL_MODE1,      CRYSTAL_BALL_MODE1_WEIGHT},
            {ZoomFilterMode::DISTANCE_FIELD_MODE,     DISTANCE_FIELD_MODE_WEIGHT},
            {ZoomFilterMode::HYPERCOS_MODE0,          HYPERCOS_MODE0_WEIGHT},
            {ZoomFilterMode::HYPERCOS_MODE1,          HYPERCOS_MODE1_WEIGHT},
            {ZoomFilterMode::HYPERCOS_MODE2,          HYPERCOS_MODE2_WEIGHT},
            {ZoomFilterMode::HYPERCOS_MODE3,          HYPERCOS_MODE3_WEIGHT},
            {ZoomFilterMode::IMAGE_DISPLACEMENT_MODE, IMAGE_DISPLACEMENT_MODE_WEIGHT},
            {ZoomFilterMode::NORMAL_MODE,             NORMAL_MODE_WEIGHT},
            {ZoomFilterMode::SCRUNCH_MODE,            SCRUNCH_MODE_WEIGHT},
            {ZoomFilterMode::SPEEDWAY_MODE0,          SPEEDWAY_MODE0_WEIGHT},
            {ZoomFilterMode::SPEEDWAY_MODE1,          SPEEDWAY_MODE1_WEIGHT},
            {ZoomFilterMode::SPEEDWAY_MODE2,          SPEEDWAY_MODE2_WEIGHT},
            {ZoomFilterMode::WAVE_MODE0,              WAVE_MODE0_WEIGHT},
            {ZoomFilterMode::WAVE_MODE1,              WAVE_MODE1_WEIGHT},
            {ZoomFilterMode::WATER_MODE,              WATER_MODE_WEIGHT},
            {ZoomFilterMode::Y_ONLY_MODE,             Y_ONLY_MODE_WEIGHT},
        },
        {
            {ZoomFilterMode::CRYSTAL_BALL_MODE0, CRYSTAL_BALL_MODE0_MULTIPLIERS},
            {ZoomFilterMode::CRYSTAL_BALL_MODE1, CRYSTAL_BALL_MODE1_MULTIPLIERS},
            {ZoomFilterMode::NORMAL_MODE,        NORMAL_MODE_MULTIPLIERS},
            {ZoomFilterMode::HYPERCOS_MODE0,     HYPERCOS_MODE0_MULTIPLIERS},
            {ZoomFilterMode::HYPERCOS_MODE1,     HYPERCOS_MODE1_MULTIPLIERS},
            {ZoomFilterMode::HYPERCOS_MODE2,     HYPERCOS_MODE2_MULTIPLIERS},
            {ZoomFilterMode::HYPERCOS_MODE3,     HYPERCOS_MODE3_MULTIPLIERS},
            {ZoomFilterMode::SPEEDWAY_MODE0,     SPEEDWAY_MODE0_MULTIPLIERS},
            {ZoomFilterMode::SPEEDWAY_MODE1,     SPEEDWAY_MODE1_MULTIPLIERS},
            {ZoomFilterMode::SPEEDWAY_MODE2,     SPEEDWAY_MODE2_MULTIPLIERS},
            {ZoomFilterMode::WAVE_MODE0,         WAVE_MODE0_MULTIPLIERS},
            {ZoomFilterMode::WAVE_MODE1,         WAVE_MODE1_MULTIPLIERS},
        },
    },
    m_zoomMidpointWeights{
        m_goomRand,
        {
            {ZoomMidpointEvents::BOTTOM_MID_POINT,               BOTTOM_MID_POINT_WEIGHT},
            {ZoomMidpointEvents::RIGHT_MID_POINT,                RIGHT_MID_POINT_WEIGHT},
            {ZoomMidpointEvents::LEFT_MID_POINT,                 LEFT_MID_POINT_WEIGHT},
            {ZoomMidpointEvents::CENTRE_MID_POINT,               CENTRE_MID_POINT_WEIGHT},
            {ZoomMidpointEvents::TOP_LEFT_QUARTER_MID_POINT,     TOP_LEFT_QUARTER_MID_POINT_WEIGHT},
            {ZoomMidpointEvents::BOTTOM_RIGHT_QUARTER_MID_POINT, BOTTOM_RIGHT_QUARTER_MID_POINT_WEIGHT},
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

inline auto FilterSettingsService::GetSpeedCoefficientsEffect()
    -> std::shared_ptr<ISpeedCoefficientsEffect>&
{
  return m_filterModeData[m_filterMode].speedCoefficientsEffect;
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

auto FilterSettingsService::SetRandomSettingsForNewFilterMode() -> void
{
  SetDefaultSettings();
  SetRandomZoomMidpoint();
  SetFilterModeExtraEffects();
  UpdateFilterSettingsFromExtraEffects();
}

auto FilterSettingsService::SetDefaultSettings() -> void
{
  m_filterSettings.filterEffectsSettings.speedCoefficientsEffect = GetSpeedCoefficientsEffect();
  m_filterSettings.filterEffectsSettings.zoomMidpoint            = m_screenMidpoint;
  m_filterSettings.filterEffectsSettings.vitesse.SetDefault();

  m_randomizedAfterEffects->SetDefaults();
}

inline auto FilterSettingsService::SetFilterModeExtraEffects() -> void
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

inline auto FilterSettingsService::SetRandomizedExtraEffects() -> void
{
  const auto& modeInfo = m_filterModeData[m_filterMode];

  m_randomizedAfterEffects->ResetAllStates(modeInfo.afterEffectsProbabilities);

  m_filterSettings.filterEffectsSettings.afterEffectsSettings.rotationAdjustments.SetMultiplyFactor(
      modeInfo.afterEffectsProbabilities.probabilities.rotateProbability,
      RotationAdjustments::AdjustmentType::AFTER_RANDOM);
}

auto FilterSettingsService::SetWaveModeExtraEffects() -> void
{
  if ((m_filterMode != ZoomFilterMode::WAVE_MODE0) && (m_filterMode != ZoomFilterMode::WAVE_MODE1))
  {
    return;
  }

  m_randomizedAfterEffects->TurnPlaneEffectOn();

  auto& filterEffectsSettings = m_filterSettings.filterEffectsSettings;
  filterEffectsSettings.vitesse.SetReverseVitesse(m_goomRand.ProbabilityOf(PROB_REVERSE_SPEED));
  if (m_goomRand.ProbabilityOf(PROB_CHANGE_SPEED))
  {
    filterEffectsSettings.vitesse.SetVitesse(
        I_HALF * (filterEffectsSettings.vitesse.GetVitesse() + Vitesse::DEFAULT_VITESSE));
  }
}

inline auto FilterSettingsService::UpdateFilterSettingsFromExtraEffects() -> void
{
  m_filterSettings.filterEffectsSettingsHaveChanged = true;
  m_randomizedAfterEffects->UpdateFilterSettingsFromStates(
      m_filterSettings.filterEffectsSettings.afterEffectsSettings);
}

auto FilterSettingsService::SetMaxSpeedCoeff() -> void
{
  static constexpr auto MIN_SPEED_FACTOR = 0.5F;
  static constexpr auto MAX_SPEED_FACTOR = 1.0F;
  m_filterSettings.filterEffectsSettings.maxSpeedCoeff =
      m_goomRand.GetRandInRange(MIN_SPEED_FACTOR, MAX_SPEED_FACTOR) * MAX_MAX_SPEED_COEFF;
}

auto FilterSettingsService::SetRandomZoomMidpoint() -> void
{
  if (IsZoomMidpointInTheMiddle())
  {
    m_filterSettings.filterEffectsSettings.zoomMidpoint = m_screenMidpoint;
    return;
  }

  const auto allowEdgePoints =
      (m_filterMode != ZoomFilterMode::WAVE_MODE0) && (m_filterMode != ZoomFilterMode::WAVE_MODE1);
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
      m_goomRand.ProbabilityOf(PROB_CRYSTAL_BALL_IN_MIDDLE))
  {
    return true;
  }

  if (((m_filterMode == ZoomFilterMode::WAVE_MODE0) ||
       (m_filterMode == ZoomFilterMode::WAVE_MODE1)) &&
      m_goomRand.ProbabilityOf(PROB_WAVE_IN_MIDDLE))
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
         (midPointEvent == ZoomMidpointEvents::RIGHT_MID_POINT) ||
         (midPointEvent == ZoomMidpointEvents::LEFT_MID_POINT);
}

auto FilterSettingsService::SetAnyRandomZoomMidpoint(const bool allowEdgePoints) -> void
{
  switch (GetWeightRandomMidPoint(allowEdgePoints))
  {
    case ZoomMidpointEvents::BOTTOM_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = {U_HALF * m_goomInfo.GetScreenWidth(),
                                                             m_goomInfo.GetScreenHeight() - 1};
      break;
    case ZoomMidpointEvents::RIGHT_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint.x =
          static_cast<int32_t>(m_goomInfo.GetScreenWidth() - 1);
      break;
    case ZoomMidpointEvents::LEFT_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint.x = 1;
      break;
    case ZoomMidpointEvents::CENTRE_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = m_screenMidpoint;
      break;
    case ZoomMidpointEvents::TOP_LEFT_QUARTER_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = {
          U_QUARTER * m_goomInfo.GetScreenWidth(), U_QUARTER * m_goomInfo.GetScreenHeight()};
      break;
    case ZoomMidpointEvents::BOTTOM_RIGHT_QUARTER_MID_POINT:
      m_filterSettings.filterEffectsSettings.zoomMidpoint = {
          U_THREE_QUARTERS * m_goomInfo.GetScreenWidth(),
          U_THREE_QUARTERS * m_goomInfo.GetScreenHeight()};
      break;
    default:
      FailFast();
  }
}

} // namespace GOOM::FILTER_FX
