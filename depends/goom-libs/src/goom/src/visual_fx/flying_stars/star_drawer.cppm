module;

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <utility>
#include <vector>

export module Goom.VisualFx.FlyingStarsFx:StarDrawer;

import Goom.Draw.GoomDrawBase;
import Goom.Draw.ShapeDrawers.BitmapDrawer;
import Goom.Draw.ShaperDrawers.CircleDrawer;
import Goom.Draw.ShaperDrawers.LineDrawer;
import Goom.Utils.Graphics.ImageBitmaps;
import Goom.Utils.Graphics.PixelUtils;
import Goom.Utils.Graphics.SmallImageBitmaps;
import Goom.Utils.Math.IncrementedValues;
import Goom.Utils.Math.Misc;
import Goom.Utils.Math.TValues;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.EnumUtils;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;
import :StarColors;
import :Stars;

using GOOM::DRAW::IGoomDraw;
using GOOM::DRAW::MultiplePixels;
using GOOM::DRAW::SHAPE_DRAWERS::BitmapDrawer;
using GOOM::DRAW::SHAPE_DRAWERS::CircleDrawer;
using GOOM::DRAW::SHAPE_DRAWERS::LineDrawerClippedEndPoints;
using GOOM::UTILS::GRAPHICS::GetColorMultiply;
using GOOM::UTILS::GRAPHICS::ImageBitmap;
using GOOM::UTILS::GRAPHICS::SmallImageBitmaps;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::IncrementedValue;
using GOOM::UTILS::MATH::NumberRange;
using GOOM::UTILS::MATH::TValue;

namespace GOOM::VISUAL_FX::FLYING_STARS
{

class StarDrawer
{
public:
  StarDrawer(IGoomDraw& draw,
             const GoomRand& goomRand,
             const SmallImageBitmaps& smallBitmaps) noexcept;

  auto ChangeDrawMode() noexcept -> void;

  auto DrawStar(const Star& star, float speedFactor) noexcept -> void;

private:
  const GoomRand* m_goomRand;
  const SmallImageBitmaps* m_smallBitmaps;
  BitmapDrawer m_bitmapDrawer;
  CircleDrawer m_circleDrawer;
  LineDrawerClippedEndPoints m_lineDrawer;

  enum class DrawElementTypes : UnderlyingEnumType
  {
    CIRCLES,
    LINES,
    DOTS,
    CIRCLES_AND_LINES,
  };
  static constexpr float DRAW_ELEMENT_TYPES_DOTS_WGT              = 30.0F;
  static constexpr float DRAW_ELEMENT_TYPES_CIRCLES_WGT           = 20.0F;
  static constexpr float DRAW_ELEMENT_TYPES_LINES_WGT             = 10.0F;
  static constexpr float DRAW_ELEMENT_TYPES_CIRCLES_AND_LINES_WGT = 15.0F;
  // clang-format off
  UTILS::MATH::Weights<DrawElementTypes> m_drawElementWeights{
      *m_goomRand,
     {
       {.key=DrawElementTypes::DOTS, .weight=DRAW_ELEMENT_TYPES_DOTS_WGT},
       {.key=DrawElementTypes::CIRCLES, .weight=DRAW_ELEMENT_TYPES_CIRCLES_WGT},
       {.key=DrawElementTypes::LINES, .weight=DRAW_ELEMENT_TYPES_LINES_WGT},
       {.key=DrawElementTypes::CIRCLES_AND_LINES, .weight=DRAW_ELEMENT_TYPES_CIRCLES_AND_LINES_WGT},
     }
  };
  // clang-format on
  DrawElementTypes m_requestedDrawElement = m_drawElementWeights.GetRandomWeighted();
  DrawElementTypes m_currentActualDrawElement{};
  auto UpdateActualDrawElement() noexcept -> void;
  enum class DrawModes : UnderlyingEnumType
  {
    CLEAN,
    SUPER_CLEAN,
    MESSY
  };
  static constexpr float DRAW_MODES_CLEAN_WEIGHT       = 20.0F;
  static constexpr float DRAW_MODES_SUPER_CLEAN_WEIGHT = 10.0F;
  static constexpr float DRAW_MODES_MESSY              = 30.0F;
  UTILS::MATH::Weights<DrawModes> m_drawModeWeights{
      *m_goomRand,
      {
                  {DrawModes::CLEAN, DRAW_MODES_CLEAN_WEIGHT},
                  {DrawModes::SUPER_CLEAN, DRAW_MODES_SUPER_CLEAN_WEIGHT},
                  {DrawModes::MESSY, DRAW_MODES_MESSY},
                  }
  };
  DrawModes m_drawMode                             = m_drawModeWeights.GetRandomWeighted();
  static constexpr uint32_t MIN_NUM_PARTS          = 2;
  static constexpr uint32_t MAX_NUM_PARTS          = 10;
  static constexpr uint32_t MAX_NUM_PARTS_FOR_LINE = 2;
  uint32_t m_currentMaxNumParts                    = MAX_NUM_PARTS;
  auto ChangeMaxNumParts() noexcept -> void;

