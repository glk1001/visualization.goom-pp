module;

#include <cmath>
#include <cstdint>
#include <format>
#include <fstream>
#include <print>
#include <stdexcept>
#include <string>
#include <utility>

module Goom.Control.GoomMusicSettingsReactor;

import Goom.Control.GoomAllVisualFx;
import Goom.FilterFx.FilterSettingsService;
import Goom.FilterFx.FilterSpeed;
import Goom.Lib.AssertUtils;
import Goom.Lib.SoundInfo;
import Goom.Lib.GoomPaths;
import Goom.Lib.GoomTypes;
import Goom.Utils.DateUtils;
import Goom.Utils.EnumUtils;
import Goom.Utils.StrUtils;
import Goom.PluginInfo;
import :GoomLock;

namespace GOOM::CONTROL
{

using FILTER_FX::FilterSettingsService;
using FILTER_FX::Vitesse;
using UTILS::EnumToString;
using UTILS::GetCurrentDateTimeAsString;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::NUM;
using UTILS::StringJoin;
using UTILS::ToStrings;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;

static constexpr auto COLLECT_EVENT_STATS = true;

static constexpr auto NORMAL_UPDATE_LOCK_TIME                  = 50U;
static constexpr auto SLOWER_SPEED_AND_SPEED_FORWARD_LOCK_TIME = 75U;
static constexpr auto REVERSE_SPEED_LOCK_TIME                  = 100U;
static constexpr auto MEGA_LENT_LOCK_TIME_INCREASE             = 50U;
static constexpr auto CHANGE_VITESSE_LOCK_TIME_INCREASE        = 50U;
static constexpr auto CHANGE_LERP_TO_END_LOCK_TIME             = 150U;

static constexpr auto PROB_CHANGE_STATE                             = 0.50F;
static constexpr auto PROB_CHANGE_FILTER_MODE                       = 0.05F;
static constexpr auto PROB_CHANGE_GPU_FILTER_MODE                   = 0.05F;
static constexpr auto PROB_CHANGE_FILTER_EXTRA_SETTINGS             = 0.90F;
static constexpr auto PROB_CHANGE_TO_MEGA_LENT_MODE                 = 1.0F / 700.F;
static constexpr auto PROB_FILTER_REVERSE_ON                        = 0.10F;
static constexpr auto PROB_FILTER_REVERSE_OFF_AND_STOP_SPEED        = 0.20F;
static constexpr auto PROB_FILTER_VITESSE_STOP_SPEED_MINUS_1        = 0.20F;
static constexpr auto PROB_FILTER_VITESSE_STOP_SPEED                = 0.10F;
static constexpr auto PROB_FILTER_CHANGE_VITESSE_AND_TOGGLE_REVERSE = 0.05F;
static constexpr auto PROB_FILTER_TOGGLE_ROTATION                   = 0.125F;
static constexpr auto PROB_FILTER_INCREASE_ROTATION                 = 0.250F;
static constexpr auto PROB_FILTER_DECREASE_ROTATION                 = 0.875F;
static constexpr auto PROB_FILTER_STOP_ROTATION                     = 0.250F;

enum class ChangeEvents : UnderlyingEnumType
{
  CHANGE_STATE,
  CHANGE_FILTER_MODE,
  CHANGE_FILTER_MODE_REJECTED,
  CHANGE_GPU_FILTER_MODE,
  CHANGE_GPU_FILTER_MODE_REJECTED,
  BIG_UPDATE,
  BIG_BREAK,
  GO_SLOWER,
  BIG_NORMAL_UPDATE,
  MEGA_LENT_UPDATE,
  CHANGE_FILTER_EXTRA_SETTINGS,
  CHANGE_FILTER_EXTRA_SETTINGS_REJECTED,
  CHANGE_ROTATION,
  TURN_OFF_ROTATION,
  TURN_OFF_ROTATION_REJECTED,
  SLOWER_ROTATION,
  SLOWER_ROTATION_REJECTED,
  FASTER_ROTATION,
  FASTER_ROTATION_REJECTED,
  TOGGLE_ROTATION,
  TOGGLE_ROTATION_REJECTED,
  SET_SPEED_REVERSE,
  SET_SLOW_SPEED,
  SET_STOP_SPEED,
  SET_SLOWER_SPEED_AND_SPEED_FORWARD,
  SET_SLOWER_SPEED_AND_TOGGLE_REVERSE,
  CHANGE_SPEED,
  UPDATE_TRANSFORM_BUFFER_LERP_DATA,
  SET_NEW_LERP_DATA_BASED_ON_SPEED,
  SET_LERP_TO_END,
  RESET_LERP_DATA,
};
using enum ChangeEvents;

class GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl
{
public:
  GoomMusicSettingsReactorImpl(const PluginInfo& goomInfo,
                               const GoomRand& goomRand,
                               GoomAllVisualFx& visualFx,
                               FilterSettingsService& filterSettingsService) noexcept;

