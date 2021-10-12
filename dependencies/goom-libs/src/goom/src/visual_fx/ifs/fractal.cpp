#include "fractal.h"

#include "color/colormaps.h"
#include "color/colorutils.h"
#include "color/random_colormaps.h"
#include "utils/goomrand.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/mathutils.h"
#include "utils/t_values.h"

#undef NDEBUG
#include <cassert>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace IFS
{
#else
namespace GOOM::IFS
{
#endif

using COLOR::ColorMapGroup;
using COLOR::GetColorAverage;
using COLOR::RandomColorMaps;
using UTILS::GetRandInRange;
using UTILS::m_pi;
using UTILS::ProbabilityOfMInN;
using UTILS::SmallImageBitmaps;
using UTILS::TValue;
using UTILS::Weights;

struct CentreType
{
  uint32_t depth;
  Dbl r1Mean;
  Dbl r2Mean;
  Dbl dr1Mean;
  Dbl dr2Mean;
};

// clang-format off
static const std::vector<CentreType> CENTRE_LIST = {
  { /*.depth = */10, /*.r1Mean = */0.7F, /*.r2Mean = */0.0F, /*.dr1Mean = */0.3F, /*.dr2Mean = */0.4F },
  { /*.depth = */ 6, /*.r1Mean = */0.6F, /*.r2Mean = */0.0F, /*.dr1Mean = */0.4F, /*.dr2Mean = */0.3F },
  { /*.depth = */ 4, /*.r1Mean = */0.5F, /*.r2Mean = */0.0F, /*.dr1Mean = */0.4F, /*.dr2Mean = */0.3F },
  { /*.depth = */ 2, /*.r1Mean = */0.4F, /*.r2Mean = */0.0F, /*.dr1Mean = */0.5F, /*.dr2Mean = */0.3F },
};

static const Weights<size_t> CENTRE_WEIGHTS{{
  {0, 10},
  {1,  5},
  {2,  3},
  {3,  1},
}};
// clang-format on

auto IfsPoint::GetSimiColor() const -> Pixel
{
  return m_simi->color;
}

auto IfsPoint::GetSimiColorMap() const -> const COLOR::IColorMap*
{
  return m_simi->colorMap;
}

auto IfsPoint::GetSimiCurrentPointBitmap() const -> const UTILS::ImageBitmap*
{
  return m_simi->currentPointBitmap;
}

auto IfsPoint::GetSimiOverExposeBitmaps() const -> bool
{
  return m_simi->overExposeBitmaps;
}

Fractal::Fractal(const uint32_t screenWidth,
                 const uint32_t screenHeight,
                 const RandomColorMaps& randomColorMaps,
                 const SmallImageBitmaps& smallBitmaps)
  : m_components{std::make_unique<std::vector<Similitude>>(NUM_SIMI_GROUPS * MAX_SIMI)},
    m_smallBitmaps{smallBitmaps},
    m_colorMaps{randomColorMaps},
    m_lx{(screenWidth - 1U) / 2U},
    m_ly{(screenHeight - 1U) / 2U},
    m_prevSpeed{INITIAL_SPEED},
    m_speed{INITIAL_SPEED},
    m_speedTransitionT{TValue::StepType::SINGLE_CYCLE, NUM_SPEED_TRANSITION_STEPS},
    m_hits1{screenWidth, screenHeight},
    m_hits2{screenWidth, screenHeight},
    m_prevHits{m_hits1},
    m_curHits{m_hits2}
{
  m_speedTransitionT.Reset(TValue::MAX_T_VALUE);

  Init();
  Reset();
}

void Fractal::Init()
{
  assert(CENTRE_WEIGHTS.GetNumElements() == CENTRE_LIST.size());

  m_prevHits.get().Reset();
  m_curHits.get().Reset();

  const auto numCentres = static_cast<uint32_t>(2 + CENTRE_WEIGHTS.GetRandomWeighted());

  m_depth = CENTRE_LIST.at(numCentres - 2).depth;
  m_r1Mean = CENTRE_LIST[numCentres - 2].r1Mean;
  m_r2Mean = CENTRE_LIST[numCentres - 2].r2Mean;
  m_dr1Mean = CENTRE_LIST[numCentres - 2].dr1Mean;
  m_dr2Mean = CENTRE_LIST[numCentres - 2].dr2Mean;

  m_numSimi = numCentres;

  for (size_t i = 0; i < NUM_SIMI_GROUPS; ++i)
  {
    RandomSimis(i * MAX_SIMI, MAX_SIMI);
  }
}

auto Fractal::GetSpeed() const -> uint32_t
{
  if (m_speedTransitionT.IsStopped())
  {
    return m_speed;
  }
  return stdnew::lerp(m_prevSpeed, m_speed, m_speedTransitionT());
}

void Fractal::SetSpeed(const uint32_t val)
{
  m_prevSpeed = GetSpeed();
  m_speed = val;
  m_speedTransitionT.Reset();
}

void Fractal::Reset()
{
  m_maxCountTimesSpeed = GetRandInRange(MIN_MAX_COUNT_TIMES_SPEED, MAX_MAX_COUNT_TIMES_SPEED + 1U);

  ResetCurrentIfsFunc();
}

void Fractal::ResetCurrentIfsFunc()
{
  if (ProbabilityOfMInN(3, 10))
  {
    m_curFunc = [&](const Similitude& simi, const Flt x1, const Flt y1, const Flt x2,
                    const Flt y2) -> FltPoint {
      return {
          DivByUnit((x1 * simi.sinA1) - (y1 * simi.cosA1) + (x2 * simi.sinA2) - (y2 * simi.cosA2)) +
              simi.cx,
          DivByUnit((x1 * simi.cosA1) + (y1 * simi.sinA1) + (x2 * simi.cosA2) + (y2 * simi.sinA2)) +
              simi.cy,
      };
    };
  }
  else
  {
    m_curFunc = [&](const Similitude& simi, const Flt x1, const Flt y1, const Flt x2,
                    const Flt y2) -> FltPoint {
      return {
          DivByUnit((x1 * simi.cosA1) - (y1 * simi.sinA1) + (x2 * simi.cosA2) - (y2 * simi.sinA2)) +
              simi.cx,
          DivByUnit((x1 * simi.sinA1) + (y1 * simi.cosA1) + (x2 * simi.sinA2) + (y2 * simi.cosA2)) +
              simi.cy,
      };
    };
  }
}

auto Fractal::GetNextIfsPoints() -> const std::vector<IfsPoint>&
{
  m_speedTransitionT.Increment();

  const Dbl u = static_cast<Dbl>(m_count * GetSpeed()) / static_cast<Dbl>(m_maxCountTimesSpeed);
  const Dbl uSq = u * u;
  const Dbl v = 1.0F - u;
  const Dbl vSq = v * v;
  const Dbl u0 = vSq * v;
  const Dbl u1 = 3.0F * vSq * u;
  const Dbl u2 = 3.0F * v * uSq;
  const Dbl u3 = u * uSq;

  Similitude* s = m_components->data();
  Similitude* s0 = s + m_numSimi;
  Similitude* s1 = s0 + m_numSimi;
  const Similitude* s2 = s1 + m_numSimi;
  const Similitude* s3 = s2 + m_numSimi;

  for (size_t i = 0; i < m_numSimi; ++i)
  {
    s[i].dbl_cx =
        (u0 * s0[i].dbl_cx) + (u1 * s1[i].dbl_cx) + (u2 * s2[i].dbl_cx) + (u3 * s3[i].dbl_cx);
    s[i].dbl_cy =
        (u0 * s0[i].dbl_cy) + (u1 * s1[i].dbl_cy) + (u2 * s2[i].dbl_cy) + (u3 * s3[i].dbl_cy);

    s[i].dbl_r1 =
        (u0 * s0[i].dbl_r1) + (u1 * s1[i].dbl_r1) + (u2 * s2[i].dbl_r1) + (u3 * s3[i].dbl_r1);
    s[i].dbl_r2 =
        (u0 * s0[i].dbl_r2) + (u1 * s1[i].dbl_r2) + (u2 * s2[i].dbl_r2) + (u3 * s3[i].dbl_r2);

    s[i].A1 = (u0 * s0[i].A1) + (u1 * s1[i].A1) + (u2 * s2[i].A1) + (u3 * s3[i].A1);
    s[i].A2 = (u0 * s0[i].A2) + (u1 * s1[i].A2) + (u2 * s2[i].A2) + (u3 * s3[i].A2);
  }

  m_curHits.get().Reset();
  DrawFractal();
  const std::vector<IfsPoint>& curBuffer = m_curHits.get().GetBuffer();
  std::swap(m_prevHits, m_curHits);

  if (m_count < (m_maxCountTimesSpeed / GetSpeed()))
  {
    ++m_count;
  }
  else
  {
    s = m_components->data();
    s0 = s + m_numSimi;
    s1 = s0 + m_numSimi;
    s2 = s1 + m_numSimi;
    s3 = s2 + m_numSimi;

    for (size_t i = 0; i < m_numSimi; ++i)
    {
      s1[i].dbl_cx = (2.0F * s3[i].dbl_cx) - s2[i].dbl_cx;
      s1[i].dbl_cy = (2.0F * s3[i].dbl_cy) - s2[i].dbl_cy;

      s1[i].dbl_r1 = (2.0F * s3[i].dbl_r1) - s2[i].dbl_r1;
      s1[i].dbl_r2 = (2.0F * s3[i].dbl_r2) - s2[i].dbl_r2;

      s1[i].A1 = (2.0F * s3[i].A1) - s2[i].A1;
      s1[i].A2 = (2.0F * s3[i].A2) - s2[i].A2;

      s0[i] = s3[i];
    }

    RandomSimis(3 * m_numSimi, m_numSimi);
    RandomSimis(4 * m_numSimi, m_numSimi);

    m_count = 0;
  }

  return curBuffer;
}

void Fractal::DrawFractal()
{
  for (size_t i = 0; i < m_numSimi; ++i)
  {
    Similitude& simi = (*m_components)[i];

    simi.cx = DblToFlt(simi.dbl_cx);
    simi.cy = DblToFlt(simi.dbl_cy);

    simi.cosA1 = DblToFlt(std::cos(simi.A1));
    simi.sinA1 = DblToFlt(std::sin(simi.A1));
    simi.cosA2 = DblToFlt(std::cos(simi.A2));
    simi.sinA2 = DblToFlt(std::sin(simi.A2));

    simi.r1 = DblToFlt(simi.dbl_r1);
    simi.r2 = DblToFlt(simi.dbl_r2);
  }

  for (size_t i = 0; i < m_numSimi; ++i)
  {
    const FltPoint p0{(*m_components)[i].cx, (*m_components)[i].cy};

    for (size_t j = 0; j < m_numSimi; ++j)
    {
      if (i != j)
      {
        const FltPoint p = Transform((*m_components)[j], p0);
        Trace(m_depth, p);
      }
    }
  }
}

void Fractal::RandomSimis(const size_t start, const size_t num)
{
#if __cplusplus <= 201703L
  static const Dbl c_factor = 0.8F * Get_1_minus_exp_neg_S(4.0);
  static const Dbl r1_1_minus_exp_neg_S = Get_1_minus_exp_neg_S(3.0);
  static const Dbl r2_1_minus_exp_neg_S = Get_1_minus_exp_neg_S(2.0);
  static const Dbl A1_factor = 360.0F * Get_1_minus_exp_neg_S(4.0);
  static const Dbl A2_factor = A1_factor;
#else
  static const constinit Dbl c_factor = 0.8f * Get_1_minus_exp_neg_S(4.0);
  static const constinit Dbl r1_1_minus_exp_neg_S = Get_1_minus_exp_neg_S(3.0);
  static const constinit Dbl r2_1_minus_exp_neg_S = Get_1_minus_exp_neg_S(2.0);
  static const constinit Dbl A1_factor = 360.0F * Get_1_minus_exp_neg_S(4.0);
  static const constinit Dbl A2_factor = A1_factor;
#endif

  const Dbl r1Factor = m_dr1Mean * r1_1_minus_exp_neg_S;
  const Dbl r2Factor = m_dr2Mean * r2_1_minus_exp_neg_S;

  const ColorMapGroup colorMapGroup = m_colorMaps.GetRandomGroup();
  const bool useBitmaps = ProbabilityOfMInN(7, 10);

  for (size_t i = start; i < (start + num); ++i)
  {
    (*m_components)[i].dbl_cx = GaussRand(0.0, 4.0, c_factor);
    (*m_components)[i].dbl_cy = GaussRand(0.0, 4.0, c_factor);
    (*m_components)[i].dbl_r1 = GaussRand(m_r1Mean, 3.0, r1Factor);
    (*m_components)[i].dbl_r2 = HalfGaussRand(m_r2Mean, 2.0, r2Factor);
    (*m_components)[i].A1 = GaussRand(0.0F, 4.0F, A1_factor) * (m_pi / 180.0F);
    (*m_components)[i].A2 = GaussRand(0.0F, 4.0F, A2_factor) * (m_pi / 180.0F);
    (*m_components)[i].cosA1 = 0;
    (*m_components)[i].sinA1 = 0;
    (*m_components)[i].cosA2 = 0;
    (*m_components)[i].sinA2 = 0;
    (*m_components)[i].cx = 0;
    (*m_components)[i].cy = 0;
    (*m_components)[i].r1 = 0;
    (*m_components)[i].r2 = 0;

    (*m_components)[i].colorMap = &m_colorMaps.GetRandomColorMap(colorMapGroup);
    (*m_components)[i].color =
        RandomColorMaps::GetRandomColor(m_colorMaps.GetRandomColorMap(colorMapGroup), 0.0F, 1.0F);

    if (!useBitmaps)
    {
      (*m_components)[i].currentPointBitmap = nullptr;
    }
    else
    {
      constexpr uint32_t MIN_RES = 3;
      constexpr uint32_t MAX_RES = 5;
      const uint32_t res = GetRandInRange(MIN_RES, MAX_RES);
      (*m_components)[i].overExposeBitmaps = ProbabilityOfMInN(9, 10);
      if (ProbabilityOfMInN(6, 10))
      {
        (*m_components)[i].currentPointBitmap =
            &m_smallBitmaps.GetImageBitmap(SmallImageBitmaps::ImageNames::SPHERE, res);
      }
      else
      {
        (*m_components)[i].currentPointBitmap =
            &m_smallBitmaps.GetImageBitmap(SmallImageBitmaps::ImageNames::CIRCLE, res);
      }
    }
  }
}

#if __cplusplus <= 201402L
inline auto Fractal::Get_1_minus_exp_neg_S(const Dbl S) -> Dbl
#else
constexpr auto Fractal::Get_1_minus_exp_neg_S(const Dbl S) -> Dbl
#endif
{
  return 1.0F - std::exp(-S);
}

auto Fractal::GaussRand(const Dbl c, const Dbl S, const Dbl A_mult_1_minus_exp_neg_S) -> Dbl
{
  const Dbl x = GetRandInRange(0.0F, 1.0F);
  const Dbl y = A_mult_1_minus_exp_neg_S * (1.0F - std::exp(-x * x * S));
  return ProbabilityOfMInN(1, 2) ? (c + y) : (c - y);
}

auto Fractal::HalfGaussRand(const Dbl c, const Dbl S, const Dbl A_mult_1_minus_exp_neg_S) -> Dbl
{
  const Dbl x = GetRandInRange(0.0F, 1.0F);
  const Dbl y = A_mult_1_minus_exp_neg_S * (1.0F - std::exp(-x * x * S));
  return c + y;
}

void Fractal::Trace(const uint32_t curDepth, const FltPoint& p0)
{
  for (size_t i = 0; i < m_numSimi; ++i)
  {
    const FltPoint p = Transform((*m_components)[i], p0);

    UpdateHits((*m_components)[i], p);

    if (0 == curDepth)
    {
      continue;
    }
    if ((0 == ((p.x - p0.x) >> 4)) || (0 == ((p.y - p0.y) >> 4)))
    {
      continue;
    }

    Trace(curDepth - 1, p);
  }
}

inline auto Fractal::Transform(const Similitude& simi, const FltPoint& p0) const -> FltPoint
{
  const Flt x1 = DivByUnit((p0.x - simi.cx) * simi.r1);
  const Flt y1 = DivByUnit((p0.y - simi.cy) * simi.r1);

  const Flt x2 = DivByUnit((+x1 - simi.cx) * simi.r2);
  const Flt y2 = DivByUnit((-y1 - simi.cy) * simi.r2);

  return m_curFunc(simi, x1, y1, x2, y2);
}

inline void Fractal::UpdateHits(const Similitude& simi, const FltPoint& p)
{
  const auto x = static_cast<uint32_t>(GetLx() + DivBy2Units(p.x * GetLx()));
  const auto y = static_cast<uint32_t>(GetLy() - DivBy2Units(p.y * GetLy()));
  m_curHits.get().AddHit(x, y, simi);
}

FractalHits::FractalHits(const uint32_t width, const uint32_t height) noexcept
  : m_width{width}, m_height{height}, m_hitInfo(m_height)
{
  for (auto& xHit : m_hitInfo)
  {
    xHit.resize(m_width);
  }
  m_hits.reserve(HITS_ESTIMATE);
}

void FractalHits::Reset()
{
  m_maxHitCount = 0;
  m_hits.resize(0);
  for (auto& xHit : m_hitInfo)
  {
    std::fill(xHit.begin(), xHit.end(), HitInfo{0, Pixel{0U}});
  }
}

void FractalHits::AddHit(const uint32_t x, const uint32_t y, const Similitude& s)
{
  const uint32_t ux = x & 0x7fffffffU;
  const uint32_t uy = y & 0x7fffffffU;

  if ((ux >= m_width) || (uy >= m_height))
  {
    return;
  }

  HitInfo& h = m_hitInfo[uy][ux];

  h.simi = &s;
  h.color = GetColorAverage(h.color, s.color);
  ++h.count;

  if (h.count > m_maxHitCount)
  {
    m_maxHitCount = h.count;
  }

  if (1 == h.count)
  {
    m_hits.emplace_back(ux, uy, 1);
  }
}

auto FractalHits::GetBuffer() -> const std::vector<IfsPoint>&
{
  m_buffer.resize(m_hits.size());

  for (size_t i = 0; i < m_hits.size(); ++i)
  {
    IfsPoint pt = m_hits[i];
    pt.SetCount(m_hitInfo[pt.GetY()][pt.GetX()].count);
    pt.SetColor(m_hitInfo[pt.GetY()][pt.GetX()].color);
    pt.m_simi = m_hitInfo[pt.GetY()][pt.GetX()].simi;
    m_buffer[i] = pt;
  }

  return m_buffer;
}

#if __cplusplus <= 201402L
} // namespace IFS
} // namespace GOOM
#else
} // namespace GOOM::IFS
#endif
