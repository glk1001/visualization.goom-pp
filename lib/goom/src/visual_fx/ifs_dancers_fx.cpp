/*
 * ifs.c --- modified iterated functions system for goom.
 */

/*-
 * Copyright (c) 1997 by Massimino Pascal <Pascal.Massimon@ens.fr>
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 *
 * If this mode is weird and you have an old MetroX server, it is buggy.
 * There is a free SuSE-enhanced MetroX X server that is fine.
 *
 * When shown ifs, Diana Rose (4 years old) said, "It looks like dancing."
 *
 * Revision History:
 * 01-Feb-2021: converted to C++14, lots of color changes (glk)
 * 13-Dec-2003: Added some goom specific stuffs (to make ifs a VisualFX).
 * 11-Apr-2002: jeko@ios-software.com: Make ifs.c system-indendant. (ifs.h added)
 * 01-Nov-2000: Allocation checks
 * 10-May-1997: jwz@jwz.org: turned into a standalone program.
 *              Made it render into an offscreen bitmap and then copy
 *              that onto the screen, to reduce flicker.
 */

#include "ifs_dancers_fx.h"

#include "draw/goom_draw.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "goomutils/goomrand.h"
#include "goomutils/graphics/small_image_bitmaps.h"
#include "goomutils/logging_control.h"
#include "ifs/colorizer.h"
#include "ifs/fractal.h"
#include "ifs/low_density_blurrer.h"
//#undef NO_LOGGING
#include "goomutils/logging.h"
#include "goomutils/random_colormaps.h"
#include "goomutils/spimpl.h"
#include "goomutils/t_values.h"

#include <array>

//#undef NDEBUG
#include <cassert>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
#else
namespace GOOM::VISUAL_FX
{
#endif

using DRAW::IGoomDraw;
using IFS::BlurrerColorMode;
using IFS::Colorizer;
using IFS::Fractal;
using IFS::IfsPoint;
using IFS::LowDensityBlurrer;
using UTILS::GetRandInRange;
using UTILS::ProbabilityOfMInN;
using UTILS::RandomColorMaps;
using UTILS::SmallImageBitmaps;
using UTILS::TValue;
using UTILS::Weights;

inline auto MegaChangeColorMapEvent() -> bool
{
  return ProbabilityOfMInN(5, 10);
}

// clang-format off
static const Weights<BlurrerColorMode> BLURRER_COLOR_MODE_WEIGHTS{{
  {BlurrerColorMode::SMOOTH_WITH_NEIGHBOURS,1000},
  {BlurrerColorMode::SMOOTH_NO_NEIGHBOURS,     1},
  {BlurrerColorMode::SIMI_WITH_NEIGHBOURS,     1},
  {BlurrerColorMode::SIMI_NO_NEIGHBOURS,       5},
  {BlurrerColorMode::SINGLE_WITH_NEIGHBOURS,   1},
  {BlurrerColorMode::SINGLE_NO_NEIGHBOURS,     5},
}};
// clang-format on

class IfsDancersFx::IfsDancersFxImpl
{
public:
  IfsDancersFxImpl(const IGoomDraw& draw,
                   const PluginInfo& goomInfo,
                   const SmallImageBitmaps& smallBitmaps) noexcept;

  void Init();

  void ApplyNoDraw();
  void UpdateIfs();
  void UpdateLowDensityThreshold();
  auto GetColorMode() const -> IfsDancersFx::ColorMode;
  void SetColorMode(IfsDancersFx::ColorMode c);
  void Renew();
  void UpdateIncr();

  void SetWeightedColorMaps(std::shared_ptr<RandomColorMaps> weightedMaps);

  void Start();
  void Finish();

private:
  static constexpr int32_t MAX_COUNT_BEFORE_NEXT_UPDATE = 1000;
  static constexpr int32_t MIN_CYCLE_LENGTH = 1000;
  static constexpr int32_t MAX_CYCLE_LENGTH = 2000;
  int32_t m_cycleLength = MIN_CYCLE_LENGTH;

  const IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;

  Colorizer m_colorizer{};

  std::unique_ptr<Fractal> m_fractal{};

  int32_t m_cycle = 0;
  int32_t m_ifsIncr = 1; // dessiner l'ifs (0 = non: > = increment)
  int32_t m_decayIfs = 0; // disparition de l'ifs
  int32_t m_recayIfs = 0; // dedisparition de l'ifs
  [[nodiscard]] auto GetIfsIncr() const -> int;
  void UpdateCycle();
  void UpdateDecay();
  void UpdateDecayAndRecay();

