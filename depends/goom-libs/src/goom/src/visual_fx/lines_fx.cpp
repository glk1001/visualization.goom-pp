module;

//#undef NO_LOGGING

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <vector>

module Goom.VisualFx.LinesFx;

import Goom.Color.ColorUtils;
import Goom.Color.RandomColorMaps;
import Goom.Control.GoomSoundEvents;
import Goom.Utils.Graphics.SmallImageBitmaps;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.GoomTime;
import Goom.VisualFx.FxHelper;
import Goom.VisualFx.FxUtils;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomGraphic;
import Goom.Lib.SoundInfo;
import Goom.Lib.SPimpl;
import :LineMorph;
import :LineTypes;

namespace GOOM::VISUAL_FX
{

using COLOR::GetSimpleColor;
using COLOR::SimpleColors;
using COLOR::WeightedRandomColorMaps;
using FX_UTILS::RandomPixelBlender;
using LINES::LineMorph;
using LINES::LineType;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::Weights;

static constexpr auto INITIAL_SCREEN_HEIGHT_FRACTION_LINE1 = 0.4F;
static constexpr auto INITIAL_SCREEN_HEIGHT_FRACTION_LINE2 = 0.2F;

static constexpr auto PROB_CHANGE_LINE_CIRCLE_AMPLITUDE = 0.05F;
static constexpr auto PROB_CHANGE_LINE_CIRCLE_PARAMS    = 0.08F;
static constexpr auto PROB_CHANGE_H_LINE_PARAMS         = 0.12F;
static constexpr auto PROB_CHANGE_V_LINE_PARAMS         = 0.10F;
static constexpr auto PROB_CHANGE_LINE_TO_BLACK         = 0.08F;
static constexpr auto PROB_REDUCE_LINE_MODE             = 0.03F;
static constexpr auto PROB_UPDATE_LINE_MODE             = 0.04F;
static constexpr auto PROB_CHANGE_GOOM_LINE             = 0.05F;
static constexpr auto PROB_NEAR_SCOPE                   = 0.01F;
static constexpr auto PROB_FAR_SCOPE                    = 0.01F;

static constexpr auto CIRCLE_LINE_TYPE_WEIGHT = 10.0F;
static constexpr auto H_LINE_LINE_TYPE_WEIGHT = 02.0F;
static constexpr auto V_LINE_LINE_TYPE_WEIGHT = 02.0F;

class LinesFx::LinesImpl
{
public:
  // construit un effet de line (une ligne horitontale pour commencer)
  // builds a line effect (a horizontal line to start with)
  LinesImpl(FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept;

  auto Start() noexcept -> void;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void;
  auto SetSoundData(const AudioSamples& soundData) noexcept -> void;
  auto ResetLineModes() noexcept -> void;

  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;
  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>;

  auto ApplyToImageBuffers() noexcept -> void;

  [[nodiscard]] auto GetRandomLineColors() const noexcept -> std::array<Pixel, NUM_LINES>;

private:
  FxHelper* m_fxHelper;
  PixelChannelType m_defaultAlpha = DEFAULT_VISUAL_FX_ALPHA;
  Pixel m_blackLineColor          = GetSimpleColor(SimpleColors::BLACK, m_defaultAlpha);

  static_assert(2 == NUM_LINES);
  static constexpr std::array<uint32_t, NUM_LINES> SOUND_SAMPLE_NUM_TO_USE{0, 1};
  const AudioSamples* m_soundData = nullptr;
  std::array<LineMorph, NUM_LINES> m_lineMorphs;
  [[nodiscard]] auto CanResetDestLines() const noexcept -> bool;
  using LineParams = LINES::LineParams;
  auto ResetDestLines(const std::array<LineParams, NUM_LINES>& newDestParams) noexcept -> void;
  auto ResetLineColorPowers() noexcept -> void;
  auto DrawLines() noexcept -> void;

  RandomPixelBlender m_pixelBlender;
  auto UpdatePixelBlender() noexcept -> void;

  float m_screenWidth         = m_fxHelper->GetDimensions().GetFltWidth();
  float m_screenHeight        = m_fxHelper->GetDimensions().GetFltHeight();
  int32_t m_drawLinesDuration = LineMorph::MIN_LINE_DURATION;
  int32_t m_lineMode          = LineMorph::MIN_LINE_DURATION; // l'effet lineaire a dessiner
  uint32_t m_stopLines        = 0;
  Weights<LineType> m_lineTypeWeights;

  bool m_isNearScope = m_fxHelper->GetGoomRand().ProbabilityOf<PROB_NEAR_SCOPE>();
  bool m_isFarScope  = m_fxHelper->GetGoomRand().ProbabilityOf<PROB_FAR_SCOPE>();
  auto UpdateScopes() noexcept -> void;
  [[nodiscard]] auto CanDisplayLines() const noexcept -> bool;
  auto ChangeGoomLines() noexcept -> void;
  auto UpdateLineModes() noexcept -> void;
  // arret demande
  auto StopLinesIfRequested() noexcept -> void;
  // arret aleatore.. changement de mode de ligne..
  auto StopRandomLineChangeMode() noexcept -> void;
  auto ResetGoomLines() noexcept -> void;
  auto StopGoomLines() noexcept -> void;
  [[nodiscard]] auto GetGoomLineResetSettings(uint32_t farVal) const noexcept
      -> std::array<LineParams, NUM_LINES>;
  [[nodiscard]] auto GetGoomLineStopSettings() const noexcept -> std::array<LineParams, NUM_LINES>;
  [[nodiscard]] auto GetResetCircleLineSettings(uint32_t farVal) const noexcept
      -> std::array<LineParams, NUM_LINES>;
  [[nodiscard]] auto GetResetHorizontalLineSettings(uint32_t farVal) const noexcept
      -> std::array<LineParams, NUM_LINES>;
  [[nodiscard]] auto GetResetVerticalLineSettings(uint32_t farVal) const noexcept
      -> std::array<LineParams, NUM_LINES>;
  [[nodiscard]] auto GetResetLineColors(uint32_t farVal) const noexcept
      -> std::array<Pixel, NUM_LINES>;
};

LinesFx::LinesFx(FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept
  : m_pimpl{spimpl::make_unique_impl<LinesImpl>(fxHelper, smallBitmaps)}
{
}

auto LinesFx::GetFxName() const noexcept -> std::string
{
  return "lines";
}

auto LinesFx::Start() noexcept -> void
{
  m_pimpl->Start();
}

auto LinesFx::Finish() noexcept -> void
{
  // nothing to do
}

auto LinesFx::ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pimpl->ChangePixelBlender(pixelBlenderParams);
}

auto LinesFx::SetSoundData(const AudioSamples& soundData) noexcept -> void
{
  m_pimpl->SetSoundData(soundData);
}

auto LinesFx::ResetLineModes() noexcept -> void
{
  m_pimpl->ResetLineModes();
}

auto LinesFx::SetWeightedColorMaps(const IVisualFx::WeightedColorMaps& weightedColorMaps) noexcept
    -> void
{
  m_pimpl->SetWeightedColorMaps(weightedColorMaps);
}

auto LinesFx::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  return m_pimpl->GetCurrentColorMapsNames();
}

auto LinesFx::ApplyToImageBuffers() noexcept -> void
{
  m_pimpl->ApplyToImageBuffers();
}

LinesFx::LinesImpl::LinesImpl(FxHelper& fxHelper,
                              const SmallImageBitmaps& smallBitmaps) noexcept
  : m_fxHelper{&fxHelper},
    m_lineMorphs{
        LineMorph{
            fxHelper.GetDraw(),
            fxHelper.GetGoomInfo(),
            fxHelper.GetGoomRand(),
            smallBitmaps,
            {
                .srceLineParams={
                    .lineType=LineType::H_LINE,
                    .param=fxHelper.GetDimensions().GetFltHeight(),
                    .color=m_blackLineColor,
                    .amplitude=1.0F
                },
                .destLineParams={
                    .lineType=LineType::CIRCLE,
                    .param=INITIAL_SCREEN_HEIGHT_FRACTION_LINE1 *
                        fxHelper.GetDimensions().GetFltHeight(),
                    .color=GetSimpleColor(SimpleColors::SHAMROCK, m_defaultAlpha),
                    .amplitude=1.0F
                }
            },
            m_defaultAlpha
        },
        LineMorph{
            fxHelper.GetDraw(),
            fxHelper.GetGoomInfo(),
            fxHelper.GetGoomRand(),
            smallBitmaps,
            {
                .srceLineParams={
                    .lineType=LineType::H_LINE,
                    .param=0.0F,
                    .color=m_blackLineColor,
                    .amplitude=1.0F
                },
                .destLineParams={
                    .lineType=LineType::CIRCLE,
                    .param=INITIAL_SCREEN_HEIGHT_FRACTION_LINE2 *
                        fxHelper.GetDimensions().GetFltHeight(),
                    .color=GetSimpleColor(SimpleColors::TANGO, m_defaultAlpha),
                    .amplitude=1.0F
                }
            },
            m_defaultAlpha
        }
    },
    m_pixelBlender{fxHelper.GetGoomRand()},
    m_lineTypeWeights{
        m_fxHelper->GetGoomRand(),
        {
            { .key=LineType::CIRCLE, .weight=CIRCLE_LINE_TYPE_WEIGHT },
            { .key=LineType::H_LINE, .weight=H_LINE_LINE_TYPE_WEIGHT },
            { .key=LineType::V_LINE, .weight=V_LINE_LINE_TYPE_WEIGHT },
        }
    }
{
}

inline auto LinesFx::LinesImpl::Start() noexcept -> void
{
  std::ranges::for_each(m_lineMorphs, [](LineMorph& line) { line.Start(); });
}

auto LinesFx::LinesImpl::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  auto allColorMapsNames = std::vector<std::string>{};
  for (const auto& lineMorph : m_lineMorphs)
  {
    const auto colorMapsNames = lineMorph.GetCurrentColorMapsNames();
    for (const auto& mapsName : colorMapsNames)
    {
      allColorMapsNames.emplace_back(mapsName);
    }
  }
  return allColorMapsNames;
}

