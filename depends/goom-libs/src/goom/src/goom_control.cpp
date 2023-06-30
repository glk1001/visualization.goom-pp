/**
* file: goom_control.c (onetime goom_core.c)
 * author: Jean-Christophe Hoelt (which is not so proud of it)
 *
 * Contains the core of goom's work.
 *
 * (c)2000-2003, by iOS-software.
 *
 *  - converted to C++17 2021-02-01 (glk)
 *
 */

#undef NO_LOGGING

#include "goom_control.h"

#include "format_utils.h"
[[nodiscard]] inline auto to_string(const GOOM::Pixel& pixel)
{
  return GOOM::UTILS::FMT::Rgba("{}", pixel);
}
#include "control/goom_all_visual_fx.h"
#include "control/goom_message_displayer.h"
#include "control/goom_music_settings_reactor.h"
#include "control/goom_random_state_handler.h"
#include "control/goom_sound_events.h"
#include "control/goom_state_monitor.h"
#include "control/goom_title_displayer.h"
#include "draw/goom_draw_to_buffer.h"
#include "filter_fx/filter_buffers.h"
#include "filter_fx/filter_buffers_service.h"
#include "filter_fx/filter_colors_service.h"
#include "filter_fx/filter_effects/zoom_in_coefficients_effect_factory.h"
#include "filter_fx/filter_settings_service.h"
#include "filter_fx/filter_zoom_vector.h"
#include "filter_fx/normalized_coords.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "goom_logger.h"
#include "goom_plugin_info.h"
#include "spimpl.h"
#include "utils/buffer_saver.h"
#include "utils/debugging_logger.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/graphics/test_patterns.h"
#include "utils/math/goom_rand.h"
#include "utils/parallel_utils.h"
#include "utils/stopwatch.h"
#include "utils/strutils.h"
#include "visual_fx/fx_helper.h"

#include <memory>
#include <string>
#include <utility>

#ifdef DO_GOOM_STATE_DUMP
#include "control/goom_state_dump.h"
#include "utils/propagate_const.h"
#endif

namespace GOOM
{

using CONTROL::GoomAllVisualFx;
using CONTROL::GoomMessageDisplayer;
using CONTROL::GoomMusicSettingsReactor;
using CONTROL::GoomRandomStateHandler;
using CONTROL::GoomSoundEvents;
#ifdef DO_GOOM_STATE_DUMP
using CONTROL::GoomStateDump;
#endif
using CONTROL::GoomStateMonitor;
using CONTROL::GoomTitleDisplayer;
using DRAW::GoomDrawToSingleBuffer;
using DRAW::GoomDrawToTwoBuffers;
using FILTER_FX::FilterBuffersService;
using FILTER_FX::FilterColorsService;
using FILTER_FX::FilterSettingsService;
using FILTER_FX::FilterZoomVector;
using FILTER_FX::NormalizedCoordsConverter;
using FILTER_FX::FILTER_EFFECTS::CreateZoomInCoefficientsEffect;
#ifdef DO_GOOM_STATE_DUMP
using std::experimental::propagate_const;
#endif
using UTILS::Parallel;
using UTILS::Stopwatch;
using UTILS::StringSplit;
using UTILS::GRAPHICS::DrawTestPattern;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::GoomRand;
using UTILS::MATH::IsBetween;
using VISUAL_FX::FxHelper;

class GoomControlLogger : public GoomLogger
{
public:
  using GoomLogger::GoomLogger;

  auto StartGoomControl(const GoomControl::GoomControlImpl* goomControl) noexcept -> void;
  auto StopGoomControl() noexcept -> void;

  [[nodiscard]] auto CanLog() const -> bool override;

private:
  const GoomControl::GoomControlImpl* m_goomControl = nullptr;
  static constexpr auto MIN_UPDATE_NUM_TO_LOG       = 0U;
  static constexpr auto MAX_UPDATE_NUM_TO_LOG       = 1000000000U;
};

class GoomControl::GoomControlImpl
{
public:
  GoomControlImpl(const Dimensions& dimensions,
                  const std::string& resourcesDirectory,
                  GoomLogger& goomLogger);

