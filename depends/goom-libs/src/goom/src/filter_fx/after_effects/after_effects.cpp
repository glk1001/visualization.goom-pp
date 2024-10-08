module;

#include <memory>
#include <string>
#include <utility>

module Goom.FilterFx.AfterEffects.AfterEffects;

import Goom.FilterFx.AfterEffects.TheEffects.Hypercos;
import Goom.FilterFx.AfterEffects.TheEffects.ImageVelocity;
import Goom.FilterFx.AfterEffects.TheEffects.Noise;
import Goom.FilterFx.AfterEffects.TheEffects.Planes;
import Goom.FilterFx.AfterEffects.TheEffects.Rotation;
import Goom.FilterFx.AfterEffects.TheEffects.TanEffect;
import Goom.FilterFx.AfterEffects.TheEffects.XYLerpEffect;
import Goom.Utils.Math.GoomRand;

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

using UTILS::MATH::GoomRand;

AfterEffects::AfterEffects(std::unique_ptr<Hypercos>&& hypercos,
                           std::unique_ptr<ImageVelocity>&& imageVelocity,
                           std::unique_ptr<Noise>&& noise,
                           std::unique_ptr<Planes>&& planes,
                           std::unique_ptr<Rotation>&& rotation,
                           std::unique_ptr<TanEffect>&& tanEffect,
                           std::unique_ptr<XYLerpEffect>&& xyLerpEffect) noexcept
  : m_hypercos{std::move(hypercos)},
    m_imageVelocity{std::move(imageVelocity)},
    m_noise{std::move(noise)},
    m_planes{std::move(planes)},
    m_rotation{std::move(rotation)},
    m_tanEffect{std::move(tanEffect)},
    m_xyLerpEffect{std::move(xyLerpEffect)}
{
}

AfterEffects::~AfterEffects() noexcept = default;

auto GetStandardAfterEffects(const GoomRand& goomRand, const std::string& resourcesDirectory)
    -> AfterEffects
{
  return {
      std::make_unique<Hypercos>(goomRand),
      std::make_unique<ImageVelocity>(goomRand, resourcesDirectory),
      std::make_unique<Noise>(goomRand),
      std::make_unique<Planes>(goomRand),
      std::make_unique<Rotation>(goomRand),
      std::make_unique<TanEffect>(goomRand),
      std::make_unique<XYLerpEffect>(goomRand),
  };
}

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