inline auto LinesFx::LinesImpl::SetWeightedColorMaps(
    const IVisualFx::WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  const auto lineNum = weightedColorMaps.id;
  Expects(lineNum < NUM_LINES);

  const auto mainColorMaps =
      WeightedRandomColorMaps{weightedColorMaps.mainColorMaps, m_defaultAlpha};

  m_lineMorphs.at(lineNum).SetWeightedColorMaps(mainColorMaps);
}

inline auto LinesFx::LinesImpl::CanResetDestLines() const noexcept -> bool
{
  return std::ranges::all_of(m_lineMorphs,

                             [](const LineMorph& line) { return line.CanResetDestLine(); });
}

inline auto LinesFx::LinesImpl::ResetDestLines(
    const std::array<LineParams, NUM_LINES>& newDestParams) noexcept -> void
{
  for (auto i = 0U; i < NUM_LINES; ++i)
  {
    m_lineMorphs.at(i).ResetDestLine(newDestParams.at(i));
  }
}

inline auto LinesFx::LinesImpl::ResetLineModes() noexcept -> void
{
  if (not m_isNearScope)
  {
    static constexpr auto SCOPE_RESET = 0xF000U & 5U;
    m_stopLines                       = SCOPE_RESET;
  }
  if (not m_isFarScope)
  {
    m_stopLines = 0;
    m_lineMode  = m_drawLinesDuration;
  }
}

