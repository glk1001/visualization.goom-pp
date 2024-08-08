module;

//#undef NO_LOGGING

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

module Goom.VisualFx.GoomDotsFx;

import Goom.Color.ColorAdjustment;
import Goom.Color.ColorMaps;
import Goom.Color.ColorUtils;
import Goom.Color.RandomColorMaps;
import Goom.Draw.ShapeDrawers.BitmapDrawer;
import Goom.Utils.Graphics.ImageBitmaps;
import Goom.Utils.Graphics.SmallImageBitmaps;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.IncrementedValues;
import Goom.Utils.Math.Misc;
import Goom.Utils.Math.ParametricFunctions2d;
import Goom.Utils.Math.Paths;
import Goom.Utils.Math.TValues;
import Goom.VisualFx.FxHelper;
import Goom.VisualFx.FxUtils;
import Goom.Lib.GoomGraphic;
import Goom.Lib.Point2d;
import Goom.Lib.SPimpl;

namespace GOOM::VISUAL_FX
{

using COLOR::ColorAdjustment;
using COLOR::ColorMapGroup;
using COLOR::GetSimpleColor;
using COLOR::RandomColorMaps;
using COLOR::SimpleColors;
using COLOR::WeightedRandomColorMaps;
using DRAW::SHAPE_DRAWERS::BitmapDrawer;
using FX_UTILS::RandomPixelBlender;
using UTILS::GRAPHICS::ImageBitmap;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::AngleParams;
using UTILS::MATH::EpicycloidFunction;
using UTILS::MATH::EpicycloidPath;
using UTILS::MATH::HypotrochoidFunction;
using UTILS::MATH::HypotrochoidPath;
using UTILS::MATH::IncrementedValue;
using UTILS::MATH::IPath;
using UTILS::MATH::LissajousFunction;
using UTILS::MATH::LissajousPath;
using UTILS::MATH::NumberRange;
using UTILS::MATH::TValue;
using UTILS::MATH::U_HALF;
using UTILS::MATH::Weights;

class GoomDotsFx::GoomDotsFxImpl
{
public:
  GoomDotsFxImpl(FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept;

  auto Start() -> void;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void;

  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;
  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>;

  auto ApplyToImageBuffers() -> void;

private:
  FxHelper* m_fxHelper;
  const SmallImageBitmaps* m_smallBitmaps;
  BitmapDrawer m_bitmapDrawer;
  Point2dInt m_screenCentre = m_fxHelper->GetDimensions().GetCentrePoint();

  SmallImageBitmaps::ImageNames m_currentBitmapName{};
  static constexpr uint32_t MAX_FLOWERS_IN_ROW = 100;
  uint32_t m_numFlowersInRow                   = 0;
  Weights<SmallImageBitmaps::ImageNames> m_flowerDotTypes;
  [[nodiscard]] auto GetImageBitmap(uint32_t size) const -> const ImageBitmap&;
  auto SetFlowerBitmap() -> void;
  auto SetNonFlowerBitmap() -> void;
  auto SetNextCurrentBitmapName() -> void;

  static constexpr auto MIN_DOT_SIZE = 5U;
  static constexpr auto MAX_DOT_SIZE = 17U;
  static_assert(MAX_DOT_SIZE <= SmallImageBitmaps::MAX_IMAGE_SIZE, "Max dot size mismatch.");

  std::array<WeightedRandomColorMaps, NUM_DOT_TYPES> m_dotColorMapsList{};
  std::array<COLOR::ConstColorMapSharedPtr, NUM_DOT_TYPES> m_dotColorMaps{};
  PixelChannelType m_defaultAlpha = DEFAULT_VISUAL_FX_ALPHA;
  RandomColorMaps m_randomColorMaps{m_defaultAlpha, m_fxHelper->GetGoomRand()};
  std::array<bool, NUM_DOT_TYPES> m_usePrimaryColors{};
  Pixel m_middleColor;
  bool m_useMiddleColor = true;
  [[nodiscard]] auto GetDotColor(size_t dotNum, float t) const -> Pixel;
  [[nodiscard]] auto GetDotPrimaryColor(size_t dotNum) const noexcept -> Pixel;
  [[nodiscard]] auto GetMixedColor(float brightness,
                                   const Point2dInt& bitmapPoint,
                                   const Pixel& color,
                                   uint32_t radius,
                                   const Pixel& bgnd) const -> Pixel;
  [[nodiscard]] static auto IsImagePointCloseToMiddle(const Point2dInt& point,
                                                      uint32_t radius) -> bool;
  [[nodiscard]] static auto GetMargin(uint32_t radius) -> size_t;
  [[nodiscard]] auto GetMiddleColor() const -> Pixel;

  RandomPixelBlender m_pixelBlender;
  auto UpdatePixelBlender() noexcept -> void;

  std::array<std::unique_ptr<IPath>, NUM_DOT_TYPES> m_dotPaths{GetDotPaths(m_screenCentre)};
  [[nodiscard]] static auto GetDotPaths(const Point2dInt& centre)
      -> std::array<std::unique_ptr<IPath>, NUM_DOT_TYPES>;

  auto ChangeColors() -> void;

  auto DotFilter(const Pixel& color, const Point2dInt& dotPosition, uint32_t radius) -> void;

  static constexpr auto GAMMA = 1.9F;
  ColorAdjustment m_colorAdjust{
      {.gamma = GAMMA, .alterChromaFactor = ColorAdjustment::INCREASED_CHROMA_FACTOR}
  };
};

GoomDotsFx::GoomDotsFx(FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept
  : m_pimpl{spimpl::make_unique_impl<GoomDotsFxImpl>(fxHelper, smallBitmaps)}
{
}

auto GoomDotsFx::GetFxName() const noexcept -> std::string
{
  return "goom dots";
}

auto GoomDotsFx::Start() noexcept -> void
{
  m_pimpl->Start();
}

auto GoomDotsFx::Finish() noexcept -> void
{
  // nothing to do
}

auto GoomDotsFx::Resume() noexcept -> void
{
  // nothing to do
}

auto GoomDotsFx::Suspend() noexcept -> void
{
  // nothing to do
}

auto GoomDotsFx::ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pimpl->ChangePixelBlender(pixelBlenderParams);
}

auto GoomDotsFx::SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  m_pimpl->SetWeightedColorMaps(weightedColorMaps);
}

auto GoomDotsFx::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  return m_pimpl->GetCurrentColorMapsNames();
}

