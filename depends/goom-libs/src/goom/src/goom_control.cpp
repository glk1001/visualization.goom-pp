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

#undef NO_LOGGING // NOLINT: This maybe be defined on command line.

#include "goom/goom_control.h"

#include "control/goom_all_visual_fx.h"
#include "control/goom_message_displayer.h"
#include "control/goom_music_settings_reactor.h"
#include "control/goom_random_state_handler.h"
#include "control/goom_sound_events.h"
#include "control/goom_state_monitor.h"
#include "control/goom_title_displayer.h"
#include "draw/goom_draw_to_buffer.h"
#include "filter_fx/filter_buffers_service.h"
#include "filter_fx/filter_effects/zoom_adjustment_effect_factory.h"
#include "filter_fx/filter_settings_service.h"
#include "filter_fx/filter_zoom_vector.h"
#include "filter_fx/normalized_coords.h"
#include "goom/frame_data.h"
#include "goom/goom_config.h"
#include "goom/goom_graphic.h"
#include "goom/goom_logger.h"
#include "goom/goom_time.h"
#include "goom/goom_types.h"
#include "goom/point2d.h"
#include "goom/sound_info.h"
#include "goom/spimpl.h"
#include "goom_plugin_info.h"
#include "utils/debugging_logger.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/goom_rand.h"
#include "utils/name_value_pairs.h"
#include "utils/parallel_utils.h"
#include "utils/stopwatch.h"
#include "utils/strutils.h"
#include "visual_fx/fx_helper.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <format> // NOLINT: Waiting to use C++20.
#include <memory>
#include <string>
#include <utility>

#ifdef DO_GOOM_STATE_DUMP
#include "control/goom_state_dump.h"
#include "utils/math/randutils.h"
#endif

namespace GOOM
{
#ifdef DO_GOOM_STATE_DUMP
using CONTROL::GoomStateDump;
#endif

using CONTROL::GoomAllVisualFx;
using CONTROL::GoomMessageDisplayer;
using CONTROL::GoomMusicSettingsReactor;
using CONTROL::GoomRandomStateHandler;
using CONTROL::GoomSoundEvents;
using CONTROL::GoomStateMonitor;
using CONTROL::GoomTitleDisplayer;
using DRAW::GoomDrawToSingleBuffer;
using DRAW::GoomDrawToTwoBuffers;
using FILTER_FX::FilterBuffersService;
using FILTER_FX::FilterSettingsService;
using FILTER_FX::FilterZoomVector;
using FILTER_FX::NormalizedCoordsConverter;
using FILTER_FX::FILTER_EFFECTS::CreateZoomAdjustmentEffect;
using UTILS::GetNameValuesString;
using UTILS::GetNumAvailablePoolThreads;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::Parallel;
using UTILS::Stopwatch;
using UTILS::StringSplit;
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
  static constexpr auto MIN_UPDATE_NUM_TO_LOG       = static_cast<uint64_t>(0);
  static constexpr auto MAX_UPDATE_NUM_TO_LOG       = static_cast<uint64_t>(1000000000);
};

class GoomControl::GoomControlImpl
{
public:
  GoomControlImpl(const GoomControl& parentGoomControl,
                  const Dimensions& dimensions,
                  const std::string& resourcesDirectory,
                  GoomLogger& goomLogger);

  [[nodiscard]] auto GetUpdateNum() const -> uint64_t;

  auto SetShowSongTitle(ShowSongTitleType value) -> void;

  auto Start() -> void;
  auto Finish() -> void;

  auto SetSongInfo(const SongInfo& songInfo) -> void;
  auto SetNoZooms(bool value) -> void;
  auto SetShowGoomState(bool value) -> void;
  auto SetDumpDirectory(const std::string& dumpDirectory) -> void;

  auto SetFrameData(FrameData& frameData) -> void;
  auto UpdateGoomBuffers(const AudioSamples& soundData, const std::string& message) -> void;