  auto SetDumpDirectory(const std::string& dumpDirectory) -> void;

  auto Start() -> void;
  auto Finish() -> void;
  auto NewCycle() -> void;

  auto UpdateSettings() -> void;

  [[nodiscard]] auto GetNameValueParams() const -> NameValuePairs;

private:
  const PluginInfo* m_goomInfo;
  const GoomRand* m_goomRand;
  GoomAllVisualFx* m_visualFx;
  FilterSettingsService* m_filterSettingsService;
  std::string m_dumpDirectory;
  GoomLock m_lock; // pour empecher de nouveaux changements

  static constexpr auto MAX_TIME_BETWEEN_FILTER_SETTINGS_CHANGE_RANGE = NumberRange{300, 500};
  int32_t m_maxTimeBetweenFilterSettingsChange = MAX_TIME_BETWEEN_FILTER_SETTINGS_CHANGE_RANGE.min;
  int32_t m_numUpdatesSinceLastFilterSettingsChange    = 0;
  int32_t m_numUpdatesSinceLastGpuFilterSettingsChange = 0;
  uint32_t m_previousZoomSpeed                         = Vitesse::STOP_SPEED;

  static constexpr auto MAX_NUM_STATE_SELECTIONS_BLOCKED = 3U;
  uint32_t m_stateSelectionBlocker                       = MAX_NUM_STATE_SELECTIONS_BLOCKED;
  uint32_t m_timeInState                                 = 0U;

  // Changement d'effet de zoom !
  auto CheckIfFilterModeChanged() -> void;
  auto CheckIfGpuFilterModeChanged() -> void;
  auto ChangeStateMaybe() -> void;
  auto ChangeFilterModeMaybe() -> void;
  auto ChangeGpuFilterModeMaybe() -> void;
  auto BigBreakIfMusicIsCalm() -> void;
  auto BigUpdateIfNotLocked() -> void;
  auto LowerTheSpeedMaybe() -> void;
  auto ChangeRotationMaybe() -> void;
  auto ChangeFilterExtraSettingsMaybe() -> void;
  auto ChangeTransformBufferLerpDataMaybe() -> void;
  auto ChangeTransformBufferLerpToEndMaybe() -> void;
  auto ChangeVitesseMaybe() -> void;
  auto ChangeStopSpeedsMaybe() -> void;
  auto RestoreZoomInMaybe() -> void;

  auto DoChangeState() -> void;
  auto DoBigUpdateMaybe() -> void;
  auto DoBigNormalUpdate() -> void;
  auto DoMegaLentUpdate() -> void;
  auto DoBigBreak() -> void;
  auto DoChangeFilterMode() -> void;
  auto DoChangeGpuFilterMode() -> void;
  auto DoChangeFilterExtraSettings() -> void;
  auto DoChangeRotationMaybe() -> void;
  auto DoLowerTheSpeed() -> void;
  auto DoChangeSpeedSlowAndForward() -> void;
  auto DoChangeSpeedReverse() -> void;
  auto DoSetSlowerSpeedAndToggleReverse() -> void;
  auto DoChangeSpeed(uint32_t currentVitesse, uint32_t newVitesse) -> void;
  auto DoUpdateTransformBufferLerpData() -> void;
  auto DoSetTransformBufferLerpToEnd() -> void;
  auto DoResetTransformBufferLerpData() -> void;
  auto DoSetNewTransformBufferLerpDataBasedOnSpeed() -> void;