  static constexpr auto DOT_SIZE_RANGE = NumberRange{3U, 5U};
  static_assert(DOT_SIZE_RANGE.max <= SmallImageBitmaps::MAX_IMAGE_SIZE);
  [[nodiscard]] auto GetImageBitmap(uint32_t size) const noexcept -> const ImageBitmap&;

  using DrawFunc = std::function<void(
      Point2dInt point1, Point2dInt point2, uint32_t elementSize, const MultiplePixels& colors)>;
  UTILS::EnumMap<DrawElementTypes, DrawFunc> m_drawFuncs;
  auto DrawStar(const Star& star,
                float speedFactor,
                const DrawFunc& drawFunc) const noexcept -> void;
  [[nodiscard]] auto GetNumPartsAndElementSize(float tAge) const noexcept
      -> std::pair<uint32_t, uint32_t>;
  [[nodiscard]] auto GetPartMultiplier() const noexcept -> float;
  [[nodiscard]] auto GetMaxPartMultiplier() const noexcept -> float;
  [[nodiscard]] auto GetLineMaxPartMultiplier() const noexcept -> float;
  [[nodiscard]] static auto GetBrightness(float tAge) noexcept -> float;
  [[nodiscard]] static auto GetPointVelocity(const Vec2dFlt& twistFrequency,
                                             const Vec2dFlt& velocity) noexcept -> Vec2dInt;
  auto DrawParticleCircle(const Point2dInt& point1,
                          const Point2dInt& point2,
                          uint32_t elementSize,
                          const MultiplePixels& colors) noexcept -> void;
  auto DrawParticleLine(const Point2dInt& point1,
                        const Point2dInt& point2,
                        uint32_t elementSize,
                        const MultiplePixels& colors) noexcept -> void;
  auto DrawParticleDot(const Point2dInt& point1,
                       const Point2dInt& point2,
                       uint32_t elementSize,
                       const MultiplePixels& colors) noexcept -> void;
};

} //namespace GOOM::VISUAL_FX::FLYING_STARS

namespace GOOM::VISUAL_FX::FLYING_STARS
{

inline auto StarDrawer::ChangeDrawMode() noexcept -> void
{
  m_drawMode             = m_drawModeWeights.GetRandomWeighted();
  m_requestedDrawElement = m_drawElementWeights.GetRandomWeighted();
}

inline auto StarDrawer::UpdateActualDrawElement() noexcept -> void
{
  if (m_requestedDrawElement != DrawElementTypes::CIRCLES_AND_LINES)
  {
    m_currentActualDrawElement = m_requestedDrawElement;
  }
  else
  {
    static constexpr auto PROB_CIRCLES = 0.5F;
    m_currentActualDrawElement         = m_goomRand->ProbabilityOf<PROB_CIRCLES>()
                                             ? DrawElementTypes::CIRCLES
                                             : DrawElementTypes::LINES;
  }
}

inline auto StarDrawer::ChangeMaxNumParts() noexcept -> void
{
  m_currentMaxNumParts = m_currentActualDrawElement == DrawElementTypes::LINES
                             ? MAX_NUM_PARTS_FOR_LINE
                             : MAX_NUM_PARTS;
}

inline auto StarDrawer::DrawStar(const Star& star, const float speedFactor) noexcept -> void
{
  UpdateActualDrawElement();
  ChangeMaxNumParts();
  DrawStar(star, speedFactor, m_drawFuncs[m_currentActualDrawElement]);
}

} //namespace GOOM::VISUAL_FX::FLYING_STARS