auto GoomDotsFx::ApplyToImageBuffers() noexcept -> void
{
  m_pimpl->ApplyToImageBuffers();
}

static constexpr auto IMAGE_NAMES_ORANGE_FLOWER_WEIGHT = 10.0F;
static constexpr auto IMAGE_NAMES_PINK_FLOWER_WEIGHT   = 05.0F;
static constexpr auto IMAGE_NAMES_RED_FLOWER_WEIGHT    = 10.0F;
static constexpr auto IMAGE_NAMES_WHITE_FLOWER_WEIGHT  = 05.0F;

using enum SmallImageBitmaps::ImageNames;

GoomDotsFx::GoomDotsFxImpl::GoomDotsFxImpl(FxHelper& fxHelper,
                                           const SmallImageBitmaps& smallBitmaps) noexcept
  : m_fxHelper{&fxHelper},
    m_smallBitmaps{&smallBitmaps},
    m_bitmapDrawer{fxHelper.GetDraw()},
    m_flowerDotTypes{
        m_fxHelper->GetGoomRand(),
        {
            {.key=ORANGE_FLOWER, .weight=IMAGE_NAMES_ORANGE_FLOWER_WEIGHT},
            {.key=PINK_FLOWER,   .weight=IMAGE_NAMES_PINK_FLOWER_WEIGHT},
            {.key=RED_FLOWER,    .weight=IMAGE_NAMES_RED_FLOWER_WEIGHT},
            {.key=WHITE_FLOWER,  .weight=IMAGE_NAMES_WHITE_FLOWER_WEIGHT},
        }
    },
    m_pixelBlender{fxHelper.GetGoomRand()}
{
}

