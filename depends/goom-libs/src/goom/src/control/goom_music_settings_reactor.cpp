#include "goom_music_settings_reactor.h"

#include "utils/name_value_pairs.h"

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

auto GoomMusicSettingsReactor::ChangeZoomEffects() -> void
{
  if ((m_updatesSinceLastZoomEffectsChange <= m_maxTimeBetweenZoomEffectsChange) and
      (not m_filterSettingsService->HasFilterModeChangedSinceLastUpdate()))
  {
    ++m_updatesSinceLastZoomEffectsChange;
  }
  else
  {
    m_updatesSinceLastZoomEffectsChange = 0;
    UpdateSettings();
    m_previousZoomSpeed = m_filterSettingsService->GetROVitesse().GetVitesse();
  }
}

inline auto GoomMusicSettingsReactor::UpdateSettings() -> void
{
  if (m_filterSettingsService->HasFilterModeChangedSinceLastUpdate())
  {
    UpdateTranLerpSettings();
  }
  else
  {
    ChangeRotation();
  }

  m_visualFx->RefreshAllFx();
}

inline auto GoomMusicSettingsReactor::UpdateTranLerpSettings() -> void
{
  if (static constexpr auto NUM_CYCLES_BEFORE_LERP_SPEED_CHANGE = 2U;
      (0 != m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom()) or
      (m_goomInfo->GetSoundEvents().GetTotalGoomsInCurrentCycle() >=
       NUM_CYCLES_BEFORE_LERP_SPEED_CHANGE))
  {
    SetNewTranLerpSettingsBasedOnSpeed();
  }
  else
  {
    ResetTranLerpSettings();
    ChangeRotation();
  }
}

inline auto GoomMusicSettingsReactor::ResetTranLerpSettings() -> void
{
  m_filterSettingsService->SetTranLerpIncrement(0);
  m_filterSettingsService->SetTranLerpToMaxDefaultSwitchMult();
}

inline auto GoomMusicSettingsReactor::SetNewTranLerpSettingsBasedOnSpeed() -> void
{
  m_filterSettingsService->SetDefaultTranLerpIncrement();

  auto diff = static_cast<int32_t>(m_filterSettingsService->GetROVitesse().GetVitesse()) -
              static_cast<int32_t>(m_previousZoomSpeed);
  if (diff < 0)
  {
    diff = -diff;
  }
  if (static constexpr auto DIFF_CUT = 2; diff > DIFF_CUT)
  {
    m_filterSettingsService->MultiplyTranLerpIncrement(
        static_cast<uint32_t>((diff + DIFF_CUT) / DIFF_CUT));
  }

  m_filterSettingsService->SetTranLerpToMaxSwitchMult(1.0F);
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
       (0 == (m_updateNum % VITESSE_CYCLES))) or
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
      GetPair(PARAM_GROUP, "updatesSinceLastChange", m_updatesSinceLastZoomEffectsChange),
  };
}

} // namespace GOOM::CONTROL