  struct PreChangeEventData
  {
    uint64_t eventTime                                 = 0U;
    int32_t numUpdatesSinceLastFilterSettingsChange    = 0;
    int32_t numUpdatesSinceLastGpuFilterSettingsChange = 0;
    int32_t maxTimeBetweenFilterSettingsChange         = 0;
    uint32_t lockTime                                  = 0U;
    uint32_t timeInState                               = 0U;
    uint32_t previousZoomSpeed                         = 0;
    uint32_t currentZoomSpeed                          = 0;
    uint32_t timeSinceLastGoom                         = 0U;
    uint32_t totalGoomsInCurrentCycle                  = 0U;
    float soundSpeed                                   = 0.0F;
  };
  struct PostChangeEventData
  {
    int32_t numUpdatesSinceLastFilterSettingsChange    = 0;
    int32_t numUpdatesSinceLastGpuFilterSettingsChange = 0;
    int32_t maxTimeBetweenFilterSettingsChange         = 0;
    uint32_t lockTime                                  = 0U;
    uint32_t timeInState                               = 0U;
    uint32_t currentZoomSpeed                          = 0;
    bool filterModeChangedSinceLastUpdate              = false;
    std::vector<ChangeEvents> changeEvents;
  };
  struct ChangeEventData
  {
    PreChangeEventData preChangeEventData{};
    PostChangeEventData postChangeEventData{};
  };
  static constexpr auto NUM_EVENT_DATA_TO_RESERVE = 500U;
  std::vector<ChangeEventData> m_allChangeEvents;
  auto ClearChangeEventData() noexcept -> void;
  auto PreUpdateChangeEventData() noexcept -> void;
  auto PostUpdateChangeEventData() noexcept -> void;
  auto DumpChangeEventData() -> void;
  auto LogChangeEvent(ChangeEvents changeEvent) noexcept -> void;
};

GoomMusicSettingsReactor::GoomMusicSettingsReactor(
    const PluginInfo& goomInfo,
    const GoomRand& goomRand,
    GoomAllVisualFx& visualFx,
    FilterSettingsService& filterSettingsService) noexcept
  : m_pimpl{spimpl::make_unique_impl<GoomMusicSettingsReactorImpl>(
        goomInfo, goomRand, visualFx, filterSettingsService)}
{
}

auto GoomMusicSettingsReactor::SetDumpDirectory(const std::string& dumpDirectory) -> void
{
  m_pimpl->SetDumpDirectory(dumpDirectory);
}

auto GoomMusicSettingsReactor::Start() -> void
{
  m_pimpl->Start();
}

auto GoomMusicSettingsReactor::Finish() -> void
{
  m_pimpl->Finish();
}

auto GoomMusicSettingsReactor::NewCycle() -> void
{
  m_pimpl->NewCycle();
}

auto GoomMusicSettingsReactor::UpdateSettings() -> void
{
  m_pimpl->UpdateSettings();
}

auto GoomMusicSettingsReactor::GetNameValueParams() const -> NameValuePairs
{
  return m_pimpl->GetNameValueParams();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::LogChangeEvent(
    const ChangeEvents changeEvent) noexcept -> void
{
  if constexpr (COLLECT_EVENT_STATS)
  {
    m_allChangeEvents.back().postChangeEventData.changeEvents.emplace_back(changeEvent);
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ClearChangeEventData() noexcept -> void
{
  if constexpr (COLLECT_EVENT_STATS)
  {
    m_allChangeEvents.clear();
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::PreUpdateChangeEventData() noexcept
    -> void
{
  if constexpr (COLLECT_EVENT_STATS)
  {
    if (m_allChangeEvents.capacity() == m_allChangeEvents.size())
    {
      m_allChangeEvents.reserve(m_allChangeEvents.size() + NUM_EVENT_DATA_TO_RESERVE);
    }

    // clang-format off
    m_allChangeEvents.emplace_back(ChangeEventData{
        .preChangeEventData = PreChangeEventData{
            .eventTime                               = m_goomInfo->GetTime().GetCurrentTime(),
            .numUpdatesSinceLastFilterSettingsChange = m_numUpdatesSinceLastFilterSettingsChange,
            .numUpdatesSinceLastGpuFilterSettingsChange
                                                     = m_numUpdatesSinceLastGpuFilterSettingsChange,
            .maxTimeBetweenFilterSettingsChange      = m_maxTimeBetweenFilterSettingsChange,
            .lockTime                                = m_lock.GetLockTime(),
            .timeInState                             = m_timeInState,
            .previousZoomSpeed                       = m_previousZoomSpeed,
            .currentZoomSpeed         = m_filterSettingsService->GetROVitesse().GetVitesse(),
            .timeSinceLastGoom        = m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom(),
            .totalGoomsInCurrentCycle = m_goomInfo->GetSoundEvents().GetTotalGoomsInCurrentCycle(),
            .soundSpeed               = m_goomInfo->GetSoundEvents().GetSoundInfo().GetSpeed(),
        },
        .postChangeEventData = PostChangeEventData{}
    });
    // clang-format on
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::PostUpdateChangeEventData() noexcept
    -> void
{
  if constexpr (COLLECT_EVENT_STATS)
  {
    auto& postData = m_allChangeEvents.back().postChangeEventData;

    postData.numUpdatesSinceLastFilterSettingsChange = m_numUpdatesSinceLastFilterSettingsChange;
    postData.numUpdatesSinceLastGpuFilterSettingsChange =
        m_numUpdatesSinceLastGpuFilterSettingsChange;
    postData.maxTimeBetweenFilterSettingsChange = m_maxTimeBetweenFilterSettingsChange;
    postData.lockTime                           = m_lock.GetLockTime();
    postData.timeInState                        = m_timeInState;
    postData.currentZoomSpeed = m_filterSettingsService->GetROVitesse().GetVitesse();
    postData.filterModeChangedSinceLastUpdate =
        m_filterSettingsService->HasFilterModeChangedSinceLastUpdate();
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DumpChangeEventData() -> void
{
  if constexpr (COLLECT_EVENT_STATS)
  {
    if (static constexpr auto DONT_CARE_SMALL_TIME = 10U;
        m_allChangeEvents.empty() or m_dumpDirectory.empty() or
        (m_goomInfo->GetTime().GetCurrentTime() < DONT_CARE_SMALL_TIME))
    {
      return;
    }

    const auto dumpFilepath = join_paths(
        m_dumpDirectory, std::format("reactor-stats-{}.txt", GetCurrentDateTimeAsString()));
    auto outFile = std::ofstream(dumpFilepath);
    if (not outFile)
    {
      throw std::runtime_error(std::format("Could not open dump stats file \"{}\".", dumpFilepath));
    }
    std::println(outFile,
                 "{:>8s}"
                 " {:>8s} {:>8s}"
                 " {:>8s} {:>8s}"
                 " {:>8s} {:>8s}"
                 " {:>8s} {:>8s}"
                 " {:>8s}"
                 " {:>8s} {:>8s}"
                 " {:>8s} {:>8s} {:>8s}"
                 " {:>8s}"
                 "  {}",
                 "TIME",

                 "<N_UPDS",
                 ">N_UPDS",

                 ">MX_UPDS",
                 "<MX_UPDS",

                 ">LK_TIME",
                 "<LK_TIME",

                 "<IN_STAT",
                 ">IN_STAT",

                 "PRV_SPD",

                 "<CUR_SPD",
                 ">CUR_SPD",

                 "LAST_GM",
                 "TOT_GM",
                 "SND_SPD",

                 "FIL_CHG",

                 "EVENTS");
    for (const auto& eventData : m_allChangeEvents)
    {
      std::println(outFile,
                   "{:8d}"
                   " {:8d} {:8d}"
                   " {:8d} {:8d}"
                   " {:8d} {:8d}"
                   " {:8d} {:8d}"
                   " {:8d}"
                   " {:8d} {:8d}"
                   " {:8d} {:8d} {:8.1f}"
                   " {:8}"
                   "  {}",
                   eventData.preChangeEventData.eventTime,

                   eventData.preChangeEventData.numUpdatesSinceLastFilterSettingsChange,
                   eventData.postChangeEventData.numUpdatesSinceLastFilterSettingsChange,

                   eventData.preChangeEventData.maxTimeBetweenFilterSettingsChange,
                   eventData.postChangeEventData.maxTimeBetweenFilterSettingsChange,

                   eventData.preChangeEventData.lockTime,
                   eventData.postChangeEventData.lockTime,

                   eventData.preChangeEventData.timeInState,
                   eventData.postChangeEventData.timeInState,

                   eventData.preChangeEventData.previousZoomSpeed,

                   eventData.preChangeEventData.currentZoomSpeed,
                   eventData.postChangeEventData.currentZoomSpeed,

                   eventData.preChangeEventData.timeSinceLastGoom,
                   eventData.preChangeEventData.totalGoomsInCurrentCycle,
                   eventData.preChangeEventData.soundSpeed,

                   eventData.postChangeEventData.filterModeChangedSinceLastUpdate,

                   StringJoin(ToStrings(eventData.postChangeEventData.changeEvents,
                                        [](const auto event) { return EnumToString(event); }),
                              ", "));
    }
  }
}

GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::GoomMusicSettingsReactorImpl(
    const PluginInfo& goomInfo,
    const GoomRand& goomRand,
    GoomAllVisualFx& visualFx,
    FilterSettingsService& filterSettingsService) noexcept
  : m_goomInfo{&goomInfo},
    m_goomRand{&goomRand},
    m_visualFx{&visualFx},
    m_filterSettingsService{&filterSettingsService}
{
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::SetDumpDirectory(
    const std::string& dumpDirectory) -> void
{
  m_dumpDirectory = dumpDirectory;
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::Start() -> void
{
  m_timeInState                                = 0;
  m_numUpdatesSinceLastFilterSettingsChange    = 0;
  m_numUpdatesSinceLastGpuFilterSettingsChange = 0;
  m_maxTimeBetweenFilterSettingsChange         = MAX_TIME_BETWEEN_FILTER_SETTINGS_CHANGE_RANGE.min;
  m_previousZoomSpeed                          = Vitesse::STOP_SPEED;

  ClearChangeEventData();
  PreUpdateChangeEventData();

  DoChangeState();

  PostUpdateChangeEventData();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::Finish() -> void
{
  DumpChangeEventData();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::NewCycle() -> void
{
  ++m_timeInState;
  m_lock.Update();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::UpdateSettings() -> void
{
  PreUpdateChangeEventData();

  ChangeFilterModeMaybe();
  ChangeGpuFilterModeMaybe();
  BigUpdateIfNotLocked();
  BigBreakIfMusicIsCalm();

  LowerTheSpeedMaybe();

  ChangeTransformBufferLerpDataMaybe();
  ChangeRotationMaybe();

  CheckIfFilterModeChanged();
  CheckIfGpuFilterModeChanged();

  m_previousZoomSpeed = m_filterSettingsService->GetROVitesse().GetVitesse();

  PostUpdateChangeEventData();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::CheckIfFilterModeChanged() -> void
{
  ++m_numUpdatesSinceLastFilterSettingsChange;
  if ((m_numUpdatesSinceLastFilterSettingsChange <= m_maxTimeBetweenFilterSettingsChange) and
      not m_filterSettingsService->HasFilterModeChangedSinceLastUpdate())
  {
    return;
  }

  m_numUpdatesSinceLastFilterSettingsChange = 0;
  m_visualFx->RefreshAllFx();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::CheckIfGpuFilterModeChanged() -> void
{
  ++m_numUpdatesSinceLastGpuFilterSettingsChange;
  if ((m_numUpdatesSinceLastGpuFilterSettingsChange <= m_maxTimeBetweenFilterSettingsChange) and
      not m_filterSettingsService->HasGpuFilterModeChangedSinceLastUpdate())
  {
    return;
  }

  m_numUpdatesSinceLastGpuFilterSettingsChange = 0;
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeStateMaybe() -> void
{
  if (m_stateSelectionBlocker > 0)
  {
    --m_stateSelectionBlocker;
    return;
  }
  if (not m_goomRand->ProbabilityOf<PROB_CHANGE_STATE>())
  {
    return;
  }

  DoChangeState();

  m_stateSelectionBlocker = MAX_NUM_STATE_SELECTIONS_BLOCKED;
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeFilterModeMaybe() -> void
{
  if ((m_numUpdatesSinceLastFilterSettingsChange <= m_maxTimeBetweenFilterSettingsChange) and
      ((m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom() > 0) or
       (not m_goomRand->ProbabilityOf<PROB_CHANGE_FILTER_MODE>())))
  {
    return;
  }

  DoChangeFilterMode();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeGpuFilterModeMaybe() -> void
{
  if ((m_numUpdatesSinceLastGpuFilterSettingsChange <= m_maxTimeBetweenFilterSettingsChange) and
      ((m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom() > 0) or
       (not m_goomRand->ProbabilityOf<PROB_CHANGE_GPU_FILTER_MODE>())))
  {
    return;
  }

  DoChangeGpuFilterMode();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::BigBreakIfMusicIsCalm() -> void
{
  static constexpr auto CALM_SOUND_SPEED = 0.3F;
  static constexpr auto CALM_CYCLES      = 16U;

  if ((m_goomInfo->GetSoundEvents().GetSoundInfo().GetSpeed() > CALM_SOUND_SPEED) or
      (not m_filterSettingsService->GetROVitesse().IsFasterThan(Vitesse::CALM_SPEED)) or
      ((m_goomInfo->GetTime().GetCurrentTime() % CALM_CYCLES) != 0))
  {
    return;
  }

  DoBigBreak();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::BigUpdateIfNotLocked() -> void
{
  if (m_lock.IsLocked())
  {
    return;
  }

  DoBigUpdateMaybe();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::LowerTheSpeedMaybe() -> void
{
  if (static constexpr auto LOWER_SPEED_CYCLES = 73U;
      ((m_goomInfo->GetTime().GetCurrentTime() % LOWER_SPEED_CYCLES) != 0) or
      (not m_filterSettingsService->GetROVitesse().IsFasterThan(Vitesse::FAST_SPEED)))
  {
    return;
  }

  DoLowerTheSpeed();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeTransformBufferLerpDataMaybe()
    -> void
{
  if (not m_filterSettingsService->HasFilterModeChangedSinceLastUpdate())
  {
    return;
  }

  DoUpdateTransformBufferLerpData();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeTransformBufferLerpToEndMaybe()
    -> void
{
  if (m_lock.GetLockTime() < CHANGE_LERP_TO_END_LOCK_TIME)
  {
    return;
  }

  DoSetTransformBufferLerpToEnd();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeRotationMaybe() -> void
{
  if (m_numUpdatesSinceLastFilterSettingsChange <= m_maxTimeBetweenFilterSettingsChange)
  {
    return;
  }

  DoChangeRotationMaybe();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeFilterExtraSettingsMaybe()
    -> void
{
  if (not m_goomRand->ProbabilityOf<PROB_CHANGE_FILTER_EXTRA_SETTINGS>())
  {
    return;
  }

  DoChangeFilterExtraSettings();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::RestoreZoomInMaybe() -> void
{
  // Retablir le zoom avant.
  // Restore zoom in.

  if (static constexpr auto REVERSE_VITESSE_CYCLES = 13U;
      (m_filterSettingsService->GetROVitesse().GetReverseVitesse()) and
      ((m_goomInfo->GetTime().GetCurrentTime() % REVERSE_VITESSE_CYCLES) != 0) and
      m_goomRand->ProbabilityOf<PROB_FILTER_REVERSE_OFF_AND_STOP_SPEED>())
  {
    DoChangeSpeedSlowAndForward();
  }
  if (m_goomRand->ProbabilityOf<PROB_FILTER_REVERSE_ON>())
  {
    DoChangeSpeedReverse();
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeStopSpeedsMaybe() -> void
{
  if (m_goomRand->ProbabilityOf<PROB_FILTER_VITESSE_STOP_SPEED_MINUS_1>())
  {
    LogChangeEvent(SET_SLOW_SPEED);
    m_filterSettingsService->GetRWVitesse().SetVitesse(Vitesse::SLOWEST_SPEED);
  }
  else if (m_goomRand->ProbabilityOf<PROB_FILTER_VITESSE_STOP_SPEED>())
  {
    LogChangeEvent(SET_STOP_SPEED);
    m_filterSettingsService->GetRWVitesse().SetVitesse(Vitesse::STOP_SPEED);
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::ChangeVitesseMaybe() -> void
{
  const auto soundSpeed = m_goomInfo->GetSoundEvents().GetSoundInfo().GetSpeed();

  static constexpr auto MIN_USABLE_SOUND_SPEED = SoundInfo::SPEED_MIDPOINT - 0.1F;
  static constexpr auto MAX_USABLE_SOUND_SPEED = SoundInfo::SPEED_MIDPOINT + 0.1F;
  const auto usableRelativeSoundSpeed =
      (std::clamp(soundSpeed, MIN_USABLE_SOUND_SPEED, MAX_USABLE_SOUND_SPEED) -
       MIN_USABLE_SOUND_SPEED) /
      (MAX_USABLE_SOUND_SPEED - MIN_USABLE_SOUND_SPEED);

  static constexpr auto MAX_SPEED_CHANGE = 10U;
  const auto newSpeedVal                 = static_cast<uint32_t>(
      std::lerp(0.0F, static_cast<float>(MAX_SPEED_CHANGE), usableRelativeSoundSpeed));

  auto& filterVitesse = m_filterSettingsService->GetRWVitesse();

  const auto currentVitesse = filterVitesse.GetVitesse();
  const auto newVitesse     = Vitesse::GetFasterBy(Vitesse::STOP_SPEED, newSpeedVal);

  if (currentVitesse > newVitesse)
  {
    // Current speed is faster than new one. Nothing to do.
    return;
  }

  // on accelere
  if (static constexpr auto VITESSE_CYCLES = 3U;
      ((currentVitesse > Vitesse::FASTER_SPEED) and (newVitesse > Vitesse::EVEN_FASTER_SPEED) and
       (0 == (m_goomInfo->GetTime().GetCurrentTime() % VITESSE_CYCLES))) or
      m_goomRand->ProbabilityOf<PROB_FILTER_CHANGE_VITESSE_AND_TOGGLE_REVERSE>())
  {
    LogChangeEvent(SET_SLOWER_SPEED_AND_TOGGLE_REVERSE);
    DoSetSlowerSpeedAndToggleReverse();
  }
  else
  {
    DoChangeSpeed(currentVitesse, newVitesse);
  }

  m_lock.IncreaseLockTime(CHANGE_VITESSE_LOCK_TIME_INCREASE);
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeState() -> void
{
  LogChangeEvent(CHANGE_STATE);

  m_visualFx->SetNextState();
  m_visualFx->ChangeAllFxColorMaps();

  m_timeInState = 0;
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoBigUpdateMaybe() -> void
{
  // Reperage de goom (acceleration forte de l'acceleration du volume).
  // Coup de boost de la vitesse si besoin.
  // Goom tracking (strong acceleration of volume acceleration).
  // Speed boost if needed.
  if (0 == m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom())
  {
    DoBigNormalUpdate();
  }

  // mode mega-lent
  if (m_goomRand->ProbabilityOf<PROB_CHANGE_TO_MEGA_LENT_MODE>())
  {
    DoMegaLentUpdate();
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoBigNormalUpdate() -> void
{
  LogChangeEvent(BIG_NORMAL_UPDATE);

  m_lock.SetLockTime(NORMAL_UPDATE_LOCK_TIME);

  ChangeStateMaybe();
  RestoreZoomInMaybe();
  ChangeStopSpeedsMaybe();
  ChangeRotationMaybe();
  ChangeFilterExtraSettingsMaybe();
  ChangeVitesseMaybe();
  ChangeTransformBufferLerpToEndMaybe();
  m_visualFx->ChangeAllFxColorMaps();

  m_maxTimeBetweenFilterSettingsChange =
      m_goomRand->GetRandInRange<MAX_TIME_BETWEEN_FILTER_SETTINGS_CHANGE_RANGE>();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoMegaLentUpdate() -> void
{
  LogChangeEvent(MEGA_LENT_UPDATE);

  m_lock.IncreaseLockTime(MEGA_LENT_LOCK_TIME_INCREASE);

  m_visualFx->ChangeAllFxColorMaps();
  m_filterSettingsService->GetRWVitesse().SetVitesse(Vitesse::SLOWEST_SPEED);

  DoSetTransformBufferLerpToEnd();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoBigBreak() -> void
{
  LogChangeEvent(BIG_BREAK);

  static constexpr auto SLOWER_BY = 3U;
  m_filterSettingsService->GetRWVitesse().GoSlowerBy(SLOWER_BY);

  m_visualFx->ChangeAllFxColorMaps();
  m_visualFx->ChangeAllFxPixelBlenders();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeFilterMode() -> void
{
  if (not m_filterSettingsService->SetNewRandomFilter())
  {
    LogChangeEvent(CHANGE_FILTER_MODE_REJECTED);
    return;
  }

  LogChangeEvent(CHANGE_FILTER_MODE);
  m_numUpdatesSinceLastFilterSettingsChange = 0;
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeGpuFilterMode() -> void
{
  if (not m_filterSettingsService->SetNewRandomGpuFilter(m_maxTimeBetweenFilterSettingsChange))
  {
    LogChangeEvent(CHANGE_GPU_FILTER_MODE_REJECTED);
    return;
  }

  LogChangeEvent(CHANGE_GPU_FILTER_MODE);
  m_numUpdatesSinceLastGpuFilterSettingsChange = 0;
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeFilterExtraSettings() -> void
{
  if (not m_filterSettingsService->ChangeMilieu())
  {
    LogChangeEvent(CHANGE_FILTER_EXTRA_SETTINGS_REJECTED);
    return;
  }

  LogChangeEvent(CHANGE_FILTER_EXTRA_SETTINGS);
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeRotationMaybe() -> void
{
  LogChangeEvent(CHANGE_ROTATION);

  if (m_goomRand->ProbabilityOf<PROB_FILTER_STOP_ROTATION>())
  {
    if (not m_filterSettingsService->TurnOffRotation())
    {
      LogChangeEvent(TURN_OFF_ROTATION_REJECTED);
      return;
    }
    LogChangeEvent(TURN_OFF_ROTATION);
  }
  else if (m_goomRand->ProbabilityOf<PROB_FILTER_DECREASE_ROTATION>())
  {
    static constexpr auto ROTATE_SLOWER_FACTOR = 0.9F;
    if (not m_filterSettingsService->MultiplyRotation(ROTATE_SLOWER_FACTOR))
    {
      LogChangeEvent(SLOWER_ROTATION_REJECTED);
      return;
    }
    LogChangeEvent(SLOWER_ROTATION);
  }
  else if (m_goomRand->ProbabilityOf<PROB_FILTER_INCREASE_ROTATION>())
  {
    static constexpr auto ROTATE_FASTER_FACTOR = 1.1F;
    if (not m_filterSettingsService->MultiplyRotation(ROTATE_FASTER_FACTOR))
    {
      LogChangeEvent(FASTER_ROTATION_REJECTED);
      return;
    }
    LogChangeEvent(FASTER_ROTATION);
  }
  else if (m_goomRand->ProbabilityOf<PROB_FILTER_TOGGLE_ROTATION>())
  {
    if (not m_filterSettingsService->ToggleRotationDirection())
    {
      LogChangeEvent(TOGGLE_ROTATION_REJECTED);
      return;
    }
    LogChangeEvent(TOGGLE_ROTATION);
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoLowerTheSpeed() -> void
{
  LogChangeEvent(GO_SLOWER);
  m_filterSettingsService->GetRWVitesse().GoSlowerBy(1U);
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeSpeedSlowAndForward() -> void
{
  LogChangeEvent(SET_SLOWER_SPEED_AND_SPEED_FORWARD);

  m_filterSettingsService->GetRWVitesse().SetReverseVitesse(false);
  m_filterSettingsService->GetRWVitesse().SetVitesse(Vitesse::SLOW_SPEED);
  m_lock.SetLockTime(SLOWER_SPEED_AND_SPEED_FORWARD_LOCK_TIME);
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeSpeedReverse() -> void
{
  LogChangeEvent(SET_SPEED_REVERSE);

  m_filterSettingsService->GetRWVitesse().SetReverseVitesse(true);
  m_lock.SetLockTime(REVERSE_SPEED_LOCK_TIME);
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoSetSlowerSpeedAndToggleReverse()
    -> void
{
  LogChangeEvent(SET_SLOWER_SPEED_AND_TOGGLE_REVERSE);

  auto& filterVitesse = m_filterSettingsService->GetRWVitesse();
  filterVitesse.SetVitesse(Vitesse::SLOWEST_SPEED);
  filterVitesse.ToggleReverseVitesse();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoChangeSpeed(
    const uint32_t currentVitesse, const uint32_t newVitesse) -> void
{
  LogChangeEvent(CHANGE_SPEED);

  auto& filterVitesse                        = m_filterSettingsService->GetRWVitesse();
  static constexpr auto OLD_TO_NEW_SPEED_MIX = 0.4F;
  filterVitesse.SetVitesse(static_cast<uint32_t>(std::lerp(
      static_cast<float>(currentVitesse), static_cast<float>(newVitesse), OLD_TO_NEW_SPEED_MIX)));
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoUpdateTransformBufferLerpData()
    -> void
{
  LogChangeEvent(UPDATE_TRANSFORM_BUFFER_LERP_DATA);

  static constexpr auto MIN_TIME_SINCE_LAST_GOOM            = 10U;
  static constexpr auto NUM_CYCLES_BEFORE_LERP_SPEED_CHANGE = 2U;
  if ((m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom() > MIN_TIME_SINCE_LAST_GOOM) or
      (m_goomInfo->GetSoundEvents().GetTotalGoomsInCurrentCycle() >=
       NUM_CYCLES_BEFORE_LERP_SPEED_CHANGE))
  {
    DoSetNewTransformBufferLerpDataBasedOnSpeed();
  }
  else
  {
    DoResetTransformBufferLerpData();

    DoChangeRotationMaybe();
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoSetTransformBufferLerpToEnd() -> void
{
  LogChangeEvent(SET_LERP_TO_END);

  m_filterSettingsService->SetDefaultTransformBufferLerpIncrement();
  m_filterSettingsService->SetTransformBufferLerpToEnd();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::DoResetTransformBufferLerpData()
    -> void
{
  LogChangeEvent(RESET_LERP_DATA);
  m_filterSettingsService->ResetTransformBufferLerpData();
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::
    DoSetNewTransformBufferLerpDataBasedOnSpeed() -> void
{
  LogChangeEvent(SET_NEW_LERP_DATA_BASED_ON_SPEED);

  m_filterSettingsService->SetDefaultTransformBufferLerpIncrement();

  auto diff = static_cast<float>(m_filterSettingsService->GetROVitesse().GetVitesse()) -
              static_cast<float>(m_previousZoomSpeed);
  if (diff < 0.0F)
  {
    diff = -diff;
  }
  if (static constexpr auto DIFF_CUT = 2.0F; diff > DIFF_CUT)
  {
    m_filterSettingsService->MultiplyTransformBufferLerpIncrement((diff + DIFF_CUT) / DIFF_CUT);
  }
}

auto GoomMusicSettingsReactor::GoomMusicSettingsReactorImpl::GetNameValueParams() const
    -> NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "Music Settings";
  return {
      GetPair(PARAM_GROUP, "vitesse", m_filterSettingsService->GetROVitesse().GetVitesse()),
      GetPair(PARAM_GROUP, "previousZoomSpeed", m_previousZoomSpeed),
      GetPair(PARAM_GROUP, "reverse", m_filterSettingsService->GetROVitesse().GetReverseVitesse()),
      GetPair(PARAM_GROUP,
              "relative speed",
              m_filterSettingsService->GetROVitesse().GetRelativeSpeed()),
      GetPair(PARAM_GROUP, "updatesSinceLastChange", m_numUpdatesSinceLastFilterSettingsChange),
      GetPair(
          PARAM_GROUP, "gpuUpdatesSinceLastChange", m_numUpdatesSinceLastGpuFilterSettingsChange),
  };
}

} // namespace GOOM::CONTROL