auto GoomDotsFx::GoomDotsFxImpl::GetDotPaths(const Point2dInt& centre)
    -> std::array<std::unique_ptr<IPath>, NUM_DOT_TYPES>
{
  static constexpr auto HYPOTROCHOID_PARAMS1 = HypotrochoidFunction::Params{
      .bigR = 10.0F, .smallR = 3.0F, .height = 5.0F, .amplitude = 30.0F};
  static constexpr auto HYPOTROCHOID_PARAMS2 = HypotrochoidFunction::Params{
      .bigR = 12.0F, .smallR = 3.0F, .height = 5.0F, .amplitude = 30.0F};
  static constexpr auto HYPOTROCHOID_PARAMS3 = HypotrochoidFunction::Params{
      .bigR = 13.0F, .smallR = 3.0F, .height = 5.0F, .amplitude = 30.0F};
  static constexpr auto LISSAJOUS_PATH_PARAMS =
      LissajousFunction::Params{.a = 60.0F, .b = 50.F, .kX = 3.0F, .kY = 2.0F};
  static constexpr auto EPICYCLOID_PARAMS =
      EpicycloidFunction::Params{.k = 5.1F, .smallR = 1.0F, .amplitude = 40.0F};

  static constexpr auto STEP_TYPE              = TValue::StepType::CONTINUOUS_REVERSIBLE;
  static constexpr auto HYPOTROCHOID_STEP_SIZE = 0.01F;
  static constexpr auto LISSAJOUS_STEP_SIZE    = 0.01F;
  static constexpr auto EPICYCLOID_STEP_SIZE   = 0.001F;

  auto hypotrochoidPositionT1 = std::make_unique<TValue>(
      TValue::StepSizeProperties{.stepSize = HYPOTROCHOID_STEP_SIZE, .stepType = STEP_TYPE});
  auto hypotrochoidPositionT2 = std::make_unique<TValue>(
      TValue::StepSizeProperties{.stepSize = HYPOTROCHOID_STEP_SIZE, .stepType = STEP_TYPE});
  auto hypotrochoidPositionT3 = std::make_unique<TValue>(
      TValue::StepSizeProperties{.stepSize = HYPOTROCHOID_STEP_SIZE, .stepType = STEP_TYPE});
  auto lissajousPositionT = std::make_unique<TValue>(
      TValue::StepSizeProperties{.stepSize = LISSAJOUS_STEP_SIZE, .stepType = STEP_TYPE});
  auto epicycloidPositionT = std::make_unique<TValue>(
      TValue::StepSizeProperties{.stepSize = EPICYCLOID_STEP_SIZE, .stepType = STEP_TYPE});

  const auto centrePos                       = ToVec2dFlt(centre);
  static constexpr auto DEFAULT_ANGLE_PARAMS = AngleParams{};

  return {
      {std::make_unique<HypotrochoidPath>(std::move(hypotrochoidPositionT1),
       centrePos, DEFAULT_ANGLE_PARAMS,
       HYPOTROCHOID_PARAMS1),
       std::make_unique<HypotrochoidPath>(std::move(hypotrochoidPositionT2),
       centrePos, DEFAULT_ANGLE_PARAMS,
       HYPOTROCHOID_PARAMS2),
       std::make_unique<HypotrochoidPath>(std::move(hypotrochoidPositionT3),
       centrePos, DEFAULT_ANGLE_PARAMS,
       HYPOTROCHOID_PARAMS3),
       std::make_unique<LissajousPath>(
           std::move(lissajousPositionT), centrePos, DEFAULT_ANGLE_PARAMS, LISSAJOUS_PATH_PARAMS),
       std::make_unique<EpicycloidPath>(
           std::move(epicycloidPositionT), centrePos, DEFAULT_ANGLE_PARAMS, EPICYCLOID_PARAMS)}
  };
}

inline auto GoomDotsFx::GoomDotsFxImpl::Start() -> void
{
  ChangeColors();
}

inline auto GoomDotsFx::GoomDotsFxImpl::GetImageBitmap(const uint32_t size) const
    -> const ImageBitmap&
{
  return m_smallBitmaps->GetImageBitmap(m_currentBitmapName,
                                        std::clamp(size, MIN_DOT_SIZE, MAX_DOT_SIZE));
}

inline auto GoomDotsFx::GoomDotsFxImpl::ChangeColors() -> void
{
  for (auto dotNum = 0U; dotNum < NUM_DOT_TYPES; ++dotNum)
  {
    m_dotColorMaps.at(dotNum) = m_dotColorMapsList.at(dotNum).GetRandomColorMapSharedPtr(
        WeightedRandomColorMaps::GetAllColorMapsTypes());
  }

  for (auto& usePrimaryColor : m_usePrimaryColors)
  {
    static constexpr auto PROB_USE_PRIMARY_COLOR = 0.3F;
    usePrimaryColor = m_fxHelper->GetGoomRand().ProbabilityOf<PROB_USE_PRIMARY_COLOR>();
  }

  static constexpr auto PROB_USE_MIDDLE_COLOR = 0.05F;
  m_useMiddleColor = m_fxHelper->GetGoomRand().ProbabilityOf<PROB_USE_MIDDLE_COLOR>();
  if (m_useMiddleColor)
  {
    m_middleColor = GetMiddleColor();
  }
}

