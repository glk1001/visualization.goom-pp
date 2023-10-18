//#undef NO_LOGGING

#include "goom_music_settings_reactor.h"

#include "filter_fx/filter_settings_service.h"
#include "filter_fx/filter_speed.h"
#include "goom/math20.h"
#include "goom/sound_info.h"
#include "utils/enum_utils.h"
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
using UTILS::NUM;
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
  m_timeInState                             = 0;
  m_numUpdatesSinceLastFilterSettingsChange = 0;
  m_maxTimeBetweenFilterSettingsChange      = MIN_MAX_TIME_BETWEEN_ZOOM_EFFECTS_CHANGE;
  m_previousZoomSpeed                       = FILTER_FX::Vitesse::STOP_SPEED;

  DoChangeState();
}

auto GoomMusicSettingsReactor::NewCycle() -> void
{
  ++m_timeInState;
  m_lock.Update();
}

auto GoomMusicSettingsReactor::GetNextChangeEvents() noexcept -> void
{
  m_changeEvents.clear();
}

auto GoomMusicSettingsReactor::UpdateSettings() -> void
{
  m_changeEvents.clear();
  m_changeEvents.reserve(NUM<ChangeEvents>);

  ChangeFilterModeIfMusicChanges();
  BigUpdateIfNotLocked();
  BigBreakIfMusicIsCalm();

  RegularlyLowerTheSpeed();

  ChangeTransformBufferLerpData();
  ChangeRotation();

  m_previousZoomSpeed = m_filterSettingsService->GetROVitesse().GetVitesse();

  ++m_numUpdatesSinceLastFilterSettingsChange;
  if ((m_numUpdatesSinceLastFilterSettingsChange > m_maxTimeBetweenFilterSettingsChange) or
      m_filterSettingsService->HasFilterModeChangedSinceLastUpdate())
  {
    m_numUpdatesSinceLastFilterSettingsChange = 0;
    m_visualFx->RefreshAllFx();
  }
}

auto GoomMusicSettingsReactor::ChangeFilterModeIfMusicChanges() -> void
{
  if ((m_numUpdatesSinceLastFilterSettingsChange <= m_maxTimeBetweenFilterSettingsChange) and
      ((m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom() > 0) or
       (not m_goomRand->ProbabilityOf(PROB_CHANGE_FILTER_MODE))))
  {
    return;
  }

  m_changeEvents.push_back(ChangeEvents::CHANGE_FILTER_MODE);
  DoChangeFilterMode();
}

auto GoomMusicSettingsReactor::BigUpdateIfNotLocked() -> void
{
  if (m_lock.IsLocked())
  {
    return;
  }

  m_changeEvents.push_back(ChangeEvents::BIG_UPDATE);
  DoBigUpdate();
}

auto GoomMusicSettingsReactor::BigBreakIfMusicIsCalm() -> void
{
  static constexpr auto CALM_SOUND_SPEED = 0.3F;
  static constexpr auto CALM_CYCLES      = 16U;

  if ((m_goomInfo->GetSoundEvents().GetSoundInfo().GetSpeed() > CALM_SOUND_SPEED) or
      (not m_filterSettingsService->GetROVitesse().IsFasterThan(FILTER_FX::Vitesse::CALM_SPEED)) or
      ((m_goomInfo->GetTime().GetCurrentTime() % CALM_CYCLES) != 0))
  {
    return;
  }

  m_changeEvents.push_back(ChangeEvents::BIG_BREAK);
  DoBigBreak();
}

auto GoomMusicSettingsReactor::RegularlyLowerTheSpeed() -> void
{
  if (static constexpr auto LOWER_SPEED_CYCLES = 73U;
      ((m_goomInfo->GetTime().GetCurrentTime() % LOWER_SPEED_CYCLES) != 0) or
      (not m_filterSettingsService->GetROVitesse().IsFasterThan(FILTER_FX::Vitesse::FAST_SPEED)))
  {
    return;
  }

  m_changeEvents.push_back(ChangeEvents::GO_SLOWER);
  m_filterSettingsService->GetRWVitesse().GoSlowerBy(1U);
}

