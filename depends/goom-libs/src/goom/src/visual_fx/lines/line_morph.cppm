module;

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

module Goom.VisualFx.LinesFx:LineMorph;

import Goom.Color.ColorAdjustment;
import Goom.Color.ColorMaps;
import Goom.Color.ColorUtils;
import Goom.Color.RandomColorMaps;
import Goom.Draw.GoomDrawBase;
import Goom.Draw.ShaperDrawers.LineDrawer;
import Goom.Utils.EnumUtils;
import Goom.Utils.Graphics.SmallImageBitmaps;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.VisualFx.FxUtils;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomGraphic;
import Goom.Lib.SoundInfo;
import Goom.Lib.Point2d;
import Goom.PluginInfo;
import :LineTypes;

using GOOM::COLOR::ColorAdjustment;
using GOOM::COLOR::ColorMapPtrWrapper;
using GOOM::COLOR::ColorMaps;
using GOOM::COLOR::ConstColorMapSharedPtr;
using GOOM::COLOR::GetLightenedColor;
using GOOM::COLOR::GetSimpleColor;
using GOOM::COLOR::SimpleColors;
using GOOM::COLOR::WeightedRandomColorMaps;
using GOOM::DRAW::IGoomDraw;
using GOOM::DRAW::MultiplePixels;
using GOOM::DRAW::SHAPE_DRAWERS::LineDrawerClippedEndPoints;
using GOOM::UTILS::NUM;
using GOOM::UTILS::GRAPHICS::SmallImageBitmaps;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::NumberRange;
using GOOM::UTILS::MATH::SMALL_FLOAT;
using GOOM::VISUAL_FX::FX_UTILS::DotSizes;
using GOOM::VISUAL_FX::FX_UTILS::GetCircularLinePoints;
using GOOM::VISUAL_FX::FX_UTILS::GetHorizontalLinePoints;
using GOOM::VISUAL_FX::FX_UTILS::GetVerticalLinePoints;
using GOOM::VISUAL_FX::FX_UTILS::LinePoint;
using GOOM::VISUAL_FX::FX_UTILS::SmoothTheCircleJoinAtEnds;

namespace GOOM::VISUAL_FX::LINES
{

class LineMorph
{
public:
  static constexpr auto MIN_LINE_DURATION = 80U;

  // construit un effet de line (une ligne horitontale pour commencer)
  // builds a line effect (a horizontal line to start with)
  struct SrceDestLineParams
  {
    LineParams srceLineParams;
    LineParams destLineParams;
  };

  LineMorph(IGoomDraw& draw,
            const PluginInfo& goomInfo,
            const GoomRand& goomRand,
            const SmallImageBitmaps& smallBitmaps,
            const SrceDestLineParams& srceDestLineParams,
            PixelChannelType defaultAlpha) noexcept;

  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>;
  auto SetWeightedColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept -> void;

  auto Start() noexcept -> void;

  [[nodiscard]] auto GetLineColorPower() const noexcept -> float;
  auto SetLineColorPower(float val) noexcept -> void;

  [[nodiscard]] auto CanResetDestLine() const noexcept -> bool;
  auto ResetDestLine(const LineParams& newParams) noexcept -> void;

  auto DrawLines(const AudioSamples::SampleArray& soundData,
                 const AudioSamples::MinMaxValues& soundMinMax) noexcept -> void;

  [[nodiscard]] auto GetRandomLineColor() const noexcept -> Pixel;

private:
  IGoomDraw* m_draw;
  LineDrawerClippedEndPoints m_lineDrawer{*m_draw};
  const PluginInfo* m_goomInfo;
  const GoomRand* m_goomRand;
  PixelChannelType m_defaultAlpha;

  WeightedRandomColorMaps m_colorMaps;
  ConstColorMapSharedPtr m_currentColorMapPtr = nullptr;
  float m_currentBrightness                   = 1.0F;

  static constexpr float GAMMA = 1.9F;
  ColorAdjustment m_colorAdjust{
      {.gamma = GAMMA, .alterChromaFactor = ColorAdjustment::INCREASED_CHROMA_FACTOR}
  };

  std::vector<LinePoint> m_srcePoints;
  std::vector<LinePoint> m_srcePointsCopy;
  LineParams m_srceLineParams;

  std::vector<LinePoint> m_destPoints;
  LineParams m_destLineParams;