inline auto LinesFx::LinesImpl::GetRandomLineColors() const noexcept -> std::array<Pixel, NUM_LINES>
{
  auto colors = std::array<Pixel, NUM_LINES>{};
  for (auto i = 0U; i < NUM_LINES; ++i)
  {
    colors.at(i) = m_lineMorphs.at(i).GetRandomLineColor();
  }
  return colors;
}

inline auto LinesFx::LinesImpl::ChangePixelBlender(
    const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pixelBlender.SetPixelBlendType(pixelBlenderParams);
}

inline auto LinesFx::LinesImpl::SetSoundData(const AudioSamples& soundData) noexcept -> void
{
  m_soundData = &soundData;
}

inline auto LinesFx::LinesImpl::ApplyToImageBuffers() noexcept -> void
{
  UpdatePixelBlender();
  UpdateScopes();
  UpdateLineModes();

  if (not CanDisplayLines())
  {
    return;
  }

  ResetLineColorPowers();
  DrawLines();

  ChangeGoomLines();
}

inline auto LinesFx::LinesImpl::UpdatePixelBlender() noexcept -> void
{
  m_fxHelper->GetDraw().SetPixelBlendFunc(m_pixelBlender.GetCurrentPixelBlendFunc());
  m_pixelBlender.Update();
}

