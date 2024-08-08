module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numeric>

export module Goom.Lib.Point2d;

export namespace GOOM
{

struct Point2dFlt;
struct Vec2dInt;
struct Vec2dFlt;

struct Point2dInt
{
  int32_t x = 0;
  int32_t y = 0;
};

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
[[nodiscard]] constexpr auto GetPoint2dInt(int32_t x, int32_t y) noexcept -> Point2dInt = delete;
[[nodiscard]] constexpr auto GetPoint2dInt(uint32_t x, uint32_t y) noexcept -> Point2dInt;
#ifndef _MSC_VER
[[nodiscard]] constexpr auto GetPoint2dInt(size_t x, size_t y) noexcept -> Point2dInt;
#endif
// NOLINTEND(bugprone-easily-swappable-parameters)
[[nodiscard]] constexpr auto ToPoint2dFlt(const Point2dInt& point) noexcept -> Point2dFlt;

[[nodiscard]] constexpr auto operator==(const Point2dInt& point1,
                                        const Point2dInt& point2) noexcept -> bool;
[[nodiscard]] constexpr auto operator!=(const Point2dInt& point1,
                                        const Point2dInt& point2) noexcept -> bool;
[[nodiscard]] constexpr auto operator+(const Point2dInt& point,
                                       const Vec2dInt& vec) noexcept -> Point2dInt;
[[nodiscard]] constexpr auto operator-(const Point2dInt& point,
                                       const Vec2dInt& vec) noexcept -> Point2dInt;
[[nodiscard]] constexpr auto operator+(const Point2dInt& point,
                                       int32_t scalar) noexcept -> Point2dInt;
[[nodiscard]] constexpr auto operator-(const Point2dInt& point,
                                       int32_t scalar) noexcept -> Point2dInt;

[[nodiscard]] constexpr auto Translate(const Point2dInt& point,
                                       const Vec2dInt& vec) noexcept -> Point2dInt;
[[nodiscard]] auto lerp(const Point2dInt& point1,
                        const Point2dInt& point2,
                        float t) noexcept -> Point2dInt;
[[nodiscard]] constexpr auto clamp(const Point2dInt& point,
                                   const Point2dInt& point1,
                                   const Point2dInt& point2) noexcept -> Point2dInt;
[[nodiscard]] constexpr auto midpoint(const Point2dInt& point1,
                                      const Point2dInt& point2) noexcept -> Point2dInt;
[[nodiscard]] constexpr auto MidpointFromOrigin(const Point2dInt& point) noexcept -> Point2dInt;
[[nodiscard]] constexpr auto SqDistance(const Point2dInt& point1,
                                        const Point2dInt& point2) noexcept -> int32_t;
[[nodiscard]] constexpr auto SqDistanceFromZero(const Point2dInt& point) noexcept -> int32_t;
[[nodiscard]] auto Distance(const Point2dInt& point1, const Point2dInt& point2) noexcept -> int32_t;

struct Vec2dInt
{
  int32_t x = 0;
  int32_t y = 0;
};

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
[[nodiscard]] constexpr auto GetVec2dInt(int32_t x, int32_t y) noexcept -> Vec2dInt = delete;
[[nodiscard]] constexpr auto GetVec2dInt(uint32_t x, uint32_t y) noexcept -> Vec2dInt;
// NOLINTEND(bugprone-easily-swappable-parameters)
[[nodiscard]] constexpr auto ToVec2dInt(const Point2dInt& point) noexcept -> Vec2dInt;
[[nodiscard]] constexpr auto ToVec2dFlt(const Vec2dInt& vec2dInt) noexcept -> Vec2dFlt;

[[nodiscard]] constexpr auto operator==(const Vec2dInt& vec1,
                                        const Vec2dInt& vec2) noexcept -> bool;
[[nodiscard]] constexpr auto operator+(const Vec2dInt& vec1,
                                       const Vec2dInt& vec2) noexcept -> Vec2dInt;
[[nodiscard]] constexpr auto operator-(const Vec2dInt& vec1,
                                       const Vec2dInt& vec2) noexcept -> Vec2dInt;
[[nodiscard]] constexpr auto operator+(const Vec2dInt& vec1, int32_t scalar) noexcept -> Vec2dInt;
[[nodiscard]] constexpr auto operator-(const Vec2dInt& vec1, int32_t scalar) noexcept -> Vec2dInt;

struct Point2dFlt
{
  float x = 0.0F;
  float y = 0.0F;
};

[[nodiscard]] auto ToPoint2dInt(const Point2dFlt& point) noexcept -> Point2dInt;

[[nodiscard]] constexpr auto operator+(const Point2dFlt& point,
                                       const Vec2dFlt& vec) noexcept -> Point2dFlt;
[[nodiscard]] constexpr auto operator-(const Point2dFlt& point,
                                       const Vec2dFlt& vec) noexcept -> Point2dFlt;
[[nodiscard]] constexpr auto operator+(const Point2dFlt& point,
                                       float scalar) noexcept -> Point2dFlt;
[[nodiscard]] constexpr auto operator-(const Point2dFlt& point,
                                       float scalar) noexcept -> Point2dFlt;
[[nodiscard]] constexpr auto operator-(float scalar,
                                       const Point2dFlt& point) noexcept -> Point2dFlt;

[[nodiscard]] constexpr auto Translate(const Point2dFlt& point,
                                       const Vec2dFlt& vec) noexcept -> Point2dFlt;
[[nodiscard]] constexpr auto Translate(const Point2dFlt& point,
                                       float scalar) noexcept -> Point2dFlt;
[[nodiscard]] constexpr auto TranslateX(const Point2dFlt& point,
                                        float xAmount) noexcept -> Point2dFlt;
[[nodiscard]] constexpr auto TranslateY(const Point2dFlt& point,
                                        float yAmount) noexcept -> Point2dFlt;
[[nodiscard]] constexpr auto Scale(const Point2dFlt& point, float scalar) noexcept -> Point2dFlt;
[[nodiscard]] constexpr auto Scale(const Point2dFlt& point,
                                   float xScalar,
                                   float yScalar) noexcept -> Point2dFlt;
[[nodiscard]] auto Rotate(const Point2dFlt& point, float angle) noexcept -> Point2dFlt;
[[nodiscard]] constexpr auto Rotate(const Point2dFlt& point,
                                    float sinAngle,
                                    float cosAngle) noexcept -> Point2dFlt;
[[nodiscard]] constexpr auto lerp(const Point2dFlt& point1,
                                  const Point2dFlt& point2,
                                  float t) noexcept -> Point2dFlt;
[[nodiscard]] constexpr auto clamp(const Point2dFlt& point,
                                   const Point2dFlt& point1,
                                   const Point2dFlt& point2) noexcept -> Point2dFlt;
[[nodiscard]] constexpr auto SqDistance(const Point2dFlt& point1,
                                        const Point2dFlt& point2) noexcept -> float;
[[nodiscard]] auto Distance(const Point2dFlt& point1, const Point2dFlt& point2) noexcept -> float;

struct Vec2dFlt
{
  float x = 0.0F;
  float y = 0.0F;
};

[[nodiscard]] constexpr auto ToVec2dFlt(const Point2dInt& point) noexcept -> Vec2dFlt;
[[nodiscard]] constexpr auto ToVec2dFlt(const Point2dFlt& point) noexcept -> Vec2dFlt;
[[nodiscard]] auto ToVec2dInt(const Vec2dFlt& vec2dFlt) noexcept -> Vec2dInt;

[[nodiscard]] constexpr auto operator+(const Vec2dFlt& vec1,
                                       const Vec2dFlt& vec2) noexcept -> Vec2dFlt;
[[nodiscard]] constexpr auto operator-(const Vec2dFlt& vec1,
                                       const Vec2dFlt& vec2) noexcept -> Vec2dFlt;
[[nodiscard]] constexpr auto operator+(const Vec2dFlt& vec, float scalar) noexcept -> Vec2dFlt;
[[nodiscard]] constexpr auto operator-(const Vec2dFlt& vec, float scalar) noexcept -> Vec2dFlt;
[[nodiscard]] constexpr auto operator*(float scale, const Vec2dFlt& vec) noexcept -> Vec2dFlt;
[[nodiscard]] constexpr auto lerp(const Vec2dFlt& vec1,
                                  const Vec2dFlt& vec2,
                                  float t) noexcept -> Vec2dFlt;

} // namespace GOOM

