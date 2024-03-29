#include "bezier_flower.h"

#include "bezier/bezier.h"
#include "color/color_maps_base.h"
#include "color/random_color_maps.h"
#include "goom/goom_config.h"
#include "goom/goom_graphic.h"
#include "goom/point2d.h"
#include "math/misc.h"

#include <cstdint>

namespace GOOM::VISUAL_FX::SHAPES
{
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using UTILS::MATH::TWO_PI;

auto BezierFlower::Draw(const uint32_t numPetals,
                        const Point2dInt& origin,
                        const uint32_t petalOffset) -> void
{
  auto petal         = GetBezierPetal(origin, petalOffset);
  const auto originX = static_cast<float>(origin.x);
  const auto originY = static_cast<float>(origin.y);

  m_petalColoring->SetNumPetals(numPetals);
  m_bezierDrawer->SetLineColorFunc(
      [this](const float t) { return m_petalColoring->GetCurrentLineColorMap().GetColor(t); });
  m_bezierDrawer->SetDotColorFunc([this](const float t)
                                  { return m_petalColoring->GetCurrentDotColorMap().GetColor(t); });

  const auto angleStep = 1.0F / static_cast<float>(numPetals);
  for (auto i = 0U; i < numPetals; ++i)
  {
    m_petalColoring->StartColoringPetal(i);
    const auto petalRotation = m_rotation + (angleStep * TWO_PI);
    petal.rotate(petalRotation, {originX, originY});
    m_bezierDrawer->Draw(petal, 0.0F, 1.0F);
  }
}

inline auto BezierFlower::GetBezierPetal(const Point2dInt& origin, const uint32_t offset)
    -> Bezier::Bezier<3>
{
  const auto originX   = static_cast<float>(origin.x);
  const auto originY   = static_cast<float>(origin.y);
  const auto fltOffset = static_cast<float>(offset);

  const auto petal = Bezier::Bezier<3>{
      {
       {originX, originY},
       {originX - fltOffset, originY - fltOffset},
       {originX - fltOffset, originY + fltOffset},
       {originX, originY},
       }
  };

  return petal;
}

void PetalColoring::SetNumPetals(const uint32_t numPetals)
{
  m_numPetals = numPetals;

  const auto randomColorMaps = RandomColorMaps{MAX_ALPHA, *m_goomRand};

  m_lineColorMaps.resize(m_numPetals, randomColorMaps.GetRandomColorMap());
  m_dotColorMaps.resize(m_numPetals, randomColorMaps.GetRandomColorMap());

  for (auto i = 0U; i < numPetals; ++i)
  {
    static constexpr auto COLOR_CHANGE_FREQUENCY = 5U;
    if (0 == (i % COLOR_CHANGE_FREQUENCY))
    {
      m_lineColorMaps[i] = randomColorMaps.GetRandomColorMap();
      m_dotColorMaps[i]  = randomColorMaps.GetRandomColorMap();
    }
  }
}

inline void PetalColoring::StartColoringPetal(const uint32_t petalNum)
{
  Expects(petalNum < m_numPetals);
  m_currentPetalNum = petalNum;
}

inline auto PetalColoring::GetCurrentLineColorMap() const -> const IColorMap&
{
  return m_lineColorMaps.at(m_currentPetalNum);
}

inline auto PetalColoring::GetCurrentDotColorMap() const -> const IColorMap&
{
  return m_dotColorMaps.at(m_currentPetalNum);
}

} // namespace GOOM::VISUAL_FX::SHAPES