  static constexpr float LINE_LERP_FINISHED_VAL = 1.1F;
  static constexpr float LINE_LERP_INC          = 1.0F / static_cast<float>(MIN_LINE_DURATION - 1);
  float m_lineLerpParam                         = 0.0F;
  auto MoveSrceLineCloserToDest() noexcept -> void;
  [[nodiscard]] auto GetFreshLine(LineType lineType,
                                  float lineParam) const noexcept -> std::vector<LinePoint>;

  // pour l'instant je stocke la couleur a terme, on stockera le mode couleur et l'on animera
  bool m_useLineColor             = true;
  float m_lineColorPower          = 0.0F;
  float m_lineColorPowerIncrement = 0.0F;
  [[nodiscard]] auto GetRandomColorMap() const noexcept -> ColorMapPtrWrapper;
  [[nodiscard]] auto GetFinalLineColor(const Pixel& color) const noexcept -> Pixel;
  auto UpdateColorInfo() noexcept -> void;

  // This factor gives the maximum height to the peaks of the audio samples lines.
  // This value seems pleasing.
  static constexpr auto MAX_NORMALIZED_PEAK_RANGE = NumberRange{100.0F, 400.0F};
  float m_maxNormalizedPeak                       = MAX_NORMALIZED_PEAK_RANGE.min;

  FX_UTILS::DotDrawer m_dotDrawer;
  float m_audioRange    = 0.0F;
  float m_minAudioValue = 0.0F;
  struct PointAndColor
  {
    Point2dInt point;
    Pixel color;
  };
  [[nodiscard]] auto GetAudioPoints(const Pixel& lineColor,
                                    const AudioSamples::SampleArray& audioData) const noexcept
      -> std::vector<PointAndColor>;
  [[nodiscard]] auto GetNextPointData(const LinePoint& linePoint,
                                      const Pixel& mainColor,
                                      const Pixel& randColor,
                                      float dataVal) const noexcept -> PointAndColor;

  [[nodiscard]] auto GetMainColor(const Pixel& lineColor, float t) const noexcept -> Pixel;
  auto DrawFlatLine(const Pixel& lineColor) noexcept -> void;
};

} // namespace GOOM::VISUAL_FX::LINES

namespace GOOM::VISUAL_FX::LINES
{

inline auto LineMorph::GetLineColorPower() const noexcept -> float
{
  return m_lineColorPower;
}

inline auto LineMorph::SetLineColorPower(const float val) noexcept -> void
{
  m_lineColorPower = val;
}

inline auto LineMorph::CanResetDestLine() const noexcept -> bool
{
  return m_lineLerpParam > LINE_LERP_FINISHED_VAL;
}

static constexpr auto MIN_DOT_SIZE01_WEIGHT = 100.0F;
static constexpr auto MIN_DOT_SIZE02_WEIGHT = 050.0F;
static constexpr auto MIN_DOT_SIZE03_WEIGHT = 020.0F;
static constexpr auto MIN_DOT_SIZE04_WEIGHT = 010.0F;

static constexpr auto NORMAL_DOT_SIZE01_WEIGHT = 50.0F;
static constexpr auto NORMAL_DOT_SIZE02_WEIGHT = 20.0F;
static constexpr auto NORMAL_DOT_SIZE03_WEIGHT = 20.0F;
static constexpr auto NORMAL_DOT_SIZE04_WEIGHT = 10.0F;
static constexpr auto NORMAL_DOT_SIZE05_WEIGHT = 10.0F;
static constexpr auto NORMAL_DOT_SIZE06_WEIGHT = 10.0F;
static constexpr auto NORMAL_DOT_SIZE07_WEIGHT = 10.0F;
static constexpr auto NORMAL_DOT_SIZE08_WEIGHT = 10.0F;

LineMorph::LineMorph(IGoomDraw& draw,
                     const PluginInfo& goomInfo,
                     const GoomRand& goomRand,
                     const SmallImageBitmaps& smallBitmaps,
                     const SrceDestLineParams& srceDestLineParams,
                     const PixelChannelType defaultAlpha) noexcept
  : m_draw{&draw},
    m_goomInfo{&goomInfo},
    m_goomRand{&goomRand},
    m_defaultAlpha{defaultAlpha},
    m_srcePoints(AudioSamples::AUDIO_SAMPLE_LEN),
    m_srcePointsCopy(AudioSamples::AUDIO_SAMPLE_LEN),
    m_srceLineParams{srceDestLineParams.srceLineParams},
    m_destPoints(AudioSamples::AUDIO_SAMPLE_LEN),
    m_destLineParams{srceDestLineParams.destLineParams},
    m_dotDrawer{
        *m_draw,
        *m_goomRand,
        smallBitmaps,
        {
          .minDotSizes =
            {
                *m_goomRand,
                {
                    {.key = DotSizes::DOT_SIZE01, .weight = MIN_DOT_SIZE01_WEIGHT},
                    {.key = DotSizes::DOT_SIZE02, .weight = MIN_DOT_SIZE02_WEIGHT},
                    {.key = DotSizes::DOT_SIZE03, .weight = MIN_DOT_SIZE03_WEIGHT},
                    {.key = DotSizes::DOT_SIZE04, .weight = MIN_DOT_SIZE04_WEIGHT},
                }
            },
          .normalDotSizes =
            {
                *m_goomRand,
                {
                    {.key = DotSizes::DOT_SIZE01, .weight = NORMAL_DOT_SIZE01_WEIGHT},
                    {.key = DotSizes::DOT_SIZE02, .weight = NORMAL_DOT_SIZE02_WEIGHT},
                    {.key = DotSizes::DOT_SIZE03, .weight = NORMAL_DOT_SIZE03_WEIGHT},
                    {.key = DotSizes::DOT_SIZE04, .weight = NORMAL_DOT_SIZE04_WEIGHT},
                    {.key = DotSizes::DOT_SIZE05, .weight = NORMAL_DOT_SIZE05_WEIGHT},
                    {.key = DotSizes::DOT_SIZE06, .weight = NORMAL_DOT_SIZE06_WEIGHT},
                    {.key = DotSizes::DOT_SIZE07, .weight = NORMAL_DOT_SIZE07_WEIGHT},
                    {.key = DotSizes::DOT_SIZE08, .weight = NORMAL_DOT_SIZE08_WEIGHT},
                }
            }
        }
    }
{
}

auto LineMorph::GetRandomColorMap() const noexcept -> ColorMapPtrWrapper
{
  return m_colorMaps.GetRandomColorMap(m_colorMaps.GetRandomGroup());
}

auto LineMorph::Start() noexcept -> void
{
  UpdateColorInfo();

  m_srcePoints     = GetFreshLine(m_srceLineParams.lineType, m_srceLineParams.param);
  m_srcePointsCopy = m_srcePoints;

  ResetDestLine(m_destLineParams);
}

auto LineMorph::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  return {m_colorMaps.GetColorMapsName()};
}