  // TODO Move to simi
  TValue m_tMix{TValue::StepType::CONTINUOUS_REVERSIBLE, 0.01F};
  static constexpr float POINT_BRIGHTNESS = 2.0F;
  static constexpr float BITMAP_BRIGHTNESS = 1.0F;
  void ChangeColorMaps();
  void DrawNextIfsPoints();
  void DrawPoint(const IfsPoint& point, float t, float tMix) const;

  static constexpr uint32_t MAX_DENSITY_COUNT = 20;
  static constexpr uint32_t MIN_DENSITY_COUNT = 5;
  uint32_t m_lowDensityCount = MIN_DENSITY_COUNT;
  LowDensityBlurrer m_blurrer;
  float m_lowDensityBlurThreshold = 0.99F;
  [[nodiscard]] auto BlurLowDensityColors(size_t numPoints,
                                          const std::vector<IfsPoint>& lowDensityPoints) const
      -> bool;
  void SetLowDensityColors(std::vector<IfsPoint>& points, uint32_t maxLowDensityCount) const;
};

IfsDancersFx::IfsDancersFx(const IGoomDraw& draw,
                           const PluginInfo& goomInfo,
                           const SmallImageBitmaps& smallBitmaps) noexcept
  : m_fxImpl{spimpl::make_unique_impl<IfsDancersFxImpl>(draw, goomInfo, smallBitmaps)}
{
}

void IfsDancersFx::Init()
{
  m_fxImpl->Init();
}

void IfsDancersFx::SetWeightedColorMaps(const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_fxImpl->SetWeightedColorMaps(weightedMaps);
}

void IfsDancersFx::Start()
{
  m_fxImpl->Start();
}

void IfsDancersFx::Resume()
{
}

void IfsDancersFx::Suspend()
{
}

void IfsDancersFx::Finish()
{
  m_fxImpl->Finish();
}

auto IfsDancersFx::GetFxName() const -> std::string
{
  return "IFS FX";
}

void IfsDancersFx::ApplyNoDraw()
{
  m_fxImpl->ApplyNoDraw();
}

void IfsDancersFx::ApplyMultiple()
{
  m_fxImpl->UpdateIfs();
}

auto IfsDancersFx::GetColorMode() const -> IfsDancersFx::ColorMode
{
  return m_fxImpl->GetColorMode();
}

void IfsDancersFx::SetColorMode(const ColorMode c)
{
  m_fxImpl->SetColorMode(c);
}

void IfsDancersFx::UpdateIncr()
{
  m_fxImpl->UpdateIncr();
}

void IfsDancersFx::Renew()
{
  m_fxImpl->Renew();
}

IfsDancersFx::IfsDancersFxImpl::IfsDancersFxImpl(const IGoomDraw& draw,
                                                 const PluginInfo& goomInfo,
                                                 const SmallImageBitmaps& smallBitmaps) noexcept
  : m_draw{draw},
    m_goomInfo{goomInfo},
    m_fractal{std::make_unique<Fractal>(m_draw.GetScreenWidth(),
                                        m_draw.GetScreenHeight(),
                                        m_colorizer.GetColorMaps(),
                                        smallBitmaps)},
    m_blurrer{m_draw, 3, &m_colorizer}
{
}

void IfsDancersFx::IfsDancersFxImpl::Init()
{
  m_fractal->Init();
  UpdateLowDensityThreshold();
}

inline auto IfsDancersFx::IfsDancersFxImpl::GetColorMode() const -> IfsDancersFx::ColorMode
{
  return m_colorizer.GetColorMode();
}

inline void IfsDancersFx::IfsDancersFxImpl::SetColorMode(const IfsDancersFx::ColorMode c)
{
  return m_colorizer.SetForcedColorMode(c);
}

inline void IfsDancersFx::IfsDancersFxImpl::SetWeightedColorMaps(
    const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_colorizer.SetWeightedColorMaps(weightedMaps);
}

inline void IfsDancersFx::IfsDancersFxImpl::Start()
{
  Init();
}

inline void IfsDancersFx::IfsDancersFxImpl::Finish()
{
}

void IfsDancersFx::IfsDancersFxImpl::Renew()
{
  ChangeColorMaps();
  m_colorizer.ChangeColorMode();

  constexpr float MIN_SPEED_AMP = 1.1F;
  constexpr float MAX_SPEED_AMP = 5.1F;
  constexpr float MAX_SPEED_WEIGHT = 10.0F;
  const float speedAmp = std::min(GetRandInRange(MIN_SPEED_AMP, MAX_SPEED_WEIGHT), MAX_SPEED_AMP);
  const float accelFactor = 1.0F / (1.2F - m_goomInfo.GetSoundInfo().GetAcceleration());

  m_fractal->SetSpeed(std::max(1U, static_cast<uint32_t>(speedAmp * accelFactor)));
}

void IfsDancersFx::IfsDancersFxImpl::ChangeColorMaps()
{
  m_colorizer.ChangeColorMaps();
  m_blurrer.SetColorMode(BLURRER_COLOR_MODE_WEIGHTS.GetRandomWeighted());
  m_blurrer.SetSingleColor(m_colorizer.GetColorMaps().GetRandomColorMap().GetColor(0.5F));
}

void IfsDancersFx::IfsDancersFxImpl::ApplyNoDraw()
{
  UpdateDecayAndRecay();
  UpdateDecay();
}

void IfsDancersFx::IfsDancersFxImpl::UpdateIfs()
{
  UpdateDecayAndRecay();
  if (GetIfsIncr() <= 0)
  {
    return;
  }

  UpdateCycle();

  DrawNextIfsPoints();
}

void IfsDancersFx::IfsDancersFxImpl::UpdateDecayAndRecay()
{
  --m_decayIfs;
  if (m_decayIfs > 0)
  {
    m_ifsIncr += 2;
  }
  if (0 == m_decayIfs)
  {
    m_ifsIncr = 0;
  }

  if (m_recayIfs)
  {
    m_ifsIncr -= 2;
    --m_recayIfs;
    if ((0 == m_recayIfs) && (m_ifsIncr <= 0))
    {
      m_ifsIncr = 1;
    }
  }
}

void IfsDancersFx::IfsDancersFxImpl::UpdateIncr()
{
  if (m_ifsIncr <= 0)
  {
    m_recayIfs = 5;
    m_ifsIncr = 11;
    Renew();
  }
}

void IfsDancersFx::IfsDancersFxImpl::UpdateDecay()
{
  if ((m_ifsIncr > 0) && (m_decayIfs <= 0))
  {
    m_decayIfs = 100;
  }
}

inline auto IfsDancersFx::IfsDancersFxImpl::GetIfsIncr() const -> int
{
  return m_ifsIncr;
}

void IfsDancersFx::IfsDancersFxImpl::UpdateCycle()
{
  // TODO: trouver meilleur soluce pour increment (mettre le code de gestion de l'ifs
  //       dans ce fichier)
  //       find the best solution for increment (put the management code of the ifs in this file)
  m_tMix.Increment();

  ++m_cycle;
  if (m_cycle < m_cycleLength)
  {
    return;
  }

  m_cycle = 0;
  m_cycleLength = GetRandInRange(MIN_CYCLE_LENGTH, MAX_CYCLE_LENGTH + 1);

  if (ProbabilityOfMInN(15, 20))
  {
    m_lowDensityBlurThreshold = 0.99F;
  }
  else
  {
    m_lowDensityBlurThreshold = 0.40F;
  }

  m_fractal->Reset();

  Renew();
}

void IfsDancersFx::IfsDancersFxImpl::DrawNextIfsPoints()
{
  const std::vector<IfsPoint>& points = m_fractal->GetNextIfsPoints();
  const uint32_t maxHitCount = m_fractal->GetMaxHitCount();

  m_colorizer.SetMaxHitCount(maxHitCount);

  const size_t numPoints = points.size();
  const float tStep = (1 == numPoints) ? 0.0F : ((1.0F - 0.0F) / static_cast<float>(numPoints - 1));
  float t = -tStep;

  bool doneColorChange =
      (m_colorizer.GetColorMode() != IfsDancersFx::ColorMode::MEGA_MAP_COLOR_CHANGE) &&
      (m_colorizer.GetColorMode() != IfsDancersFx::ColorMode::MEGA_MIX_COLOR_CHANGE);
  uint32_t maxLowDensityCount = 0;
  uint32_t numSelectedPoints = 0;
  std::vector<IfsPoint> lowDensityPoints{};

  for (size_t i = 0; i < numPoints; i += static_cast<size_t>(GetIfsIncr()))
  {
    t += tStep;

    const uint32_t x = points[i].GetX();
    const uint32_t y = points[i].GetY();
    if ((x >= m_goomInfo.GetScreenInfo().width) || (y >= m_goomInfo.GetScreenInfo().height))
    {
      continue;
    }

    if ((!doneColorChange) && MegaChangeColorMapEvent())
    {
      ChangeColorMaps();
      doneColorChange = true;
    }

    ++numSelectedPoints;
    DrawPoint(points[i], t, m_tMix());

    if (points[i].GetCount() <= m_lowDensityCount)
    {
      (void)lowDensityPoints.emplace_back(points[i]);
      if (maxLowDensityCount < points[i].GetCount())
      {
        maxLowDensityCount = points[i].GetCount();
      }
    }
  }

  if (!BlurLowDensityColors(numSelectedPoints, lowDensityPoints))
  {
    SetLowDensityColors(lowDensityPoints, maxLowDensityCount);
  }
  else
  {
    // Enough dense points to make blurring worthwhile.
    if (ProbabilityOfMInN(4, 5))
    {
      m_blurrer.SetNeighbourMixFactor(0.98F);
    }
    else
    {
      m_blurrer.SetNeighbourMixFactor(GetRandInRange(0.90F, 1.0F));
    }
    m_blurrer.DoBlur(lowDensityPoints, maxLowDensityCount);
  }
}

inline void IfsDancersFx::IfsDancersFxImpl::DrawPoint(const IfsPoint& point,
                                                      const float t,
                                                      const float tMix) const
{
  const auto pX = static_cast<int32_t>(point.GetX());
  const auto pY = static_cast<int32_t>(point.GetY());

  const auto tX = static_cast<float>(pX) / static_cast<float>(m_draw.GetScreenWidth());
  const auto tY = static_cast<float>(pY) / static_cast<float>(m_draw.GetScreenHeight());

  const Pixel baseColor = point.GetSimiColorMap()->GetColor(t);

  //  const float t = static_cast<float>(m_cycle) / static_cast<float>(m_cycleLength);
  if (nullptr == point.GetSimiCurrentPointBitmap())
  {
    const Pixel mixedColor =
        m_colorizer.GetMixedColor(baseColor, point.GetCount(), POINT_BRIGHTNESS, tMix, tX, tY);
    m_draw.DrawPixels(pX, pY, {mixedColor, mixedColor});
  }
  else
  {
    const Pixel mixedColor =
        m_colorizer.GetMixedColor(baseColor, point.GetCount(), BITMAP_BRIGHTNESS, tMix, tX, tY);
    const auto getColor = [&]([[maybe_unused]] const size_t x, [[maybe_unused]] const size_t y,
                              [[maybe_unused]] const Pixel& b) -> Pixel { return mixedColor; };
    const PixelBuffer& bitmap{*point.GetSimiCurrentPointBitmap()};
    m_draw.Bitmap(pX, pY, bitmap, {getColor, getColor}, point.GetSimiOverExposeBitmaps());
  }
}

inline auto IfsDancersFx::IfsDancersFxImpl::BlurLowDensityColors(
    const size_t numPoints, const std::vector<IfsPoint>& lowDensityPoints) const -> bool
{
  if (0 == numPoints)
  {
    return false;
  }
  return (static_cast<float>(lowDensityPoints.size()) / static_cast<float>(numPoints)) >
         m_lowDensityBlurThreshold;
}

void IfsDancersFx::IfsDancersFxImpl::SetLowDensityColors(std::vector<IfsPoint>& points,
                                                         const uint32_t maxLowDensityCount) const
{
  const float logMaxLowDensityCount = std::log(static_cast<float>(maxLowDensityCount));

  float t = 0.0F;
  const float tStep = 1.0F / static_cast<float>(points.size());
  for (const auto& point : points)
  {
    const float logAlpha = point.GetCount() <= 1 ? 1.0F
                                                 : (std::log(static_cast<float>(point.GetCount())) /
                                                       logMaxLowDensityCount);

    DrawPoint(point, t, logAlpha);

    t += tStep;
  }
}

void IfsDancersFx::IfsDancersFxImpl::UpdateLowDensityThreshold()
{
  m_lowDensityCount = GetRandInRange(MIN_DENSITY_COUNT, MAX_DENSITY_COUNT);

  uint32_t blurWidth;
  constexpr uint32_t NUM_WIDTHS = 3;
  constexpr uint32_t WIDTH_RANGE = (MAX_DENSITY_COUNT - MIN_DENSITY_COUNT) / NUM_WIDTHS;
  if (m_lowDensityCount <= (MIN_DENSITY_COUNT + WIDTH_RANGE))
  {
    blurWidth = 7;
  }
  else if (m_lowDensityCount <= (MIN_DENSITY_COUNT + (2 * WIDTH_RANGE)))
  {
    blurWidth = 5;
  }
  else
  {
    blurWidth = 3;
  }
  m_blurrer.SetWidth(blurWidth);
}

#if __cplusplus <= 201402L
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX
#endif