auto GoomDotsFx::GoomDotsFxImpl::GetMiddleColor() const -> Pixel
{
  if (static constexpr auto PROB_PRIMARY_COLOR = 0.1F;
      m_fxHelper->GetGoomRand().ProbabilityOf<PROB_PRIMARY_COLOR>())
  {
    return GetDotPrimaryColor(
        m_fxHelper->GetGoomRand().GetRandInRange<NumberRange{0U, NUM_DOT_TYPES - 1}>());
  }

  static constexpr auto MIN_MIX_T = 0.1F;
  static constexpr auto MAX_MIX_T = 1.0F;
  return m_randomColorMaps.GetRandomColor(
      *m_dotColorMapsList[0].GetRandomColorMapSharedPtr(ColorMapGroup::MISC,
                                                        RandomColorMaps::GetAllColorMapsTypes()),
      MIN_MIX_T,
      MAX_MIX_T);
}

inline auto GoomDotsFx::GoomDotsFxImpl::ChangePixelBlender(
    const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pixelBlender.SetPixelBlendType(pixelBlenderParams);
}

inline auto GoomDotsFx::GoomDotsFxImpl::GetCurrentColorMapsNames() const noexcept
    -> std::vector<std::string>
{
  return {m_dotColorMapsList.at(0).GetColorMapsName()};
}

auto GoomDotsFx::GoomDotsFxImpl::SetWeightedColorMaps(
    const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  const auto dotNum = weightedColorMaps.id;

  m_dotColorMapsList.at(dotNum) =
      WeightedRandomColorMaps{weightedColorMaps.mainColorMaps, m_defaultAlpha};
}

inline auto GoomDotsFx::GoomDotsFxImpl::ApplyToImageBuffers() -> void
{
  UpdatePixelBlender();

  auto radius = MIN_DOT_SIZE / 2U;
  if (static constexpr auto PROB_CHANGE_DOT_COLORS = 0.25F;
      m_fxHelper->GetGoomRand().ProbabilityOf<PROB_CHANGE_DOT_COLORS>() or
      (0 == m_fxHelper->GetSoundEvents().GetTimeSinceLastGoom()))
  {
    ChangeColors();
    radius = m_fxHelper->GetGoomRand().GetRandInRange(NumberRange{radius, U_HALF * MAX_DOT_SIZE});
    SetNextCurrentBitmapName();
  }

  const auto speedFactor          = 0.35F * m_fxHelper->GetSoundEvents().GetSoundInfo().GetSpeed();
  const auto speedVarMult80Plus15 = static_cast<uint32_t>((speedFactor * 80.0F) + 15.0F);

  const auto speedVarMult80Plus15Div15 = speedVarMult80Plus15 / 15;
  static constexpr auto COLOR_T_MIN    = 0.1F;
  static constexpr auto COLOR_T_MAX    = 1.0F;
  auto colorT                          = IncrementedValue<float>{
      COLOR_T_MIN, COLOR_T_MAX, TValue::StepType::SINGLE_CYCLE, speedVarMult80Plus15Div15};

  for (auto i = 1U; i <= speedVarMult80Plus15Div15; ++i)
  {
    for (auto dotNum = 0U; dotNum < NUM_DOT_TYPES; ++dotNum)
    {
      const auto dotColor    = GetDotColor(dotNum, colorT());
      const auto dotPosition = m_dotPaths.at(dotNum)->GetNextPoint();
      m_dotPaths.at(dotNum)->IncrementT();
      DotFilter(dotColor, dotPosition, radius);
    }

    colorT.Increment();
  }
}

inline auto GoomDotsFx::GoomDotsFxImpl::UpdatePixelBlender() noexcept -> void
{
  m_fxHelper->GetDraw().SetPixelBlendFunc(m_pixelBlender.GetCurrentPixelBlendFunc());
  m_pixelBlender.Update();
}

inline auto GoomDotsFx::GoomDotsFxImpl::GetDotColor(const size_t dotNum,
                                                    const float t) const -> Pixel
{
  if (m_usePrimaryColors.at(dotNum))
  {
    return GetDotPrimaryColor(dotNum);
  }

  return m_dotColorMaps.at(dotNum)->GetColor(t);
}

inline auto GoomDotsFx::GoomDotsFxImpl::GetDotPrimaryColor(const size_t dotNum) const noexcept
    -> Pixel
{
  static constexpr auto S_PRIMARY_COLORS = std::array{
      SimpleColors::PURE_RED,
      SimpleColors::PURE_LIME,
      SimpleColors::PURE_BLUE,
      SimpleColors::PURE_YELLOW,
      SimpleColors::PURE_AQUA,
  };
  return GetSimpleColor(S_PRIMARY_COLORS.at(dotNum), m_defaultAlpha);
}