  [[nodiscard]] auto GetUpdateNum() const -> uint32_t;

  auto SetShowSongTitle(ShowSongTitleType value) -> void;

  auto Start() -> void;
  auto Finish() -> void;

  auto SetSongInfo(const SongInfo& songInfo) -> void;
  auto SetNoZooms(bool value) -> void;
  auto SetShowGoomState(bool value) -> void;
  auto SetDumpDirectory(const std::string& dumpDirectory) -> void;

  auto InitFrameData(std::vector<FrameData>& frameDataArray) noexcept -> void;
  auto SetFrameData(FrameData& frameData) -> void;
  auto UpdateGoomBuffers(const AudioSamples& soundData, const std::string& message) -> void;

  [[nodiscard]] auto GetLastShaderVariables() const -> const GoomShaderVariables&;

private:
  Parallel m_parallel{-1}; // max cores - 1
  SoundInfo m_soundInfo{};
  GoomSoundEvents m_goomSoundEvents{m_soundInfo};
  PluginInfo m_goomInfo;
  GoomControlLogger* m_goomLogger;
  GoomRand m_goomRand{};
  GoomDrawToTwoBuffers m_multiBufferDraw{m_goomInfo.GetDimensions(), *m_goomLogger};
  FxHelper m_fxHelper;

  bool m_noZooms         = false;
  uint32_t m_updateNum   = 0;
  FrameData* m_frameData = nullptr;
  PixelBuffer* m_p1      = nullptr;
  PixelBuffer* m_p2      = nullptr;
  static auto InitMiscData(MiscData& miscData) noexcept -> void;
  static auto InitImageArrays(ImageArrays& imageArrays) noexcept -> void;
  static auto InitFilterPosArrays(FilterPosArrays& filterPosArrays) noexcept -> void;

  //  static inline const auto saveDir = std::string{"/home/greg/.kodi/junk_cpu/"};
  //  UTILS::BufferSaver<Pixel> m_p1BufferSaver{saveDir + "filter_buff_p1_in_cpu"};
  //  UTILS::BufferSaver<Pixel> m_p2BufferSaver{saveDir + "filter_buff_p2_in_cpu"};

  FilterSettingsService m_filterSettingsService;

  SmallImageBitmaps m_smallBitmaps;
  GoomRandomStateHandler m_stateHandler{m_goomRand};
  NormalizedCoordsConverter m_normalizedCoordsConverter{
      {m_goomInfo.GetDimensions().GetWidth(), m_goomInfo.GetDimensions().GetHeight()},
      FILTER_FX::FILTER_BUFFERS::MIN_SCREEN_COORD_ABS_VAL
  };
  GoomAllVisualFx m_visualFx;

  GoomMusicSettingsReactor m_musicSettingsReactor{
      m_goomInfo, m_goomRand, m_visualFx, m_filterSettingsService};

  auto NewCycle() -> void;
  auto ProcessAudio(const AudioSamples& soundData) -> void;
  auto UseMusicToChangeSettings() -> void;
  auto ResetDrawBuffSettings(const FXBuffSettings& settings) -> void;

  auto DrawAndZoom(const AudioSamples& soundData) -> void;
  auto ApplyStateToMultipleBuffers(const AudioSamples& soundData) -> void;
  auto ApplyZoomEffects() -> void;
  auto ApplyEndEffectIfNearEnd() -> void;

  auto UpdateFilterSettings() -> void;

  auto UpdateBuffers() -> void;
  auto RotateBuffers() -> void;