  [[nodiscard]] auto GetFrameData() const noexcept -> const FrameData&;
  [[nodiscard]] auto GetNumPoolThreads() const noexcept -> size_t;

private:
  [[maybe_unused]] const GoomControl* m_parentGoomControl;
  Parallel m_parallel{GetNumAvailablePoolThreads()};
  GoomTime m_goomTime{};
  SoundInfo m_soundInfo{};
  GoomSoundEvents m_goomSoundEvents{m_goomTime, m_soundInfo};
  PluginInfo m_goomInfo;
  GoomControlLogger* m_goomLogger;
  GoomRand m_goomRand{};
  GoomDrawToTwoBuffers m_multiBufferDraw{m_goomInfo.GetDimensions(), *m_goomLogger};
  FxHelper m_fxHelper;

  FrameData* m_frameData            = nullptr;
  float m_transformBufferLerpFactor = 0.0F;
  auto UpdateFrameData() -> void;
  auto UpdateFrameDataFilterSrcePosBuffer() const noexcept -> void;
  auto UpdateFrameDataFilterDestPosBuffer() noexcept -> void;

  bool m_noZooms    = false;
  PixelBuffer* m_p1 = nullptr;
  PixelBuffer* m_p2 = nullptr;

  FilterSettingsService m_filterSettingsService;
  FilterBuffersService m_filterBuffersService;
  auto StartFilterServices() noexcept -> void;
  auto UpdateFilterSettings() -> void;

  GoomMusicSettingsReactor m_musicSettingsReactor{
      m_goomInfo, m_goomRand, m_visualFx, m_filterSettingsService};

  SmallImageBitmaps m_smallBitmaps;
  GoomRandomStateHandler m_stateHandler{m_goomRand};
  NormalizedCoordsConverter m_normalizedCoordsConverter{
      {m_goomInfo.GetDimensions().GetWidth(), m_goomInfo.GetDimensions().GetHeight()}
  };
  GoomAllVisualFx m_visualFx;
  auto StartVisualFx() noexcept -> void;

  auto NewCycle() -> void;
  auto ProcessAudio(const AudioSamples& soundData) -> void;
  auto UseMusicToChangeSettings() -> void;
  auto ResetDrawBuffSettings(const FXBuffSettings& settings) -> void;

  auto ApplyStateToImageBuffers(const AudioSamples& soundData) -> void;
  auto UpdateTransformBuffer() -> void;
  auto ApplyEndEffectIfNearEnd() -> void;

  Stopwatch m_runningTimeStopwatch{};
  static constexpr auto DEFAULT_NUM_UPDATES_BETWEEN_TIME_CHECKS = 8U;
  uint32_t m_numUpdatesBetweenTimeChecks            = DEFAULT_NUM_UPDATES_BETWEEN_TIME_CHECKS;
  static constexpr float UPDATE_TIME_ESTIMATE_IN_MS = 40.0F;
  static constexpr float UPDATE_TIME_SAFETY_FACTOR  = 10.0F;
  float m_upperLimitOfTimeIntervalInMsSinceLastMarked =
      UPDATE_TIME_SAFETY_FACTOR *
      (static_cast<float>(m_numUpdatesBetweenTimeChecks) * UPDATE_TIME_ESTIMATE_IN_MS);
  auto UpdateTimeDependencies() -> void;

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
  auto GetNameValueParams() const -> NameValuePairs;

#ifdef DO_GOOM_STATE_DUMP
  std::unique_ptr<GoomStateDump> m_goomStateDump{};
  std::string m_dumpDirectory{};
  auto StartGoomStateDump() -> void;
  auto UpdateGoomStateDump() -> void;
  auto FinishGoomStateDump() -> void;
#endif
  GoomStateMonitor m_goomStateMonitor{
      m_visualFx, m_musicSettingsReactor, m_filterSettingsService, m_filterBuffersService};
  bool m_showGoomState = false;
  auto DisplayGoomState() -> void;
  [[nodiscard]] auto GetGoomTimeInfo() -> std::string;
};

GoomControl::GoomControl(const Dimensions& dimensions,
                         const std::string& resourcesDirectory,
                         GoomLogger& goomLogger)
  : m_pimpl{spimpl::make_unique_impl<GoomControlImpl>(
        *this, dimensions, resourcesDirectory, goomLogger)}
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

auto GoomControl::SetFrameData(FrameData& frameData) -> void
{
  m_pimpl->SetFrameData(frameData);
}

auto GoomControl::UpdateGoomBuffers(const AudioSamples& audioSamples, const std::string& message)
    -> void
{
  m_pimpl->UpdateGoomBuffers(audioSamples, message);
}

