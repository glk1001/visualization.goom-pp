#pragma once

#include "filter_fx/filter_settings_service.h"
#include "filter_fx/filter_speed.h"
#include "goom_all_visual_fx.h"
#include "goom_lock.h"
#include "goom_plugin_info.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

#include <cstdint>

namespace GOOM
{
class PluginInfo;
}

namespace GOOM::CONTROL
{
class GoomAllVisualFx;

class GoomMusicSettingsReactor
{
public:
  GoomMusicSettingsReactor(const PluginInfo& goomInfo,
                           const UTILS::MATH::IGoomRand& goomRand,
                           GoomAllVisualFx& visualFx,
                           FILTER_FX::FilterSettingsService& filterSettingsService) noexcept;

  auto Start() -> void;
  auto NewCycle() -> void;

  auto UpdateSettings() -> void;

  [[nodiscard]] auto GetNameValueParams() const -> UTILS::NameValuePairs;

private:
  const PluginInfo* m_goomInfo;
  const UTILS::MATH::IGoomRand* m_goomRand;
  GoomAllVisualFx* m_visualFx;
  FILTER_FX::FilterSettingsService* m_filterSettingsService;

  static constexpr uint32_t NORMAL_UPDATE_LOCK_TIME                = 50;
  static constexpr uint32_t REVERSE_SPEED_AND_STOP_SPEED_LOCK_TIME = 75;
  static constexpr uint32_t REVERSE_SPEED_LOCK_TIME                = 100;
  static constexpr uint32_t MEGA_LENT_LOCK_TIME_INCREASE           = 50;
  static constexpr uint32_t CHANGE_VITESSE_LOCK_TIME_INCREASE      = 50;
  static constexpr uint32_t CHANGE_LERP_TO_END_LOCK_TIME           = 150;
  GoomLock m_lock{}; // pour empecher de nouveaux changements

  static constexpr auto MIN_MAX_TIME_BETWEEN_ZOOM_EFFECTS_CHANGE = 300;
  static constexpr auto MAX_MAX_TIME_BETWEEN_ZOOM_EFFECTS_CHANGE = 500;
  int32_t m_maxTimeBetweenFilterSettingsChange      = MIN_MAX_TIME_BETWEEN_ZOOM_EFFECTS_CHANGE;
  int32_t m_numUpdatesSinceLastFilterSettingsChange = 0;
  uint32_t m_previousZoomSpeed                      = FILTER_FX::Vitesse::STOP_SPEED;

  static constexpr auto MAX_NUM_STATE_SELECTIONS_BLOCKED = 3U;
  uint32_t m_stateSelectionBlocker                       = MAX_NUM_STATE_SELECTIONS_BLOCKED;
  uint32_t m_timeInState                                 = 0U;
  auto ChangeState() -> void;
  auto DoChangeState() -> void;

  auto ChangeLerpData() -> void;
  auto ChangeRotation() -> void;
  auto ChangeFilterModeIfMusicChanges() -> void;

  // gros frein si la musique est calme
  // big break if the music is quiet
  auto BigBreakIfMusicIsCalm() -> void;

  // tout ceci ne sera fait qu'en cas de non-blocage
  // all this will only be done in case of non-blocking
  auto BigUpdateIfNotLocked() -> void;

  // baisser regulierement la vitesse
  // steadily lower the speed
  auto RegularlyLowerTheSpeed() -> void;

  // Changement d'effet de zoom !
  auto BigNormalUpdate() -> void;
  auto MegaLentUpdate() -> void;
  auto BigUpdate() -> void;
  auto BigBreak() -> void;
  auto ChangeFilterMode() -> void;
  auto CheckIfUpdateFilterSettingsNow() -> void;
  auto ChangeFilterExtraSettings() -> void;
  auto UpdateTransformBufferLerpData() -> void;
  auto SetNewTransformBufferLerpDataBasedOnSpeed() -> void;
  auto ChangeTransformBufferLerpToEnd() -> void;
  auto SetTransformBufferLerpToEnd() -> void;
  auto DoChangeRotation() -> void;
  auto ChangeSpeedReverse() -> void;
  auto ChangeVitesse() -> void;
  auto ChangeStopSpeeds() -> void;

  static constexpr auto PROB_CHANGE_FILTER_MODE                       = 0.05F;
  static constexpr auto PROB_CHANGE_STATE                             = 0.50F;
  static constexpr auto PROB_CHANGE_TO_MEGA_LENT_MODE                 = 1.0F / 700.F;
  static constexpr auto PROB_SLOW_FILTER_SETTINGS_UPDATE              = 0.5F;
  static constexpr auto PROB_FILTER_REVERSE_ON                        = 0.10F;
  static constexpr auto PROB_FILTER_REVERSE_OFF_AND_STOP_SPEED        = 0.20F;
  static constexpr auto PROB_FILTER_VITESSE_STOP_SPEED_MINUS_1        = 0.20F;
  static constexpr auto PROB_FILTER_VITESSE_STOP_SPEED                = 0.10F;
  static constexpr auto PROB_FILTER_CHANGE_VITESSE_AND_TOGGLE_REVERSE = 0.05F;
  static constexpr auto PROB_FILTER_TOGGLE_ROTATION                   = 0.125F;
  static constexpr auto PROB_FILTER_INCREASE_ROTATION                 = 0.25F;
  static constexpr auto PROB_FILTER_DECREASE_ROTATION                 = 0.875F;
  static constexpr auto PROB_FILTER_STOP_ROTATION                     = 0.25F;
};

} // namespace GOOM::CONTROL
