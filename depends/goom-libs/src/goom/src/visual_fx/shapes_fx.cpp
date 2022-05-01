#include "shapes_fx.h"

//#undef NO_LOGGING

#include "color/colorutils.h"
#include "color/random_colormaps.h"
#include "color/random_colormaps_manager.h"
#include "draw/goom_draw.h"
#include "fx_helper.h"
#include "goom/logging.h"
#include "goom/spimpl.h"
#include "goom_config.h"
#include "goom_plugin_info.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/math/paths.h"
#include "utils/math/transform2d.h"
#include "utils/t_values.h"
#include "utils/timer.h"

#include <cassert>
#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX
{

using COLOR::GetAllMapsUnweighted;
using COLOR::GetBrighterColor;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using COLOR::RandomColorMapsManager;
using DRAW::IGoomDraw;
using UTILS::Logging;
using UTILS::Timer;
using UTILS::TValue;
using UTILS::MATH::AngleParams;
using UTILS::MATH::Hypotrochoid;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::IPath;
using UTILS::MATH::LissajousPath;
using UTILS::MATH::Transform2d;
using UTILS::MATH::TransformedPath;
using UTILS::MATH::U_HALF;

class ShapeGroup;

class ShapePath
{
public:
  struct ColorInfo
  {
    RandomColorMapsManager::ColorMapId mainColorMapId{};
    RandomColorMapsManager::ColorMapId lowColorMapId{};
    RandomColorMapsManager::ColorMapId innerColorMapId{};
  };

  ShapePath(std::shared_ptr<IPath> path, ColorInfo colorInfo) noexcept;

  auto UpdateMainColorInfo(ShapeGroup& parentShapeGroup) const noexcept -> void;
  auto UpdateLowColorInfo(ShapeGroup& parentShapeGroup) const noexcept -> void;
  auto UpdateInnerColorInfo(ShapeGroup& parentShapeGroup) const noexcept -> void;

  [[nodiscard]] auto GetNextPoint() const noexcept -> Point2dInt;
  [[nodiscard]] auto GetColorInfo() const noexcept -> const ColorInfo&;

private:
  std::shared_ptr<IPath> m_path;
  ColorInfo m_colorInfo;
};

class ShapeGroup
{
public:
  ShapeGroup(const IGoomRand& goomRand,
             RandomColorMapsManager& colorMapsManager,
             Point2dInt screenMidpoint,
             float tMinMaxLerp) noexcept;
  ShapeGroup(const ShapeGroup&) noexcept = delete;
  ShapeGroup(ShapeGroup&&) noexcept = default;
  ~ShapeGroup() noexcept = default;
  auto operator=(const ShapeGroup&) -> ShapeGroup& = delete;
  auto operator=(ShapeGroup&&) -> ShapeGroup& = delete;

  auto SetWeightedMainColorMaps(std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void;
  auto SetWeightedLowColorMaps(std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void;
  auto SetWeightedInnerColorMaps(std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void;

  auto Start() noexcept -> void;

  auto IncrementTs() noexcept -> void;

  auto DoRandomChanges() noexcept -> void;
  auto UseRandomShapesSpeed() noexcept -> void;
  auto UseFixedShapesSpeed(float tMinMaxLerp) noexcept -> void;

  auto UpdateMainColorMapId(RandomColorMapsManager::ColorMapId mainColorMapId) noexcept -> void;
  auto UpdateLowColorMapId(RandomColorMapsManager::ColorMapId lowColorMapId) noexcept -> void;
  auto UpdateInnerColorMapId(RandomColorMapsManager::ColorMapId innerColorMapId) noexcept -> void;

  [[nodiscard]] auto GetNumShapes() const noexcept -> size_t;
  [[nodiscard]] auto GetShapePath(size_t shapeNum) const noexcept -> const ShapePath&;
  [[nodiscard]] auto GetColorMap(RandomColorMapsManager::ColorMapId colorMapId) const noexcept
      -> const IColorMap&;

  [[nodiscard]] auto GetCurrentColor(RandomColorMapsManager::ColorMapId colorMapId) const noexcept
      -> Pixel;
  [[nodiscard]] auto GetInnerColorMix() const noexcept -> float;
  [[nodiscard]] auto GetCurrentShapesRadius() const noexcept -> int32_t;

private:
  const IGoomRand& m_goomRand;
  const Point2dInt m_screenMidpoint;
  static constexpr float MIN_SHAPE_SPEED = 0.001F;
  static constexpr float MAX_SHAPE_SPEED = 0.005F;
  TValue m_allShapesPositionT;
  bool m_useRandomShapesSpeed = true;
  float m_tMinMaxLerp = 0.0F;
  auto SetShapesSpeed() noexcept -> void;
  [[nodiscard]] static auto GetShapesSpeed(float tMinMaxLerp) noexcept -> float;

  static constexpr int32_t MIN_SHAPE_RADIUS = 10;
  static constexpr int32_t MAX_SHAPE_RADIUS = 50;
  static constexpr uint32_t MIN_RADIUS_STEPS = 10;
  static constexpr uint32_t MAX_RADIUS_STEPS = 50;
  static constexpr uint32_t INITIAL_RADIUS_STEPS = 20;
  TValue m_radiusT{TValue::StepType::CONTINUOUS_REVERSIBLE, INITIAL_RADIUS_STEPS};

  static constexpr float MIN_INNER_COLOR_MIX_T = 0.1F;
  static constexpr float MAX_INNER_COLOR_MIX_T = 0.9F;
  RandomColorMapsManager& m_colorMapsManager;
  struct ColorInfo
  {
    std::shared_ptr<RandomColorMaps> mainColorMaps;
    std::shared_ptr<RandomColorMaps> lowColorMaps;
    std::shared_ptr<RandomColorMaps> innerColorMaps;
    float innerColorMix;
  };
  ColorInfo m_colorInfo;
  [[nodiscard]] auto GetInitialColorInfo() const noexcept -> ColorInfo;
  [[nodiscard]] auto MakeNewColorMapId() noexcept -> RandomColorMapsManager::ColorMapId;

  static constexpr float MIN_COLOR_MAP_SPEED = 10.0F * MIN_SHAPE_SPEED;
  static constexpr float MAX_COLOR_MAP_SPEED = 10.0F * MAX_SHAPE_SPEED;
  TValue m_allColorsT;
  auto ChangeAllColorsT() noexcept -> void;

  std::vector<ShapePath> m_shapePaths{};
  auto ChangeAllColorMapsNow() noexcept -> void;
  auto SetRandomizedShapePaths() noexcept -> void;
  [[nodiscard]] auto GetRandomizedShapePaths() noexcept -> std::vector<ShapePath>;
  [[nodiscard]] auto GetShapePaths(std::shared_ptr<IPath> basePath,
                                   const Point2dInt& screenMidpoint) noexcept
      -> std::vector<ShapePath>;
};

static_assert(std::is_nothrow_move_constructible_v<ShapeGroup>);

ShapeGroup::ShapeGroup(const IGoomRand& goomRand,
                       RandomColorMapsManager& colorMapsManager,
                       const Point2dInt screenMidpoint,
                       const float tMinMaxLerp) noexcept
  : m_goomRand{goomRand},
    m_screenMidpoint{screenMidpoint},
    m_allShapesPositionT{TValue::StepType::CONTINUOUS_REVERSIBLE, GetShapesSpeed(tMinMaxLerp)},
    m_colorMapsManager{colorMapsManager},
    m_colorInfo{GetInitialColorInfo()},
    m_allColorsT{TValue::StepType::CONTINUOUS_REVERSIBLE, GetShapesSpeed(tMinMaxLerp)}
{
}

auto ShapeGroup::GetInitialColorInfo() const noexcept -> ColorInfo
{
  return {GetAllMapsUnweighted(m_goomRand), GetAllMapsUnweighted(m_goomRand),
          GetAllMapsUnweighted(m_goomRand),
          m_goomRand.GetRandInRange(MIN_INNER_COLOR_MIX_T, MAX_INNER_COLOR_MIX_T)};
}

inline auto ShapeGroup::Start() noexcept -> void
{
  SetRandomizedShapePaths();
}

inline auto ShapeGroup::GetRandomizedShapePaths() noexcept -> std::vector<ShapePath>
{
  const Hypotrochoid::Params params = {
      m_goomRand.GetRandInRange(5.0F, 12.0F),
      m_goomRand.GetRandInRange(3.0F, 10.0F),
      m_goomRand.GetRandInRange(5.0F, 10.F),
      m_goomRand.GetRandInRange(35.0F, 45.0F),
  };
  const auto baseHypotrochoid =
      std::make_shared<Hypotrochoid>(Point2dInt{0, 0}, m_allShapesPositionT, params);

  return GetShapePaths(baseHypotrochoid, m_screenMidpoint);
}

auto ShapeGroup::GetShapePaths(const std::shared_ptr<IPath> basePath,
                               const Point2dInt& screenMidpoint) noexcept -> std::vector<ShapePath>
{
  const Vec2dFlt screenMidpointFlt{screenMidpoint.ToFlt()};

  std::vector<ShapePath> shapePaths{};
  Transform2d transform{};

  static constexpr float OFFSET = 20.0F;

  static constexpr float ROTATE0 = 0.0F;
  static constexpr float SCALE0 = 1.0F;
  transform = Transform2d{
      ROTATE0, SCALE0, screenMidpointFlt + Vec2dFlt{-OFFSET, 0.0F}
  };
  shapePaths.emplace_back(
      std::make_shared<TransformedPath>(basePath, transform),
      ShapePath::ColorInfo{MakeNewColorMapId(), MakeNewColorMapId(), MakeNewColorMapId()});

  static constexpr float ROTATE1 = 45.0F;
  static constexpr float SCALE1 = 1.2F;
  transform = Transform2d{
      ROTATE1, SCALE1, screenMidpointFlt + Vec2dFlt{+OFFSET, 0.0F}
  };
  shapePaths.emplace_back(
      std::make_shared<TransformedPath>(basePath, transform),
      ShapePath::ColorInfo{MakeNewColorMapId(), MakeNewColorMapId(), MakeNewColorMapId()});

  static constexpr float ROTATE2 = 90.0F;
  static constexpr float SCALE2 = 1.3F;
  transform = Transform2d{
      ROTATE2, SCALE2, screenMidpointFlt + Vec2dFlt{0.0F, -OFFSET}
  };
  shapePaths.emplace_back(
      std::make_shared<TransformedPath>(basePath, transform),
      ShapePath::ColorInfo{MakeNewColorMapId(), MakeNewColorMapId(), MakeNewColorMapId()});

  static constexpr float ROTATE3 = 135.0F;
  static constexpr float SCALE3 = 1.2F;
  transform = Transform2d{
      ROTATE3, SCALE3, screenMidpointFlt + Vec2dFlt{0.0F, +OFFSET}
  };
  shapePaths.emplace_back(
      std::make_shared<TransformedPath>(basePath, transform),
      ShapePath::ColorInfo{MakeNewColorMapId(), MakeNewColorMapId(), MakeNewColorMapId()});

  return shapePaths;
}

inline auto ShapeGroup::GetNumShapes() const noexcept -> size_t
{
  return m_shapePaths.size();
}

inline auto ShapeGroup::GetShapePath(const size_t shapeNum) const noexcept -> const ShapePath&
{
  return m_shapePaths.at(shapeNum);
}

inline auto ShapeGroup::GetColorMap(
    const RandomColorMapsManager::ColorMapId colorMapId) const noexcept -> const IColorMap&
{
  return m_colorMapsManager.GetColorMap(colorMapId);
}

inline auto ShapeGroup::GetCurrentColor(
    const RandomColorMapsManager::ColorMapId colorMapId) const noexcept -> Pixel
{
  return GetColorMap(colorMapId).GetColor(m_allColorsT());
}

inline auto ShapeGroup::GetInnerColorMix() const noexcept -> float
{
  return m_colorInfo.innerColorMix;
}

auto ShapeGroup::GetCurrentShapesRadius() const noexcept -> int32_t
{
  return STD20::lerp(MIN_SHAPE_RADIUS, MAX_SHAPE_RADIUS, m_radiusT());
}

inline auto ShapeGroup::UpdateMainColorMapId(
    const RandomColorMapsManager::ColorMapId mainColorMapId) noexcept -> void
{
  const std::shared_ptr<RandomColorMaps>& mainColorMaps = m_colorInfo.mainColorMaps;

  m_colorMapsManager.UpdateColorMapInfo(
      mainColorMapId,
      {mainColorMaps, mainColorMaps->GetRandomColorMapName(mainColorMaps->GetRandomGroup()),
       RandomColorMaps::ALL_COLOR_MAP_TYPES});
}

inline auto ShapeGroup::UpdateLowColorMapId(
    const RandomColorMapsManager::ColorMapId lowColorMapId) noexcept -> void
{
  const std::shared_ptr<RandomColorMaps>& lowColorMaps = m_colorInfo.lowColorMaps;

  m_colorMapsManager.UpdateColorMapInfo(
      lowColorMapId,
      {lowColorMaps, lowColorMaps->GetRandomColorMapName(lowColorMaps->GetRandomGroup()),
       RandomColorMaps::ALL_COLOR_MAP_TYPES});
}

inline auto ShapeGroup::UpdateInnerColorMapId(
    const RandomColorMapsManager::ColorMapId innerColorMapId) noexcept -> void
{
  const std::shared_ptr<RandomColorMaps>& innerColorMaps = m_colorInfo.innerColorMaps;

  m_colorMapsManager.UpdateColorMapInfo(
      innerColorMapId,
      {innerColorMaps, innerColorMaps->GetRandomColorMapName(innerColorMaps->GetRandomGroup()),
       RandomColorMaps::ALL_COLOR_MAP_TYPES});
}

inline auto ShapeGroup::IncrementTs() noexcept -> void
{
  m_allShapesPositionT.Increment();
  m_allColorsT.Increment();
  m_radiusT.Increment();
}

inline auto ShapeGroup::UseRandomShapesSpeed() noexcept -> void
{
  m_useRandomShapesSpeed = true;
}

inline auto ShapeGroup::UseFixedShapesSpeed(const float tMinMaxLerp) noexcept -> void
{
  m_tMinMaxLerp = tMinMaxLerp;
  m_useRandomShapesSpeed = false;
}

inline void ShapeGroup::DoRandomChanges() noexcept
{
  if (not m_allShapesPositionT.HasJustHitStartBoundary())
  {
    return;
  }

  SetRandomizedShapePaths();
  SetShapesSpeed();
  ChangeAllColorMapsNow();
  ChangeAllColorsT();
}

inline auto ShapeGroup::ChangeAllColorMapsNow() noexcept -> void
{
  m_colorMapsManager.ChangeAllColorMapsNow();
}

inline auto ShapeGroup::ChangeAllColorsT() noexcept -> void
{
  const float t = m_goomRand.GetRandInRange(MIN_COLOR_MAP_SPEED, MAX_COLOR_MAP_SPEED);
  m_allColorsT.SetStepSize(STD20::lerp(MIN_COLOR_MAP_SPEED, MAX_COLOR_MAP_SPEED, t));
}

inline auto ShapeGroup::SetRandomizedShapePaths() noexcept -> void
{
  m_shapePaths = GetRandomizedShapePaths();
}

inline auto ShapeGroup::SetShapesSpeed() noexcept -> void
{
  const float tMinMaxLerp =
      m_useRandomShapesSpeed ? m_goomRand.GetRandInRange(0.0F, 1.0F) : m_tMinMaxLerp;
  m_allShapesPositionT.SetStepSize(GetShapesSpeed(tMinMaxLerp));

  m_radiusT.SetNumSteps(STD20::lerp(MIN_RADIUS_STEPS, MAX_RADIUS_STEPS, tMinMaxLerp));
}

inline auto ShapeGroup::GetShapesSpeed(const float tMinMaxLerp) noexcept -> float
{
  return STD20::lerp(MIN_SHAPE_SPEED, MAX_SHAPE_SPEED, tMinMaxLerp);
}

inline auto ShapeGroup::MakeNewColorMapId() noexcept -> RandomColorMapsManager::ColorMapId
{
  return m_colorMapsManager.AddDefaultColorMapInfo(m_goomRand);
}

inline auto ShapeGroup::SetWeightedMainColorMaps(
    const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void
{
  m_colorInfo.mainColorMaps = weightedMaps;

  for (const auto& shapePath : m_shapePaths)
  {
    shapePath.UpdateMainColorInfo(*this);
  }
}

inline auto ShapeGroup::SetWeightedLowColorMaps(
    const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void
{
  m_colorInfo.lowColorMaps = weightedMaps;

  for (const auto& shapePath : m_shapePaths)
  {
    shapePath.UpdateLowColorInfo(*this);
  }
}

inline auto ShapeGroup::SetWeightedInnerColorMaps(
    const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void
{
  m_colorInfo.innerColorMix =
      m_goomRand.GetRandInRange(MIN_INNER_COLOR_MIX_T, MAX_INNER_COLOR_MIX_T);

  m_colorInfo.innerColorMaps = weightedMaps;

  for (const auto& shapePath : m_shapePaths)
  {
    shapePath.UpdateInnerColorInfo(*this);
  }
}

inline ShapePath::ShapePath(const std::shared_ptr<IPath> path, const ColorInfo colorInfo) noexcept
  : m_path{path}, m_colorInfo{colorInfo}
{
}

inline auto ShapePath::GetNextPoint() const noexcept -> Point2dInt
{
  return m_path->GetNextPoint();
}

inline auto ShapePath::GetColorInfo() const noexcept -> const ColorInfo&
{
  return m_colorInfo;
}

inline auto ShapePath::UpdateMainColorInfo(ShapeGroup& parentShapeGroup) const noexcept -> void
{
  parentShapeGroup.UpdateMainColorMapId(m_colorInfo.mainColorMapId);
}

inline auto ShapePath::UpdateLowColorInfo(ShapeGroup& parentShapeGroup) const noexcept -> void
{
  parentShapeGroup.UpdateLowColorMapId(m_colorInfo.lowColorMapId);
}

inline auto ShapePath::UpdateInnerColorInfo(ShapeGroup& parentShapeGroup) const noexcept -> void
{
  parentShapeGroup.UpdateInnerColorMapId(m_colorInfo.innerColorMapId);
}

class ShapesFx::ShapesFxImpl
{
public:
  explicit ShapesFxImpl(const FxHelper& fxHelper) noexcept;

  auto SetWeightedMainColorMaps(size_t shapeGroupNum,
                                std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void;
  auto SetWeightedLowColorMaps(size_t shapeGroupNum,
                               std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void;
  auto SetWeightedInnerColorMaps(size_t shapeGroupNum,
                                 std::shared_ptr<RandomColorMaps> weightedMaps) noexcept -> void;

  auto Start() noexcept -> void;
  auto ApplyMultiple() noexcept -> void;

private:
  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const IGoomRand& m_goomRand;
  const Point2dInt m_screenMidpoint;
  RandomColorMapsManager m_colorMapsManager{};
  static constexpr uint32_t TIME_BEFORE_SYNCHRONISED_CHANGE = 5000;
  Timer m_synchronisedShapeChangesTimer{TIME_BEFORE_SYNCHRONISED_CHANGE};

  [[nodiscard]] auto AllColorMapsValid() const noexcept -> bool;

  std::vector<ShapeGroup> m_shapeGroups;
  [[nodiscard]] auto GetInitialShapeGroups() noexcept -> std::vector<ShapeGroup>;
  auto DoChanges() noexcept -> void;
  auto DoRandomChanges() noexcept -> void;
  auto SetShapeSpeeds() noexcept -> void;
  auto SetFixedShapeSpeeds() noexcept -> void;
  auto SetRandomShapeSpeeds() noexcept -> void;
  auto DrawShapeGroups() noexcept -> void;
  auto DrawShapes(const ShapeGroup& shapeGroup) noexcept -> void;
  auto DrawShape(const ShapeGroup& shapeGroup, size_t shapeNum) noexcept -> void;
  [[nodiscard]] static auto GetInnerColorCutoffRadius(int32_t maxRadius) noexcept -> int32_t;
  struct ShapeColors
  {
    Pixel mainColor;
    Pixel lowColor;
  };
  [[nodiscard]] static auto GetCurrentShapeColors(
      const ShapeGroup& shapeGroup, const ShapePath::ColorInfo& shapePathColorInfo) noexcept
      -> ShapeColors;
  [[nodiscard]] static auto GetColors(float brightness, const ShapeColors& shapeColors) noexcept
      -> std::vector<Pixel>;
  [[nodiscard]] static auto GetColorsWithInner(float brightness,
                                               const ShapeColors& shapeColors,
                                               const Pixel& innerColor,
                                               float innerColorMix) noexcept -> std::vector<Pixel>;
};

ShapesFx::ShapesFx(const FxHelper& fxHelper) noexcept
  : m_fxImpl{spimpl::make_unique_impl<ShapesFxImpl>(fxHelper)}
{
}

auto ShapesFx::GetFxName() const noexcept -> std::string
{
  return "shapes";
}

auto ShapesFx::SetWeightedMainColorMaps(
    const size_t shapeGroupNum, const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept
    -> void
{
  m_fxImpl->SetWeightedMainColorMaps(shapeGroupNum, weightedMaps);
}

auto ShapesFx::SetWeightedLowColorMaps(const size_t shapeGroupNum,
                                       const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept
    -> void
{
  m_fxImpl->SetWeightedLowColorMaps(shapeGroupNum, weightedMaps);
}

auto ShapesFx::SetWeightedInnerColorMaps(
    const size_t shapeGroupNum, const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept
    -> void
{
  m_fxImpl->SetWeightedInnerColorMaps(shapeGroupNum, weightedMaps);
}

auto ShapesFx::Start() noexcept -> void
{
  m_fxImpl->Start();
}

auto ShapesFx::Finish() noexcept -> void
{
  // nothing to do
}

auto ShapesFx::ApplyMultiple() noexcept -> void
{
  m_fxImpl->ApplyMultiple();
}

ShapesFx::ShapesFxImpl::ShapesFxImpl(const FxHelper& fxHelper) noexcept
  : m_draw{fxHelper.GetDraw()},
    m_goomInfo{fxHelper.GetGoomInfo()},
    m_goomRand{fxHelper.GetGoomRand()},
    m_screenMidpoint{U_HALF * m_goomInfo.GetScreenInfo().width,
                     U_HALF * m_goomInfo.GetScreenInfo().height},
    m_shapeGroups{GetInitialShapeGroups()}
{
  assert(AllColorMapsValid());
}

auto ShapesFx::ShapesFxImpl::GetInitialShapeGroups() noexcept -> std::vector<ShapeGroup>
{
  std::vector<ShapeGroup> shapeGroups{};

  for (size_t i = 0; i < NUM_SHAPE_GROUPS; ++i)
  {
    static constexpr float T_MIN_MAX_LERP = 0.5F;
    shapeGroups.emplace_back(m_goomRand, m_colorMapsManager, m_screenMidpoint, T_MIN_MAX_LERP);
  }

  return shapeGroups;
}

auto ShapesFx::ShapesFxImpl::AllColorMapsValid() const noexcept -> bool
{
  for (const auto& shapeGroup : m_shapeGroups)
  {
    for (size_t shapeNum = 0; shapeNum < shapeGroup.GetNumShapes(); ++shapeNum)
    {
      assert(shapeGroup.GetShapePath(shapeNum).GetColorInfo().mainColorMapId.IsSet());
      assert(shapeGroup.GetShapePath(shapeNum).GetColorInfo().lowColorMapId.IsSet());
      assert(shapeGroup.GetShapePath(shapeNum).GetColorInfo().innerColorMapId.IsSet());
    }
  }
  return true;
}

inline auto ShapesFx::ShapesFxImpl::SetWeightedMainColorMaps(
    const size_t shapeGroupNum, const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept
    -> void
{
  assert(AllColorMapsValid());
  m_shapeGroups.at(shapeGroupNum).SetWeightedMainColorMaps(weightedMaps);
  assert(AllColorMapsValid());
}

inline auto ShapesFx::ShapesFxImpl::SetWeightedLowColorMaps(
    const size_t shapeGroupNum, const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept
    -> void
{
  assert(AllColorMapsValid());
  m_shapeGroups.at(shapeGroupNum).SetWeightedLowColorMaps(weightedMaps);
  assert(AllColorMapsValid());
}

inline auto ShapesFx::ShapesFxImpl::SetWeightedInnerColorMaps(
    const size_t shapeGroupNum, const std::shared_ptr<RandomColorMaps> weightedMaps) noexcept
    -> void
{
  assert(AllColorMapsValid());
  m_shapeGroups.at(shapeGroupNum).SetWeightedInnerColorMaps(weightedMaps);
  assert(AllColorMapsValid());
}

inline auto ShapesFx::ShapesFxImpl::Start() noexcept -> void
{
  for (auto& shapeGroup : m_shapeGroups)
  {
    shapeGroup.Start();
  }

  assert(AllColorMapsValid());
}

inline auto ShapesFx::ShapesFxImpl::ApplyMultiple() noexcept -> void
{
  DrawShapeGroups();
  DoChanges();
}

inline auto ShapesFx::ShapesFxImpl::DrawShapeGroups() noexcept -> void
{
  assert(AllColorMapsValid());

  for (auto& shapeGroup : m_shapeGroups)
  {
    DrawShapes(shapeGroup);
    shapeGroup.IncrementTs();
  }

  assert(AllColorMapsValid());
}

inline auto ShapesFx::ShapesFxImpl::DrawShapes(const ShapeGroup& shapeGroup) noexcept -> void
{
  for (size_t shapeNum = 0; shapeNum < shapeGroup.GetNumShapes(); ++shapeNum)
  {
    DrawShape(shapeGroup, shapeNum);
  }
}

inline auto ShapesFx::ShapesFxImpl::DoChanges() noexcept -> void
{
  m_synchronisedShapeChangesTimer.Increment();
  if (m_synchronisedShapeChangesTimer.Finished())
  {
    SetShapeSpeeds();
    m_synchronisedShapeChangesTimer.ResetToZero();
  }

  DoRandomChanges();
}

inline auto ShapesFx::ShapesFxImpl::DoRandomChanges() noexcept -> void
{
  for (auto& shapeGroup : m_shapeGroups)
  {
    shapeGroup.DoRandomChanges();
  }
}

inline auto ShapesFx::ShapesFxImpl::SetShapeSpeeds() noexcept -> void
{
  if (constexpr float PROB_FIXED_SPEEDS = 0.3F; m_goomRand.ProbabilityOf(PROB_FIXED_SPEEDS))
  {
    SetFixedShapeSpeeds();
  }
  else
  {
    SetRandomShapeSpeeds();
  }
}

inline auto ShapesFx::ShapesFxImpl::SetFixedShapeSpeeds() noexcept -> void
{
  const float tMinMaxLerp = m_goomRand.GetRandInRange(0.0F, 1.0F);

  for (auto& shapeGroup : m_shapeGroups)
  {
    shapeGroup.UseFixedShapesSpeed(tMinMaxLerp);
  }
}

inline auto ShapesFx::ShapesFxImpl::SetRandomShapeSpeeds() noexcept -> void
{
  for (auto& shapeGroup : m_shapeGroups)
  {
    shapeGroup.UseRandomShapesSpeed();
  }
}

inline auto ShapesFx::ShapesFxImpl::DrawShape(const ShapeGroup& shapeGroup,
                                              const size_t shapeNum) noexcept -> void
{
  const ShapePath& shapePath = shapeGroup.GetShapePath(shapeNum);
  const ShapePath::ColorInfo& shapePathColorInfo = shapePath.GetColorInfo();

  const Point2dInt point = shapePath.GetNextPoint();
  const ShapeColors shapeColors = GetCurrentShapeColors(shapeGroup, shapePathColorInfo);
  const IColorMap& innerColorMap = shapeGroup.GetColorMap(shapePathColorInfo.innerColorMapId);

  static constexpr int32_t MAX_RADIUS_JITTER = 10;
  const int32_t maxRadius =
      shapeGroup.GetCurrentShapesRadius() + m_goomRand.GetRandInRange(0, MAX_RADIUS_JITTER + 1);
  TValue innerColorT{UTILS::TValue::StepType::SINGLE_CYCLE, static_cast<uint32_t>(maxRadius - 1)};
  const int32_t innerColorCutoffRadius = GetInnerColorCutoffRadius(maxRadius);

  static constexpr float MIN_BRIGHTNESS = 1.0F;
  static constexpr float MAX_BRIGHTNESS = 10.0F;
  TValue brightnessT{TValue::StepType::SINGLE_CYCLE, static_cast<uint32_t>(maxRadius)};

  for (int32_t radius = maxRadius; radius > 1; --radius)
  {
    const float brightness = STD20::lerp(MIN_BRIGHTNESS, MAX_BRIGHTNESS, brightnessT());
    const Pixel innerColor = innerColorMap.GetColor(innerColorT());
    const std::vector<Pixel> colors = radius <= innerColorCutoffRadius
                                          ? GetColors(brightness, shapeColors)
                                          : GetColorsWithInner(brightness, shapeColors, innerColor,
                                                               shapeGroup.GetInnerColorMix());

    m_draw.Circle(point, radius, colors);

    brightnessT.Increment();
    innerColorT.Increment();
  }
}

inline auto ShapesFx::ShapesFxImpl::GetInnerColorCutoffRadius(const int32_t maxRadius) noexcept
    -> int32_t
{
  static constexpr int32_t RADIUS_FRAC = 3;
  static constexpr int32_t MIN_CUTOFF = 10;
  return std::max(MIN_CUTOFF, maxRadius / RADIUS_FRAC);
}

inline auto ShapesFx::ShapesFxImpl::GetCurrentShapeColors(
    const ShapeGroup& shapeGroup, const ShapePath::ColorInfo& shapePathColorInfo) noexcept
    -> ShapeColors
{
  return {shapeGroup.GetCurrentColor(shapePathColorInfo.mainColorMapId),
          shapeGroup.GetCurrentColor(shapePathColorInfo.lowColorMapId)};
}

static constexpr float LOW_COLOR_BRIGHTNESS_FACTOR = 0.9F;

inline auto ShapesFx::ShapesFxImpl::GetColors(const float brightness,
                                              const ShapeColors& shapeColors) noexcept
    -> std::vector<Pixel>
{
  const Pixel mainColor = GetBrighterColor(brightness, shapeColors.mainColor);
  const Pixel lowColor =
      GetBrighterColor(LOW_COLOR_BRIGHTNESS_FACTOR * brightness, shapeColors.lowColor);
  return {mainColor, lowColor};
}

inline auto ShapesFx::ShapesFxImpl::GetColorsWithInner(const float brightness,
                                                       const ShapeColors& shapeColors,
                                                       const Pixel& innerColor,
                                                       const float innerColorMix) noexcept
    -> std::vector<Pixel>
{
  const Pixel mainColor = GetBrighterColor(
      brightness, IColorMap::GetColorMix(shapeColors.mainColor, innerColor, innerColorMix));
  const Pixel lowColor =
      GetBrighterColor(LOW_COLOR_BRIGHTNESS_FACTOR * brightness,
                       IColorMap::GetColorMix(shapeColors.lowColor, innerColor, innerColorMix));

  return {mainColor, lowColor};
}

} // namespace GOOM::VISUAL_FX