auto GoomMusicSettingsReactor::ChangeTransformBufferLerpData() -> void
{
  if (not m_filterSettingsService->HasFilterModeChangedSinceLastUpdate())
  {
    return;
  }

  m_changeEvents.push_back(ChangeEvents::UPDATE_TRANSFORM_BUFFER_LERP_DATA);
  DoUpdateTransformBufferLerpData();
}

auto GoomMusicSettingsReactor::ChangeRotation() -> void
{
  if (m_numUpdatesSinceLastFilterSettingsChange <= m_maxTimeBetweenFilterSettingsChange)
  {
    return;
  }

  m_changeEvents.push_back(ChangeEvents::CHANGE_ROTATION);
  DoChangeRotation();
}

auto GoomMusicSettingsReactor::DoBigBreak() -> void
{
  static constexpr auto SLOWER_BY = 3U;
  m_filterSettingsService->GetRWVitesse().GoSlowerBy(SLOWER_BY);

  m_visualFx->ChangeAllFxColorMaps();
  m_visualFx->ChangeAllFxPixelBlenders();
}

auto GoomMusicSettingsReactor::DoChangeFilterMode() -> void
{
  m_filterSettingsService->SetNewRandomFilter();

  if (UpdateFilterSettingsNow())
  {
    m_changeEvents.push_back(ChangeEvents::UPDATE_FILTER_SETTINGS_NOW);
    DoUpdateFilterSettingsNow();
  }
}

auto GoomMusicSettingsReactor::UpdateFilterSettingsNow() const noexcept -> bool
{
  return (not m_goomRand->ProbabilityOf(PROB_SLOW_FILTER_SETTINGS_UPDATE)) or
         (0 == m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom());
}

auto GoomMusicSettingsReactor::DoUpdateFilterSettingsNow() -> void
{
  const auto& newFilterSettings = std::as_const(*m_filterSettingsService).GetFilterSettings();
  m_visualFx->SetZoomMidpoint(newFilterSettings.filterEffectsSettings.zoomMidpoint);
  m_filterSettingsService->NotifyUpdatedFilterEffectsSettings();
  m_numUpdatesSinceLastFilterSettingsChange = 0;
}

auto GoomMusicSettingsReactor::DoBigUpdate() -> void
{
  // Reperage de goom (acceleration forte de l'acceleration du volume).
  // Coup de boost de la vitesse si besoin.
  // Goom tracking (strong acceleration of volume acceleration).
  // Speed boost if needed.
  if (0 == m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom())
  {
    m_changeEvents.push_back(ChangeEvents::BIG_NORMAL_UPDATE);
    DoBigNormalUpdate();
  }

  // mode mega-lent
  if (m_goomRand->ProbabilityOf(PROB_CHANGE_TO_MEGA_LENT_MODE))
  {
    m_changeEvents.push_back(ChangeEvents::MEGA_LENT_UPDATE);
    DoMegaLentUpdate();
  }
}

auto GoomMusicSettingsReactor::DoMegaLentUpdate() -> void
{
  m_lock.IncreaseLockTime(MEGA_LENT_LOCK_TIME_INCREASE);

  m_visualFx->ChangeAllFxColorMaps();
  m_filterSettingsService->GetRWVitesse().SetVitesse(FILTER_FX::Vitesse::SLOWEST_SPEED);

  DoSetTransformBufferLerpToEnd();
}

auto GoomMusicSettingsReactor::DoBigNormalUpdate() -> void
{
  m_lock.SetLockTime(NORMAL_UPDATE_LOCK_TIME);

  ChangeState();
  ChangeSpeedReverse();
  ChangeStopSpeeds();
  DoChangeRotation();
  ChangeFilterExtraSettings();
  ChangeVitesse();
  ChangeTransformBufferLerpToEnd();
  m_visualFx->ChangeAllFxColorMaps();

  m_maxTimeBetweenFilterSettingsChange = m_goomRand->GetRandInRange(
      MIN_MAX_TIME_BETWEEN_ZOOM_EFFECTS_CHANGE, MAX_MAX_TIME_BETWEEN_ZOOM_EFFECTS_CHANGE + 1);
}

