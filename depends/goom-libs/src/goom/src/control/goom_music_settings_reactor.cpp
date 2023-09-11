//#undef NO_LOGGING

#include "goom_music_settings_reactor.h"

//#include "utils/debugging_logger.h"
#include "filter_fx/filter_settings_service.h"
#include "filter_fx/filter_speed.h"
#include "goom/math20.h"
#include "goom/sound_info.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

#include <algorithm>
#include <utility>

namespace GOOM::CONTROL
{

using FILTER_FX::FilterSettingsService;
using FILTER_FX::Vitesse;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;

GoomMusicSettingsReactor::GoomMusicSettingsReactor(
    const PluginInfo& goomInfo,
    const IGoomRand& goomRand,
    GoomAllVisualFx& visualFx,
    FilterSettingsService& filterSettingsService) noexcept
  : m_goomInfo{&goomInfo},
    m_goomRand{&goomRand},
    m_visualFx{&visualFx},
    m_filterSettingsService{&filterSettingsService}
{
}

auto GoomMusicSettingsReactor::Start() -> void
{
  m_timeInState = 0;

  DoChangeState();
}

auto GoomMusicSettingsReactor::NewCycle() -> void
{
  ++m_timeInState;
  m_lock.Update();
}

auto GoomMusicSettingsReactor::RegularlyLowerTheSpeed() -> void
{
  static constexpr auto LOWER_SPEED_CYCLES = 73U;

  if ((0 == (m_goomInfo->GetTime().GetCurrentTime() % LOWER_SPEED_CYCLES)) and
      (m_filterSettingsService->GetROVitesse().IsFasterThan(FILTER_FX::Vitesse::FAST_SPEED)))
  {
    m_filterSettingsService->GetRWVitesse().GoSlowerBy(1U);
  }
}

auto GoomMusicSettingsReactor::BigBreakIfMusicIsCalm() -> void
{
  static constexpr auto CALM_SOUND_SPEED = 0.3F;
  static constexpr auto CALM_CYCLES      = 16U;

  if ((m_goomInfo->GetSoundEvents().GetSoundInfo().GetSpeed() < CALM_SOUND_SPEED) and
      (m_filterSettingsService->GetROVitesse().IsFasterThan(FILTER_FX::Vitesse::CALM_SPEED)) and
      (0 == (m_goomInfo->GetTime().GetCurrentTime() % CALM_CYCLES)))
  {
    BigBreak();
  }
}

inline auto GoomMusicSettingsReactor::BigBreak() -> void
{
  static constexpr auto SLOWER_BY = 3U;
  m_filterSettingsService->GetRWVitesse().GoSlowerBy(SLOWER_BY);

  m_visualFx->ChangeAllFxColorMaps();
  m_visualFx->ChangeAllFxPixelBlenders();
}

inline auto GoomMusicSettingsReactor::ChangeFilterMode() -> void
{
  m_filterSettingsService->SetNewRandomFilter();
  CheckIfUpdateFilterSettingsNow();
  m_visualFx->RefreshAllFx();
}

inline auto GoomMusicSettingsReactor::CheckIfUpdateFilterSettingsNow() -> void
{
  if (m_goomRand->ProbabilityOf(PROB_SLOW_FILTER_SETTINGS_UPDATE) and
      (m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom() > 0))
  {
    return;
  }

  const auto& newFilterSettings = std::as_const(*m_filterSettingsService).GetFilterSettings();
  m_visualFx->SetZoomMidpoint(newFilterSettings.filterEffectsSettings.zoomMidpoint);
  m_filterSettingsService->NotifyUpdatedFilterEffectsSettings();
}

auto GoomMusicSettingsReactor::BigUpdateIfNotLocked() -> void
{
  if (m_lock.IsLocked())
  {
    return;
  }

  BigUpdate();
}

inline auto GoomMusicSettingsReactor::BigUpdate() -> void
{
  // Reperage de goom (acceleration forte de l'acceleration du volume).
  // Coup de boost de la vitesse si besoin.
  // Goom tracking (strong acceleration of volume acceleration).
  // Speed boost if needed.
  if (0 == m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom())
  {
    BigNormalUpdate();
  }

  // mode mega-lent
  if (m_goomRand->ProbabilityOf(PROB_CHANGE_TO_MEGA_LENT_MODE))
  {
    MegaLentUpdate();
  }
}

inline auto GoomMusicSettingsReactor::MegaLentUpdate() -> void
{
  m_lock.IncreaseLockTime(MEGA_LENT_LOCK_TIME_INCREASE);

  m_visualFx->ChangeAllFxColorMaps();
  m_filterSettingsService->GetRWVitesse().SetVitesse(FILTER_FX::Vitesse::SLOWEST_SPEED);
  m_filterSettingsService->SetDefaultTransformBufferLerpIncrement();
  m_filterSettingsService->SetTransformBufferLerpToMaxLerp(1.0F);
}

inline auto GoomMusicSettingsReactor::BigNormalUpdate() -> void
{
  m_lock.SetLockTime(NORMAL_UPDATE_LOCK_TIME);

  ChangeState();
  ChangeSpeedReverse();
  ChangeStopSpeeds();
  ChangeRotation();
  ChangeFilterExtraSettings();
  ChangeVitesse();
  ChangeTransformBufferLerpToMaxLerp();
  m_visualFx->ChangeAllFxColorMaps();

  m_maxTimeBetweenFilterSettingsChange = m_goomRand->GetRandInRange(
      MIN_MAX_TIME_BETWEEN_ZOOM_EFFECTS_CHANGE, MAX_MAX_TIME_BETWEEN_ZOOM_EFFECTS_CHANGE + 1);
}

inline auto GoomMusicSettingsReactor::ChangeState() -> void
{
  if (m_stateSelectionBlocker > 0)
  {
    --m_stateSelectionBlocker;
    return;
  }
  if (not m_goomRand->ProbabilityOf(PROB_CHANGE_STATE))
  {
    return;
  }

  DoChangeState();

  m_stateSelectionBlocker = MAX_NUM_STATE_SELECTIONS_BLOCKED;
}

inline auto GoomMusicSettingsReactor::DoChangeState() -> void
{
  m_visualFx->SetNextState();
  m_visualFx->ChangeAllFxColorMaps();

  m_timeInState = 0;
}

inline auto GoomMusicSettingsReactor::ChangeSpeedReverse() -> void
{
  // Retablir le zoom avant.
  // Restore zoom in.

  if (static constexpr auto REVERSE_VITESSE_CYCLES = 13U;
      (m_filterSettingsService->GetROVitesse().GetReverseVitesse()) and
      ((m_goomInfo->GetTime().GetCurrentTime() % REVERSE_VITESSE_CYCLES) != 0) and
      m_goomRand->ProbabilityOf(PROB_FILTER_REVERSE_OFF_AND_STOP_SPEED))
  {
    m_filterSettingsService->GetRWVitesse().SetReverseVitesse(false);
    m_filterSettingsService->GetRWVitesse().SetVitesse(FILTER_FX::Vitesse::SLOW_SPEED);
    m_lock.SetLockTime(REVERSE_SPEED_AND_STOP_SPEED_LOCK_TIME);
  }
  if (m_goomRand->ProbabilityOf(PROB_FILTER_REVERSE_ON))
  {
    m_filterSettingsService->GetRWVitesse().SetReverseVitesse(true);
    m_lock.SetLockTime(REVERSE_SPEED_LOCK_TIME);
  }
}

inline auto GoomMusicSettingsReactor::ChangeStopSpeeds() -> void
{
  if (m_goomRand->ProbabilityOf(PROB_FILTER_VITESSE_STOP_SPEED_MINUS_1))
  {
    m_filterSettingsService->GetRWVitesse().SetVitesse(FILTER_FX::Vitesse::SLOWEST_SPEED);
  }
  else if (m_goomRand->ProbabilityOf(PROB_FILTER_VITESSE_STOP_SPEED))
  {
    m_filterSettingsService->GetRWVitesse().SetVitesse(FILTER_FX::Vitesse::STOP_SPEED);
  }
}

inline auto GoomMusicSettingsReactor::ChangeRotation() -> void
{
  if (m_goomRand->ProbabilityOf(PROB_FILTER_STOP_ROTATION))
  {
    m_filterSettingsService->TurnOffRotation();
  }
  else if (m_goomRand->ProbabilityOf(PROB_FILTER_DECREASE_ROTATION))
  {
    static constexpr auto ROTATE_SLOWER_FACTOR = 0.9F;
    m_filterSettingsService->MultiplyRotation(ROTATE_SLOWER_FACTOR);
  }
  else if (m_goomRand->ProbabilityOf(PROB_FILTER_INCREASE_ROTATION))
  {
    static constexpr auto ROTATE_FASTER_FACTOR = 1.1F;
    m_filterSettingsService->MultiplyRotation(ROTATE_FASTER_FACTOR);
  }
  else if (m_goomRand->ProbabilityOf(PROB_FILTER_TOGGLE_ROTATION))
  {
    m_filterSettingsService->ToggleRotationDirection();
  }
}

inline auto GoomMusicSettingsReactor::ChangeFilterExtraSettings() -> void
{
  m_filterSettingsService->ChangeMilieu();
  m_filterSettingsService->ResetRandomAfterEffects();
}

auto GoomMusicSettingsReactor::ChangeVitesse() -> void
{
  const auto soundSpeed = m_goomInfo->GetSoundEvents().GetSoundInfo().GetSpeed();

  static constexpr auto MIN_USABLE_SOUND_SPEED = SoundInfo::SPEED_MIDPOINT - 0.1F;
  static constexpr auto MAX_USABLE_SOUND_SPEED = SoundInfo::SPEED_MIDPOINT + 0.1F;
  const auto usableRelativeSoundSpeed =
      (std::clamp(soundSpeed, MIN_USABLE_SOUND_SPEED, MAX_USABLE_SOUND_SPEED) -
       MIN_USABLE_SOUND_SPEED) /
      (MAX_USABLE_SOUND_SPEED - MIN_USABLE_SOUND_SPEED);

  static constexpr auto MAX_SPEED_CHANGE = 10U;
  const auto newSpeedVal = STD20::lerp(0U, MAX_SPEED_CHANGE, usableRelativeSoundSpeed);

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
      m_goomRand->ProbabilityOf(PROB_FILTER_CHANGE_VITESSE_AND_TOGGLE_REVERSE))
  {
    filterVitesse.SetVitesse(Vitesse::SLOWEST_SPEED);
    filterVitesse.ToggleReverseVitesse();
  }
  else
  {
    static constexpr auto OLD_TO_NEW_SPEED_MIX = 0.4F;
    filterVitesse.SetVitesse(STD20::lerp(currentVitesse, newVitesse, OLD_TO_NEW_SPEED_MIX));
  }

