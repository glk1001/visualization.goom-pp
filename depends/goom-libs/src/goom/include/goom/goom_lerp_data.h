#pragma once

#include "goom_config.h"
#include "math20.h"

#include <algorithm>

namespace GOOM
{

class GoomLerpData
{
public:
  constexpr GoomLerpData() noexcept = default;
  constexpr GoomLerpData(float lerpFactor, float increment, float lerpToOneFactor) noexcept;

  constexpr auto Reset() noexcept -> void;

  [[nodiscard]] constexpr auto GetIncrement() const noexcept -> float;
  constexpr auto SetIncrement(float increment) noexcept -> void;
  [[nodiscard]] constexpr auto GetLerpToOneFactor() const noexcept -> float;
  constexpr auto SetLerpToOneFactor(float lerpToOneFactor) noexcept -> void;
  constexpr auto SetLerpFactor(float lerpFactor) noexcept -> void;

  constexpr auto Update() noexcept -> void;

  constexpr auto GetLerpFactor() const noexcept -> float;

private:
  float m_lerpFactor      = 0.0F;
  float m_increment       = 0.0F;
  float m_lerpToOneFactor = 0.0F;
};

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
constexpr GoomLerpData::GoomLerpData(const float lerpFactor,
                                     const float increment,
                                     const float lerpToOneFactor) noexcept
  : m_lerpFactor{lerpFactor}, m_increment{increment}, m_lerpToOneFactor{lerpToOneFactor}
{
  Expects(lerpFactor >= 0.0F);
  Expects(lerpFactor <= 1.0F);
  Expects(increment > -1.0F);
  Expects(increment < +1.0F);
  Expects(lerpToOneFactor >= 0.0F);
  Expects(lerpToOneFactor <= 1.0F);
}

constexpr auto GoomLerpData::Reset() noexcept -> void
{
  m_lerpFactor      = 0.0F;
  m_increment       = 0.0F;
  m_lerpToOneFactor = 0.0F;
}

constexpr auto GoomLerpData::GetIncrement() const noexcept -> float
{
  return m_increment;
}

constexpr auto GoomLerpData::SetIncrement(const float increment) noexcept -> void
{
  Expects(increment > -1.0F);
  Expects(increment < +1.0F);

  m_increment = increment;
}

constexpr auto GoomLerpData::GetLerpToOneFactor() const noexcept -> float
{
  return m_lerpToOneFactor;
}

constexpr auto GoomLerpData::SetLerpToOneFactor(const float lerpToOneFactor) noexcept -> void
{
  Expects(lerpToOneFactor >= 0.0F);
  Expects(lerpToOneFactor <= 1.0F);

  m_lerpToOneFactor = lerpToOneFactor;
}

constexpr auto GoomLerpData::SetLerpFactor(const float lerpFactor) noexcept -> void
{
  Expects(lerpFactor >= 0.0F);
  Expects(lerpFactor <= 1.0F);
  m_lerpFactor = lerpFactor;
}

constexpr auto GoomLerpData::Update() noexcept -> void
{
  m_lerpFactor = std::clamp(m_lerpFactor + m_increment, 0.0F, 1.0F);
  m_lerpFactor = STD20::lerp(m_lerpFactor, 1.0F, m_lerpToOneFactor);
}

constexpr auto GoomLerpData::GetLerpFactor() const noexcept -> float
{
  return m_lerpFactor;
}

} // namespace GOOM
