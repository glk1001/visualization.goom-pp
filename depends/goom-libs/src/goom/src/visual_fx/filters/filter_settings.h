#pragma once

#include "goom/goom_graphic.h"
#include "utils/mathutils.h"
#include "v2d.h"

#include <cstdint>
#include <memory>

namespace GOOM
{
namespace VISUAL_FX
{
namespace FILTERS
{

// 128 = vitesse nule...
// 256 = en arriere
//   hyper vite.. * * 0 = en avant hype vite.
// 128 = zero speed
// 256 = reverse
//   super fast ... 0 = forward quickly.
class Vitesse
{
  static constexpr int32_t MAX_VITESSE = 128;

public:
  static constexpr int32_t STOP_SPEED = MAX_VITESSE;
  static constexpr int32_t FASTEST_SPEED = 0;
  static constexpr int32_t DEFAULT_VITESSE = 127;

  auto GetVitesse() const -> int32_t { return m_vitesse; };
  void SetVitesse(int32_t val);
  void SetDefault();
  void GoSlowerBy(int32_t val);
  void GoFasterBy(int32_t val);

  auto GetReverseVitesse() const -> bool { return m_reverseVitesse; }
  void SetReverseVitesse(const bool val) { m_reverseVitesse = val; }
  void ToggleReverseVitesse() { m_reverseVitesse = !m_reverseVitesse; }

  auto GetRelativeSpeed() const -> float;

private:
  int32_t m_vitesse = DEFAULT_VITESSE;
  bool m_reverseVitesse = true;
};

enum class HypercosOverlay
{
  NONE,
  MODE0,
  MODE1,
  MODE2,
  MODE3,
  _NUM // unused and must be last
};

struct ZoomFilterBufferSettings
{
  int32_t tranLerpIncrement;
  float tranLerpToMaxSwitchMult;
};

struct ZoomFilterColorSettings
{
  bool blockyWavy;
};

class ISpeedCoefficientsEffect;
class Rotation;

struct ZoomFilterEffectsSettings
{
  Vitesse vitesse;
  HypercosOverlay hypercosOverlay;

  float maxSpeedCoeff;
  std::shared_ptr<ISpeedCoefficientsEffect> speedCoefficientsEffect;
  std::shared_ptr<Rotation> rotation;

  V2dInt zoomMidPoint; // milieu de l'effet

  bool imageVelocityEffect;
  bool tanEffect;
  bool planeEffect;
  bool noiseEffect; // ajoute un bruit a la transformation
};

struct ZoomFilterSettings
{
  ZoomFilterEffectsSettings filterEffectsSettings{};
  ZoomFilterBufferSettings filterBufferSettings{};
  ZoomFilterColorSettings filterColorSettings{};
};

inline void Vitesse::SetVitesse(const int32_t val)
{
  m_vitesse = std::clamp(val, FASTEST_SPEED, STOP_SPEED);
}

inline void Vitesse::SetDefault()
{
  m_vitesse = DEFAULT_VITESSE;
  m_reverseVitesse = true;
}

inline void Vitesse::GoSlowerBy(const int32_t val)
{
  SetVitesse(m_vitesse + val);
}

inline void Vitesse::GoFasterBy(const int32_t val)
{
  SetVitesse(m_vitesse - (30 * val));
}

inline auto Vitesse::GetRelativeSpeed() const -> float
{
  const float speed = static_cast<float>(m_vitesse - MAX_VITESSE) / static_cast<float>(MAX_VITESSE);
  return m_reverseVitesse ? -speed : +speed;
}

} // namespace FILTERS
} // namespace VISUAL_FX
} // namespace GOOM

