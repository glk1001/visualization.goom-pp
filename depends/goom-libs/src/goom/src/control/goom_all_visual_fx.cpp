#include "goom_all_visual_fx.h"

//#undef NO_LOGGING

#include "all_standard_visual_fx.h"
#include "color/color_maps.h"
#include "color/color_utils.h"
#include "draw/goom_draw.h"
#include "filter_fx/filter_buffers_service.h"
#include "filter_fx/filter_colors_service.h"
#include "goom_config.h"
#include "goom_plugin_info.h"
#include "logging.h"
#include "sound_info.h"
#include "utils/name_value_pairs.h"
#include "utils/stopwatch.h"
#include "visual_fx/fx_helper.h"
#include "visual_fx_color_maps.h"

#include <memory>

namespace GOOM::CONTROL
{

using COLOR::GetBrighterColorInt;
using COLOR::GetLuma;
using COLOR::IColorMap;
using CONTROL::GoomDrawables;
using DRAW::IGoomDraw;
using FILTER_FX::FilterBuffersService;
using FILTER_FX::FilterColorsService;
using FILTER_FX::ZoomFilterFx;
using FILTER_FX::ZoomFilterSettings;
using UTILS::Logging;
using UTILS::NameValuePairs;
using UTILS::Parallel;
using UTILS::Stopwatch;
using UTILS::GRAPHICS::SmallImageBitmaps;
using VISUAL_FX::FxHelper;
using VISUAL_FX::LinesFx;

static const Pixel RED_LINE = LinesFx::GetRedLineColor();
static const Pixel GREEN_LINE = LinesFx::GetGreenLineColor();
static const Pixel BLACK_LINE = LinesFx::GetBlackLineColor();

static constexpr float SMALL_LUMA = 0.1F;

GoomAllVisualFx::GoomAllVisualFx(Parallel& parallel,
                                 const FxHelper& fxHelper,
                                 const SmallImageBitmaps& smallBitmaps,
                                 const std::string& resourcesDirectory,
                                 IGoomStateHandler& goomStateHandler,
                                 std::unique_ptr<FilterBuffersService> filterBuffersService,
                                 std::unique_ptr<FilterColorsService> filterColorsService) noexcept
  : m_allStandardVisualFx{spimpl::make_unique_impl<AllStandardVisualFx>(
        parallel, fxHelper, smallBitmaps, resourcesDirectory)},
    m_zoomFilterFx{std::make_unique<ZoomFilterFx>(parallel,
                                                  fxHelper.GetGoomInfo(),
                                                  std::move(filterBuffersService),
                                                  std::move(filterColorsService))},
    m_goomLine1{std::make_unique<LinesFx>(
        fxHelper,
        smallBitmaps,
        LinesFx::LineType::H_LINE,
        static_cast<float>(fxHelper.GetGoomInfo().GetScreenInfo().height),
        BLACK_LINE,
        LinesFx::LineType::CIRCLE,
        INITIAL_SCREEN_HEIGHT_FRACTION_LINE1 *
            static_cast<float>(fxHelper.GetGoomInfo().GetScreenInfo().height),
        GREEN_LINE)},
    m_goomLine2{std::make_unique<LinesFx>(
        fxHelper,
        smallBitmaps,
        LinesFx::LineType::H_LINE,
        0.0F,
        BLACK_LINE,
        LinesFx::LineType::CIRCLE,
        INITIAL_SCREEN_HEIGHT_FRACTION_LINE2 *
            static_cast<float>(fxHelper.GetGoomInfo().GetScreenInfo().height),
        RED_LINE)},
    m_goomDraw{fxHelper.GetDraw()},
    m_goomRand{fxHelper.GetGoomRand()},
    m_goomStateHandler{goomStateHandler}
{
  m_allStandardVisualFx->SetResetDrawBuffSettingsFunc([this](const GoomDrawables fx)
                                                      { ResetCurrentDrawBuffSettings(fx); });
}

GoomAllVisualFx::~GoomAllVisualFx() noexcept = default;

void GoomAllVisualFx::Start()
{
  m_goomLine1->Start();
  m_goomLine2->Start();

  m_allStandardVisualFx->Start();
  m_adaptiveExposure.Start();
  m_zoomFilterFx->Start();
}

void GoomAllVisualFx::Finish()
{
  m_allStandardVisualFx->Finish();

  m_zoomFilterFx->Finish();

  m_goomLine1->Finish();
  m_goomLine2->Finish();
}

void GoomAllVisualFx::ChangeState()
{
  m_allStandardVisualFx->SuspendFx();

  static constexpr size_t MAX_TRIES = 10;
  const GoomStates oldState = m_goomStateHandler.GetCurrentState();

  for (size_t numTry = 0; numTry < MAX_TRIES; ++numTry)
  {
    m_goomStateHandler.ChangeToNextState();

    if ((not m_allowMultiThreadedStates) and
        GoomStateInfo::IsMultiThreaded(m_goomStateHandler.GetCurrentState()))
    {
      continue;
    }

    // Pick a different state if possible
    if (oldState != m_goomStateHandler.GetCurrentState())
    {
      break;
    }
  }

  m_currentGoomDrawables = m_goomStateHandler.GetCurrentDrawables();
  m_allStandardVisualFx->SetCurrentGoomDrawables(m_currentGoomDrawables);
  m_allStandardVisualFx->ChangeShaderEffects();

  m_allStandardVisualFx->ResumeFx();
}

void GoomAllVisualFx::StartExposureControl()
{
  m_doExposureControl = true;
}

auto GoomAllVisualFx::GetLastShaderEffects() const -> const GoomShaderEffects&
{
  return m_allStandardVisualFx->GetLastShaderEffects();
}

void GoomAllVisualFx::SetSingleBufferDots(const bool value)
{
  m_allStandardVisualFx->SetSingleBufferDots(value);
}

void GoomAllVisualFx::PostStateUpdate(const std::unordered_set<GoomDrawables>& oldGoomDrawables)
{
  m_allStandardVisualFx->PostStateUpdate(oldGoomDrawables);
}

void GoomAllVisualFx::RefreshAllFx()
{
  m_allStandardVisualFx->RefreshAllFx();
}

inline void GoomAllVisualFx::ResetCurrentDrawBuffSettings(const GoomDrawables fx)
{
  if (GoomDrawables::SHADER == fx)
  {
    return;
  }
  m_resetDrawBuffSettings(GetCurrentBuffSettings(fx));
}

inline auto GoomAllVisualFx::GetCurrentBuffSettings(const GoomDrawables fx) const -> FXBuffSettings
{
  const float buffIntensity = m_goomRand.GetRandInRange(
      GoomStateInfo::GetBuffIntensityRange(m_goomStateHandler.GetCurrentState(), fx));
  // Careful here. > 1 reduces smearing.
  static constexpr float INTENSITY_FACTOR = 1.0F;
  return {INTENSITY_FACTOR * buffIntensity};
}

void GoomAllVisualFx::ChangeAllFxColorMaps()
{
  m_allStandardVisualFx->ChangeColorMaps();
  ChangeLineColorMaps();
}

void GoomAllVisualFx::ChangeDrawPixelBlend()
{
  if (m_goomRand.ProbabilityOf(1.0F))
  {
    m_goomDraw.SetDefaultBlendPixelFunc();
  }
  else if (m_goomRand.ProbabilityOf(0.0F))
  {
    m_goomDraw.SetBlendPixelFunc(GetSameLumaBlendPixelFunc());
  }
  else if (m_goomRand.ProbabilityOf(0.0F))
  {
    m_goomDraw.SetBlendPixelFunc(GetSameLumaMixBlendPixelFunc());
  }
  else
  {
    m_goomDraw.SetBlendPixelFunc(GetReverseColorAddBlendPixelPixelFunc());
  }
}

auto GoomAllVisualFx::GetReverseColorAddBlendPixelPixelFunc() -> IGoomDraw::BlendPixelFunc
{
  return [](const Pixel& oldColor, const Pixel& newColor, const uint32_t intBuffIntensity)
  { return COLOR::GetColorAdd(COLOR::GetBrighterColorInt(intBuffIntensity, oldColor), newColor); };
}

auto GoomAllVisualFx::GetSameLumaBlendPixelFunc() -> IGoomDraw::BlendPixelFunc
{
  return [](const Pixel& oldColor, const Pixel& newColor, const uint32_t intBuffIntensity)
  {
    const float newColorLuma =
        GetLuma(newColor) * (static_cast<float>(intBuffIntensity) / channel_limits<float>::max());
    if (newColorLuma < SMALL_LUMA)
    {
      return COLOR::GetColorAdd(oldColor, newColor);
    }
    const float oldColorLuma = GetLuma(oldColor);
    const float brightness = 1.0F + (oldColorLuma / newColorLuma);

    const auto red = static_cast<uint32_t>(brightness * static_cast<float>(newColor.R()));
    const auto green = static_cast<uint32_t>(brightness * static_cast<float>(newColor.G()));
    const auto blue = static_cast<uint32_t>(brightness * static_cast<float>(newColor.B()));

    return Pixel{red, green, blue, MAX_ALPHA};
  };
}

auto GoomAllVisualFx::GetSameLumaMixBlendPixelFunc() -> IGoomDraw::BlendPixelFunc
{
  return [](const Pixel& oldColor, const Pixel& newColor, const uint32_t intBuffIntensity)
  {
    const float newColorLuma =
        GetLuma(newColor) * (static_cast<float>(intBuffIntensity) / channel_limits<float>::max());
    if (newColorLuma < SMALL_LUMA)
    {
      return COLOR::GetColorAdd(oldColor, newColor);
    }
    const float oldColorLuma = GetLuma(oldColor);
    const float brightness = 0.5F * (1.0F + (oldColorLuma / newColorLuma));

    const Pixel finalNewColor = IColorMap::GetColorMix(oldColor, newColor, 0.7F);
    const auto red = static_cast<uint32_t>(brightness * static_cast<float>(finalNewColor.R()));
    const auto green = static_cast<uint32_t>(brightness * static_cast<float>(finalNewColor.G()));
    const auto blue = static_cast<uint32_t>(brightness * static_cast<float>(finalNewColor.B()));

    return Pixel{red, green, blue, MAX_ALPHA};
  };
}

void GoomAllVisualFx::UpdateFilterSettings(const ZoomFilterSettings& filterSettings,
                                           const bool updateFilterEffects)
{
  if (updateFilterEffects)
  {
    m_zoomFilterFx->UpdateFilterEffectsSettings(filterSettings.filterEffectsSettings);
  }

  m_zoomFilterFx->UpdateFilterBufferSettings(filterSettings.filterBufferSettings);
  m_zoomFilterFx->UpdateFilterColorSettings(filterSettings.filterColorSettings);

  m_allStandardVisualFx->SetZoomMidpoint(filterSettings.filterEffectsSettings.zoomMidpoint);
}

void GoomAllVisualFx::ApplyCurrentStateToSingleBuffer()
{
  m_allStandardVisualFx->ApplyCurrentStateToSingleBuffer();
}

void GoomAllVisualFx::ApplyCurrentStateToMultipleBuffers()
{
  m_allStandardVisualFx->ApplyCurrentStateToMultipleBuffers();
}

auto GoomAllVisualFx::ApplyEndEffectIfNearEnd(const Stopwatch::TimeValues& timeValues) -> void
{
  m_allStandardVisualFx->ApplyEndEffectIfNearEnd(timeValues);
}

void GoomAllVisualFx::DisplayGoomLines(const AudioSamples& soundData)
{
  Expects(IsCurrentlyDrawable(GoomDrawables::LINES));

  m_goomLine2->SetLineColorPower(m_goomLine1->GetLineColorPower());

  m_goomLine1->DrawLines(soundData.GetSample(0), soundData.GetSampleMinMax(0));
  m_goomLine2->DrawLines(soundData.GetSample(1), soundData.GetSampleMinMax(1));
}

void GoomAllVisualFx::ChangeLineColorMaps()
{
  m_goomLine1->SetWeightedColorMaps(
      m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::LINES1));
  m_goomLine2->SetWeightedColorMaps(
      m_visualFxColorMaps.GetCurrentRandomColorMaps(GoomEffect::LINES2));
}

auto GoomAllVisualFx::GetCurrentColorMapsNames() const -> std::unordered_set<std::string>
{
  std::unordered_set<std::string> currentColorMapsNames{
      m_allStandardVisualFx->GetActiveColorMapsNames()};

  // TODO - clean this up.
  for (const auto& colorMapsName : m_goomLine1->GetCurrentColorMapsNames())
  {
    currentColorMapsNames.emplace(colorMapsName);
  }
  for (const auto& colorMapsName : m_goomLine2->GetCurrentColorMapsNames())
  {
    currentColorMapsNames.emplace(colorMapsName);
  }

  return m_allStandardVisualFx->GetActiveColorMapsNames();
}

auto GoomAllVisualFx::GetZoomFilterFxNameValueParams() const -> NameValuePairs
{
  return m_zoomFilterFx->GetNameValueParams();
}

} // namespace GOOM::CONTROL