auto LineMorph::SetWeightedColorMaps(const WeightedRandomColorMaps& weightedMaps) noexcept -> void
{
  m_colorMaps = weightedMaps;
}

inline auto LineMorph::UpdateColorInfo() noexcept -> void
{
  m_currentColorMapPtr =
      m_colorMaps.GetRandomColorMapSharedPtr(WeightedRandomColorMaps::GetAllColorMapsTypes());

  static constexpr auto PROB_USE_LINE_COLOR = 0.5F;
  m_useLineColor                            = m_goomRand->ProbabilityOf<PROB_USE_LINE_COLOR>();
}

inline auto LineMorph::GetFreshLine(const LineType lineType,
                                    const float lineParam) const noexcept -> std::vector<LinePoint>
{
  switch (lineType)
  {
    case LineType::H_LINE:
      return GetHorizontalLinePoints({.numPoints = AudioSamples::AUDIO_SAMPLE_LEN,
                                      .length    = m_goomInfo->GetDimensions().GetWidth()},
                                     lineParam);
    case LineType::V_LINE:
      return GetVerticalLinePoints({.numPoints = AudioSamples::AUDIO_SAMPLE_LEN,
                                    .length    = m_goomInfo->GetDimensions().GetHeight()},
                                   lineParam);
    case LineType::CIRCLE:
      return GetCircularLinePoints(
          AudioSamples::AUDIO_SAMPLE_LEN,
          {m_goomInfo->GetDimensions().GetWidth(), m_goomInfo->GetDimensions().GetHeight()},
          lineParam);
  }
}