  Stopwatch m_runningTimeStopwatch{};
  static constexpr auto DEFAULT_NUM_UPDATES_BETWEEN_TIME_CHECKS = 8U;
  uint32_t m_numUpdatesBetweenTimeChecks            = DEFAULT_NUM_UPDATES_BETWEEN_TIME_CHECKS;
  static constexpr float UPDATE_TIME_ESTIMATE_IN_MS = 40.0F;
  static constexpr float UPDATE_TIME_SAFETY_FACTOR  = 10.0F;
  float m_upperLimitOfTimeIntervalInMsSinceLastMarked =
      UPDATE_TIME_SAFETY_FACTOR *
      (static_cast<float>(m_numUpdatesBetweenTimeChecks) * UPDATE_TIME_ESTIMATE_IN_MS);
  auto UpdateTime() -> void;

#ifdef DO_GOOM_STATE_DUMP
  propagate_const<std::unique_ptr<GoomStateDump>> m_goomStateDump{};
  std::string m_dumpDirectory{};
  auto StartGoomStateDump() -> void;
  auto UpdateGoomStateDump() -> void;
  auto FinishGoomStateDump() -> void;
#endif
  GoomStateMonitor m_goomStateMonitor{m_visualFx, m_musicSettingsReactor, m_filterSettingsService};
  bool m_showGoomState = false;
  auto DisplayGoomState() -> void;
  [[nodiscard]] auto GetGoomTimeInfo() -> std::string;

