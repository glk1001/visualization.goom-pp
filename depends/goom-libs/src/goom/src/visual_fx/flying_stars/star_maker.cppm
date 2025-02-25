module;

#include <cmath>
#include <cstdint>

export module Goom.VisualFx.FlyingStarsFx:StarMaker;

import Goom.Utils.Math.TValues;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.AssertUtils;
import Goom.Lib.Point2d;

// TODO(glk): fix this
#if defined(__clang_major__) && __clang_major__ >= 20
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimport-implementation-partition-unit-in-interface-unit"
#endif
import :StarTypesContainer;
import :StarColorsMaker;
import :Stars;
#if defined(__clang_major__) && __clang_major__ >= 20
#pragma GCC diagnostic pop
#endif

using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::NumberRange;

namespace GOOM::VISUAL_FX::FLYING_STARS
{

class StarMaker
{
public:
  explicit StarMaker(const GoomRand& goomRand) noexcept;

  struct StarProperties
  {
    float heightRatio{};
    float defaultPathLength{};
    float nominalPathLengthFactor{};
  };
  auto StartNewCluster(const IStarType& starType,
                       uint32_t numStarsInCluster,
                       const StarProperties& starProperties) noexcept -> void;

  [[nodiscard]] auto MoreStarsToMake() const noexcept -> bool;
  [[nodiscard]] auto MakeNewStar() noexcept -> Star;

private:
  const GoomRand* m_goomRand;
  const IStarType* m_starType{};
  uint32_t m_numStarsToMake = 0U;
  IStarType::SetupParams m_starSetupParams{};
  UTILS::MATH::TValue m_withinClusterT{
      {.stepType = UTILS::MATH::TValue::StepType::SINGLE_CYCLE, .numSteps = 1U}
  };

  [[nodiscard]] auto GetStarSetupParams(const StarProperties& starProperties) const noexcept
      -> IStarType::SetupParams;
  [[nodiscard]] auto GetNewStarParams(float starPathAngle) const noexcept -> Star::Params;
};

} //namespace GOOM::VISUAL_FX::FLYING_STARS

namespace GOOM::VISUAL_FX::FLYING_STARS
{

inline auto StarMaker::MoreStarsToMake() const noexcept -> bool
{
  return m_numStarsToMake > 0;
}

StarMaker::StarMaker(const GoomRand& goomRand) noexcept : m_goomRand{&goomRand}
{
}

auto StarMaker::StartNewCluster(const IStarType& starType,
                                const uint32_t numStarsInCluster,
                                const StarProperties& starProperties) noexcept -> void
{
  Expects(numStarsInCluster > 0);

  m_starType        = &starType;
  m_starSetupParams = GetStarSetupParams(starProperties);
  m_numStarsToMake  = numStarsInCluster;
  m_withinClusterT.SetNumSteps(numStarsInCluster);
  m_withinClusterT.Reset();
}

auto StarMaker::GetStarSetupParams(const StarProperties& starProperties) const noexcept
    -> IStarType::SetupParams
{
  auto setupParams = m_starType->GetRandomizedSetupParams(starProperties.defaultPathLength);

  setupParams.nominalPathLength *= starProperties.heightRatio;
  setupParams.nominalPathLength *= starProperties.nominalPathLengthFactor;

  return setupParams;
}

auto StarMaker::MakeNewStar() noexcept -> Star
{
  Expects(m_numStarsToMake > 0);

  const auto newStarPathAngle = m_starType->GetRandomizedStarPathAngle(m_starSetupParams.startPos);
  const auto newStarParams    = GetNewStarParams(newStarPathAngle);

  auto newStar =
      Star{newStarParams, m_starType->GetStarColorsMaker().GetNewStarColors(m_withinClusterT())};

  --m_numStarsToMake;
  m_withinClusterT.Increment();

  return newStar;
}

auto StarMaker::GetNewStarParams(const float starPathAngle) const noexcept -> Star::Params
{
  const auto initialPosition = ToPoint2dFlt(m_starSetupParams.startPos);

  static constexpr auto PATH_LENGTH_RANGE = NumberRange{0.01F, 2.00F};
  const auto starPathLength =
      m_starSetupParams.nominalPathLength * m_goomRand->GetRandInRange<PATH_LENGTH_RANGE>();
  static constexpr auto LENGTH_OFFSET = -0.2F;
  const auto initialVelocity =
      Vec2dFlt{.x = starPathLength * std::cos(starPathAngle),
               .y = LENGTH_OFFSET + (starPathLength * std::sin(starPathAngle))};

  const auto initialAcceleration =
      Vec2dFlt{.x = m_starSetupParams.sideWind, .y = m_starSetupParams.gravity};

  return {.currentPosition = initialPosition,
          .velocity        = initialVelocity,
          .acceleration    = initialAcceleration,
          .tAge            = 0.0F,
          .tAgeInc         = m_starSetupParams.starTAgeInc};
}

} //namespace GOOM::VISUAL_FX::FLYING_STARS