auto GoomMusicSettingsReactor::ChangeState() -> void
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

  m_changeEvents.push_back(ChangeEvents::CHANGE_STATE);
  DoChangeState();

  m_stateSelectionBlocker = MAX_NUM_STATE_SELECTIONS_BLOCKED;
}

auto GoomMusicSettingsReactor::DoChangeState() -> void
{
  m_visualFx->SetNextState();
  m_visualFx->ChangeAllFxColorMaps();

  m_timeInState = 0;
}

auto GoomMusicSettingsReactor::ChangeSpeedReverse() -> void
{
  // Retablir le zoom avant.
  // Restore zoom in.

  if (static constexpr auto REVERSE_VITESSE_CYCLES = 13U;
      (m_filterSettingsService->GetROVitesse().GetReverseVitesse()) and
      ((m_goomInfo->GetTime().GetCurrentTime() % REVERSE_VITESSE_CYCLES) != 0) and
      m_goomRand->ProbabilityOf(PROB_FILTER_REVERSE_OFF_AND_STOP_SPEED))
  {
    m_changeEvents.push_back(ChangeEvents::SET_SLOWER_SPEED_AND_SPEED_FORWARD);
    DoChangeSpeedSlowAndForward();
  }
  if (m_goomRand->ProbabilityOf(PROB_FILTER_REVERSE_ON))
  {
    m_changeEvents.push_back(ChangeEvents::SET_SPEED_REVERSE);
    DoChangeSpeedReverse();
  }
}

auto GoomMusicSettingsReactor::DoChangeSpeedSlowAndForward() -> void
{
  m_filterSettingsService->GetRWVitesse().SetReverseVitesse(false);
  m_filterSettingsService->GetRWVitesse().SetVitesse(FILTER_FX::Vitesse::SLOW_SPEED);
  m_lock.SetLockTime(SLOWER_SPEED_AND_SPEED_FORWARD_LOCK_TIME);
}

auto GoomMusicSettingsReactor::DoChangeSpeedReverse() -> void
{
  m_filterSettingsService->GetRWVitesse().SetReverseVitesse(true);
  m_lock.SetLockTime(REVERSE_SPEED_LOCK_TIME);
}

auto GoomMusicSettingsReactor::ChangeStopSpeeds() -> void
{
  if (m_goomRand->ProbabilityOf(PROB_FILTER_VITESSE_STOP_SPEED_MINUS_1))
  {
    m_changeEvents.push_back(ChangeEvents::SET_SLOW_SPEED);
    m_filterSettingsService->GetRWVitesse().SetVitesse(FILTER_FX::Vitesse::SLOWEST_SPEED);
  }
  else if (m_goomRand->ProbabilityOf(PROB_FILTER_VITESSE_STOP_SPEED))
  {
    m_changeEvents.push_back(ChangeEvents::SET_STOP_SPEED);
    m_filterSettingsService->GetRWVitesse().SetVitesse(FILTER_FX::Vitesse::STOP_SPEED);
  }
}

auto GoomMusicSettingsReactor::DoChangeRotation() -> void
{
  if (m_goomRand->ProbabilityOf(PROB_FILTER_STOP_ROTATION))
  {
    m_changeEvents.push_back(ChangeEvents::TURN_OFF_ROTATION);
    m_filterSettingsService->TurnOffRotation();
  }
  else if (m_goomRand->ProbabilityOf(PROB_FILTER_DECREASE_ROTATION))
  {
    m_changeEvents.push_back(ChangeEvents::SLOWER_ROTATION);
    static constexpr auto ROTATE_SLOWER_FACTOR = 0.9F;
    m_filterSettingsService->MultiplyRotation(ROTATE_SLOWER_FACTOR);
  }
  else if (m_goomRand->ProbabilityOf(PROB_FILTER_INCREASE_ROTATION))
  {
    m_changeEvents.push_back(ChangeEvents::FASTER_ROTATION);
    static constexpr auto ROTATE_FASTER_FACTOR = 1.1F;
    m_filterSettingsService->MultiplyRotation(ROTATE_FASTER_FACTOR);
  }
  else if (m_goomRand->ProbabilityOf(PROB_FILTER_TOGGLE_ROTATION))
  {
    m_changeEvents.push_back(ChangeEvents::TOGGLE_ROTATION);
    m_filterSettingsService->ToggleRotationDirection();
  }
}