  SongInfo m_songInfo{};
  ShowSongTitleType m_showTitle = ShowSongTitleType::AT_START;
  GoomDrawToSingleBuffer m_goomTextOutput{m_goomInfo.GetDimensions(), *m_goomLogger};
  GoomTitleDisplayer m_goomTitleDisplayer;
  GoomMessageDisplayer m_messageDisplayer;
  [[nodiscard]] static auto GetMessagesFontFile(const std::string& resourcesDirectory)
      -> std::string;
  [[nodiscard]] static auto GetFontDirectory(const std::string& resourcesDirectory) -> std::string;
  auto InitTitleDisplay() -> void;
  auto DisplayTitleAndMessages(const std::string& message) -> void;
  auto DisplayCurrentTitle() -> void;
  auto UpdateMessages(const std::string& messages) -> void;
};

GoomControl::GoomControl(const Dimensions& dimensions,
                         const std::string& resourcesDirectory,
                         GoomLogger& goomLogger)
  : m_pimpl{spimpl::make_unique_impl<GoomControlImpl>(dimensions, resourcesDirectory, goomLogger)}
{
}

auto GoomControl::SetShowSongTitle(const ShowSongTitleType value) -> void
{
  m_pimpl->SetShowSongTitle(value);
}

auto GoomControl::SetShowGoomState(const bool value) -> void
{
  m_pimpl->SetShowGoomState(value);
}

auto GoomControl::SetDumpDirectory(const std::string& dumpDirectory) -> void
{
  m_pimpl->SetDumpDirectory(dumpDirectory);
}

auto GoomControl::Start() -> void
{
  m_pimpl->Start();
}

auto GoomControl::Finish() -> void
{
  m_pimpl->Finish();
}

auto GoomControl::SetSongInfo(const SongInfo& songInfo) -> void
{
  m_pimpl->SetSongInfo(songInfo);
}

auto GoomControl::SetNoZooms(const bool value) -> void
{
  m_pimpl->SetNoZooms(value);
}

auto GoomControl::InitFrameData(std::vector<FrameData>& frameDataArray) noexcept -> void
{
  m_pimpl->InitFrameData(frameDataArray);
}

auto GoomControl::SetFrameData(FrameData& frameData) -> void
{
  m_pimpl->SetFrameData(frameData);
}

auto GoomControl::UpdateGoomBuffers(const AudioSamples& audioSamples, const std::string& message)
    -> void
{
  m_pimpl->UpdateGoomBuffers(audioSamples, message);
}

auto GoomControl::GetLastShaderVariables() const -> const GoomShaderVariables&
{
  return m_pimpl->GetLastShaderVariables();
}

auto GoomControlLogger::StartGoomControl(
    const GoomControl::GoomControlImpl* const goomControl) noexcept -> void
{
  m_goomControl = goomControl;
}

auto GoomControlLogger::StopGoomControl() noexcept -> void
{
  Flush();
  m_goomControl = nullptr;
}

auto GoomControlLogger::CanLog() const -> bool
{
  return ((nullptr == m_goomControl) or
          IsBetween(m_goomControl->GetUpdateNum(), MIN_UPDATE_NUM_TO_LOG, MAX_UPDATE_NUM_TO_LOG));
}

auto GoomControl::MakeGoomLogger() noexcept -> std::unique_ptr<GoomLogger>
{
  return std::make_unique<GoomControlLogger>();
}

GoomControl::GoomControlImpl::GoomControlImpl(const Dimensions& dimensions,
                                              const std::string& resourcesDirectory,
                                              GoomLogger& goomLogger)
  : m_goomInfo{dimensions, m_goomSoundEvents},
    m_goomLogger{&dynamic_cast<GoomControlLogger&>(goomLogger)},
    m_fxHelper{&m_multiBufferDraw, &m_goomInfo, &m_goomRand, m_goomLogger},
    m_filterSettingsService{
        m_goomInfo, m_goomRand, resourcesDirectory, CreateZoomInCoefficientsEffect},
    m_smallBitmaps{resourcesDirectory},
    m_visualFx{m_parallel,
               m_fxHelper,
               m_smallBitmaps,
               resourcesDirectory,
               m_stateHandler,
               std::make_unique<FilterBuffersService>(
                   m_parallel,
                   m_goomInfo,
                   m_normalizedCoordsConverter,
                   std::make_unique<FilterZoomVector>(m_goomInfo.GetDimensions().GetWidth(),
                                                      resourcesDirectory,
                                                      m_goomRand,
                                                      m_normalizedCoordsConverter)),
               std::make_unique<FilterColorsService>(m_goomRand)},
    m_goomTitleDisplayer{m_goomTextOutput, m_goomRand, GetFontDirectory(resourcesDirectory)},
    m_messageDisplayer{m_goomTextOutput, GetMessagesFontFile(resourcesDirectory)}
{
  //RotateBuffers();
  UTILS::SetGoomLogger(*m_goomLogger);
}

inline auto GoomControl::GoomControlImpl::GetUpdateNum() const -> uint32_t
{
  return m_updateNum;
}

inline auto GoomControl::GoomControlImpl::SetShowSongTitle(const ShowSongTitleType value) -> void
{
  m_showTitle = value;
}

inline auto GoomControl::GoomControlImpl::InitFrameData(
    std::vector<FrameData>& frameDataArray) noexcept -> void
{
  for (auto& frameData : frameDataArray)
  {
    InitMiscData(frameData.miscData);
    InitImageArrays(frameData.imageArrays);
    InitFilterPosArrays(frameData.filterPosArrays);
  }
}

auto GoomControl::GoomControlImpl::InitMiscData(GOOM::MiscData& miscData) noexcept -> void
{
  miscData.lerpFactor = 0.0F;
  miscData.brightness = 0.5F;
}

auto GoomControl::GoomControlImpl::InitImageArrays(GOOM::ImageArrays& imageArrays) noexcept -> void
{
  imageArrays.mainImageDataNeedsUpdating = false;
  imageArrays.lowImageDataNeedsUpdating  = false;
}

auto GoomControl::GoomControlImpl::InitFilterPosArrays(
    GOOM::FilterPosArrays& filterPosArrays) noexcept -> void
{
  filterPosArrays.filterDestPosNeedsUpdating    = false;
  filterPosArrays.lerpFactorForDestToSrceUpdate = 0.0F;
}

inline auto GoomControl::GoomControlImpl::SetFrameData(FrameData& frameData) -> void
{
  m_frameData = &frameData;
  m_p1        = &m_frameData->imageArrays.mainImageData;
  m_p2        = &m_frameData->imageArrays.lowImageData;

  m_p1->Fill(BLACK_PIXEL);
  m_p2->Fill(BLACK_PIXEL);

  m_multiBufferDraw.SetBuffers(*m_p1, *m_p2);

  m_frameData->imageArrays.mainImageDataNeedsUpdating = true;
  m_frameData->imageArrays.lowImageDataNeedsUpdating  = true;
  m_frameData->miscData.brightness                    = 1.0F;

  using FilterBuffers          = FILTER_FX::ZoomFilterBuffers<FILTER_FX::ZoomFilterBufferStriper>;
  const auto currentLerpFactor = static_cast<float>(m_visualFx.GetTranLerpFactor()) /
                                 static_cast<float>(FilterBuffers::GetMaxTranLerpFactor());

  if (not m_visualFx.IsTranBufferFltReady())
  {
    m_frameData->filterPosArrays.filterDestPosNeedsUpdating = false;
    m_frameData->miscData.lerpFactor                        = currentLerpFactor;
  }
  else
  {
    LogInfo(*m_goomLogger, "Filter dest needs updating. Data passed on.");
    m_visualFx.CopyTranBufferFlt(m_frameData->filterPosArrays.filterDestPos);
    m_frameData->filterPosArrays.filterDestPosNeedsUpdating    = true;
    m_frameData->filterPosArrays.lerpFactorForDestToSrceUpdate = currentLerpFactor;
    m_frameData->miscData.lerpFactor                           = 0.0F;
  }

  //  LogInfo(*m_goomLogger,
  //          "FrameData lerpFactor = {} ({}), currentLerpFactor = {}.",
  //          m_visualFx.GetTranLerpFactor(),
  //          m_frameData->miscData.lerpFactor,
  //          currentLerpFactor);
}

inline auto GoomControl::GoomControlImpl::SetNoZooms(const bool value) -> void
{
  m_noZooms = value;
}

inline auto GoomControl::GoomControlImpl::SetShowGoomState(const bool value) -> void
{
  m_showGoomState = value;
}

#ifdef DO_GOOM_STATE_DUMP
inline auto GoomControl::GoomControlImpl::SetDumpDirectory(const std::string& dumpDirectory) -> void
{
  m_dumpDirectory = dumpDirectory;
}
#else
inline auto GoomControl::GoomControlImpl::SetDumpDirectory(
    [[maybe_unused]] const std::string& dumpDirectory) -> void
{
  // #define not set
}
#endif

inline auto GoomControl::GoomControlImpl::GetLastShaderVariables() const
    -> const GoomShaderVariables&
{
  return m_visualFx.GetLastShaderVariables();
}

inline auto GoomControl::GoomControlImpl::Start() -> void
{
  Expects(m_frameData != nullptr);

  m_goomLogger->StartGoomControl(this);

  m_filterSettingsService.Start();

  m_visualFx.SetAllowMultiThreadedStates(false);
  m_visualFx.SetResetDrawBuffSettingsFunc([this](const FXBuffSettings& settings)
                                          { ResetDrawBuffSettings(settings); });
  m_visualFx.ChangeAllFxColorMaps();

  UpdateFilterSettings();
  m_visualFx.Start();

  m_musicSettingsReactor.Start();

  m_visualFx.StartExposureControl();

#ifdef DO_GOOM_STATE_DUMP
  StartGoomStateDump();
#endif

  m_runningTimeStopwatch.SetUpperLimitOfTimeIntervalInMsSinceLastMarked(
      m_upperLimitOfTimeIntervalInMsSinceLastMarked);
  static constexpr auto START_TIME_DELAY_IN_MS = 457.0F;
  m_runningTimeStopwatch.SetStartDelayAdjustInMs(START_TIME_DELAY_IN_MS);
  m_runningTimeStopwatch.StartNow();

  m_numUpdatesBetweenTimeChecks = DEFAULT_NUM_UPDATES_BETWEEN_TIME_CHECKS;
  m_updateNum                   = 0;
}

inline auto GoomControl::GoomControlImpl::Finish() -> void
{
  LogInfo(*m_goomLogger,
          "Stopping now. Time remaining = {}, {}%%",
          m_runningTimeStopwatch.GetTimeValues().timeRemainingInMs,
          m_runningTimeStopwatch.GetTimeValues().timeRemainingAsPercent);

#ifdef DO_GOOM_STATE_DUMP
  FinishGoomStateDump();
#endif

  m_visualFx.Finish();

  m_goomLogger->StopGoomControl();
}

#ifdef DO_GOOM_STATE_DUMP
inline auto GoomControl::GoomControlImpl::StartGoomStateDump() -> void
{
  m_goomStateDump = std::make_unique<GoomStateDump>(
      m_goomInfo, *m_goomLogger, m_visualFx, m_musicSettingsReactor, m_filterSettingsService);
  m_goomStateDump->Start();
}

inline auto GoomControl::GoomControlImpl::UpdateGoomStateDump() -> void
{
  m_goomStateDump->AddCurrentState();
}

inline auto GoomControl::GoomControlImpl::FinishGoomStateDump() -> void
{
  if (m_dumpDirectory.empty())
  {
    return;
  }
  m_goomStateDump->SetSongTitle(m_songInfo.title);
  m_goomStateDump->SetGoomSeed(UTILS::MATH::RAND::GetRandSeed());
  m_goomStateDump->SetStopWatch(m_runningTimeStopwatch);
  m_goomStateDump->DumpData(m_dumpDirectory);
}
#endif

inline auto GoomControl::GoomControlImpl::GetFontDirectory(const std::string& resourcesDirectory)
    -> std::string
{
  return resourcesDirectory + PATH_SEP + FONTS_DIR;
}

inline auto GoomControl::GoomControlImpl::GetMessagesFontFile(const std::string& resourcesDirectory)
    -> std::string
{
  return GetFontDirectory(resourcesDirectory) + PATH_SEP + "verdana.ttf";
}

inline auto GoomControl::GoomControlImpl::UpdateGoomBuffers(const AudioSamples& soundData,
                                                            const std::string& message) -> void
{
  NewCycle();

  // Elargissement de l'intervalle d'évolution des points!
  // Calcul du deplacement des petits points ...
  // Widening of the interval of evolution of the points!
  // Calculation of the displacement of small points ...

  ProcessAudio(soundData);

  UseMusicToChangeSettings();

  UpdateFilterSettings();

  DrawAndZoom(soundData);

  UpdateBuffers();

  DisplayGoomState();
  DisplayTitleAndMessages(message);

#ifdef DO_GOOM_STATE_DUMP
  UpdateGoomStateDump();
#endif
}

inline auto GoomControl::GoomControlImpl::NewCycle() -> void
{
  ++m_updateNum;

  m_visualFx.SetAllowMultiThreadedStates(m_goomTitleDisplayer.IsFinished());
  m_musicSettingsReactor.NewCycle();
  m_filterSettingsService.NewCycle();

  UpdateTime();
}

inline auto GoomControl::GoomControlImpl::SetSongInfo(const SongInfo& songInfo) -> void
{
  Expects(not songInfo.title.empty());
  Expects(songInfo.duration > 0);

  m_songInfo = songInfo;

  static constexpr auto MS_PER_SECOND = 1000U;
  m_runningTimeStopwatch.SetDuration(MS_PER_SECOND * m_songInfo.duration);
  m_runningTimeStopwatch.MarkTimeNow();
  m_runningTimeStopwatch.DoUpperLimitOfTimeIntervalCheck(true);

  InitTitleDisplay();
}

auto GoomControl::GoomControlImpl::UpdateTime() -> void
{
  if (0 == m_songInfo.duration)
  {
    return;
  }
  if (0 != (m_updateNum % m_numUpdatesBetweenTimeChecks))
  {
    return;
  }

  m_runningTimeStopwatch.MarkTimeNow();

  if (static constexpr auto ACCURACY_CUTOFF_MS = 10000.0F;
      m_runningTimeStopwatch.GetTimeValues().timeRemainingInMs < ACCURACY_CUTOFF_MS)
  {
    m_numUpdatesBetweenTimeChecks = 1;
    m_runningTimeStopwatch.DoUpperLimitOfTimeIntervalCheck(false);
  }
}

inline auto GoomControl::GoomControlImpl::UseMusicToChangeSettings() -> void
{
  m_musicSettingsReactor.ChangeFilterModeIfMusicChanges();
  m_musicSettingsReactor.BigUpdateIfNotLocked();
  m_musicSettingsReactor.BigBreakIfMusicIsCalm();

  m_musicSettingsReactor.RegularlyLowerTheSpeed();

  m_musicSettingsReactor.ChangeZoomEffects();
}

inline auto GoomControl::GoomControlImpl::DrawAndZoom(const AudioSamples& soundData) -> void
{
  ApplyZoomEffects();

  ApplyStateToMultipleBuffers(soundData);

  ApplyEndEffectIfNearEnd();
}

inline auto GoomControl::GoomControlImpl::ProcessAudio(const AudioSamples& soundData) -> void
{
  /* ! etude du signal ... */
  m_soundInfo.ProcessSample(soundData);
  m_goomSoundEvents.Update();
}

inline auto GoomControl::GoomControlImpl::ApplyStateToMultipleBuffers(
    [[maybe_unused]] const AudioSamples& soundData) -> void
{
  m_visualFx.ApplyCurrentStateToMultipleBuffers(soundData);

  //  auto draw = DRAW::GoomDrawToSingleBuffer{m_goomInfo.GetDimensions(), *m_goomLogger};
  //  draw.SetBuffer(*m_p2);
  //  DrawTestPattern(
  //      draw,
  //      {m_goomInfo.GetDimensions().GetIntWidth() / 2, m_goomInfo.GetDimensions().GetIntHeight() / 2},
  //      {m_goomInfo.GetDimensions().GetWidth() / 2, m_goomInfo.GetDimensions().GetHeight() / 2});

  //  for (auto y = 0; y < m_goomInfo.GetDimensions().GetIntHeight(); ++y)
  //  {
  //    for (auto x = 0; x < m_goomInfo.GetDimensions().GetIntWidth(); ++x)
  //    {
  //      (*m_p2)(x, y) =
  //          Pixel{static_cast<uint16_t>(x),
  //                static_cast<uint16_t>(m_goomInfo.GetDimensions().GetIntHeight() - 1 - y),
  //                0,
  //                MAX_ALPHA};
  //    }
  //  }
}

inline auto GoomControl::GoomControlImpl::ApplyEndEffectIfNearEnd() -> void
{
  if (not m_runningTimeStopwatch.AreTimesValid())
  {
    return;
  }
  m_visualFx.ApplyEndEffectIfNearEnd(m_runningTimeStopwatch.GetTimeValues());
}

inline auto GoomControl::GoomControlImpl::ApplyZoomEffects() -> void
{
  if (m_noZooms)
  {
    return;
  }

  m_visualFx.ApplyZoom(*m_p1, *m_p2);

  //  auto p1BufferView = UTILS::BufferView<Pixel>{m_p1->GetWidth() * m_p1->GetHeight(),
  //                                               reinterpret_cast<const Pixel*>(m_p1->GetBuffPtr())};
  //  LogInfo(*m_goomLogger, "Saving p1 buffer of length {}.", p1BufferView.GetBufferLen());
  //  auto getImageBufferIndexString = std::function<std::string(size_t bufferIndex)>{
  //      [width = m_goomInfo.GetDimensions().GetIntWidth()](const size_t bufferIndex)
  //      { return UTILS::ImageBufferIndexToString(width, bufferIndex); }};
  //  m_p1BufferSaver.SetBufferIndexFormatter(getImageBufferIndexString);
  //  m_p1BufferSaver.Write(p1BufferView, false);
  //
  //  auto p2BufferView = UTILS::BufferView<Pixel>{m_p2->GetWidth() * m_p2->GetHeight(),
  //                                               reinterpret_cast<const Pixel*>(m_p2->GetBuffPtr())};
  //  LogInfo(*m_goomLogger, "Saving p2 buffer of length {}.", p2BufferView.GetBufferLen());
  //  m_p2BufferSaver.SetBufferIndexFormatter(getImageBufferIndexString);
  //  m_p2BufferSaver.Write(p2BufferView, false);
}

inline auto GoomControl::GoomControlImpl::UpdateFilterSettings() -> void
{
  const auto& newFilterSettings = std::as_const(m_filterSettingsService).GetFilterSettings();
  m_visualFx.UpdateFilterSettings(newFilterSettings);
  m_filterSettingsService.NotifyUpdatedFilterEffectsSettings();
}

inline auto GoomControl::GoomControlImpl::ResetDrawBuffSettings(const FXBuffSettings& settings)
    -> void
{
  m_multiBufferDraw.SetBuffIntensity(settings.buffIntensity);
}

inline auto GoomControl::GoomControlImpl::UpdateBuffers() -> void
{
  // affichage et swappage des buffers...
  //m_p1->CopyTo(*m_outputBuff);

  RotateBuffers();

  auto draw = DRAW::GoomDrawToSingleBuffer{m_goomInfo.GetDimensions(), *m_goomLogger};
  draw.SetBuffer(*m_p1);
  //  DrawTestPattern(
  //      draw,
  //      {m_goomInfo.GetDimensions().GetIntWidth() / 2, m_goomInfo.GetDimensions().GetIntHeight() / 2},
  //      m_goomInfo.GetDimensions());
}

inline auto GoomControl::GoomControlImpl::RotateBuffers() -> void
{
  Expects(m_p1 != nullptr);
  Expects(m_p2 != nullptr);
  //std::swap(m_p1, m_p2);
  m_multiBufferDraw.SetBuffers(*m_p1, *m_p2);
}

inline auto GoomControl::GoomControlImpl::DisplayTitleAndMessages(const std::string& message)
    -> void
{
  UpdateMessages(message);

  if (m_showTitle == ShowSongTitleType::NEVER)
  {
    return;
  }

  DisplayCurrentTitle();
}

inline auto GoomControl::GoomControlImpl::InitTitleDisplay() -> void
{
  const auto xPosFraction = m_showTitle == ShowSongTitleType::ALWAYS ? 0.050F : 0.085F;
  const auto yPosFraction = m_showTitle == ShowSongTitleType::ALWAYS ? 0.130F : 0.300F;
  const auto xPos = static_cast<int>(xPosFraction * m_goomInfo.GetDimensions().GetFltWidth());
  const auto yPos = static_cast<int>(yPosFraction * m_goomInfo.GetDimensions().GetFltHeight());

  m_goomTitleDisplayer.SetInitialPosition(xPos, yPos);
}

inline auto GoomControl::GoomControlImpl::DisplayCurrentTitle() -> void
{
  if (m_songInfo.title.empty())
  {
    return;
  }

  if (m_showTitle == ShowSongTitleType::ALWAYS)
  {
    m_goomTextOutput.SetBuffer(*m_p1);
    m_goomTitleDisplayer.DrawStaticText(m_songInfo.title);
    return;
  }

  if (m_goomTitleDisplayer.IsFinished())
  {
    return;
  }

  if (m_goomTitleDisplayer.IsFinalPhase())
  {
    static constexpr auto FINAL_TITLE_BUFF_INTENSITY = 0.2F;
    m_goomTextOutput.SetBuffIntensity(FINAL_TITLE_BUFF_INTENSITY);
    m_goomTextOutput.SetBuffer(*m_p1);
  }
  else
  {
    m_goomTextOutput.SetBuffer(*m_p1);
  }
  m_goomTitleDisplayer.DrawMovingText(m_songInfo.title);
}

/*
 * Met a jour l'affichage du message defilant
 */
auto GoomControl::GoomControlImpl::UpdateMessages(const std::string& messages) -> void
{
  if (messages.empty())
  {
    return;
  }

  m_goomTextOutput.SetBuffer(*m_p1);

  m_messageDisplayer.UpdateMessages(StringSplit(messages, "\n"));
}

auto GoomControl::GoomControlImpl::DisplayGoomState() -> void
{
  if (!m_showGoomState)
  {
    return;
  }

  const std::string message = GetGoomTimeInfo() + "\n" + m_goomStateMonitor.GetCurrentState();

  UpdateMessages(message);
}

inline auto GoomControl::GoomControlImpl::GetGoomTimeInfo() -> std::string
{
  const auto timeLeftStr =
      not m_runningTimeStopwatch.AreTimesValid()
          ? "Time left: not valid!"
          : std_fmt::format("Time left: {}  ({}%)",
                            m_runningTimeStopwatch.GetTimeValues().timeRemainingInMs,
                            m_runningTimeStopwatch.GetTimeValues().timeRemainingAsPercent);

  return timeLeftStr + "\n" + std_fmt::format("Update Num: {}", m_updateNum);
}

} // namespace GOOM