namespace GOOM
{

constexpr auto GetPoint2dInt(const uint32_t x, const uint32_t y) noexcept -> Point2dInt
{
  return {.x = static_cast<int32_t>(x), .y = static_cast<int32_t>(y)};
}

#ifndef _MSC_VER
constexpr auto GetPoint2dInt(const size_t x, const size_t y) noexcept -> Point2dInt
{
  return {.x = static_cast<int32_t>(x), .y = static_cast<int32_t>(y)};
}
#endif

constexpr auto ToPoint2dFlt(const Point2dInt& point) noexcept -> Point2dFlt
{
  return {.x = static_cast<float>(point.x), .y = static_cast<float>(point.y)};
}

constexpr auto operator==(const Point2dInt& point1, const Point2dInt& point2) noexcept -> bool
{
  return (point1.x == point2.x) && (point1.y == point2.y);
}

constexpr auto operator!=(const Point2dInt& point1, const Point2dInt& point2) noexcept -> bool
{
  return not(point1 == point2);
}

constexpr auto operator+(const Point2dInt& point, const Vec2dInt& vec) noexcept -> Point2dInt
{
  return {.x = point.x + vec.x, .y = point.y + vec.y};
}

constexpr auto operator-(const Point2dInt& point, const Vec2dInt& vec) noexcept -> Point2dInt
{
  return {.x = point.x - vec.x, .y = point.y - vec.y};
}

constexpr auto operator+(const Point2dInt& point, const int32_t scalar) noexcept -> Point2dInt
{
  return {.x = point.x + scalar, .y = point.y + scalar};
}

constexpr auto operator-(const Point2dInt& point, const int32_t scalar) noexcept -> Point2dInt
{
  return {.x = point.x - scalar, .y = point.y - scalar};
}

constexpr auto Translate(const Point2dInt& point, const Vec2dInt& vec) noexcept -> Point2dInt
{
  return {.x = point.x + vec.x, .y = point.y + vec.y};
}

constexpr auto GetVec2dInt(const uint32_t x, const uint32_t y) noexcept -> Vec2dInt
{
  return {.x = static_cast<int32_t>(x), .y = static_cast<int32_t>(y)};
}

constexpr auto ToVec2dInt(const Point2dInt& point) noexcept -> Vec2dInt
{
  return {.x = point.x, .y = point.y};
}

constexpr auto ToVec2dFlt(const Vec2dInt& vec2dInt) noexcept -> Vec2dFlt
{
  return {.x = static_cast<float>(vec2dInt.x), .y = static_cast<float>(vec2dInt.y)};
}

constexpr auto operator==(const Vec2dInt& vec1, const Vec2dInt& vec2) noexcept -> bool
{
  return (vec1.x == vec2.x) && (vec1.y == vec2.y);
}

constexpr auto operator+(const Vec2dInt& vec1, const Vec2dInt& vec2) noexcept -> Vec2dInt
{
  return {.x = vec1.x + vec2.x, .y = vec1.y + vec2.y};
}

constexpr auto operator-(const Vec2dInt& vec1, const Vec2dInt& vec2) noexcept -> Vec2dInt
{
  return {.x = vec1.x - vec2.x, .y = vec1.y - vec2.y};
}

constexpr auto operator+(const Vec2dInt& vec1, const int32_t scalar) noexcept -> Vec2dInt
{
  return {.x = vec1.x + scalar, .y = vec1.y + scalar};
}

constexpr auto operator-(const Vec2dInt& vec1, const int32_t scalar) noexcept -> Vec2dInt
{
  return {.x = vec1.x - scalar, .y = vec1.y - scalar};
}

inline auto ToPoint2dInt(const Point2dFlt& point) noexcept -> Point2dInt
{
  return {.x = static_cast<int32_t>(std::round(point.x)),
          .y = static_cast<int32_t>(std::round(point.y))};
}

constexpr auto operator+(const Point2dFlt& point, const Vec2dFlt& vec) noexcept -> Point2dFlt
{
  return {.x = point.x + vec.x, .y = point.y + vec.y};
}

constexpr auto operator-(const Point2dFlt& point, const Vec2dFlt& vec) noexcept -> Point2dFlt
{
  return {.x = point.x - vec.x, .y = point.y - vec.y};
}

constexpr auto operator+(const Point2dFlt& point, const float scalar) noexcept -> Point2dFlt
{
  return {.x = point.x + scalar, .y = point.y + scalar};
}

constexpr auto operator-(const Point2dFlt& point, const float scalar) noexcept -> Point2dFlt
{
  return {.x = point.x - scalar, .y = point.y - scalar};
}

constexpr auto operator-(const float scalar, const Point2dFlt& point) noexcept -> Point2dFlt
{
  return {.x = scalar - point.x, .y = scalar - point.y};
}

constexpr auto operator*(const float scalar, const Point2dFlt& point) noexcept -> Point2dFlt
{
  return {.x = scalar * point.x, .y = scalar * point.y};
}

constexpr auto Translate(const Point2dFlt& point, const Vec2dFlt& vec) noexcept -> Point2dFlt
{
  return {.x = point.x + vec.x, .y = point.y + vec.y};
}

constexpr auto Translate(const Point2dFlt& point, const float scalar) noexcept -> Point2dFlt
{
  return {.x = point.x + scalar, .y = point.y + scalar};
}

constexpr auto TranslateX(const Point2dFlt& point, const float xAmount) noexcept -> Point2dFlt
{
  return {.x = point.x + xAmount, .y = point.y};
}

constexpr auto TranslateY(const Point2dFlt& point, const float yAmount) noexcept -> Point2dFlt
{
  return {.x = point.x, .y = point.y + yAmount};
}

inline auto Rotate(const Point2dFlt& point, const float angle) noexcept -> Point2dFlt
{
  const float sinAngle = std::sin(angle);
  const float cosAngle = std::cos(angle);
  return Rotate(point, sinAngle, cosAngle);
}

constexpr auto Rotate(const Point2dFlt& point,
                      const float sinAngle,
                      float const cosAngle) noexcept -> Point2dFlt
{
  return {.x = (point.x * cosAngle) - (point.y * sinAngle),
          .y = (point.x * sinAngle) + (point.y * cosAngle)};
}

constexpr auto Scale(const Point2dFlt& point, const float scalar) noexcept -> Point2dFlt
{
  return {.x = point.x * scalar, .y = point.y * scalar};
}

constexpr auto Scale(const Point2dFlt& point,
                     const float xScalar,
                     const float yScalar) noexcept -> Point2dFlt
{
  return {.x = point.x * xScalar, .y = point.y * yScalar};
}

constexpr auto ToVec2dFlt(const Point2dInt& point) noexcept -> Vec2dFlt
{
  return {.x = static_cast<float>(point.x), .y = static_cast<float>(point.y)};
}

constexpr auto ToVec2dFlt(const Point2dFlt& point) noexcept -> Vec2dFlt
{
  return {.x = point.x, .y = point.y};
}

inline auto ToVec2dInt(const Vec2dFlt& vec2dFlt) noexcept -> Vec2dInt
{
  return {.x = static_cast<int32_t>(std::round(vec2dFlt.x)),
          .y = static_cast<int32_t>(std::round(vec2dFlt.y))};
}

constexpr auto operator+(const Vec2dFlt& vec1, const Vec2dFlt& vec2) noexcept -> Vec2dFlt
{
  return {.x = vec1.x + vec2.x, .y = vec1.y + vec2.y};
}

constexpr auto operator-(const Vec2dFlt& vec1, const Vec2dFlt& vec2) noexcept -> Vec2dFlt
{
  return {.x = vec1.x - vec2.x, .y = vec1.y - vec2.y};
}

constexpr auto operator+(const Vec2dFlt& vec, const float scalar) noexcept -> Vec2dFlt
{
  return {.x = vec.x + scalar, .y = vec.y + scalar};
}

constexpr auto operator-(const Vec2dFlt& vec, const float scalar) noexcept -> Vec2dFlt
{
  return {.x = vec.x - scalar, .y = vec.y - scalar};
}

constexpr auto operator*(const float scale, const Vec2dFlt& vec) noexcept -> Vec2dFlt
{
  return {.x = scale * vec.x, .y = scale * vec.y};
}

inline auto lerp(const Point2dInt& point1,
                 const Point2dInt& point2,
                 const float t) noexcept -> Point2dInt
{
  return {
      .x = static_cast<int32_t>(
          std::round(std::lerp(static_cast<float>(point1.x), static_cast<float>(point2.x), t))),
      .y = static_cast<int32_t>(
          std::round(std::lerp(static_cast<float>(point1.y), static_cast<float>(point2.y), t))),
  };
}

constexpr auto clamp(const Point2dInt& point,
                     const Point2dInt& point1,
                     const Point2dInt& point2) noexcept -> Point2dInt
{
  return {.x = std::clamp(point.x, point1.x, point2.x),
          .y = std::clamp(point.y, point1.y, point2.y)};
}

constexpr auto lerp(const Point2dFlt& point1,
                    const Point2dFlt& point2,
                    const float t) noexcept -> Point2dFlt
{
  return {
      .x = std::lerp(point1.x, point2.x, t),
      .y = std::lerp(point1.y, point2.y, t),
  };
}

constexpr auto clamp(const Point2dFlt& point,
                     const Point2dFlt& point1,
                     const Point2dFlt& point2) noexcept -> Point2dFlt
{
  return {.x = std::clamp(point.x, point1.x, point2.x),
          .y = std::clamp(point.y, point1.y, point2.y)};
}

constexpr auto lerp(const Vec2dFlt& vec1, const Vec2dFlt& vec2, const float t) noexcept -> Vec2dFlt
{
  return {
      .x = std::lerp(vec1.x, vec2.x, t),
      .y = std::lerp(vec1.y, vec2.y, t),
  };
}

constexpr auto midpoint(const Point2dInt& point1, const Point2dInt& point2) noexcept -> Point2dInt
{
  return {.x = std::midpoint(point1.x, point2.x), .y = std::midpoint(point1.y, point2.y)};
}

constexpr auto MidpointFromOrigin(const Point2dInt& point) noexcept -> Point2dInt
{
  return {.x = std::midpoint(0, point.x), .y = std::midpoint(0, point.y)};
}

inline auto Distance(const Point2dInt& point1, const Point2dInt& point2) noexcept -> int32_t
{
  return static_cast<int32_t>(std::lround(Distance(ToPoint2dFlt(point1), ToPoint2dFlt(point2))));
}

constexpr auto SqDistance(const Point2dInt& point1, const Point2dInt& point2) noexcept -> int32_t
{
  const auto xDiff = point1.x - point2.x;
  const auto yDiff = point1.y - point2.y;
  return (xDiff * xDiff) + (yDiff * yDiff);
}

constexpr auto SqDistance(const Point2dFlt& point1, const Point2dFlt& point2) noexcept -> float
{
  const auto xDiff = point1.x - point2.x;
  const auto yDiff = point1.y - point2.y;
  return (xDiff * xDiff) + (yDiff * yDiff);
}

constexpr auto SqDistanceFromZero(const Point2dInt& point) noexcept -> int32_t
{
  return (point.x * point.x) + (point.y * point.y);
}

inline auto Distance(const Point2dFlt& point1, const Point2dFlt& point2) noexcept -> float
{
  return std::sqrt(SqDistance(point1, point2));
}

} // namespace GOOM
