module;

#include <string>

export module Goom.FilterFx.AfterEffects.TheEffects.Rotation;

import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.GoomTypes;

using GOOM::UTILS::NameValuePairs;
using GOOM::UTILS::MATH::GoomRand;

export namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

class RotationAdjustments
{
public:
  RotationAdjustments() noexcept = default;

  enum class AdjustmentType : UnderlyingEnumType
  {
    NONE,
    AFTER_RANDOM,
    INSTEAD_OF_RANDOM,
  };
  [[nodiscard]] auto GetAdjustmentType() const -> AdjustmentType;

  [[nodiscard]] auto IsToggle() const -> bool;
  auto Toggle(AdjustmentType adjustmentType) -> void;

  [[nodiscard]] auto GetMultiplyFactor() const -> float;
  auto SetMultiplyFactor(float value, AdjustmentType adjustmentType) -> void;

  auto Reset() -> void;

private:
  bool m_toggle                   = false;
  float m_multiplyFactor          = 1.0F;
  AdjustmentType m_adjustmentType = AdjustmentType::NONE;
};

class Rotation
{
public:
  explicit Rotation(const GoomRand& goomRand) noexcept;
  Rotation(const Rotation&) noexcept           = delete;
  Rotation(Rotation&&) noexcept                = delete;
  virtual ~Rotation() noexcept                 = default;
  auto operator=(const Rotation&) -> Rotation& = delete;
  auto operator=(Rotation&&) -> Rotation&      = delete;

  virtual auto SetRandomParams() noexcept -> void;
  auto ApplyAdjustments(const RotationAdjustments& rotationAdjustments) -> void;

  [[nodiscard]] auto GetVelocity(const NormalizedCoords& velocity) const -> NormalizedCoords;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs;

  struct Params
  {
    float xRotateSpeed;
    float yRotateSpeed;
    float sinAngle;
    float cosAngle;
  };
  [[nodiscard]] auto GetParams() const -> const Params&;

protected:
  auto SetParams(const Params& params) -> void;

private:
  const GoomRand* m_goomRand;
  Params m_params;
  [[nodiscard]] auto GetRandomParams() const noexcept -> Params;

  auto Multiply(float factor) -> void;
  auto Toggle() -> void;
};

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

inline auto RotationAdjustments::GetAdjustmentType() const -> AdjustmentType
{
  return m_adjustmentType;
}

inline auto RotationAdjustments::IsToggle() const -> bool
{
  return m_toggle;
}

inline auto RotationAdjustments::Toggle(const AdjustmentType adjustmentType) -> void
{
  m_toggle         = true;
  m_adjustmentType = adjustmentType;
}

inline auto RotationAdjustments::GetMultiplyFactor() const -> float
{
  return m_multiplyFactor;
}

inline auto RotationAdjustments::SetMultiplyFactor(const float value,
                                                   const AdjustmentType adjustmentType) -> void
{
  m_multiplyFactor = value;
  m_adjustmentType = adjustmentType;
}

inline auto RotationAdjustments::Reset() -> void
{
  m_adjustmentType = AdjustmentType::NONE;
  m_toggle         = false;
  m_multiplyFactor = 1.0F;
}

inline auto Rotation::GetVelocity(const NormalizedCoords& velocity) const -> NormalizedCoords
{
  auto xRotateSpeed = m_params.xRotateSpeed;
  auto yRotateSpeed = m_params.yRotateSpeed;
  auto sinAngle     = m_params.sinAngle;
  if (m_params.xRotateSpeed < 0.0F)
  {
    xRotateSpeed = -xRotateSpeed;
    yRotateSpeed = -yRotateSpeed;
    sinAngle     = -sinAngle;
  }

  const auto cosAngle = m_params.cosAngle;

  return {xRotateSpeed * ((cosAngle * velocity.GetX()) - (sinAngle * velocity.GetY())),
          yRotateSpeed * ((sinAngle * velocity.GetX()) + (cosAngle * velocity.GetY()))};
}

inline auto Rotation::ApplyAdjustments(const RotationAdjustments& rotationAdjustments) -> void
{
  if (rotationAdjustments.IsToggle())
  {
    Toggle();
  }
  Multiply(rotationAdjustments.GetMultiplyFactor());
}

inline auto Rotation::Multiply(const float factor) -> void
{
  m_params.xRotateSpeed *= factor;
  m_params.yRotateSpeed *= factor;
}

inline auto Rotation::Toggle() -> void
{
  m_params.xRotateSpeed = -m_params.xRotateSpeed;
  m_params.yRotateSpeed = -m_params.yRotateSpeed;
}

inline auto Rotation::GetParams() const -> const Params&
{
  return m_params;
}

inline auto Rotation::SetParams(const Params& params) -> void
{
  m_params = params;
}

inline auto Rotation::SetRandomParams() noexcept -> void
{
  m_params = GetRandomParams();
}

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