inline auto LinesFx::LinesImpl::UpdateScopes() noexcept -> void
{
  static constexpr auto NUM_UPDATES_BEFORE_SCOPE_CHANGE = 200U;
  if (0 == (m_fxHelper->GetGoomTime().GetCurrentTime() % NUM_UPDATES_BEFORE_SCOPE_CHANGE))
  {
    m_isNearScope = m_fxHelper->GetGoomRand().ProbabilityOf<PROB_NEAR_SCOPE>();
    m_isFarScope  = m_fxHelper->GetGoomRand().ProbabilityOf<PROB_FAR_SCOPE>();
  }
}

inline auto LinesFx::LinesImpl::ResetLineColorPowers() noexcept -> void
{
  static_assert(2 == NUM_LINES);
  m_lineMorphs.at(1).SetLineColorPower(m_lineMorphs.at(0).GetLineColorPower());
}

inline auto LinesFx::LinesImpl::DrawLines() noexcept -> void
{
  Expects(m_soundData != nullptr);

  for (auto i = 0U; i < NUM_LINES; ++i)
  {
    m_lineMorphs.at(i).DrawLines(m_soundData->GetSample(SOUND_SAMPLE_NUM_TO_USE.at(i)),
                                 m_soundData->GetSampleMinMax(SOUND_SAMPLE_NUM_TO_USE.at(i)));
  }
}

auto LinesFx::LinesImpl::ChangeGoomLines() noexcept -> void
{
  if (not CanResetDestLines())
  {
    return;
  }

  static constexpr auto CHANGE_GOOM_LINE_CYCLES = 121U;
  static constexpr auto GOOM_CYCLE_MOD_CHANGE   = 9U;

  if ((GOOM_CYCLE_MOD_CHANGE ==
       (m_fxHelper->GetGoomTime().GetCurrentTime() % CHANGE_GOOM_LINE_CYCLES)) and
      m_fxHelper->GetGoomRand().ProbabilityOf<PROB_CHANGE_GOOM_LINE>() and
      ((0 == m_lineMode) or (m_lineMode == m_drawLinesDuration)))
  {
    ResetGoomLines();
  }
}

inline auto LinesFx::LinesImpl::CanDisplayLines() const noexcept -> bool
{
  static constexpr auto DISPLAY_LINES_GOOM_NUM = 5U;

  return ((m_lineMode != 0) or
          (m_fxHelper->GetSoundEvents().GetTimeSinceLastGoom() < DISPLAY_LINES_GOOM_NUM));
}

inline auto LinesFx::LinesImpl::UpdateLineModes() noexcept -> void
{
  StopLinesIfRequested();
  StopRandomLineChangeMode();
}

inline auto LinesFx::LinesImpl::StopLinesIfRequested() noexcept -> void
{
  static constexpr auto LARGE_STOP_LINE = 0xF000U;
  if (((m_stopLines & LARGE_STOP_LINE) != 0) or (not m_isNearScope))
  {
    StopGoomLines();
  }
}

inline auto LinesFx::LinesImpl::StopGoomLines() noexcept -> void
{
  if (not CanResetDestLines())
  {
    return;
  }

  ResetDestLines(GetGoomLineStopSettings());

  static constexpr auto STOP_MASK = 0x0FFFU;
  m_stopLines &= STOP_MASK;
}

inline auto LinesFx::LinesImpl::ResetGoomLines() noexcept -> void
{
  ResetDestLines(GetGoomLineResetSettings(m_stopLines));

  if (m_stopLines > 0)
  {
    --m_stopLines;
  }
}

auto LinesFx::LinesImpl::GetGoomLineStopSettings() const noexcept
    -> std::array<LineParams, NUM_LINES>
{
  auto lineParams        = GetGoomLineResetSettings(1);
  lineParams.at(0).color = m_blackLineColor;
  lineParams.at(1).color = m_blackLineColor;
  return lineParams;
}