auto LineMorph::MoveSrceLineCloserToDest() noexcept -> void
{
  m_lineLerpParam += LINE_LERP_INC;
  const auto t = std::min(1.0F, m_lineLerpParam);
  for (auto i = 0U; i < AudioSamples::AUDIO_SAMPLE_LEN; ++i)
  {
    m_srcePoints[i].point = lerp(m_srcePointsCopy[i].point, m_destPoints[i].point, t);
    m_srcePoints[i].angle = std::lerp(m_srcePointsCopy[i].angle, m_destPoints[i].angle, t);
  }
  if (m_lineLerpParam >= 1.0F)
  {
    m_srceLineParams.lineType              = m_destLineParams.lineType;
    static constexpr auto BRIGHTNESS_RANGE = NumberRange{5.0F, 10.0F};
    m_currentBrightness                    = m_goomRand->GetRandInRange<BRIGHTNESS_RANGE>();
  }

  Ensures((m_srceLineParams.lineType != LineType::CIRCLE) or (m_lineLerpParam < 1.0F) or
          (UTILS::MATH::FloatsEqual(m_srcePoints[0].point.x,
                                    m_srcePoints[AudioSamples::AUDIO_SAMPLE_LEN - 1].point.x) and
           UTILS::MATH::FloatsEqual(m_srcePoints[0].point.y,
                                    m_srcePoints[AudioSamples::AUDIO_SAMPLE_LEN - 1].point.y)));

  static constexpr auto COLOR_MIX_AMOUNT = 1.0F / 64.0F;
  m_srceLineParams.color =
      ColorMaps::GetColorMix(m_srceLineParams.color, m_destLineParams.color, COLOR_MIX_AMOUNT);

  static constexpr auto POW_INC_RANGE = NumberRange{0.03F, 0.10F};
  static constexpr auto MIN_POWER     = 1.1F;
  static constexpr auto MAX_POWER     = 17.5F;
  m_lineColorPower += m_lineColorPowerIncrement;
  if (m_lineColorPower < MIN_POWER)
  {
    m_lineColorPower          = MIN_POWER;
    m_lineColorPowerIncrement = m_goomRand->GetRandInRange<POW_INC_RANGE>();
  }
  if (m_lineColorPower > MAX_POWER)
  {
    m_lineColorPower          = MAX_POWER;
    m_lineColorPowerIncrement = -m_goomRand->GetRandInRange<POW_INC_RANGE>();
  }

  static constexpr auto AMP_MIX_AMOUNT = 0.01F;
  m_srceLineParams.amplitude =
      std::lerp(m_srceLineParams.amplitude, m_destLineParams.amplitude, AMP_MIX_AMOUNT);
}

auto LineMorph::ResetDestLine(const LineParams& newParams) noexcept -> void
{
  UpdateColorInfo();

  m_destPoints     = GetFreshLine(newParams.lineType, m_destLineParams.param);
  m_destLineParams = newParams;

  m_lineLerpParam = 0.0;

  static constexpr auto BRIGHTNESS_RANGE = NumberRange{1.5F, 3.0F};
  m_currentBrightness                    = m_goomRand->GetRandInRange<BRIGHTNESS_RANGE>();

  m_dotDrawer.ChangeDotSizes();

  m_maxNormalizedPeak = m_goomRand->GetRandInRange<MAX_NORMALIZED_PEAK_RANGE>();

  m_srcePointsCopy = m_srcePoints;
}

auto LineMorph::GetRandomLineColor() const noexcept -> Pixel
{
  if (static constexpr auto PROB_LINE_COLOR = 0.02F; m_goomRand->ProbabilityOf<PROB_LINE_COLOR>())
  {
    return GetSimpleColor(static_cast<SimpleColors>(m_goomRand->GetNRand(NUM<SimpleColors>)),
                          m_defaultAlpha);
  }
  return m_colorMaps.GetRandomColor(GetRandomColorMap(), 0.0F, 1.0F);
}

inline auto LineMorph::GetFinalLineColor(const Pixel& color) const noexcept -> Pixel
{
  return GetLightenedColor(color, m_lineColorPower);
}

auto LineMorph::DrawLines(const AudioSamples::SampleArray& soundData,
                          const AudioSamples::MinMaxValues& soundMinMax) noexcept -> void
{
  static constexpr auto LAST_POINT_INDEX = AudioSamples::AUDIO_SAMPLE_LEN - 1;

  Expects(
      (m_srceLineParams.lineType != LineType::CIRCLE) or (m_lineLerpParam < 1.0F) or
      (UTILS::MATH::FloatsEqual(m_srcePoints[0].point.x, m_srcePoints[LAST_POINT_INDEX].point.x) and
       UTILS::MATH::FloatsEqual(m_srcePoints[0].point.y, m_srcePoints[LAST_POINT_INDEX].point.y)));

  const auto lineColor = GetFinalLineColor(m_srceLineParams.color);

  m_audioRange = soundMinMax.maxVal - soundMinMax.minVal;
  Ensures(m_audioRange >= 0.0F);
  m_minAudioValue = soundMinMax.minVal;

  if (m_audioRange < SMALL_FLOAT)
  {
    // No range - flatline audio
    DrawFlatLine(lineColor);
    MoveSrceLineCloserToDest();
    return;
  }

  const auto audioPoints = GetAudioPoints(lineColor, soundData);

  auto point1 = audioPoints[0].point;

  for (auto i = 1U; i < audioPoints.size(); ++i)
  {
    const auto& nextPointData = audioPoints[i];

    const auto point2 = nextPointData.point;
    const auto colors = MultiplePixels{.color1 = lineColor, .color2 = nextPointData.color};

    m_lineDrawer.DrawLine(point1, point2, colors);

    static constexpr auto DOT_BRIGHTNESS = 3.0F;
    m_dotDrawer.DrawDot(point2, colors, DOT_BRIGHTNESS);

    point1 = point2;
  }

  MoveSrceLineCloserToDest();

  m_dotDrawer.ChangeDotSizes();
}