  m_lock.IncreaseLockTime(CHANGE_VITESSE_LOCK_TIME_INCREASE);
}

inline auto GoomMusicSettingsReactor::ChangeTransformBufferLerpToMaxLerp() -> void
{
  if (m_lock.GetLockTime() > CHANGE_SWITCH_VALUES_LOCK_TIME)
  {
    m_filterSettingsService->SetDefaultTransformBufferLerpIncrement();
    m_filterSettingsService->SetTransformBufferLerpToMaxLerp(1.0F);
  }
}

auto GoomMusicSettingsReactor::ChangeFilterSettings() -> void
{
  //  LogInfo(UTILS::GetGoomLogger(),
  //          "ChangeFilterSettings: "
  //          "m_numUpdatesSinceLastFilterSettingsChange = {}, "
  //          "m_maxTimeBetweenFilterSettingsChange= {}, "
  //          "m_filterSettingsService->HasFilterModeChangedSinceLastUpdate = {}",
  //          m_numUpdatesSinceLastFilterSettingsChange,
  //          m_maxTimeBetweenFilterSettingsChange,
  //          m_filterSettingsService->HasFilterModeChangedSinceLastUpdate());

  if ((m_numUpdatesSinceLastFilterSettingsChange <= m_maxTimeBetweenFilterSettingsChange) and
      (not m_filterSettingsService->HasFilterModeChangedSinceLastUpdate()))
  {
    ++m_numUpdatesSinceLastFilterSettingsChange;
    return;
  }

  //  LogInfo(UTILS::GetGoomLogger(), "UpdateFilterSettings.");
  m_numUpdatesSinceLastFilterSettingsChange = 0;
  UpdateFilterSettings();
  m_previousZoomSpeed = m_filterSettingsService->GetROVitesse().GetVitesse();
}

