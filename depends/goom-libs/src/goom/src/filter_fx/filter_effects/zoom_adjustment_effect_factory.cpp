#include "zoom_adjustment_effect_factory.h"

#include "filter_fx/filter_effects/adjustment_effects/amulet.h"
#include "filter_fx/filter_effects/adjustment_effects/crystal_ball.h"
#include "filter_fx/filter_effects/adjustment_effects/distance_field.h"
#include "filter_fx/filter_effects/adjustment_effects/exp_reciprocal.h"
#include "filter_fx/filter_effects/adjustment_effects/image_zoom_adjustment.h"
#include "filter_fx/filter_effects/adjustment_effects/mobius.h"
#include "filter_fx/filter_effects/adjustment_effects/scrunch.h"
#include "filter_fx/filter_effects/adjustment_effects/speedway.h"
#include "filter_fx/filter_effects/adjustment_effects/uniform_zoom_adjustment_effect.h"
#include "filter_fx/filter_effects/adjustment_effects/wave.h"
#include "filter_fx/filter_effects/adjustment_effects/y_only.h"
#include "filter_fx/filter_settings_service.h"
#include "filter_fx/zoom_adjustment_effect.h"
#include "goom/goom_config.h"
#include "utils/math/goom_rand_base.h"

#include <memory>
#include <string>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using GOOM::UTILS::MATH::IGoomRand;

auto CreateZoomAdjustmentEffect(const ZoomFilterMode filterMode,
                                const IGoomRand& goomRand,
                                const std::string& resourcesDirectory)
    -> std::shared_ptr<IZoomAdjustmentEffect>
{
  switch (filterMode)
  {
    case ZoomFilterMode::AMULET_MODE:
      return std::make_shared<Amulet>(goomRand);
    case ZoomFilterMode::CRYSTAL_BALL_MODE0:
      return std::make_shared<CrystalBall>(CrystalBall::Modes::MODE0, goomRand);
    case ZoomFilterMode::CRYSTAL_BALL_MODE1:
      return std::make_shared<CrystalBall>(CrystalBall::Modes::MODE1, goomRand);
    case ZoomFilterMode::DISTANCE_FIELD_MODE0:
      return std::make_shared<DistanceField>(DistanceField::Modes::MODE0, goomRand);
    case ZoomFilterMode::DISTANCE_FIELD_MODE1:
      return std::make_shared<DistanceField>(DistanceField::Modes::MODE1, goomRand);
    case ZoomFilterMode::DISTANCE_FIELD_MODE2:
      return std::make_shared<DistanceField>(DistanceField::Modes::MODE2, goomRand);
    case ZoomFilterMode::EXP_RECIPROCAL_MODE:
      return std::make_shared<ExpReciprocal>(goomRand);
    case ZoomFilterMode::HYPERCOS_MODE0:
    case ZoomFilterMode::HYPERCOS_MODE1:
    case ZoomFilterMode::HYPERCOS_MODE2:
    case ZoomFilterMode::HYPERCOS_MODE3:
      return std::make_shared<UniformZoomAdjustmentEffect>();
    case ZoomFilterMode::IMAGE_DISPLACEMENT_MODE:
      return std::make_shared<ImageZoomAdjustment>(resourcesDirectory, goomRand);
    case ZoomFilterMode::MOBIUS_MODE:
      return std::make_shared<Mobius>(goomRand);
    case ZoomFilterMode::NORMAL_MODE:
      return std::make_shared<UniformZoomAdjustmentEffect>();
    case ZoomFilterMode::SCRUNCH_MODE:
      return std::make_shared<Scrunch>(goomRand);
    case ZoomFilterMode::SPEEDWAY_MODE0:
      return std::make_shared<Speedway>(Speedway::Modes::MODE0, goomRand);
    case ZoomFilterMode::SPEEDWAY_MODE1:
      return std::make_shared<Speedway>(Speedway::Modes::MODE1, goomRand);
    case ZoomFilterMode::SPEEDWAY_MODE2:
      return std::make_shared<Speedway>(Speedway::Modes::MODE2, goomRand);
    case ZoomFilterMode::WATER_MODE:
      return std::make_shared<UniformZoomAdjustmentEffect>();
    case ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE0:
      return std::make_shared<Wave>(Wave::Modes::SQ_DIST_ANGLE_EFFECT_MODE0, goomRand);
    case ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE1:
      return std::make_shared<Wave>(Wave::Modes::SQ_DIST_ANGLE_EFFECT_MODE1, goomRand);
    case ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE0:
      return std::make_shared<Wave>(Wave::Modes::ATAN_ANGLE_EFFECT_MODE0, goomRand);
    case ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE1:
      return std::make_shared<Wave>(Wave::Modes::ATAN_ANGLE_EFFECT_MODE1, goomRand);
    case ZoomFilterMode::Y_ONLY_MODE:
      return std::make_shared<YOnly>(goomRand);
    default:
      FailFast();
  }
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
