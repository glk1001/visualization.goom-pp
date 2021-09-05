#include "filter_zoom_vector_effects.h"

#include "filter_hypercos.h"
#include "filter_normalized_coords.h"
#include "filter_planes.h"
#include "filter_settings.h"
#include "goomutils/goomrand.h"
#include "goomutils/mathutils.h"

#include <cmath>
#include <cstdint>
#include <memory>
#include <string>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

using UTILS::GetRandInRange;
using UTILS::m_half_pi;

ZoomVectorEffects::ZoomVectorEffects(const uint32_t screenWidth,
                                     const std::string& resourcesDirectory) noexcept
  : m_screenWidth{screenWidth},
    m_resourcesDirectory{resourcesDirectory},
    m_hypercos{std::make_unique<Hypercos>()},
    m_planes{std::make_unique<Planes>()}
{
}

ZoomVectorEffects::~ZoomVectorEffects() noexcept = default;

void ZoomVectorEffects::SetFilterSettings(const ZoomFilterEffectsSettings& filterEffectsSettings)
{
  m_filterEffectsSettings = &filterEffectsSettings;

  m_filterEffectsSettings->speedCoefficientsEffect->SetRandomParams();

  SetHypercosOverlaySettings();
  SetRandomPlaneEffects();
}

void ZoomVectorEffects::SetRandomPlaneEffects()
{
  if (m_filterEffectsSettings->planeEffect)
  {
    m_planes->SetRandomParams(m_filterEffectsSettings->zoomMidPoint, m_screenWidth);
  }
}

void ZoomVectorEffects::SetHypercosOverlaySettings()
{
  switch (m_filterEffectsSettings->hypercosOverlay)
  {
    case HypercosOverlay::NONE:
      m_hypercos->SetDefaultParams();
      break;
    case HypercosOverlay::MODE0:
      m_hypercos->SetMode0RandomParams();
      break;
    case HypercosOverlay::MODE1:
      m_hypercos->SetMode1RandomParams();
      break;
    case HypercosOverlay::MODE2:
      m_hypercos->SetMode2RandomParams();
      break;
    case HypercosOverlay::MODE3:
      m_hypercos->SetMode3RandomParams();
      break;
  }
}

auto ZoomVectorEffects::GetCleanedVelocity(const NormalizedCoords& velocity) -> NormalizedCoords
{
  return {GetMinVelocityVal(velocity.GetX()), GetMinVelocityVal(velocity.GetY())};
}

inline auto ZoomVectorEffects::GetMinVelocityVal(const float velocityVal) -> float
{
  if (std::fabs(velocityVal) < NormalizedCoords::GetMinNormalizedCoordVal())
  {
    return velocityVal < 0.0F ? -NormalizedCoords::GetMinNormalizedCoordVal()
                              : NormalizedCoords::GetMinNormalizedCoordVal();
  }
  return velocityVal;
}

auto ZoomVectorEffects::GetHypercosVelocity(const NormalizedCoords& coords) const
    -> NormalizedCoords
{
  return m_hypercos->GetVelocity(coords);
}

auto ZoomVectorEffects::IsHorizontalPlaneVelocityActive() const -> bool
{
  return m_planes->IsHorizontalPlaneVelocityActive();
}

auto ZoomVectorEffects::GetHorizontalPlaneVelocity(const NormalizedCoords& coords) const -> float
{
  return m_planes->GetHorizontalPlaneVelocity(coords);
}

auto ZoomVectorEffects::IsVerticalPlaneVelocityActive() const -> bool
{
  return m_planes->IsVerticalPlaneVelocityActive();
}

auto ZoomVectorEffects::GetVerticalPlaneVelocity(const NormalizedCoords& coords) const -> float
{
  return m_planes->GetVerticalPlaneVelocity(coords);
}

auto ZoomVectorEffects::GetNoiseVelocity() const -> NormalizedCoords
{
  //    const float xAmp = 1.0/getRandInRange(50.0f, 200.0f);
  //    const float yAmp = 1.0/getRandInRange(50.0f, 200.0f);
  const float amp =
      (0.5F * m_filterEffectsSettings->noiseFactor) / GetRandInRange(NOISE_MIN, NOISE_MAX);
  return {GetRandInRange(-amp, +amp), GetRandInRange(-amp, +amp)};
}

auto ZoomVectorEffects::GetTanEffectVelocity(const float sqDistFromZero,
                                             const NormalizedCoords& velocity) const
    -> NormalizedCoords
{
  const float tanArg =
      stdnew::clamp(std::fmod(sqDistFromZero, m_half_pi), -0.75F * m_half_pi, 0.75F * m_half_pi);
  const float tanSqDist = std::tan(tanArg);
  return {tanSqDist * velocity.GetX(), tanSqDist * velocity.GetY()};
}

auto ZoomVectorEffects::GetRotatedVelocity(const NormalizedCoords& velocity) const
    -> NormalizedCoords
{
  if (m_filterEffectsSettings->rotateSpeed < 0.0F)
  {
    return {-m_filterEffectsSettings->rotateSpeed * (velocity.GetX() - velocity.GetY()),
            -m_filterEffectsSettings->rotateSpeed * (velocity.GetX() + velocity.GetY())};
  }

  return {m_filterEffectsSettings->rotateSpeed * (velocity.GetY() + velocity.GetX()),
          m_filterEffectsSettings->rotateSpeed * (velocity.GetY() - velocity.GetX())};
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
