#include "goom_dots_fx.h"

#include "color/colormaps.h"
#include "color/colorutils.h"
#include "draw/goom_draw.h"
#include "goom/logging_control.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "utils/graphics/image_bitmaps.h"
//#undef NO_LOGGING
#include "color/random_colormaps.h"
#include "color/random_colormaps_manager.h"
#include "goom/logging.h"
#include "goom/spimpl.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/mathutils.h"
#include "v2d.h"

#include <cmath>
#include <cstdint>
#include <memory>
#include <string>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
#else
namespace GOOM::VISUAL_FX
{
#endif

using DRAW::IGoomDraw;
using COLOR::COLOR_DATA::ColorMapName;
using COLOR::ColorMapGroup;
using COLOR::GetColorMultiply;
using COLOR::GetBrighterColor;
using COLOR::GammaCorrection;
using COLOR::RandomColorMaps;
using COLOR::RandomColorMapsManager;
using UTILS::GetRandInRange;
using UTILS::ImageBitmap;
using UTILS::ProbabilityOfMInN;
using UTILS::SmallImageBitmaps;
using UTILS::Weights;

inline auto ChangeDotColorsEvent() -> bool
{
  return ProbabilityOfMInN(1, 3);
}

class GoomDotsFx::GoomDotsFxImpl
{
public:
  GoomDotsFxImpl(IGoomDraw& draw,
                 const PluginInfo& goomInfo,
                 const SmallImageBitmaps& smallBitmaps) noexcept;

  void Start();

  void SetWeightedColorMaps(uint32_t dotNum, std::shared_ptr<RandomColorMaps> weightedMaps);

  void ApplySingle();
  void ApplyMultiple();

private:
  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const SmallImageBitmaps& m_smallBitmaps;
  const V2dInt m_screenMidPoint;
  const uint32_t m_pointWidth;
  const uint32_t m_pointHeight;

  const float m_pointWidthDiv2;
  const float m_pointHeightDiv2;
  const float m_pointWidthDiv3;
  const float m_pointHeightDiv3;

  SmallImageBitmaps::ImageNames m_currentBitmapName{};
  static constexpr uint32_t MAX_FLOWERS_IN_ROW = 100;
  uint32_t m_numFlowersInRow = 0;
  auto GetImageBitmap(size_t size) const -> const ImageBitmap&;

  static constexpr size_t MIN_DOT_SIZE = 3;
  static constexpr size_t MAX_DOT_SIZE = 17;
  static_assert(MAX_DOT_SIZE <= SmallImageBitmaps::MAX_IMAGE_SIZE, "Max dot size mismatch.");

  std::array<std::shared_ptr<RandomColorMaps>, NUM_DOT_TYPES> m_colorMaps{};
  std::array<RandomColorMapsManager, NUM_DOT_TYPES> m_colorMapsManagers{};
  std::array<uint32_t, NUM_DOT_TYPES> m_colorMapIds{};
  Pixel m_middleColor{};
  bool m_useSingleBufferOnly = true;
  bool m_thereIsOneBuffer = true;

  uint32_t m_loopVar = 0; // mouvement des points

  static constexpr float GAMMA = 1.0F / 1.0F;
  static constexpr float GAMMA_BRIGHTNESS_THRESHOLD = 0.01F;
  GammaCorrection m_gammaCorrect{GAMMA, GAMMA_BRIGHTNESS_THRESHOLD};
  auto GetGammaCorrection(float brightness, const Pixel& color) const -> Pixel;

  void Update();

  void ChangeColors();
  [[nodiscard]] static auto GetLargeSoundFactor(const SoundInfo& soundInfo) -> float;

