#pragma once

#include "../fx_helper.h"
#include "color/color_adjustment.h"
#include "color/color_maps.h"
#include "color/random_color_maps.h"
#include "draw/goom_draw.h"
#include "draw/shape_drawers/circle_drawer.h"
#include "goom/goom_graphic.h"
#include "goom/point2d.h"
#include "utils/math/paths.h"
#include "utils/t_values.h"

#include <cstdint>
#include <memory>
#include <set>

namespace GOOM::VISUAL_FX::SHAPES
{

class ShapePath
{
public:
  struct ColorInfo
  {
    COLOR::ColorMapSharedPtr mainColorMapPtr  = nullptr;
    COLOR::ColorMapSharedPtr lowColorMapPtr   = nullptr;
    COLOR::ColorMapSharedPtr innerColorMapPtr = nullptr;
  };
  ShapePath(FxHelper& fxHelper,
            const std::shared_ptr<UTILS::MATH::IPath>& path,
            const ColorInfo& colorInfo) noexcept;

  auto UpdateMainColorInfo(const COLOR::WeightedRandomColorMaps& mainColorMaps) noexcept -> void;
  auto UpdateLowColorInfo(const COLOR::WeightedRandomColorMaps& lowColorMaps) noexcept -> void;
  auto UpdateInnerColorInfo(const COLOR::WeightedRandomColorMaps& innerColorMaps) noexcept -> void;

  auto SetNumSteps(uint32_t val) noexcept -> void;
  auto IncrementT() noexcept -> void;
  auto ResetT(float val) noexcept -> void;
  [[nodiscard]] auto HasJustHitStartBoundary() const noexcept -> bool;
  [[nodiscard]] auto HasJustHitEndBoundary() const noexcept -> bool;
  [[nodiscard]] auto HasJustHitAnyBoundary() const noexcept -> bool;
  [[nodiscard]] auto GetNextPoint() const noexcept -> Point2dInt;
  [[nodiscard]] auto GetColorInfo() const noexcept -> const ColorInfo&;
  [[nodiscard]] auto GetCurrentT() const noexcept -> float;

  [[nodiscard]] auto GetIPath() const noexcept -> const UTILS::MATH::IPath&;
  [[nodiscard]] auto GetIPath() noexcept -> UTILS::MATH::IPath&;

  struct DrawParams
  {
    float brightnessAttenuation{};
    bool firstShapePathAtMeetingPoint{};
    int32_t maxRadius{};
    float innerColorMix{};
    DRAW::MultiplePixels meetingPointColors;
  };
  auto Draw(const DrawParams& drawParams) noexcept -> void;

private:
  FxHelper* m_fxHelper;
  DRAW::SHAPE_DRAWERS::CircleDrawer m_circleDrawer;
  std::shared_ptr<UTILS::MATH::IPath> m_path;

  ColorInfo m_colorInfo;
  [[nodiscard]] static auto GetColorMapTypes() noexcept
      -> const std::set<COLOR::RandomColorMaps::ColorMapTypes>&;
  static constexpr auto NUM_COLOR_STEPS = 200U;
  UTILS::TValue m_innerColorT{
      {UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE, NUM_COLOR_STEPS}
  };

  [[nodiscard]] static auto GetInnerColorCutoffRadius(int32_t maxRadius) noexcept -> int32_t;
  [[nodiscard]] auto GetCurrentShapeColors() const noexcept -> DRAW::MultiplePixels;
  [[nodiscard]] auto GetColors(const DrawParams& drawParams,
                               float brightness,
                               const DRAW::MultiplePixels& shapeColors) const noexcept
      -> DRAW::MultiplePixels;
  [[nodiscard]] auto GetColorsWithoutInner(float brightness,
                                           const DRAW::MultiplePixels& shapeColors) const noexcept
      -> DRAW::MultiplePixels;
  [[nodiscard]] auto GetColorsWithInner(float brightness,
                                        const DRAW::MultiplePixels& shapeColors,
                                        const Pixel& innerColor,
                                        float innerColorMix) const noexcept -> DRAW::MultiplePixels;
  [[nodiscard]] auto GetFinalMeetingPointColors(const DRAW::MultiplePixels& meetingPointColors,
                                                float brightness) const noexcept
      -> DRAW::MultiplePixels;

  static constexpr float GAMMA = 1.3F;
  COLOR::ColorAdjustment m_colorAdjust{
      {GAMMA, COLOR::ColorAdjustment::INCREASED_CHROMA_FACTOR}
  };
};

inline auto ShapePath::SetNumSteps(const uint32_t val) noexcept -> void
{
  m_path->SetNumSteps(val);
}

inline auto ShapePath::IncrementT() noexcept -> void
{
  m_path->IncrementT();
}

inline auto ShapePath::ResetT(const float val) noexcept -> void
{
  m_path->Reset(val);
}

inline auto ShapePath::HasJustHitStartBoundary() const noexcept -> bool
{
  return m_path->GetPositionT().HasJustHitStartBoundary();
}

inline auto ShapePath::HasJustHitEndBoundary() const noexcept -> bool
{
  return m_path->GetPositionT().HasJustHitEndBoundary();
}

inline auto ShapePath::HasJustHitAnyBoundary() const noexcept -> bool
{
  return HasJustHitStartBoundary() || HasJustHitEndBoundary();
}

inline auto ShapePath::GetNextPoint() const noexcept -> Point2dInt
{
  return m_path->GetNextPoint();
}

inline auto ShapePath::GetCurrentT() const noexcept -> float
{
  return m_path->GetPositionT()();
}

inline auto ShapePath::GetIPath() const noexcept -> const UTILS::MATH::IPath&
{
  return *m_path;
}

inline auto ShapePath::GetIPath() noexcept -> UTILS::MATH::IPath&
{
  return *m_path;
}

inline auto ShapePath::GetColorInfo() const noexcept -> const ColorInfo&
{
  return m_colorInfo;
}

inline auto ShapePath::GetColorMapTypes() noexcept
    -> const std::set<COLOR::RandomColorMaps::ColorMapTypes>&
{
  return COLOR::RandomColorMaps::GetAllColorMapsTypes();
}

inline auto ShapePath::UpdateMainColorInfo(
    const COLOR::WeightedRandomColorMaps& mainColorMaps) noexcept -> void
{
  m_colorInfo.mainColorMapPtr = mainColorMaps.GetRandomColorMapSharedPtr(GetColorMapTypes());
}

inline auto ShapePath::UpdateLowColorInfo(
    const COLOR::WeightedRandomColorMaps& lowColorMaps) noexcept -> void
{
  m_colorInfo.lowColorMapPtr = lowColorMaps.GetRandomColorMapSharedPtr(GetColorMapTypes());
}

inline auto ShapePath::UpdateInnerColorInfo(
    const COLOR::WeightedRandomColorMaps& innerColorMaps) noexcept -> void
{
  m_colorInfo.innerColorMapPtr = innerColorMaps.GetRandomColorMapSharedPtr(GetColorMapTypes());
}

} // namespace GOOM::VISUAL_FX::SHAPES