auto LineMorph::DrawFlatLine(const Pixel& lineColor) noexcept -> void
{
  const auto& pt0   = m_srcePoints[0];
  const auto& ptN   = m_srcePoints[AudioSamples::AUDIO_SAMPLE_LEN - 1];
  const auto colors = MultiplePixels{.color1 = lineColor, .color2 = lineColor};

  m_lineDrawer.DrawLine(ToPoint2dInt(pt0.point), ToPoint2dInt(ptN.point), colors);
}

auto LineMorph::GetAudioPoints(const Pixel& lineColor, const AudioSamples::SampleArray& audioData)
    const noexcept -> std::vector<PointAndColor>
{
  const auto randColor = GetRandomLineColor();

  static constexpr auto T_STEP    = 1.0F / static_cast<float>(AudioSamples::AUDIO_SAMPLE_LEN - 1);
  static constexpr auto HALFWAY_T = 0.5F;
  auto currentTStep               = T_STEP;
  auto t                          = 0.0F;

  auto audioPoints = std::vector<PointAndColor>{};
  audioPoints.reserve(audioData.size());

  for (auto i = 0U; i < audioData.size(); ++i)
  {
    audioPoints.emplace_back(
        GetNextPointData(m_srcePoints[i], GetMainColor(lineColor, t), randColor, audioData.at(i)));

    if (t >= HALFWAY_T)
    {
      currentTStep = -T_STEP;
    }
    t += currentTStep;
  }

  if ((m_srceLineParams.lineType == LineType::CIRCLE) and (m_lineLerpParam >= 1.0F))
  {
    // This is a complete circle -- lerp the last few points to nicely join back to start.
    static constexpr auto NUM_POINTS_TO_LERP = 50U;
    SmoothTheCircleJoinAtEnds(audioPoints, NUM_POINTS_TO_LERP);
  }

  return audioPoints;
}

auto LineMorph::GetNextPointData(const LinePoint& linePoint,
                                 const Pixel& mainColor,
                                 const Pixel& randColor,
                                 const float dataVal) const noexcept -> PointAndColor
{
  Expects(m_goomInfo->GetSoundEvents().GetSoundInfo().GetAllTimesMinVolume() <=
          (dataVal + SMALL_FLOAT));
  Expects(m_minAudioValue <= (dataVal + SMALL_FLOAT));
  Expects(dataVal <= ((m_minAudioValue + m_audioRange) + SMALL_FLOAT));

  const auto tData = (dataVal - m_minAudioValue) / m_audioRange;
  Ensures((0.0F <= tData) && (tData <= 1.0F));

  const auto cosAngle          = std::cos(linePoint.angle);
  const auto sinAngle          = std::sin(linePoint.angle);
  const auto normalizedDataVal = m_maxNormalizedPeak * tData;
  Ensures(normalizedDataVal >= 0.0F);
  // TODO(glk) - Is 'm_srceLineParams.amplitude' the right abstraction level?
  const auto nextPointData = Point2dInt{
      .x = static_cast<int32_t>(linePoint.point.x +
                                (m_srceLineParams.amplitude * cosAngle * normalizedDataVal)),
      .y = static_cast<int32_t>(linePoint.point.y +
                                (m_srceLineParams.amplitude * sinAngle * normalizedDataVal))};

  const auto brightness = m_currentBrightness * tData;
  const auto modColor =
      m_colorAdjust.GetAdjustment(brightness, ColorMaps::GetColorMix(mainColor, randColor, tData));

  return {.point = nextPointData, .color = modColor};
}

inline auto LineMorph::GetMainColor(const Pixel& lineColor, const float t) const noexcept -> Pixel
{
  if (m_useLineColor)
  {
    return lineColor;
  }
  return m_currentColorMapPtr->GetColor(t);
}

} // namespace GOOM::VISUAL_FX::LINES
