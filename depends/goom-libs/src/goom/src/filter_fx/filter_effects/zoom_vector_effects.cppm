module;

#include <cstdint>
#include <functional>
#include <string>

export module Goom.FilterFx.FilterEffects.ZoomVectorEffects;

import Goom.FilterFx.AfterEffects.AfterEffects;
import Goom.FilterFx.AfterEffects.ZoomVectorAfterEffects;
import Goom.FilterFx.FilterSettings;
import Goom.FilterFx.FilterSettingsService;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.AssertUtils;
import Goom.Lib.Point2d;

using GOOM::UTILS::MATH::NumberRange;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class ZoomVectorEffects
{
public:
  using GetAfterEffectsFunc = std::function<auto(const UTILS::MATH::GoomRand& goomRand,
                                                 const std::string& resourcesDirectory)
                                                ->AFTER_EFFECTS::AfterEffects>;

  ZoomVectorEffects(uint32_t screenWidth,
                    const std::string& resourcesDirectory,
                    const UTILS::MATH::GoomRand& goomRand,
                    const GetAfterEffectsFunc& getAfterEffects) noexcept;

  [[nodiscard]] static constexpr auto IsValidMultiplierRange(
      const NumberRange<float>& range) noexcept -> bool;

  [[nodiscard]] static auto GetStandardAfterEffects(const UTILS::MATH::GoomRand& goomRand,
                                                    const std::string& resourcesDirectory) noexcept
      -> AFTER_EFFECTS::AfterEffects;

  auto SetFilterSettings(const FilterEffectsSettings& filterEffectsSettings) noexcept -> void;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept -> Vec2dFlt;

  [[nodiscard]] auto GetMultiplierEffect(const NormalizedCoords& coords,
                                         const Vec2dFlt& zoomAdjustment) const noexcept
      -> Point2dFlt;

  [[nodiscard]] auto GetAfterEffectsVelocity(const NormalizedCoords& coords,
                                             float sqDistFromZero,
                                             const NormalizedCoords& zoomVelocity) const noexcept
      -> NormalizedCoords;
  [[nodiscard]] auto GetAfterEffectsVelocityMultiplier() const noexcept -> float;

  [[nodiscard]] auto GetZoomEffectsNameValueParams() const noexcept -> UTILS::NameValuePairs;

protected:
  static constexpr auto RAW_BASE_ZOOM_ADJUSTMENT_FACTOR = 0.02F;

private:
  const FilterEffectsSettings* m_filterEffectsSettings{};
  AFTER_EFFECTS::ZoomVectorAfterEffects m_zoomVectorAfterEffects;

  static constexpr auto MIN_ALLOWED_BASE_ZOOM_ADJUSTMENT = 0.0F;
  static constexpr auto MAX_ALLOWED_BASE_ZOOM_ADJUSTMENT = 0.5F;
  float m_baseZoomAdjustmentFactor                       = RAW_BASE_ZOOM_ADJUSTMENT_FACTOR;
  auto SetBaseZoomAdjustmentFactor(float multiplier) noexcept -> void;
  auto SetBaseZoomAdjustmentFactor() noexcept -> void;
  [[nodiscard]] static constexpr auto GetBaseZoomAdjustment(float baseZoomAdjustmentFactor,
                                                            float relativeSpeed) noexcept -> float;

  [[nodiscard]] auto GetClampedZoomAdjustment(const Vec2dFlt& zoomCoeffs) const noexcept
      -> Vec2dFlt;
  [[nodiscard]] auto GetClampedZoomAdjustment(float zoomAdjustment) const noexcept -> float;

  [[nodiscard]] auto GetZoomAdjustmentNameValueParams() const noexcept -> UTILS::NameValuePairs;
};

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