auto GoomMusicSettingsReactor::ChangeFilterExtraSettings() -> void
{
  m_changeEvents.push_back(ChangeEvents::CHANGE_FILTER_EXTRA_SETTINGS);
  DoChangeFilterExtraSettings();
}

auto GoomMusicSettingsReactor::DoChangeFilterExtraSettings() -> void
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
    m_changeEvents.push_back(ChangeEvents::SET_SLOWER_SPEED_AND_TOGGLE_REVERSE);
    DoSetSlowerSpeedAndToggleReverse();
  }
  else
  {
    m_changeEvents.push_back(ChangeEvents::CHANGE_SPEED);
    DoChangeSpeed(currentVitesse, newVitesse);
  }

  m_lock.IncreaseLockTime(CHANGE_VITESSE_LOCK_TIME_INCREASE);
}

auto GoomMusicSettingsReactor::DoSetSlowerSpeedAndToggleReverse() -> void
{
  auto& filterVitesse = m_filterSettingsService->GetRWVitesse();
  filterVitesse.SetVitesse(Vitesse::SLOWEST_SPEED);
  filterVitesse.ToggleReverseVitesse();
}

auto GoomMusicSettingsReactor::DoChangeSpeed(const uint32_t currentVitesse,
                                             const uint32_t newVitesse) -> void
{
  auto& filterVitesse                        = m_filterSettingsService->GetRWVitesse();
  static constexpr auto OLD_TO_NEW_SPEED_MIX = 0.4F;
  filterVitesse.SetVitesse(STD20::lerp(currentVitesse, newVitesse, OLD_TO_NEW_SPEED_MIX));
}

auto GoomMusicSettingsReactor::ChangeTransformBufferLerpToEnd() -> void
{
  if (m_lock.GetLockTime() < CHANGE_LERP_TO_END_LOCK_TIME)
  {
    return;
  }

  m_changeEvents.push_back(ChangeEvents::SET_LERP_TO_END);
  DoSetTransformBufferLerpToEnd();
}

auto GoomMusicSettingsReactor::DoUpdateTransformBufferLerpData() -> void
{
  static constexpr auto MIN_TIME_SINCE_LAST_GOOM            = 10U;
  static constexpr auto NUM_CYCLES_BEFORE_LERP_SPEED_CHANGE = 2U;
  if ((m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom() > MIN_TIME_SINCE_LAST_GOOM) or
      (m_goomInfo->GetSoundEvents().GetTotalGoomsInCurrentCycle() >=
       NUM_CYCLES_BEFORE_LERP_SPEED_CHANGE))
  {
    m_changeEvents.push_back(ChangeEvents::SET_NEW_LERP_DATA_BASED_ON_SPEED);
    DoSetNewTransformBufferLerpDataBasedOnSpeed();
  }
  else
  {
    //    LogInfo(UTILS::GetGoomLogger(), "Resetting lerp.");
    m_changeEvents.push_back(ChangeEvents::RESET_LERP_DATA);
    m_filterSettingsService->ResetTransformBufferLerpData();

    m_changeEvents.push_back(ChangeEvents::CHANGE_ROTATION);
    DoChangeRotation();
  }
}

auto GoomMusicSettingsReactor::DoSetTransformBufferLerpToEnd() -> void
{
  m_filterSettingsService->SetDefaultTransformBufferLerpIncrement();
  m_filterSettingsService->SetTransformBufferLerpToEnd();
}

auto GoomMusicSettingsReactor::DoSetNewTransformBufferLerpDataBasedOnSpeed() -> void
{
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