auto LinesFx::LinesImpl::GetGoomLineResetSettings(const uint32_t farVal) const noexcept
    -> std::array<LineParams, NUM_LINES>
{
  switch (m_lineTypeWeights.GetRandomWeighted())
  {
    case LineType::CIRCLE:
      return GetResetCircleLineSettings(farVal);
    case LineType::H_LINE:
      return GetResetHorizontalLineSettings(farVal);
    case LineType::V_LINE:
      return GetResetVerticalLineSettings(farVal);
  }
}

auto LinesFx::LinesImpl::GetResetCircleLineSettings(const uint32_t farVal) const noexcept
    -> std::array<LineParams, NUM_LINES>
{
  static constexpr auto NEW_FAR_VAL_PARAM1 = 0.47F;
  static constexpr auto NEW_FAR_VAL_PARAM2 = 0.47F;

  static constexpr auto NEW_NON_FAR_VAL_PARAM1_FACTOR     = 0.40F;
  static constexpr auto NEW_NON_FAR_VAL_PARAM2_FACTOR     = 0.22F;
  static constexpr auto DEFAULT_NON_FAR_VAL_PARAM1_FACTOR = 0.35F;

  static constexpr auto NEW_FAR_VAL_AMPLITUDE     = 0.8F;
  static constexpr auto NEW_NON_FAR_VAL_AMPLITUDE = 3.0F;
  static constexpr auto DEFAULT_AMPLITUDE         = 1.0F;

  float amplitude; // NOLINT(cppcoreguidelines-init-variables)
  float param1; // NOLINT(cppcoreguidelines-init-variables)
  float param2; // NOLINT(cppcoreguidelines-init-variables)

  if (farVal)
  {
    param1    = NEW_FAR_VAL_PARAM1;
    param2    = NEW_FAR_VAL_PARAM2;
    amplitude = NEW_FAR_VAL_AMPLITUDE;
  }
  else if (m_fxHelper->GetGoomRand().ProbabilityOf<PROB_CHANGE_LINE_CIRCLE_AMPLITUDE>())
  {
    param1    = 0.0F;
    param2    = 0.0F;
    amplitude = NEW_NON_FAR_VAL_AMPLITUDE;
  }
  else if (m_fxHelper->GetGoomRand().ProbabilityOf<PROB_CHANGE_LINE_CIRCLE_PARAMS>())
  {
    param1    = NEW_NON_FAR_VAL_PARAM1_FACTOR * m_screenHeight;
    param2    = NEW_NON_FAR_VAL_PARAM2_FACTOR * m_screenHeight;
    amplitude = DEFAULT_AMPLITUDE;
  }
  else
  {
    param1    = DEFAULT_NON_FAR_VAL_PARAM1_FACTOR * m_screenHeight;
    param2    = param1;
    amplitude = DEFAULT_AMPLITUDE;
  }

  const auto [line1Color, line2Color] = GetResetLineColors(farVal);

  return {
      {{.lineType = LineType::CIRCLE, .param = param1, .color = line1Color, .amplitude = amplitude},
       {.lineType  = LineType::CIRCLE,
        .param     = param2,
        .color     = line2Color,
        .amplitude = amplitude}}
  };
}

auto LinesFx::LinesImpl::GetResetHorizontalLineSettings(const uint32_t farVal) const noexcept
    -> std::array<LineParams, NUM_LINES>
{
  static constexpr auto NEW_PARAM1_FACTOR     = 1.0F / 7.0F;
  static constexpr auto NEW_PARAM2_FACTOR     = 6.0F / 7.0F;
  static constexpr auto DEFAULT_PARAM1_FACTOR = 1.0F / 2.0F;
  static constexpr auto DEFAULT_PARAM2_FACTOR = 1.0F / 2.0F;

  static constexpr auto NEW_AMPLITUDE     = 1.0F;
  static constexpr auto DEFAULT_AMPLITUDE = 2.0F;

  float amplitude; // NOLINT(cppcoreguidelines-init-variables)
  float param1; // NOLINT(cppcoreguidelines-init-variables)
  float param2; // NOLINT(cppcoreguidelines-init-variables)

  if (m_fxHelper->GetGoomRand().ProbabilityOf<PROB_CHANGE_H_LINE_PARAMS>() or (farVal != 0))
  {
    param1    = NEW_PARAM1_FACTOR * m_screenHeight;
    param2    = NEW_PARAM2_FACTOR * m_screenHeight;
    amplitude = NEW_AMPLITUDE;
  }
  else
  {
    param1    = DEFAULT_PARAM1_FACTOR * m_screenHeight;
    param2    = DEFAULT_PARAM2_FACTOR * m_screenHeight;
    amplitude = DEFAULT_AMPLITUDE;
  }

  const auto [line1Color, line2Color] = GetResetLineColors(farVal);

  return {
      {{.lineType = LineType::H_LINE, .param = param1, .color = line1Color, .amplitude = amplitude},
       {.lineType  = LineType::H_LINE,
        .param     = param2,
        .color     = line2Color,
        .amplitude = amplitude}}
  };
}

