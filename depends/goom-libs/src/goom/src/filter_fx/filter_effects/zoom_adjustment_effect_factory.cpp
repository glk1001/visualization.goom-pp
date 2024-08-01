module;

#include <format>
#include <memory>
#include <string>

module Goom.FilterFx.FilterEffects.ZoomAdjustmentEffectFactory;

import Goom.FilterFx.FilterEffects.AdjustmentEffects.Amulet;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.ComplexRational;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.CrystalBall;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.DistanceField;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.ExpReciprocal;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.FlowField;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.FunctionOfFunction;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.ImageZoomAdjustment;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.Mobius;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.Newton;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.PerlinNoise;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.Scrunch;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.Speedway;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.UniformZoomAdjustmentEffect;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.Wave;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.YOnly;
import Goom.FilterFx.FilterSettingsService;
import Goom.FilterFx.ZoomAdjustmentEffect;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.EnumUtils;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::NUM;
using UTILS::MATH::GoomRand;

namespace
{

using UTILS::MATH::NumberRange;
using enum ZoomFilterMode;

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto IsAllowedFuncOfMode(const ZoomFilterMode funcOfMode,
                         [[maybe_unused]] const ZoomFilterMode baseFilterMode) noexcept -> bool
{
  return funcOfMode != WATER_MODE;

  //static const auto s_NON_FUNC_OF_FUNC_MODES = std::map<ZoomFilterMode, std::set<ZoomFilterMode>>{
  //     {WAVE_SQ_DIST_ANGLE_EFFECT_MODE0, {EXP_RECIPROCAL_MODE}},
  //     {WAVE_SQ_DIST_ANGLE_EFFECT_MODE1, {EXP_RECIPROCAL_MODE}},
  //     {   WAVE_ATAN_ANGLE_EFFECT_MODE0, {EXP_RECIPROCAL_MODE}},
  //     {   WAVE_ATAN_ANGLE_EFFECT_MODE1, {EXP_RECIPROCAL_MODE}},
  // };
  //
  // if (not s_NON_FUNC_OF_FUNC_MODES.contains(baseFilterMode))
  // {
  //   return true;
  // }
  //
  // return not s_NON_FUNC_OF_FUNC_MODES.at(baseFilterMode).contains(funcOfMode);
}


auto CreateFuncZoomAdjustmentEffect(const ZoomFilterMode filterMode,
                                    const GoomRand& goomRand,
                                    const std::string& resourcesDirectory)
    -> std::unique_ptr<IZoomAdjustmentEffect>
{
  switch (filterMode)
  {
    case AMULET_MODE:
      return std::make_unique<Amulet>(goomRand);
    case COMPLEX_RATIONAL_MODE:
      return std::make_unique<ComplexRational>(goomRand);
    case CRYSTAL_BALL_MODE0:
      return std::make_unique<CrystalBall>(CrystalBall::Modes::MODE0, goomRand);
    case CRYSTAL_BALL_MODE1:
      return std::make_unique<CrystalBall>(CrystalBall::Modes::MODE1, goomRand);
    case DISTANCE_FIELD_MODE0:
      return std::make_unique<DistanceField>(DistanceField::Modes::MODE0, goomRand);
    case DISTANCE_FIELD_MODE1:
      return std::make_unique<DistanceField>(DistanceField::Modes::MODE1, goomRand);
    case DISTANCE_FIELD_MODE2:
      return std::make_unique<DistanceField>(DistanceField::Modes::MODE2, goomRand);
    case EXP_RECIPROCAL_MODE:
      return std::make_unique<ExpReciprocal>(goomRand);
    case FLOW_FIELD_MODE:
      return std::make_unique<FlowField>(goomRand);
    case HYPERCOS_MODE0:
    case HYPERCOS_MODE1:
    case HYPERCOS_MODE2:
    case HYPERCOS_MODE3:
      return std::make_unique<UniformZoomAdjustmentEffect>();
    case IMAGE_DISPLACEMENT_MODE:
      return std::make_unique<ImageZoomAdjustment>(resourcesDirectory, goomRand);
    case MOBIUS_MODE:
      return std::make_unique<Mobius>(goomRand);
    case NEWTON_MODE:
      return std::make_unique<Newton>(goomRand);
    case NORMAL_MODE:
      return std::make_unique<UniformZoomAdjustmentEffect>();
    case PERLIN_NOISE_MODE:
      return std::make_unique<PerlinNoise>(goomRand);
    case SCRUNCH_MODE:
      return std::make_unique<Scrunch>(goomRand);
    case SPEEDWAY_MODE0:
      return std::make_unique<Speedway>(Speedway::Modes::MODE0, goomRand);
    case SPEEDWAY_MODE1:
      return std::make_unique<Speedway>(Speedway::Modes::MODE1, goomRand);
    case SPEEDWAY_MODE2:
      return std::make_unique<Speedway>(Speedway::Modes::MODE2, goomRand);
    case WATER_MODE:
      return std::make_unique<UniformZoomAdjustmentEffect>();
    case WAVE_SQ_DIST_ANGLE_EFFECT_MODE0:
      return std::make_unique<Wave>(Wave::Modes::SQ_DIST_ANGLE_EFFECT_MODE0, goomRand);
    case WAVE_SQ_DIST_ANGLE_EFFECT_MODE1:
      return std::make_unique<Wave>(Wave::Modes::SQ_DIST_ANGLE_EFFECT_MODE1, goomRand);
    case WAVE_ATAN_ANGLE_EFFECT_MODE0:
      return std::make_unique<Wave>(Wave::Modes::ATAN_ANGLE_EFFECT_MODE0, goomRand);
    case WAVE_ATAN_ANGLE_EFFECT_MODE1:
      return std::make_unique<Wave>(Wave::Modes::ATAN_ANGLE_EFFECT_MODE1, goomRand);
    case Y_ONLY_MODE:
      return std::make_unique<YOnly>(goomRand);
  }
}

auto GetFuncOfMode(const ZoomFilterMode baseFilterMode,
                   const GoomRand& goomRand) noexcept -> ZoomFilterMode
{
  static constexpr auto MAX_RETRIES = 10U;

  for (auto i = 0U; i < MAX_RETRIES; ++i)
  {
    static constexpr auto FILTER_MODE_RANGE = NumberRange{0U, NUM<ZoomFilterMode> - 1};
    if (const auto funcOfMode =
            static_cast<ZoomFilterMode>(goomRand.GetRandInRange<FILTER_MODE_RANGE>());
        IsAllowedFuncOfMode(funcOfMode, baseFilterMode))
    {
      return funcOfMode;
    }
  }

  return NORMAL_MODE;
}

auto CreateFuncOfFuncZoomAdjustmentEffect(const ZoomFilterMode filterMode,
                                          const GoomRand& goomRand,
                                          const std::string& resourcesDirectory)
    -> std::unique_ptr<IZoomAdjustmentEffect>
{
  auto zoomAdjustmentEffect =
      CreateFuncZoomAdjustmentEffect(filterMode, goomRand, resourcesDirectory);
  const auto funcOfMode = GetFuncOfMode(filterMode, goomRand);
  auto funcOf           = CreateFuncZoomAdjustmentEffect(funcOfMode, goomRand, resourcesDirectory);
  const auto fofName =
      std::format("{}({})", GetFilterModeName(funcOfMode), GetFilterModeName(filterMode));
  return std::make_unique<FunctionOfFunction>(
      goomRand, fofName, std::move(funcOf), std::move(zoomAdjustmentEffect));
}

} // namespace

auto CreateZoomAdjustmentEffect(const ZoomFilterMode filterMode,
                                const GoomRand& goomRand,
                                const std::string& resourcesDirectory)
    -> std::unique_ptr<IZoomAdjustmentEffect>
{
  if (filterMode == EXP_RECIPROCAL_MODE)
  {
    return CreateFuncZoomAdjustmentEffect(filterMode, goomRand, resourcesDirectory);
  }

  if (static constexpr auto PROB_FUNC_OF_FUNC = 0.5F; goomRand.ProbabilityOf<PROB_FUNC_OF_FUNC>())
  {
    return CreateFuncOfFuncZoomAdjustmentEffect(filterMode, goomRand, resourcesDirectory);
  }

  return CreateFuncZoomAdjustmentEffect(filterMode, goomRand, resourcesDirectory);
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
