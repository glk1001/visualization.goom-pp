#pragma once

#include "goom_graphic.h"
#include "ifs_types.h"
#include "utils/t_values.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{

namespace COLOR
{
class IColorMap;
class ImageBitmap;
class RandomColorMaps;
class SmallImageBitmaps;
} // namespace COLOR

namespace UTILS
{
class ImageBitmap;
class SmallImageBitmaps;
} // namespace UTILS

namespace IFS
{
#else
namespace GOOM::IFS
{
#endif

struct Similitude;
class IfsPoint;

class FractalHits
{
public:
  FractalHits(uint32_t width, uint32_t height) noexcept;
  void Reset();
  void AddHit(uint32_t x, uint32_t y, const Similitude& s);
  [[nodiscard]] auto GetBuffer() -> const std::vector<IfsPoint>&;
  [[nodiscard]] auto GetMaxHitCount() const -> uint32_t;

private:
  const uint32_t m_width = 0;
  const uint32_t m_height = 0;
  struct HitInfo
  {
    uint32_t count = 0;
    Pixel color{0U};
    const Similitude* simi{};
  };
  std::vector<std::vector<HitInfo>> m_hitInfo{};
  uint32_t m_maxHitCount = 0;
  static constexpr size_t HITS_ESTIMATE = 1000;
  std::vector<IfsPoint> m_hits{};
  std::vector<IfsPoint> m_buffer{};
};

class Fractal
{
public:
  Fractal(uint32_t screenWidth,
          uint32_t screenHeight,
          const COLOR::RandomColorMaps& randomColorMaps,
          const UTILS::SmallImageBitmaps& smallBitmaps);

  void Init();
  void Reset();

  [[nodiscard]] auto GetSpeed() const -> uint32_t;
  void SetSpeed(uint32_t val);

  [[nodiscard]] auto GetNextIfsPoints() -> const std::vector<IfsPoint>&;
  [[nodiscard]] auto GetMaxHitCount() const -> uint32_t;

private:
  static constexpr size_t MAX_SIMI = 6;
  static constexpr size_t NUM_SIMI_GROUPS = 5;
  static constexpr uint32_t MIN_MAX_COUNT_TIMES_SPEED = 950;
  static constexpr uint32_t MAX_MAX_COUNT_TIMES_SPEED = 1500;
  std::unique_ptr<std::vector<Similitude>> m_components;

  const UTILS::SmallImageBitmaps& m_smallBitmaps;
  const COLOR::RandomColorMaps& m_colorMaps;

  const uint32_t m_lx;
  const uint32_t m_ly;
  uint32_t m_numSimi = 0;
  uint32_t m_depth = 0;
  uint32_t m_count = 0;

  Dbl m_r1Mean = 0.0;
  Dbl m_r2Mean = 0.0;
  Dbl m_dr1Mean = 0.0;
  Dbl m_dr2Mean = 0.0;

  static constexpr uint32_t INITIAL_SPEED = 6;
  uint32_t m_prevSpeed;
  uint32_t m_speed;
  static constexpr uint32_t NUM_SPEED_TRANSITION_STEPS = 500;
  UTILS::TValue m_speedTransitionT;
  uint32_t m_maxCountTimesSpeed = MAX_MAX_COUNT_TIMES_SPEED;

  FractalHits m_hits1;
  FractalHits m_hits2;
  std::reference_wrapper<FractalHits> m_prevHits;
  std::reference_wrapper<FractalHits> m_curHits;

  [[nodiscard]] auto GetLx() const -> Flt;
  [[nodiscard]] auto GetLy() const -> Flt;
  void DrawFractal();
  void RandomSimis(size_t start, size_t num);
  void ResetCurrentIfsFunc();
  void Trace(uint32_t curDepth, const FltPoint& p0);
  void UpdateHits(const Similitude& simi, const FltPoint& p);
  using IfsFunc = std::function<FltPoint(const Similitude& simi, Flt x1, Flt y1, Flt x2, Flt y2)>;
  IfsFunc m_curFunc{};
  [[nodiscard]] auto Transform(const Similitude& simi, const FltPoint& p0) const -> FltPoint;
  [[nodiscard]] static auto GaussRand(Dbl c, Dbl S, Dbl A_mult_1_minus_exp_neg_S) -> Dbl;
  [[nodiscard]] static auto HalfGaussRand(Dbl c, Dbl S, Dbl A_mult_1_minus_exp_neg_S) -> Dbl;
#if __cplusplus <= 201402L
  [[nodiscard]] static auto Get_1_minus_exp_neg_S(Dbl S) -> Dbl;
#else
  [[nodiscard]] static constexpr auto Get_1_minus_exp_neg_S(Dbl S) -> Dbl;
#endif
};

struct Similitude
{
  Dbl dbl_cx = 0;
  Dbl dbl_cy = 0;
  Dbl dbl_r1 = 0;
  Dbl dbl_r2 = 0;
  Dbl A1 = 0;
  Dbl A2 = 0;
  Flt cosA1 = 0;
  Flt sinA1 = 0;
  Flt cosA2 = 0;
  Flt sinA2 = 0;
  Flt cx = 0;
  Flt cy = 0;
  Flt r1 = 0;
  Flt r2 = 0;
  const COLOR::IColorMap* colorMap{};
  Pixel color = Pixel::BLACK;
  const UTILS::ImageBitmap* currentPointBitmap{};
  bool overExposeBitmaps = true;
};

class IfsPoint
{
public:
  IfsPoint() noexcept = default;
  IfsPoint(const uint32_t x, const uint32_t y, const uint32_t count) noexcept
    : m_x{x}, m_y{y}, m_count{count}
  {
  }
  [[nodiscard]] auto GetX() const -> uint32_t { return m_x; }
  [[nodiscard]] auto GetY() const -> uint32_t { return m_y; }
  void SetX(const uint32_t val) { m_x = val; }
  void SetY(const uint32_t val) { m_y = val; }
  [[nodiscard]] auto GetColor() const -> Pixel { return m_color; }
  void SetColor(const Pixel& val) { m_color = val; }
  [[nodiscard]] auto GetCount() const -> uint32_t { return m_count; }
  void SetCount(const uint32_t val) { m_count = val; }
  [[nodiscard]] auto GetSimiColor() const -> Pixel;
  [[nodiscard]] auto GetSimiColorMap() const -> const COLOR::IColorMap*;
  [[nodiscard]] auto GetSimiCurrentPointBitmap() const -> const UTILS::ImageBitmap*;
  [[nodiscard]] auto GetSimiOverExposeBitmaps() const -> bool;

private:
  uint32_t m_x = 0;
  uint32_t m_y = 0;
  uint32_t m_count = 0;
  Pixel m_color = Pixel::BLACK;
  const Similitude* m_simi{};
  friend class FractalHits;
};

inline auto Fractal::GetMaxHitCount() const -> uint32_t
{
  return m_curHits.get().GetMaxHitCount();
}

inline auto Fractal::GetLx() const -> Flt
{
  return static_cast<Flt>(m_lx);
}

inline auto Fractal::GetLy() const -> Flt
{
  return static_cast<Flt>(m_ly);
}

inline auto FractalHits::GetMaxHitCount() const -> uint32_t
{
  return m_maxHitCount;
}

#if __cplusplus <= 201402L
} // namespace IFS
} // namespace GOOM
#else
} // namespace GOOM::IFS
#endif