namespace GOOM::VISUAL_FX::FLYING_STARS
{

StarDrawer::StarDrawer(IGoomDraw& draw,
                       const GoomRand& goomRand,
                       const SmallImageBitmaps& smallBitmaps) noexcept
  : m_goomRand{&goomRand},
    m_smallBitmaps{&smallBitmaps},
    m_bitmapDrawer{draw},
    m_circleDrawer{draw},
    m_lineDrawer{draw},
    m_drawFuncs{{{
        {DrawElementTypes::CIRCLES,
         [this](const Point2dInt& point1,
                const Point2dInt& point2,
                const uint32_t size,
                const MultiplePixels& colors)
         { DrawParticleCircle(point1, point2, size, colors); }},
        {DrawElementTypes::LINES,
         [this](const Point2dInt& point1,
                const Point2dInt& point2,
                const uint32_t size,
                const MultiplePixels& colors) { DrawParticleLine(point1, point2, size, colors); }},
        {DrawElementTypes::DOTS,
         [this](const Point2dInt& point1,
                const Point2dInt& point2,
                const uint32_t size,
                const MultiplePixels& colors) { DrawParticleDot(point1, point2, size, colors); }},
        {DrawElementTypes::CIRCLES_AND_LINES,
         [](const Point2dInt, const Point2dInt, const uint32_t, const DRAW::MultiplePixels&)
         { FailFast(); }},
    }}}
{
}

auto StarDrawer::DrawStar(const Star& star,
                          const float speedFactor,
                          const DrawFunc& drawFunc) const noexcept -> void
{
  const auto tAge                   = star.GetTAge();
  static constexpr auto EXTRA_T_AGE = 0.25F;
  const auto tAgeMax                = std::min(tAge + EXTRA_T_AGE, 1.0F);

  const auto brightness              = GetBrightness(tAge);
  const auto partMultiplier          = GetPartMultiplier();
  const auto [numParts, elementSize] = GetNumPartsAndElementSize(tAge);

  auto tAgeMix = IncrementedValue<float>{tAge, tAgeMax, TValue::StepType::SINGLE_CYCLE, numParts};
  const auto point0 = ToPoint2dInt(star.GetStartPos());

  auto point1 = point0;
  for (auto j = 1U; j <= numParts; ++j)
  {
    const auto thisPartFraction = static_cast<float>(j) / static_cast<float>(numParts);
    const auto thisPartVelocity = partMultiplier * (thisPartFraction * star.GetVelocity());
    const auto twistFrequency   = speedFactor * thisPartVelocity;

    const auto point2 = point0 - GetPointVelocity(twistFrequency, thisPartVelocity);

    const auto thisPartBrightness = thisPartFraction * brightness;
    const auto mixedColorParams =
        StarColors::MixedColorsParams{.brightness = thisPartBrightness, .lengthT = tAgeMix()};
    const auto thisPartColors = star.GetStarColors().GetMixedColors(mixedColorParams);

    drawFunc(point1, point2, elementSize, thisPartColors);

    point1 = point2;
    tAgeMix.Increment();
  }
}

inline auto StarDrawer::GetPointVelocity(const Vec2dFlt& twistFrequency,
                                         const Vec2dFlt& velocity) noexcept -> Vec2dInt
{
  static constexpr auto HALF = 0.5F;
  return {.x = static_cast<int32_t>(HALF * (1.0F + std::sin(twistFrequency.x)) * velocity.x),
          .y = static_cast<int32_t>(HALF * (1.0F + std::cos(twistFrequency.y)) * velocity.y)};
}

inline auto StarDrawer::GetBrightness(const float tAge) noexcept -> float
{
  static constexpr auto BRIGHTNESS_FACTOR = 15.0F;
  static constexpr auto BRIGHTNESS_MIN    = 0.4F;
  const auto ageBrightness                = (0.8F * std::fabs(0.10F - tAge)) / 0.25F;

  return BRIGHTNESS_FACTOR * (BRIGHTNESS_MIN + ageBrightness);
}

inline auto StarDrawer::GetPartMultiplier() const noexcept -> float
{
  if (m_currentActualDrawElement == DrawElementTypes::LINES)
  {
    return m_goomRand->GetRandInRange(NumberRange{1.0F, GetLineMaxPartMultiplier()});
  }

  return m_goomRand->GetRandInRange(NumberRange{1.0F, GetMaxPartMultiplier()});
}

inline auto StarDrawer::GetMaxPartMultiplier() const noexcept -> float
{
  static constexpr auto MAX_MULTIPLIER = 20.0F;

  switch (m_drawMode)
  {
    case DrawModes::CLEAN:
    case DrawModes::SUPER_CLEAN:
      return 1.0F + UTILS::MATH::SMALL_FLOAT;
    case DrawModes::MESSY:
      return MAX_MULTIPLIER;
  }
}

inline auto StarDrawer::GetLineMaxPartMultiplier() const noexcept -> float
{
  static constexpr auto LINE_MAX_MULTIPLIER = 4.0F;

  switch (m_drawMode)
  {
    case DrawModes::SUPER_CLEAN:
      return 1.0F + UTILS::MATH::SMALL_FLOAT;
    case DrawModes::CLEAN:
    case DrawModes::MESSY:
      return LINE_MAX_MULTIPLIER;
  }
}

inline auto StarDrawer::GetNumPartsAndElementSize(const float tAge) const noexcept
    -> std::pair<uint32_t, uint32_t>
{
  if (static constexpr auto T_OLD_AGE = 0.95F; tAge > T_OLD_AGE)
  {
    return {m_currentMaxNumParts, m_goomRand->GetRandInRange<DOT_SIZE_RANGE>()};
  }

  static constexpr auto MIN_ELEMENT_SIZE = 1U;
  const auto numParts =
      MIN_NUM_PARTS +
      static_cast<uint32_t>(
          std::lround((1.0F - tAge) * static_cast<float>(m_currentMaxNumParts - MIN_NUM_PARTS)));

  return {numParts, MIN_ELEMENT_SIZE};
}

inline auto StarDrawer::DrawParticleCircle(const Point2dInt& point1,
                                           [[maybe_unused]] const Point2dInt& point2,
                                           const uint32_t elementSize,
                                           const MultiplePixels& colors) noexcept -> void
{
  m_circleDrawer.DrawCircle(point1, static_cast<int>(elementSize), colors);
}

inline auto StarDrawer::DrawParticleLine(const Point2dInt& point1,
                                         const Point2dInt& point2,
                                         const uint32_t elementSize,
                                         const MultiplePixels& colors) noexcept -> void
{
  m_lineDrawer.SetLineThickness(static_cast<uint8_t>(elementSize));
  m_lineDrawer.DrawLine(point1, point2, colors);
}

inline auto StarDrawer::DrawParticleDot(const Point2dInt& point1,
                                        [[maybe_unused]] const Point2dInt& point2,
                                        const uint32_t elementSize,
                                        const MultiplePixels& colors) noexcept -> void
{
  const auto getMainColor =
      [&colors]([[maybe_unused]] const Point2dInt& bitmapPoint, const Pixel& bgnd)
  { return GetColorMultiply(bgnd, colors.color1, colors.color1.A()); };
  const auto getLowColor =
      [&colors]([[maybe_unused]] const Point2dInt& bitmapPoint, const Pixel& bgnd)
  { return GetColorMultiply(bgnd, colors.color2, colors.color2.A()); };

  const auto& bitmap   = GetImageBitmap(elementSize);
  const auto getColors = std::vector<BitmapDrawer::GetBitmapColorFunc>{getMainColor, getLowColor};
  m_bitmapDrawer.Bitmap(point1, bitmap, getColors);
}

inline auto StarDrawer::GetImageBitmap(const uint32_t size) const noexcept -> const ImageBitmap&
{
  return m_smallBitmaps->GetImageBitmap(SmallImageBitmaps::ImageNames::CIRCLE,
                                        std::clamp(size, DOT_SIZE_RANGE.min, DOT_SIZE_RANGE.max));
}

} //namespace GOOM::VISUAL_FX::FLYING_STARS