auto GoomControl::GetFrameData() const noexcept -> const FrameData&
{
  return m_pimpl->GetFrameData();
}

auto GoomControl::GetNumPoolThreads() const noexcept -> size_t
{
  return m_pimpl->GetNumPoolThreads();
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

GoomControl::GoomControlImpl::GoomControlImpl(const GoomControl& parentGoomControl,
                                              const Dimensions& dimensions,
                                              const std::string& resourcesDirectory,
                                              GoomLogger& goomLogger)
  : m_parentGoomControl{&parentGoomControl},
    m_goomInfo{dimensions, m_goomTime, m_goomSoundEvents},
    m_goomLogger{&dynamic_cast<GoomControlLogger&>(goomLogger)},
    m_fxHelper{&m_multiBufferDraw, &m_goomInfo, &m_goomRand, m_goomLogger},
    m_filterSettingsService{m_goomInfo, m_goomRand, resourcesDirectory, CreateZoomAdjustmentEffect},
    m_filterBuffersService{m_goomInfo,
                           m_normalizedCoordsConverter,
                           std::make_unique<FilterZoomVector>(m_goomInfo.GetDimensions().GetWidth(),
                                                              resourcesDirectory,
                                                              m_goomRand)},
    m_smallBitmaps{resourcesDirectory},
    m_visualFx{m_parallel, m_fxHelper, m_smallBitmaps, resourcesDirectory, m_stateHandler},
    m_goomTitleDisplayer{m_goomTextOutput, m_goomRand, GetFontDirectory(resourcesDirectory)},
    m_messageDisplayer{m_goomTextOutput, GetMessagesFontFile(resourcesDirectory)}
{
  UTILS::SetGoomLogger(*m_goomLogger);
}

inline auto GoomControl::GoomControlImpl::GetUpdateNum() const -> uint64_t
{
  return m_goomTime.GetCurrentTime();
}

inline auto GoomControl::GoomControlImpl::SetShowSongTitle(const ShowSongTitleType value) -> void
{
  m_showTitle = value;
}


inline auto GoomControl::GoomControlImpl::SetFrameData(FrameData& frameData) -> void
{
  m_frameData = &frameData;

  m_visualFx.SetFrameMiscData(m_frameData->miscData);

  m_p1 = &m_frameData->imageArrays.mainImagePixelBuffer;
  m_p2 = &m_frameData->imageArrays.lowImagePixelBuffer;
  m_p1->Fill(ZERO_PIXEL);
  m_p2->Fill(ZERO_PIXEL);
  m_multiBufferDraw.SetBuffers(*m_p1, *m_p2);
}

auto GoomControl::GoomControlImpl::UpdateFrameData() -> void
{
  m_frameData->imageArrays.mainImagePixelBufferNeedsUpdating = true;
  m_frameData->imageArrays.lowImagePixelBufferNeedsUpdating  = true;

  if (not m_filterBuffersService.IsTransformBufferReadyToCopy())
  {
    m_frameData->filterPosArrays.filterSrcePosNeedsUpdating = false;
    m_frameData->filterPosArrays.filterDestPosNeedsUpdating = false;
    m_frameData->miscData.lerpFactor                        = m_transformBufferLerpFactor;
  }
  else
  {
    UpdateFrameDataFilterSrcePosBuffer();
    UpdateFrameDataFilterDestPosBuffer();
    m_frameData->miscData.lerpFactor = 0.0F;

    m_transformBufferLerpFactor = 0.0F;
    m_musicSettingsReactor.ResetTransformBufferLerpData();
  }
}

auto GoomControl::GoomControlImpl::UpdateFrameDataFilterSrcePosBuffer() const noexcept -> void
{
  auto srceFilterPosBuffer        = m_frameData->filterPosArrays.filterSrcePos;
  const auto& destFilterPosBuffer = m_filterBuffersService.GetPreviousTransformBuffer();

  std::transform(destFilterPosBuffer.begin(),
                 destFilterPosBuffer.end(),
                 srceFilterPosBuffer.begin(),
                 srceFilterPosBuffer.begin(),
                 [this](const Point2dFlt& destPos, const Point2dFlt& srcePos)
                 { return lerp(srcePos, destPos, m_transformBufferLerpFactor); });

  m_frameData->filterPosArrays.filterSrcePosNeedsUpdating = true;
}

auto GoomControl::GoomControlImpl::UpdateFrameDataFilterDestPosBuffer() noexcept -> void
{
  m_filterBuffersService.CopyTransformBuffer(m_frameData->filterPosArrays.filterDestPos);

  m_frameData->filterPosArrays.filterDestPosNeedsUpdating = true;
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

inline auto GoomControl::GoomControlImpl::GetFrameData() const noexcept -> const FrameData&
{
  return *m_frameData;
}

inline auto GoomControl::GoomControlImpl::GetNumPoolThreads() const noexcept -> size_t
{
  return m_parallel.GetNumThreadsUsed();
}

inline auto GoomControl::GoomControlImpl::Start() -> void
{
  Expects(m_p1 != nullptr);
  Expects(m_p2 != nullptr);

  m_goomLogger->StartGoomControl(this);

  StartFilterServices();

  StartVisualFx();

  m_musicSettingsReactor.Start();

#ifdef DO_GOOM_STATE_DUMP
  StartGoomStateDump();
#endif

  m_runningTimeStopwatch.SetUpperLimitOfTimeIntervalInMsSinceLastMarked(
      m_upperLimitOfTimeIntervalInMsSinceLastMarked);
  static constexpr auto START_TIME_DELAY_IN_MS = 457.0F;
  m_runningTimeStopwatch.SetStartDelayAdjustInMs(START_TIME_DELAY_IN_MS);
  m_runningTimeStopwatch.StartNow();

  m_numUpdatesBetweenTimeChecks = DEFAULT_NUM_UPDATES_BETWEEN_TIME_CHECKS;
  m_goomTime.ResetTime();
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
  m_filterBuffersService.Finish();

  m_goomLogger->StopGoomControl();
}

auto GoomControl::GoomControlImpl::StartFilterServices() noexcept -> void
{
  m_filterSettingsService.Start();
  m_filterSettingsService.NotifyUpdatedFilterEffectsSettings();

  const auto& filterSettings = std::as_const(m_filterSettingsService).GetFilterSettings();
  m_filterBuffersService.SetFilterEffectsSettings(filterSettings.filterEffectsSettings);
  m_filterBuffersService.Start();
}

auto GoomControl::GoomControlImpl::StartVisualFx() noexcept -> void
{
  m_visualFx.SetAllowMultiThreadedStates(false);
  m_visualFx.SetResetDrawBuffSettingsFunc([this](const FXBuffSettings& settings)
                                          { ResetDrawBuffSettings(settings); });
  m_visualFx.ChangeAllFxColorMaps();

  const auto& filterSettings = std::as_const(m_filterSettingsService).GetFilterSettings();
  m_visualFx.SetZoomMidpoint(filterSettings.filterEffectsSettings.zoomMidpoint);

  m_visualFx.Start();
}

#ifdef DO_GOOM_STATE_DUMP
inline auto GoomControl::GoomControlImpl::StartGoomStateDump() -> void
{
  m_goomStateDump = std::make_unique<GoomStateDump>(m_goomInfo,
                                                    *m_goomLogger,
                                                    *m_parentGoomControl,
                                                    m_visualFx,
                                                    m_musicSettingsReactor,
                                                    m_filterSettingsService);
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

  ProcessAudio(soundData);

  UseMusicToChangeSettings();
  UpdateFilterSettings();

  UpdateTransformBuffer();

  ApplyStateToImageBuffers(soundData);
  ApplyEndEffectIfNearEnd();

  UpdateFrameData();

  DisplayTitleAndMessages(message);
  DisplayGoomState();

#ifdef DO_GOOM_STATE_DUMP
  UpdateGoomStateDump();
#endif
}

inline auto GoomControl::GoomControlImpl::NewCycle() -> void
{
  m_goomTime.UpdateTime();

  m_visualFx.SetAllowMultiThreadedStates(m_goomTitleDisplayer.IsFinished());
  m_musicSettingsReactor.NewCycle();
  m_filterSettingsService.NewCycle();

  UpdateTimeDependencies();
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

auto GoomControl::GoomControlImpl::UpdateTimeDependencies() -> void
{
  if (0 == m_songInfo.duration)
  {
    return;
  }
  if (0 != (m_goomTime.GetCurrentTime() % m_numUpdatesBetweenTimeChecks))
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

  m_musicSettingsReactor.ChangeFilterSettings();
}

inline auto GoomControl::GoomControlImpl::ProcessAudio(const AudioSamples& soundData) -> void
{
  /* ! etude du signal ... */
  m_soundInfo.ProcessSample(soundData);
  m_goomSoundEvents.Update();
}

inline auto GoomControl::GoomControlImpl::ApplyStateToImageBuffers(const AudioSamples& soundData)
    -> void
{
  m_visualFx.ApplyCurrentStateToImageBuffers(soundData);
}

inline auto GoomControl::GoomControlImpl::ApplyEndEffectIfNearEnd() -> void
{
  if (not m_runningTimeStopwatch.AreTimesValid())
  {
    return;
  }
  m_visualFx.ApplyEndEffectIfNearEnd(m_runningTimeStopwatch.GetTimeValues());
}

inline auto GoomControl::GoomControlImpl::UpdateTransformBuffer() -> void
{
  if (m_noZooms)
  {
    return;
  }

  m_filterBuffersService.UpdateTransformBuffer();
}

inline auto GoomControl::GoomControlImpl::UpdateFilterSettings() -> void
{
  const auto& newFilterSettings = std::as_const(m_filterSettingsService).GetFilterSettings();

  m_transformBufferLerpFactor =
      m_filterSettingsService.GetNextTransformBufferLerpFactor(m_transformBufferLerpFactor);

  m_visualFx.SetZoomMidpoint(newFilterSettings.filterEffectsSettings.zoomMidpoint);

  if (newFilterSettings.filterEffectsSettingsHaveChanged)
  {
    m_filterBuffersService.SetFilterEffectsSettings(newFilterSettings.filterEffectsSettings);
    m_filterSettingsService.NotifyUpdatedFilterEffectsSettings();
  }
}

inline auto GoomControl::GoomControlImpl::ResetDrawBuffSettings(const FXBuffSettings& settings)
    -> void
{
  m_multiBufferDraw.SetBuffIntensity(settings.buffIntensity);
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

  Expects(m_p1 != nullptr);
  Expects(m_p2 != nullptr);

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

  if (not m_goomTitleDisplayer.IsFinalPhase())
  {
    m_goomTextOutput.SetBuffer(*m_p1);
  }
  else if (not m_goomTitleDisplayer.IsFinalMoments())
  {
    static constexpr auto FINAL_PHASE_BUFF_INTENSITY = 0.2F;
    m_goomTextOutput.SetBuffIntensity(FINAL_PHASE_BUFF_INTENSITY);
    m_goomTextOutput.SetBuffer(*m_p1);
  }
  else
  {
    static constexpr auto FINAL_MOMENTS_BUFF_INTENSITY = 0.1F;
    m_goomTextOutput.SetBuffIntensity(FINAL_MOMENTS_BUFF_INTENSITY);
    m_goomTextOutput.SetBuffer(*m_p2);
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

  Expects(m_p1 != nullptr);
  m_goomTextOutput.SetBuffer(*m_p1);

  m_messageDisplayer.UpdateMessages(StringSplit(messages, "\n"));
}

auto GoomControl::GoomControlImpl::DisplayGoomState() -> void
{
  if (not m_showGoomState)
  {
    return;
  }

  const std::string message = GetGoomTimeInfo() + "\n" + GetNameValuesString(GetNameValueParams()) +
                              "\n" + m_goomStateMonitor.GetCurrentState();
  UpdateMessages(message);
}

auto GoomControl::GoomControlImpl::GetNameValueParams() const -> NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "Control Settings";
  return {
      GetPair(PARAM_GROUP, "lerp", m_transformBufferLerpFactor),
  };
}

inline auto GoomControl::GoomControlImpl::GetGoomTimeInfo() -> std::string
{
  const auto timeLeftStr =
      not m_runningTimeStopwatch.AreTimesValid()
          ? "Time left: not valid!"
          // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
          : std_fmt::format("Time left: {}  ({}%)",
                            m_runningTimeStopwatch.GetTimeValues().timeRemainingInMs,
                            m_runningTimeStopwatch.GetTimeValues().timeRemainingAsPercent);

  // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
  return timeLeftStr + "\n" + std_fmt::format("Update Num: {}", m_goomTime.GetCurrentTime());
}

} // namespace GOOM