auto GoomMusicSettingsReactor::ChangeFilterModeIfMusicChanges() -> void
{
  if ((m_numUpdatesSinceLastFilterSettingsChange <= m_maxTimeBetweenFilterSettingsChange) and
      ((m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom() > 0) or
       (not m_goomRand->ProbabilityOf(PROB_CHANGE_FILTER_MODE))))
  {
    return;
  }

  ChangeFilterMode();
}

inline auto GoomMusicSettingsReactor::UpdateFilterSettings() -> void
{
  if (m_filterSettingsService->HasFilterModeChangedSinceLastUpdate())
  {
    //    LogInfo(UTILS::GetGoomLogger(), "UpdateTransformBufferLerpData.");
    UpdateTransformBufferLerpData();
  }
  else
  {
    //    LogInfo(UTILS::GetGoomLogger(), "ChangeRotation.");
    ChangeRotation();
  }

  m_visualFx->RefreshAllFx();
}

inline auto GoomMusicSettingsReactor::UpdateTransformBufferLerpData() -> void
{
  static constexpr auto MIN_TIME_SINCE_LAST_GOOM            = 10U;
  static constexpr auto NUM_CYCLES_BEFORE_LERP_SPEED_CHANGE = 2U;
  if ((m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom() > MIN_TIME_SINCE_LAST_GOOM) or
      (m_goomInfo->GetSoundEvents().GetTotalGoomsInCurrentCycle() >=
       NUM_CYCLES_BEFORE_LERP_SPEED_CHANGE))
  {
    //    LogInfo(UTILS::GetGoomLogger(), "Set speed based lerp.");
    SetNewTransformBufferLerpDataBasedOnSpeed();
  }
  else
  {
    //    LogInfo(UTILS::GetGoomLogger(), "Resetting lerp.");
    m_filterSettingsService->ResetTransformBufferLerpData();
    ChangeRotation();
  }
}