auto LinesFx::LinesImpl::GetResetVerticalLineSettings(const uint32_t farVal) const noexcept
    -> std::array<LineParams, NUM_LINES>
{
  static constexpr auto NEW_PARAM1_FACTOR     = 1.0F / 7.0F;
  static constexpr auto NEW_PARAM2_FACTOR     = 6.0F / 7.0F;
  static constexpr auto DEFAULT_PARAM1_FACTOR = 1.0F / 2.0F;
  static constexpr auto DEFAULT_PARAM2_FACTOR = 1.0F / 2.0F;

  static constexpr auto NEW_AMPLITUDE     = 1.0F;
  static constexpr auto DEFAULT_AMPLITUDE = 1.5F;

  float amplitude; // NOLINT(cppcoreguidelines-init-variables)
  float param1; // NOLINT(cppcoreguidelines-init-variables)
  float param2; // NOLINT(cppcoreguidelines-init-variables)

  if (m_fxHelper->GetGoomRand().ProbabilityOf<PROB_CHANGE_V_LINE_PARAMS>() or (farVal != 0))
  {
    param1    = NEW_PARAM1_FACTOR * m_screenWidth;
    param2    = NEW_PARAM2_FACTOR * m_screenWidth;
    amplitude = NEW_AMPLITUDE;
  }
  else
  {
    param1    = DEFAULT_PARAM1_FACTOR * m_screenWidth;
    param2    = DEFAULT_PARAM2_FACTOR * m_screenWidth;
    amplitude = DEFAULT_AMPLITUDE;
  }

  const auto [line1Color, line2Color] = GetResetLineColors(farVal);

  return {
      {{.lineType = LineType::V_LINE, .param = param1, .color = line1Color, .amplitude = amplitude},
       {.lineType  = LineType::V_LINE,
        .param     = param2,
        .color     = line2Color,
        .amplitude = amplitude}}
  };
}

auto LinesFx::LinesImpl::GetResetLineColors(const uint32_t farVal) const noexcept
    -> std::array<Pixel, NUM_LINES>
{
  if ((farVal != 0) and m_fxHelper->GetGoomRand().ProbabilityOf<PROB_CHANGE_LINE_TO_BLACK>())
  {
    return {m_blackLineColor, m_blackLineColor};
  }

  return GetRandomLineColors();
}

/* arret aleatore.. changement de mode de ligne..
  */
auto LinesFx::LinesImpl::StopRandomLineChangeMode() noexcept -> void
{
  static constexpr auto DEC_LINE_MODE_CYCLES    = 80U;
  static constexpr auto UPDATE_LINE_MODE_CYCLES = 120U;

  if (m_lineMode != m_drawLinesDuration)
  {
    --m_lineMode;
    if (-1 == m_lineMode)
    {
      m_lineMode = 0;
    }
  }
  else if ((0 == (m_fxHelper->GetGoomTime().GetCurrentTime() % DEC_LINE_MODE_CYCLES)) and
           m_fxHelper->GetGoomRand().ProbabilityOf<PROB_REDUCE_LINE_MODE>() and (m_lineMode != 0))
  {
    --m_lineMode;
  }

  if ((0 == (m_fxHelper->GetGoomTime().GetCurrentTime() % UPDATE_LINE_MODE_CYCLES)) and
      m_fxHelper->GetGoomRand().ProbabilityOf<PROB_UPDATE_LINE_MODE>() and m_isNearScope)
  {
    if (0 == m_lineMode)
    {
      m_lineMode = m_drawLinesDuration;
    }
    else if ((m_lineMode == m_drawLinesDuration) and CanResetDestLines())
    {
      --m_lineMode;

      ResetGoomLines();
    }
  }
}

} // namespace GOOM::VISUAL_FX