constexpr auto ZoomVectorEffects::IsValidMultiplierRange(const NumberRange<float>& range) noexcept
    -> bool
{
  if (MIN_ALLOWED_BASE_ZOOM_ADJUSTMENT >
      GetBaseZoomAdjustment(range.min * RAW_BASE_ZOOM_ADJUSTMENT_FACTOR, -1.0F))
  {
    return false;
  }
  if (MIN_ALLOWED_BASE_ZOOM_ADJUSTMENT >
      GetBaseZoomAdjustment(range.max * RAW_BASE_ZOOM_ADJUSTMENT_FACTOR, -1.0F))
  {
    return false;
  }
  if (MAX_ALLOWED_BASE_ZOOM_ADJUSTMENT <
      GetBaseZoomAdjustment(range.min * RAW_BASE_ZOOM_ADJUSTMENT_FACTOR, +1.0F))
  {
    return false;
  }
  if (MAX_ALLOWED_BASE_ZOOM_ADJUSTMENT <
      GetBaseZoomAdjustment(range.max * RAW_BASE_ZOOM_ADJUSTMENT_FACTOR, +1.0F))
  {
    return false;
  }

  return true;
}

inline auto ZoomVectorEffects::SetBaseZoomAdjustmentFactor(const float multiplier) noexcept -> void
{
  Expects(multiplier > 0.0F);

  m_baseZoomAdjustmentFactor = multiplier * RAW_BASE_ZOOM_ADJUSTMENT_FACTOR;

  Ensures(GetBaseZoomAdjustment(m_baseZoomAdjustmentFactor,
                                m_filterEffectsSettings->vitesse.GetRelativeSpeed()) <=
          MAX_ALLOWED_BASE_ZOOM_ADJUSTMENT);

  SetBaseZoomAdjustmentFactor();
}

inline auto ZoomVectorEffects::SetBaseZoomAdjustmentFactor() noexcept -> void
{
  // TODO(glk) Does GetRelativeSpeed change in between this setter use?
  const auto baseZoomAdjustment = GetBaseZoomAdjustment(
      m_baseZoomAdjustmentFactor, m_filterEffectsSettings->vitesse.GetRelativeSpeed());

  Ensures(MIN_ALLOWED_BASE_ZOOM_ADJUSTMENT <= baseZoomAdjustment);
  Ensures(baseZoomAdjustment <= MAX_ALLOWED_BASE_ZOOM_ADJUSTMENT);

  m_filterEffectsSettings->zoomAdjustmentEffect->SetBaseZoomAdjustment(
      {.x = baseZoomAdjustment, .y = baseZoomAdjustment});
}

constexpr auto ZoomVectorEffects::GetBaseZoomAdjustment(const float baseZoomAdjustmentFactor,
                                                        const float relativeSpeed) noexcept -> float
{
  return baseZoomAdjustmentFactor * (1.0F + relativeSpeed);
}

inline auto ZoomVectorEffects::GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
    -> Vec2dFlt
{
  // Amulet 2
  // vx = X * tan(dist);
  // vy = Y * tan(dist);
  return GetClampedZoomAdjustment(
      m_filterEffectsSettings->zoomAdjustmentEffect->GetZoomAdjustment(coords));
}

inline auto ZoomVectorEffects::GetClampedZoomAdjustment(const Vec2dFlt& zoomCoeffs) const noexcept
    -> Vec2dFlt
{
  return {.x = GetClampedZoomAdjustment(zoomCoeffs.x), .y = GetClampedZoomAdjustment(zoomCoeffs.y)};
}

inline auto ZoomVectorEffects::GetClampedZoomAdjustment(const float zoomAdjustment) const noexcept
    -> float
{
  if (zoomAdjustment < FilterSettingsService::MIN_ZOOM_ADJUSTMENT)
  {
    return FilterSettingsService::MIN_ZOOM_ADJUSTMENT;
  }
  if (zoomAdjustment > m_filterEffectsSettings->maxZoomAdjustment)
  {
    return m_filterEffectsSettings->maxZoomAdjustment;
  }
  return zoomAdjustment;
}

inline auto ZoomVectorEffects::GetAfterEffectsVelocity(
    const NormalizedCoords& coords,
    const float sqDistFromZero,
    const NormalizedCoords& zoomVelocity) const noexcept -> NormalizedCoords
{
  return m_zoomVectorAfterEffects.GetAfterEffectsVelocity(coords, sqDistFromZero, zoomVelocity);
}

inline auto ZoomVectorEffects::GetAfterEffectsVelocityMultiplier() const noexcept -> float
{
  return m_filterEffectsSettings->afterEffectsVelocityMultiplier;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