inline auto GoomDotsFx::GoomDotsFxImpl::SetNextCurrentBitmapName() -> void
{
  static constexpr auto PROB_MORE_FLOWERS = 1.0F / 50.0F;

  if (m_numFlowersInRow > 0)
  {
    SetFlowerBitmap();
  }
  else if (m_fxHelper->GetGoomRand().ProbabilityOf<PROB_MORE_FLOWERS>())
  {
    m_numFlowersInRow = 1;
    SetFlowerBitmap();
  }
  else
  {
    SetNonFlowerBitmap();
  }
}

inline auto GoomDotsFx::GoomDotsFxImpl::SetFlowerBitmap() -> void
{
  ++m_numFlowersInRow;
  if (m_numFlowersInRow > MAX_FLOWERS_IN_ROW)
  {
    m_numFlowersInRow = 0;
  }

  m_currentBitmapName = m_flowerDotTypes.GetRandomWeighted();
}

inline auto GoomDotsFx::GoomDotsFxImpl::SetNonFlowerBitmap() -> void
{
  static constexpr auto PROB_SPHERE = 0.7F;

  if (m_fxHelper->GetGoomRand().ProbabilityOf<PROB_SPHERE>())
  {
    m_currentBitmapName = SmallImageBitmaps::ImageNames::SPHERE;
  }
  else
  {
    m_currentBitmapName = SmallImageBitmaps::ImageNames::CIRCLE;
  }
}

auto GoomDotsFx::GoomDotsFxImpl::DotFilter(const Pixel& color,
                                           const Point2dInt& dotPosition,
                                           const uint32_t radius) -> void
{
  const auto diameter                 = static_cast<int32_t>((2 * radius) + 1); // must be odd
  const auto screenWidthLessDiameter  = m_fxHelper->GetDimensions().GetIntWidth() - diameter;
  const auto screenHeightLessDiameter = m_fxHelper->GetDimensions().GetIntHeight() - diameter;

  if ((dotPosition.x < diameter) or (dotPosition.y < diameter) or
      (dotPosition.x >= screenWidthLessDiameter) or (dotPosition.y >= screenHeightLessDiameter))
  {
    return;
  }

  const auto getColor1 = [this, &radius, &color](const Point2dInt& bitmapPoint, const Pixel& bgnd)
  {
    static constexpr auto MAIN_BRIGHTNESS = 10.0F;
    return GetMixedColor(MAIN_BRIGHTNESS, bitmapPoint, color, radius, bgnd);
  };
  const auto getColor2 = [this, &radius, &color](const Point2dInt& bitmapPoint, const Pixel& bgnd)
  {
    static constexpr auto LOW_BRIGHTNESS = 12.0F;
    return GetMixedColor(LOW_BRIGHTNESS, bitmapPoint, color, radius, bgnd);
  };

  const auto midPoint = Point2dInt{.x = dotPosition.x + static_cast<int32_t>(radius),
                                   .y = dotPosition.y + static_cast<int32_t>(radius)};
  m_bitmapDrawer.Bitmap(
      midPoint, GetImageBitmap(static_cast<uint32_t>(diameter)), {getColor1, getColor2});
}

auto GoomDotsFx::GoomDotsFxImpl::GetMixedColor(const float brightness,
                                               const Point2dInt& bitmapPoint,
                                               const Pixel& color,
                                               const uint32_t radius,
                                               const Pixel& bgnd) const -> Pixel
{
  if (0 == bgnd.A())
  {
    return BLACK_PIXEL;
  }
  const auto newColor =
      m_useMiddleColor and IsImagePointCloseToMiddle(bitmapPoint, radius) ? m_middleColor : color;
  static constexpr auto COLOR_MIX_T = 0.75F;
  const auto mixedColor             = COLOR::ColorMaps::GetColorMix(bgnd, newColor, COLOR_MIX_T);
  return m_colorAdjust.GetAdjustment(brightness, mixedColor);
}

inline auto GoomDotsFx::GoomDotsFxImpl::IsImagePointCloseToMiddle(const Point2dInt& point,
                                                                  const uint32_t radius) -> bool
{
  const auto margin = GetMargin(radius);
  const auto minVal = static_cast<int32_t>(radius - margin);
  const auto maxVal = static_cast<int32_t>(radius + margin);
  return (minVal <= point.x) and (point.x <= maxVal) and (minVal <= point.y) and
         (point.y <= maxVal);
}

inline auto GoomDotsFx::GoomDotsFxImpl::GetMargin(const uint32_t radius) -> size_t
{
  static constexpr auto SMALLEST_MARGIN = 2U;
  if (static constexpr auto SMALL_RADIUS = 7U; radius < SMALL_RADIUS)
  {
    return SMALLEST_MARGIN;
  }
  return SMALLEST_MARGIN + 1;
}

} // namespace GOOM::VISUAL_FX
