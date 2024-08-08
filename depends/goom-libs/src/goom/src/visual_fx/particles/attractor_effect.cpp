module;

#include <cmath>
#include <glm/vec4.hpp>
#include <memory>

module Goom.VisualFx.ParticlesFx.Particles.AttractorEffect;

import Particles.ParticleGenerators;
import Particles.ParticleUpdaters;
import Goom.Utils.Math.GoomRand;

namespace GOOM::VISUAL_FX::PARTICLES
{

using ::PARTICLES::ParticleEmitter;
using ::PARTICLES::GENERATORS::BasicColorGenerator;
using ::PARTICLES::GENERATORS::BasicTimeGenerator;
using ::PARTICLES::GENERATORS::BoxPositionGenerator;
using ::PARTICLES::GENERATORS::SphereVelocityGenerator;
using ::PARTICLES::UPDATERS::AttractorUpdater;
using ::PARTICLES::UPDATERS::BasicTimeUpdater;
using ::PARTICLES::UPDATERS::EulerUpdater;
using ::PARTICLES::UPDATERS::IColorUpdater;
using ::PARTICLES::UPDATERS::PositionColorUpdater;
using ::PARTICLES::UPDATERS::VelocityColorUpdater;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;

static constexpr auto EMIT_RATE_FACTOR_RANGE = NumberRange{0.1F, 0.9F};

static constexpr auto MIN_SPHERE_VELOCITY = 0.1F;
static constexpr auto MAX_SPHERE_VELOCITY = 0.5F;

static constexpr auto MIN_LIFETIME = 2.0F;
static constexpr auto MAX_LIFETIME = 200.0F;

static constexpr auto MIN_START_COLOR = glm::vec4{0.10F, 0.10F, 0.10F, 1.00F};
static constexpr auto MAX_START_COLOR = glm::vec4{0.90F, 0.90F, 0.90F, 1.00F};
static constexpr auto MIN_END_COLOR   = glm::vec4{0.05F, 0.05F, 0.05F, 0.00F};
static constexpr auto MAX_END_COLOR   = glm::vec4{0.30F, 0.30F, 0.30F, 0.25F};

static constexpr auto ATTRACTOR_POSITIONS = std::array{
    glm::vec4{0.0F, +0.00F, +0.75F, 1.0F},
    glm::vec4{0.0F, +0.00F, -0.75F, 1.0F},
    glm::vec4{0.0F, +0.75F, +0.00F, 1.0F},
    glm::vec4{0.0F, -0.75F, +0.00F, 1.0F},
};

static constexpr auto Z_GEN_POS = std::array{
    -0.25F,
    +0.25F,
    +0.25F,
};
static_assert(Z_GEN_POS.size() == AttractorEffect::NUM_EMITTERS);

struct GenPosAndMaxStartPosOffset
{
  glm::vec4 pos;
  glm::vec4 startPosOffset;
};
static constexpr auto GEN_POS_AND_MAX_START_POS_OFFSETS = std::array{
    GenPosAndMaxStartPosOffset{.pos            = glm::vec4{0.0F, 0.0F, Z_GEN_POS[0], 0.0F},
                               .startPosOffset = glm::vec4{0.0F, 0.0F, 0.00F, 0.0F}},
    GenPosAndMaxStartPosOffset{.pos            = glm::vec4{0.0F, 0.0F, Z_GEN_POS[1], 0.0F},
                               .startPosOffset = glm::vec4{0.0F, 0.0F, 0.00F, 0.0F}},
    GenPosAndMaxStartPosOffset{.pos            = glm::vec4{0.0F, 0.0F, Z_GEN_POS[2], 0.0F},
                               .startPosOffset = glm::vec4{0.0F, 0.0F, 0.00F, 0.0F}},
};
static_assert(GEN_POS_AND_MAX_START_POS_OFFSETS.size() == AttractorEffect::NUM_EMITTERS);

static constexpr auto POS_LIFETIME_FACTORS = std::array{
    2.5F,
    2.0F,
    6.0F,
};
static_assert(POS_LIFETIME_FACTORS.size() == AttractorEffect::NUM_EMITTERS);

static constexpr auto UPDATE_RADIUS   = 0.55F;
static constexpr auto UPDATE_RADIUS_X = std::array{
    +UPDATE_RADIUS,
    -UPDATE_RADIUS,
    -UPDATE_RADIUS,
};
static_assert(UPDATE_RADIUS_X.size() == AttractorEffect::NUM_EMITTERS);

static constexpr auto UPDATE_RADIUS_Y = std::array{
    +UPDATE_RADIUS,
    +UPDATE_RADIUS,
    +UPDATE_RADIUS,
};
static_assert(UPDATE_RADIUS_Y.size() == AttractorEffect::NUM_EMITTERS);

static constexpr auto PROB_BIG_EULER_ACCELERATION    = 0.05F;
static constexpr auto PROB_EQUAL_EULER_ACCELERATION  = 0.95F;
static constexpr auto SMALL_EULER_ACCELERATION_RANGE = NumberRange{0.0F, 0.1F};
static constexpr auto BIG_EULER_ACCELERATION_RANGE   = NumberRange{0.1F, 10.0F};

static constexpr auto PROB_POS_COLOR_UPDATER = 0.5F;

static constexpr auto MIN_COLOR_POSITION = glm::vec4{-0.5F, -0.5F, -0.5F, 0.0F};
static constexpr auto MAX_COLOR_POSITION = glm::vec4{+2.0F, +3.0F, +3.0F, 2.0F};

static constexpr auto MIN_VELOCITY = glm::vec4{-0.5F, -0.5F, -0.5F, 0.0F};
static constexpr auto MAX_VELOCITY = glm::vec4{+2.0F, +2.0F, +2.0F, 2.0F};

static constexpr auto DEFAULT_NUM_PARTICLES = 250000U;

AttractorEffect::AttractorEffect(const GoomRand& goomRand, const size_t numParticles) noexcept
  : m_goomRand{&goomRand},
    m_system{numParticles == 0 ? DEFAULT_NUM_PARTICLES : numParticles},
    m_colorUpdater{MakeColorUpdater()},
    m_eulerUpdater{std::make_shared<EulerUpdater>(GetNewEulerAcceleration())}
{
  AddEmitters();
  AddUpdaters();
}

auto AttractorEffect::MakeColorUpdater() const noexcept -> std::shared_ptr<IColorUpdater>
{
  if (m_goomRand->ProbabilityOf<PROB_POS_COLOR_UPDATER>())
  {
    return std::make_shared<PositionColorUpdater>(MIN_COLOR_POSITION, MAX_COLOR_POSITION);
  }
  return std::make_shared<VelocityColorUpdater>(MIN_VELOCITY, MAX_VELOCITY);
}

auto AttractorEffect::GetNewEmitRate() const noexcept -> float
{
  return m_goomRand->GetRandInRange<EMIT_RATE_FACTOR_RANGE>() *
         static_cast<float>(m_system.GetNumAllParticles());
}

auto AttractorEffect::GetNewEulerAcceleration() const noexcept -> glm::vec4
{
  if (m_goomRand->ProbabilityOf<PROB_EQUAL_EULER_ACCELERATION>())
  {
    const auto eulerAcceleration =
        m_goomRand->ProbabilityOf<PROB_BIG_EULER_ACCELERATION>()
            ? m_goomRand->GetRandInRange<BIG_EULER_ACCELERATION_RANGE>()
            : m_goomRand->GetRandInRange<SMALL_EULER_ACCELERATION_RANGE>();
    return {eulerAcceleration, eulerAcceleration, eulerAcceleration, 0.0F};
  }

  if (m_goomRand->ProbabilityOf<PROB_BIG_EULER_ACCELERATION>())
  {
    return {m_goomRand->GetRandInRange<BIG_EULER_ACCELERATION_RANGE>(),
            m_goomRand->GetRandInRange<BIG_EULER_ACCELERATION_RANGE>(),
            m_goomRand->GetRandInRange<BIG_EULER_ACCELERATION_RANGE>(),
            0.0F};
  }
  return {m_goomRand->GetRandInRange<SMALL_EULER_ACCELERATION_RANGE>(),
          m_goomRand->GetRandInRange<SMALL_EULER_ACCELERATION_RANGE>(),
          m_goomRand->GetRandInRange<SMALL_EULER_ACCELERATION_RANGE>(),
          0.0F};
}

auto AttractorEffect::AddEmitters() noexcept -> void
{
  const auto emitRate      = GetNewEmitRate();
  const auto timeGenerator = std::make_shared<BasicTimeGenerator>(MIN_LIFETIME, MAX_LIFETIME);
  const auto velocityGenerator =
      std::make_shared<SphereVelocityGenerator>(MIN_SPHERE_VELOCITY, MAX_SPHERE_VELOCITY);
  const auto colorGenerator = std::make_shared<BasicColorGenerator>(
      MIN_START_COLOR, MAX_START_COLOR, MIN_END_COLOR, MAX_END_COLOR);

  for (auto i = 0U; i < NUM_EMITTERS; ++i)
  {
    m_particleEmitters.at(i) = std::make_shared<ParticleEmitter>();
    m_particleEmitters.at(i)->SetEmitRate(emitRate);

    m_positionGenerators.at(i) = std::make_shared<BoxPositionGenerator>(
        GEN_POS_AND_MAX_START_POS_OFFSETS.at(i).pos,
        GEN_POS_AND_MAX_START_POS_OFFSETS.at(i).startPosOffset);
    m_particleEmitters.at(i)->AddGenerator(m_positionGenerators.at(i));

    m_particleEmitters.at(i)->AddGenerator(colorGenerator);
    m_particleEmitters.at(i)->AddGenerator(velocityGenerator);
    m_particleEmitters.at(i)->AddGenerator(timeGenerator);

    m_system.AddEmitter(m_particleEmitters.at(i));
  }
}

auto AttractorEffect::AddUpdaters() noexcept -> void
{
  m_system.AddUpdater(m_colorUpdater);
  m_system.AddUpdater(m_eulerUpdater);

  auto attractorUpdater = std::make_shared<AttractorUpdater>();
  for (const auto& attractorPos : ATTRACTOR_POSITIONS)
  {
    attractorUpdater->AddAttractorPosition(attractorPos);
  }
  m_system.AddUpdater(attractorUpdater);

  const auto timeUpdater = std::make_shared<BasicTimeUpdater>();
  m_system.AddUpdater(timeUpdater);
}

auto AttractorEffect::Reset() noexcept -> void
{
  m_system.Reset();

  const auto oldColorUpdater = m_colorUpdater;
  m_colorUpdater             = MakeColorUpdater();
  m_system.ReplaceUpdater(oldColorUpdater, m_colorUpdater);

  const auto oldEulerUpdater = m_eulerUpdater;
  m_eulerUpdater             = std::make_shared<EulerUpdater>(GetNewEulerAcceleration());
  m_system.ReplaceUpdater(oldEulerUpdater, m_eulerUpdater);

  const auto emitRate = GetNewEmitRate();
  for (auto i = 0U; i < NUM_EMITTERS; ++i)
  {
    m_particleEmitters.at(i)->SetEmitRate(emitRate);
  }
}

auto AttractorEffect::UpdateEffect(const double dt) noexcept -> void
{
  static auto s_lifetime = 0.0F;
  s_lifetime += static_cast<float>(dt);

  const auto zScale = 1.0F;

  for (auto i = 0U; i < NUM_EMITTERS; ++i)
  {
    const auto angle = s_lifetime * POS_LIFETIME_FACTORS.at(i);

    m_positionGenerators.at(i)->SetPosition({UPDATE_RADIUS_X.at(i) * std::sin(angle),
                                             UPDATE_RADIUS_Y.at(i) * std::cos(angle),
                                             zScale * Z_GEN_POS.at(i) * std::cos(angle),
                                             0.0F});
  }
}

} // namespace GOOM::VISUAL_FX::PARTICLES
