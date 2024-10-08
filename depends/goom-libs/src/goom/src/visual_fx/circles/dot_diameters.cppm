module;

#include <algorithm>
#include <cstdint>
#include <vector>

export module Goom.VisualFx.CirclesFx.DotDiameters;

import Goom.Utils.Math.GoomRand;
import Goom.Lib.AssertUtils;

using GOOM::UTILS::MATH::NumberRange;

export namespace GOOM::VISUAL_FX::CIRCLES
{

class DotDiameters
{
public:
  struct DotProperties
  {
    uint32_t numDots;
    uint32_t minDiameter;
    uint32_t maxDiameter;
  };

  DotDiameters(const UTILS::MATH::GoomRand& goomRand, const DotProperties& dotProperties) noexcept;

  auto ChangeDiameters() noexcept -> void;

  [[nodiscard]] auto GetDiameters() const noexcept -> const std::vector<uint32_t>&;

private:
  const UTILS::MATH::GoomRand* m_goomRand;
  uint32_t m_numDots;

  static constexpr uint32_t MIN_DIAMETER_EXTRA = 2;
  uint32_t m_minDiameter;
  uint32_t m_maxDiameter;
  struct NumDotsAndMaxDiameter
  {
    uint32_t numDots;
    uint32_t maxDiameter;
  };
  std::vector<uint32_t> m_diameters{
      GetInitialDiameters({.numDots = m_numDots, .maxDiameter = m_maxDiameter})};
  [[nodiscard]] static auto GetInitialDiameters(
      const NumDotsAndMaxDiameter& numDotsAndMaxDiameter) noexcept -> std::vector<uint32_t>;

  static constexpr float PROB_FIXED_DIAMETER = 0.0F;
  auto ChangeToFixedDiameters() noexcept -> void;
  auto ChangeToVariableDiameters() noexcept -> void;
};

} // namespace GOOM::VISUAL_FX::CIRCLES

namespace GOOM::VISUAL_FX::CIRCLES
{

inline auto DotDiameters::GetDiameters() const noexcept -> const std::vector<uint32_t>&
{
  return m_diameters;
}

} // namespace GOOM::VISUAL_FX::CIRCLES

module :private;

namespace GOOM::VISUAL_FX::CIRCLES
{

DotDiameters::DotDiameters(const UTILS::MATH::GoomRand& goomRand,
                           const DotProperties& dotProperties) noexcept
  : m_goomRand{&goomRand},
    m_numDots{dotProperties.numDots},
    m_minDiameter{dotProperties.minDiameter + MIN_DIAMETER_EXTRA},
    m_maxDiameter{dotProperties.maxDiameter}
{
  Expects(dotProperties.numDots > 0);
  Expects((dotProperties.minDiameter + MIN_DIAMETER_EXTRA) <= dotProperties.maxDiameter);
  ChangeDiameters();
}

auto DotDiameters::GetInitialDiameters(const NumDotsAndMaxDiameter& numDotsAndMaxDiameter) noexcept
    -> std::vector<uint32_t>
{
  auto diameters = std::vector<uint32_t>(numDotsAndMaxDiameter.numDots);
  std::ranges::fill(diameters, numDotsAndMaxDiameter.maxDiameter);
  return diameters;
}

auto DotDiameters::ChangeDiameters() noexcept -> void
{
  if (m_goomRand->ProbabilityOf<PROB_FIXED_DIAMETER>())
  {
    ChangeToFixedDiameters();
  }
  else
  {
    ChangeToVariableDiameters();
  }
}

auto DotDiameters::ChangeToFixedDiameters() noexcept -> void
{
  const auto fixedDiameter = m_goomRand->GetRandInRange(NumberRange{m_minDiameter, m_maxDiameter});

  std::ranges::fill(m_diameters, fixedDiameter);
}

auto DotDiameters::ChangeToVariableDiameters() noexcept -> void
{
  const auto smallDiameter =
      m_goomRand->GetRandInRange(NumberRange{m_minDiameter, m_maxDiameter - 2});

  static constexpr auto INCREASED_DIAMETER = 3U;
  const auto minLargerDiameter = std::min(m_maxDiameter, smallDiameter + INCREASED_DIAMETER);
  const auto largerDotDiameter =
      m_goomRand->GetRandInRange(NumberRange{minLargerDiameter, m_maxDiameter});

  static constexpr auto LARGER_DIAMETER_FREQ_RANGE = NumberRange{2U, 5U};
  const auto largerDiameterEvery = m_goomRand->GetRandInRange<LARGER_DIAMETER_FREQ_RANGE>();

  for (auto i = 1U; i < m_numDots; ++i)
  {
    m_diameters.at(i) = 0 == (i % largerDiameterEvery) ? largerDotDiameter : smallDiameter;
  }
}

} // namespace GOOM::VISUAL_FX::CIRCLES