inline auto GoomMusicSettingsReactor::SetNewTransformBufferLerpDataBasedOnSpeed() -> void
{
  m_filterSettingsService->SetDefaultTransformBufferLerpIncrement();
  //  LogInfo(UTILS::GetGoomLogger(),
  //          "Lerp Inc = {}.",
  //          m_filterSettingsService->GetFilterSettings()
  //              .filterTransformBufferSettings.lerpData.lerpIncrement);

  auto diff = static_cast<float>(m_filterSettingsService->GetROVitesse().GetVitesse()) -
              static_cast<float>(m_previousZoomSpeed);
  if (diff < 0.0F)
  {
    diff = -diff;
  }
  //  LogInfo(UTILS::GetGoomLogger(), "diff = {}.", diff);
  if (static constexpr auto DIFF_CUT = 2.0F; diff > DIFF_CUT)
  {
    m_filterSettingsService->MultiplyTransformBufferLerpIncrement((diff + DIFF_CUT) / DIFF_CUT);
  }
  //  LogInfo(UTILS::GetGoomLogger(),
  //          "Final Lerp Inc = {}.",
  //          m_filterSettingsService->GetFilterSettings()
  //              .filterTransformBufferSettings.lerpData.lerpIncrement);

  m_filterSettingsService->SetTransformBufferLerpToMaxLerp(0.0F);
}

auto GoomMusicSettingsReactor::GetNameValueParams() const -> NameValuePairs
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
  };
}

} // namespace GOOM::CONTROL
