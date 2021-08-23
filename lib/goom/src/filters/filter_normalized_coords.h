#ifndef VISUALIZATION_GOOM_FILTER_NORMALIZED_COORDS_H
#define VISUALIZATION_GOOM_FILTER_NORMALIZED_COORDS_H

#include "v2d.h"

#include <cstdint>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

class NormalizedCoords
{
public:
  // clang compiler doesn't like 'static constexpr' for C++14
  /*static constexpr*/ static const float MAX_NORMALIZED_COORD;
  /*static constexpr*/ static const float MIN_NORMALIZED_COORD;

  static void SetScreenDimensions(uint32_t width, uint32_t height, float minScreenCoordVal);
  static auto GetMinNormalizedCoordVal() -> float;

  explicit NormalizedCoords(const V2dInt& screenCoords) noexcept;
  explicit NormalizedCoords(const V2dFlt& normalized) noexcept;
  NormalizedCoords(float xNormalized, float yNormalized) noexcept;

  [[nodiscard]] auto GetScreenCoordsFlt() const -> V2dFlt;

  void Inc();
  void IncX();
  void IncY();

  [[nodiscard]] auto GetX() const -> float;
  [[nodiscard]] auto GetY() const -> float;
  [[nodiscard]] auto ToFlt() const -> V2dFlt;
  void SetX(float xNormalized);
  void SetY(float yNormalized);

  auto operator+=(const NormalizedCoords& other) -> NormalizedCoords&;
  auto operator-=(const NormalizedCoords& other) -> NormalizedCoords&;
  auto operator*=(float scalar) -> NormalizedCoords&;

private:
  static float s_ratioScreenToNormalizedCoord;
  static float s_ratioNormalizedToScreenCoord;
  static float s_minNormalizedCoordVal;
  V2dFlt m_normalizedCoords;

  [[nodiscard]] static auto NormalizedToScreenCoordsFlt(const V2dFlt& normalizedCoords) -> V2dFlt;
  [[nodiscard]] static auto ScreenToNormalizedCoords(const V2dInt& screenCoords) -> V2dFlt;
  [[nodiscard]] static auto ScreenToNormalizedCoord(int32_t screenCoord) -> float;
};

auto operator+(const NormalizedCoords& c1, const NormalizedCoords& c2)
    -> NormalizedCoords;
auto operator-(const NormalizedCoords& c1, const NormalizedCoords& c2)
    -> NormalizedCoords;
auto operator*(float scalar, const NormalizedCoords& c) -> NormalizedCoords;

inline auto NormalizedCoords::NormalizedToScreenCoordsFlt(const V2dFlt& normalizedCoords) -> V2dFlt
{
  return {s_ratioNormalizedToScreenCoord * (normalizedCoords.x - MIN_NORMALIZED_COORD),
          s_ratioNormalizedToScreenCoord * (normalizedCoords.y - MIN_NORMALIZED_COORD)};
}

inline auto NormalizedCoords::ScreenToNormalizedCoords(const V2dInt& screenCoords) -> V2dFlt
{
  return {ScreenToNormalizedCoord(screenCoords.x), ScreenToNormalizedCoord(screenCoords.y)};
}

inline auto NormalizedCoords::ScreenToNormalizedCoord(const int32_t screenCoord) -> float
{
  return MIN_NORMALIZED_COORD + (s_ratioScreenToNormalizedCoord * static_cast<float>(screenCoord));
}

inline auto NormalizedCoords::GetMinNormalizedCoordVal() -> float
{
  return s_minNormalizedCoordVal;
}

inline void NormalizedCoords::Inc()
{
  IncX();
  IncY();
}

inline void NormalizedCoords::IncX()
{
  m_normalizedCoords.x += s_ratioScreenToNormalizedCoord;
}

inline void NormalizedCoords::IncY()
{
  m_normalizedCoords.y += s_ratioScreenToNormalizedCoord;
}

inline NormalizedCoords::NormalizedCoords(const V2dInt& screenCoords) noexcept
  : m_normalizedCoords{ScreenToNormalizedCoords(screenCoords)}
{
}

inline NormalizedCoords::NormalizedCoords(const V2dFlt& normalized) noexcept
  : m_normalizedCoords{normalized}
{
}

inline NormalizedCoords::NormalizedCoords(const float xNormalized, const float yNormalized) noexcept
  : m_normalizedCoords{xNormalized, yNormalized}
{
}

inline auto NormalizedCoords::GetScreenCoordsFlt() const -> V2dFlt
{
  return NormalizedToScreenCoordsFlt(m_normalizedCoords);
}

inline auto NormalizedCoords::GetX() const -> float
{
  return m_normalizedCoords.x;
}

inline auto NormalizedCoords::GetY() const -> float
{
  return m_normalizedCoords.y;
}

inline auto NormalizedCoords::ToFlt() const -> V2dFlt
{
  return {GetX(), GetY()};
}

inline void NormalizedCoords::SetX(const float xNormalized)
{
  m_normalizedCoords.x = xNormalized;
}

inline void NormalizedCoords::SetY(const float yNormalized)
{
  m_normalizedCoords.y = yNormalized;
}

inline auto NormalizedCoords::operator+=(const NormalizedCoords& other) -> NormalizedCoords&
{
  m_normalizedCoords += other.m_normalizedCoords;
  return *this;
}

inline auto NormalizedCoords::operator-=(const NormalizedCoords& other) -> NormalizedCoords&
{
  m_normalizedCoords -= other.m_normalizedCoords;
  return *this;
}

inline auto NormalizedCoords::operator*=(const float scalar) -> NormalizedCoords&
{
  m_normalizedCoords *= scalar;
  return *this;
}

inline auto operator+(const NormalizedCoords& c1, const NormalizedCoords& c2) -> NormalizedCoords
{
  NormalizedCoords c3{c1};
  return c3 += c2;
}

inline auto operator-(const NormalizedCoords& c1, const NormalizedCoords& c2) -> NormalizedCoords
{
  NormalizedCoords c3{c1};
  return c3 -= c2;
}

inline auto operator*(const float scalar, const NormalizedCoords& c) -> NormalizedCoords
{
  NormalizedCoords c1{c};
  return c1 *= scalar;
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif

#endif //VISUALIZATION_GOOM_FILTER_NORMALIZED_COORDS_H