  void DotFilter(const Pixel& color, const V2dInt& dotPosition, uint32_t radius);
  [[nodiscard]] auto GetDotPosition(float xOffsetAmp,
                                    float yOffsetAmp,
                                    float xOffsetFreqDenom,
                                    float yOffsetFreqDenom,
                                    uint32_t offsetCycle) const -> V2dInt;
  void SetNextCurrentBitmapName();
};

GoomDotsFx::GoomDotsFx(IGoomDraw& draw,
                       const PluginInfo& goomInfo,
                       const SmallImageBitmaps& smallBitmaps) noexcept
  : m_fxImpl{spimpl::make_unique_impl<GoomDotsFxImpl>(draw, goomInfo, smallBitmaps)}
{
}

void GoomDotsFx::SetWeightedColorMaps(const uint32_t dotNum,
                                      const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_fxImpl->SetWeightedColorMaps(dotNum, weightedMaps);
}

void GoomDotsFx::Start()
{
  m_fxImpl->Start();
}

void GoomDotsFx::Resume()
{
}

void GoomDotsFx::Suspend()
{
}

void GoomDotsFx::Finish()
{
}

auto GoomDotsFx::GetFxName() const -> std::string
{
  return "goom dots";
}

void GoomDotsFx::ApplySingle()
{
  m_fxImpl->ApplySingle();
}

void GoomDotsFx::ApplyMultiple()
{
  m_fxImpl->ApplyMultiple();
}


GoomDotsFx::GoomDotsFxImpl::GoomDotsFxImpl(IGoomDraw& draw,
                                           const PluginInfo& goomInfo,
                                           const SmallImageBitmaps& smallBitmaps) noexcept
  : m_draw{draw},
    m_goomInfo{goomInfo},
    m_smallBitmaps{smallBitmaps},
    m_screenMidPoint{m_goomInfo.GetScreenInfo().width / 2, m_goomInfo.GetScreenInfo().height / 2},
    m_pointWidth{(m_goomInfo.GetScreenInfo().width * 2) / 5},
    m_pointHeight{(m_goomInfo.GetScreenInfo().height * 2) / 5},
    m_pointWidthDiv2{static_cast<float>(m_pointWidth) / 2.0F},
    m_pointHeightDiv2{static_cast<float>(m_pointHeight) / 2.0F},
    m_pointWidthDiv3{static_cast<float>(m_pointWidth) / 3.0F},
    m_pointHeightDiv3{static_cast<float>(m_pointHeight) / 3.0F}
{
}

inline void GoomDotsFx::GoomDotsFxImpl::Start()
{
  ChangeColors();
}

inline auto GoomDotsFx::GoomDotsFxImpl::GetImageBitmap(const size_t size) const
    -> const ImageBitmap&
{
  return m_smallBitmaps.GetImageBitmap(m_currentBitmapName,
                                       stdnew::clamp(size, MIN_DOT_SIZE, MAX_DOT_SIZE));
}

inline void GoomDotsFx::GoomDotsFxImpl::ChangeColors()
{
  for (auto& colorMapsManager : m_colorMapsManagers)
  {
    colorMapsManager.ChangeAllColorMapsNow();
  }

  m_middleColor = RandomColorMaps::GetRandomColor(
      *m_colorMaps[0]->GetRandomColorMapPtr(ColorMapGroup::MISC, RandomColorMaps::ALL), 0.1F, 1.0F);

  m_useSingleBufferOnly = ProbabilityOfMInN(0, 2);
}

inline void GoomDotsFx::GoomDotsFxImpl::SetWeightedColorMaps(
    const uint32_t dotNum, const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_colorMaps.at(dotNum) = weightedMaps;
  m_colorMapIds[dotNum] = m_colorMapsManagers[dotNum].AddColorMapInfo(
      {m_colorMaps[dotNum],
       m_colorMaps[dotNum]->GetRandomColorMapName(m_colorMaps[dotNum]->GetRandomGroup()),
       RandomColorMaps::ALL});
}

inline void GoomDotsFx::GoomDotsFxImpl::ApplySingle()
{
  m_thereIsOneBuffer = true;
  Update();
}

inline void GoomDotsFx::GoomDotsFxImpl::ApplyMultiple()
{
  m_thereIsOneBuffer = false;
  Update();
}

void GoomDotsFx::GoomDotsFxImpl::Update()
{
  uint32_t radius = MIN_DOT_SIZE / 2;
  if ((0 == m_goomInfo.GetSoundInfo().GetTimeSinceLastGoom()) || ChangeDotColorsEvent())
  {
    ChangeColors();
    radius = GetRandInRange(radius, (MAX_DOT_SIZE / 2) + 1);
    SetNextCurrentBitmapName();
  }

  const float speedFactor = 0.25F * m_goomInfo.GetSoundInfo().GetSpeed();

  const float largeFactor = GetLargeSoundFactor(m_goomInfo.GetSoundInfo());
  const auto speedVarMult80Plus15 = static_cast<uint32_t>((speedFactor * 80.0F) + 15.0F);
  const auto speedVarMult50Plus1 = static_cast<uint32_t>((speedFactor * 50.0F) + 1.0F);

  const float pointWidthDiv2MultLarge = m_pointWidthDiv2 * largeFactor;
  const float pointHeightDiv2MultLarge = m_pointHeightDiv2 * largeFactor;
  const float pointWidthDiv3MultLarge = (m_pointWidthDiv3 + 5.0F) * largeFactor;
  const float pointHeightDiv3MultLarge = (m_pointHeightDiv3 + 5.0F) * largeFactor;
  const float pointWidthMultLarge = static_cast<float>(m_pointWidth) * largeFactor;
  const float pointHeightMultLarge = static_cast<float>(m_pointHeight) * largeFactor;

  const float dot0XOffsetAmp = ((static_cast<float>(m_pointWidth) - 6.0F) * largeFactor) + 5.0F;
  const float dot0YOffsetAmp = ((static_cast<float>(m_pointHeight) - 6.0F) * largeFactor) + 5.0F;
  const float dot3XOffsetAmp = (m_pointHeightDiv3 * largeFactor) + 20.0F;
  const float dot3YOffsetAmp = dot3XOffsetAmp;

  const size_t speedVarMult80Plus15Div15 = speedVarMult80Plus15 / 15;
  constexpr float T_MIN = 0.5F;
  constexpr float T_MAX = 1.0F;
  const float t_step = (T_MAX - T_MIN) / static_cast<float>(speedVarMult80Plus15Div15);

  float t = T_MIN;
  for (uint32_t i = 1; i <= speedVarMult80Plus15Div15; ++i)
  {
    m_loopVar += speedVarMult50Plus1;

    const uint32_t loopvarDivI = m_loopVar / i;
    const float iMult10 = 10.0F * static_cast<float>(i);

    const Pixel dot0Color = m_colorMapsManagers[0].GetColorMap(m_colorMapIds[0]).GetColor(t);
    const float dot0XFreqDenom = static_cast<float>(i) * 152.0F;
    const float dot0YFreqDenom = 128.0F;
    const uint32_t dot0Cycle = m_loopVar + (i * 2032);
    const V2dInt dot0Position =
        GetDotPosition(dot0XOffsetAmp, dot0YOffsetAmp, dot0XFreqDenom, dot0YFreqDenom, dot0Cycle);

    const Pixel dot1Color = m_colorMapsManagers[1].GetColorMap(m_colorMapIds[1]).GetColor(t);
    const float dot1XOffsetAmp = (pointWidthDiv2MultLarge / static_cast<float>(i)) + iMult10;
    const float dot1YOffsetAmp = (pointHeightDiv2MultLarge / static_cast<float>(i)) + iMult10;
    const float dot1XFreqDenom = 96.0F;
    const float dot1YFreqDenom = static_cast<float>(i) * 80.0F;
    const uint32_t dot1Cycle = loopvarDivI;
    const V2dInt dot1Position =
        GetDotPosition(dot1XOffsetAmp, dot1YOffsetAmp, dot1XFreqDenom, dot1YFreqDenom, dot1Cycle);

    const Pixel dot2Color = m_colorMapsManagers[2].GetColorMap(m_colorMapIds[2]).GetColor(t);
    const float dot2XOffsetAmp = (pointWidthDiv3MultLarge / static_cast<float>(i)) + iMult10;
    const float dot2YOffsetAmp = (pointHeightDiv3MultLarge / static_cast<float>(i)) + iMult10;
    const float dot2XFreqDenom = static_cast<float>(i) + 122.0F;
    const float dot2YFreqDenom = 134.0F;
    const uint32_t dot2Cycle = loopvarDivI;
    const V2dInt dot2Position =
        GetDotPosition(dot2XOffsetAmp, dot2YOffsetAmp, dot2XFreqDenom, dot2YFreqDenom, dot2Cycle);

    const Pixel dot3Color = m_colorMapsManagers[3].GetColorMap(m_colorMapIds[3]).GetColor(t);
    const float dot3XFreqDenom = 58.0F;
    const float dot3YFreqDenom = static_cast<float>(i) * 66.0F;
    const uint32_t dot3Cycle = loopvarDivI;
    const V2dInt dot3Position =
        GetDotPosition(dot3XOffsetAmp, dot3YOffsetAmp, dot3XFreqDenom, dot3YFreqDenom, dot3Cycle);

    const Pixel dot4Color = m_colorMapsManagers[4].GetColorMap(m_colorMapIds[4]).GetColor(t);
    const float dot4XOffsetAmp = (pointWidthMultLarge + iMult10) / static_cast<float>(i);
    const float dot4YOffsetAmp = (pointHeightMultLarge + iMult10) / static_cast<float>(i);
    const float dot4XFreqDenom = 66.0F;
    const float dot4YFreqDenom = 74.0F;
    const uint32_t dot4Cycle = m_loopVar + (i * 500);
    const V2dInt dot4Position =
        GetDotPosition(dot4XOffsetAmp, dot4YOffsetAmp, dot4XFreqDenom, dot4YFreqDenom, dot4Cycle);

    DotFilter(dot0Color, dot0Position, radius);
    DotFilter(dot1Color, dot1Position, radius);
    DotFilter(dot2Color, dot2Position, radius);
    DotFilter(dot3Color, dot3Position, radius);
    DotFilter(dot4Color, dot4Position, radius);

    t += t_step;
  }
}

void GoomDotsFx::GoomDotsFxImpl::SetNextCurrentBitmapName()
{
  if (m_numFlowersInRow > 0)
  {
    ++m_numFlowersInRow;
    if (m_numFlowersInRow > MAX_FLOWERS_IN_ROW)
    {
      m_numFlowersInRow = 0;
    }
    if (ProbabilityOfMInN(1, 3))
    {
      m_currentBitmapName = SmallImageBitmaps::ImageNames::RED_FLOWER;
    }
    else if (ProbabilityOfMInN(1, 3))
    {
      m_currentBitmapName = SmallImageBitmaps::ImageNames::ORANGE_FLOWER;
    }
    else
    {
      m_currentBitmapName = SmallImageBitmaps::ImageNames::WHITE_FLOWER;
    }
  }
  else if (ProbabilityOfMInN(1, 50))
  {
    m_numFlowersInRow = 1;
    SetNextCurrentBitmapName();
  }
  else
  {
    static const Weights<SmallImageBitmaps::ImageNames> s_dotTypes{{
        {SmallImageBitmaps::ImageNames::SPHERE, 50},
        {SmallImageBitmaps::ImageNames::CIRCLE, 20},
        {SmallImageBitmaps::ImageNames::RED_FLOWER, 5},
        {SmallImageBitmaps::ImageNames::ORANGE_FLOWER, 5},
        {SmallImageBitmaps::ImageNames::WHITE_FLOWER, 5},
    }};
    m_currentBitmapName = s_dotTypes.GetRandomWeighted();
  }
}

inline auto GoomDotsFx::GoomDotsFxImpl::GetLargeSoundFactor(const SoundInfo& soundInfo) -> float
{
  constexpr float MAX_LARGE_FACTOR = 1.45F;
  return (soundInfo.GetSpeed() / 750.0F) + (soundInfo.GetVolume() / MAX_LARGE_FACTOR);
}

inline auto GoomDotsFx::GoomDotsFxImpl::GetDotPosition(const float xOffsetAmp,
                                                       const float yOffsetAmp,
                                                       const float xOffsetFreqDenom,
                                                       const float yOffsetFreqDenom,
                                                       const uint32_t offsetCycle) const -> V2dInt
{
  const auto xOffset = static_cast<int32_t>(
      xOffsetAmp * std::cos(static_cast<float>(offsetCycle) / xOffsetFreqDenom));
  const auto yOffset = static_cast<int32_t>(
      yOffsetAmp * std::sin(static_cast<float>(offsetCycle) / yOffsetFreqDenom));

  return {m_screenMidPoint.x + xOffset, m_screenMidPoint.y + yOffset};
}

void GoomDotsFx::GoomDotsFxImpl::DotFilter(const Pixel& color,
                                           const V2dInt& dotPosition,
                                           const uint32_t radius)
{
  const uint32_t diameter = (2 * radius) + 1; // must be odd
  const auto screenWidthLessDiameter =
      static_cast<int32_t>(m_goomInfo.GetScreenInfo().width - diameter);
  const auto screenHeightLessDiameter =
      static_cast<int32_t>(m_goomInfo.GetScreenInfo().height - diameter);

  if ((dotPosition.x < static_cast<int32_t>(diameter)) ||
      (dotPosition.y < static_cast<int32_t>(diameter)) ||
      (dotPosition.x >= screenWidthLessDiameter) || (dotPosition.y >= screenHeightLessDiameter))
  {
    return;
  }

  constexpr float BRIGHTNESS = 10.0F;
  const auto getColor1 = [&]([[maybe_unused]] const size_t x, [[maybe_unused]] const size_t y,
                             const Pixel& b) -> Pixel {
    // const Pixel newColor = x == xMid && y == yMid ? m_middleColor : color;
    if (0 == b.A())
    {
      return Pixel::BLACK;
    }
    return GetGammaCorrection(BRIGHTNESS, GetColorMultiply(b, color, m_draw.GetAllowOverexposed()));
  };
  const auto getColor2 = [&]([[maybe_unused]] const size_t x, [[maybe_unused]] const size_t y,
                             [[maybe_unused]] const Pixel& b) -> Pixel {
    if (0 == b.A())
    {
      return Pixel::BLACK;
    }
    return GetGammaCorrection(BRIGHTNESS, color);
  };

  const auto xMid = dotPosition.x + static_cast<int32_t>(radius);
  const auto yMid = dotPosition.y + static_cast<int32_t>(radius);
  if (m_thereIsOneBuffer || m_useSingleBufferOnly)
  {
    m_draw.Bitmap(xMid, yMid, GetImageBitmap(diameter), getColor1);
  }
  else
  {
    m_draw.Bitmap(xMid, yMid, GetImageBitmap(diameter), {getColor1, getColor2});
  }
}

inline auto GoomDotsFx::GoomDotsFxImpl::GetGammaCorrection(const float brightness,
                                                           const Pixel& color) const -> Pixel
{
  // if constexpr (GAMMA == 1.0F)
  if (1.0F == GAMMA)
  {
    return GetBrighterColor(brightness, color, true);
  }
  return m_gammaCorrect.GetCorrection(brightness, color);
}

#if __cplusplus <= 201402L
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX
#endif
