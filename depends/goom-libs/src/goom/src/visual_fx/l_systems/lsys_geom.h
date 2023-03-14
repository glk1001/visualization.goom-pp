#pragma once

#include "point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/t_values.h"

#include <cstdint>
#include <vector>

namespace GOOM::VISUAL_FX::L_SYSTEM
{

class LSysGeometry
{
public:
  LSysGeometry(const UTILS::MATH::IGoomRand& goomRand, float xScale, float yScale) noexcept;

  auto SetNumLSysCopies(uint32_t numLSysCopies) noexcept -> void;
  auto SetVerticalMoveMaxMin(float verticalMoveMin, float verticalMoveMax) noexcept -> void;
  auto SetVerticalMoveNumSteps(uint32_t numSteps) noexcept -> void;
  auto SetYScaleNumSteps(uint32_t numSteps) noexcept -> void;
  auto SetRotateDegreesAdjustNumSteps(uint32_t numSteps) noexcept -> void;
  auto SetSpinDegreesAdjustNumSteps(uint32_t numSteps) noexcept -> void;
  auto ReverseRotateDirection() noexcept -> void;
  auto ReverseSpinDirection() noexcept -> void;
  auto SetTranslateAdjust(const Vec2dFlt& translateAdjust) noexcept -> void;

  auto UpdateCurrentTransformArray() noexcept -> void;
  [[nodiscard]] auto GetTransformedPoint(const Point2dFlt& point, uint32_t copyNum) const noexcept
      -> Point2dFlt;
  auto IncrementTs() noexcept -> void;

  [[nodiscard]] auto GetVerticalMove() const noexcept -> const UTILS::IncrementedValue<float>&;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  float m_xScale;
  float m_yScale;
  uint32_t m_numLSysCopies = 1U;

  struct TransformAdjust
  {
    float xScale{};
    float yScale{};
    float rotateDegrees{};
    Vec2dFlt translate;
  };
  struct TransformLSys
  {
    // This is the order the transform operations should occur in.
    float xScale{};
    float yScale{};
    float sinSpinAngle{};
    float cosSpinAngle{};
    float verticalMove{};
    float sinRotateAngle{};
    float cosRotateAngle{};
    Vec2dFlt translate;
  };
  std::vector<TransformAdjust> m_transformAdjustArray{};
  [[nodiscard]] auto GetTransformAdjustArray() const noexcept -> std::vector<TransformAdjust>;
  std::vector<TransformLSys> m_currentTransformArray{};

  Vec2dFlt m_translateAdjust{};

  static constexpr auto DEFAULT_NUM_ROTATE_DEGREES_STEPS = 100U;
  UTILS::IncrementedValue<float> m_rotateDegreesAdjust{
      0.0F,
      UTILS::MATH::DEGREES_360,
      UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE,
      DEFAULT_NUM_ROTATE_DEGREES_STEPS};
  float m_rotateSign = +1.0F;

  static constexpr auto DEFAULT_NUM_SPIN_DEGREES_STEPS = 100U;
  UTILS::IncrementedValue<float> m_spinDegreesAdjust{0.0F,
                                                     UTILS::MATH::DEGREES_360,
                                                     UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE,
                                                     DEFAULT_NUM_SPIN_DEGREES_STEPS};
  float m_spinSign = -1.0F;

  static constexpr auto MIN_Y_SCALE_ADJUST           = 1.0F;
  static constexpr auto MAX_Y_SCALE_ADJUST           = 1.9F;
  static constexpr auto DEFAULT_Y_SCALE_ADJUST_STEPS = 300U;
  UTILS::IncrementedValue<float> m_yScaleAdjust{MIN_Y_SCALE_ADJUST,
                                                MAX_Y_SCALE_ADJUST,
                                                UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE,
                                                DEFAULT_Y_SCALE_ADJUST_STEPS};

  static constexpr auto MIN_VERTICAL_MOVE           = -100.0F;
  static constexpr auto MAX_VERTICAL_MOVE           = +100.0F;
  static constexpr auto DEFAULT_VERTICAL_MOVE_STEPS = 150U;
  UTILS::IncrementedValue<float> m_verticalMove{MIN_VERTICAL_MOVE,
                                                MAX_VERTICAL_MOVE,
                                                UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE,
                                                DEFAULT_VERTICAL_MOVE_STEPS};
};

inline auto LSysGeometry::ReverseRotateDirection() noexcept -> void
{
  m_rotateSign = -m_rotateSign;
}

inline auto LSysGeometry::ReverseSpinDirection() noexcept -> void
{
  m_spinSign = -m_spinSign;
}

inline auto LSysGeometry::SetTranslateAdjust(const Vec2dFlt& translateAdjust) noexcept -> void
{
  m_translateAdjust = translateAdjust;
}

inline auto LSysGeometry::SetVerticalMoveMaxMin(const float verticalMoveMin,
                                                const float verticalMoveMax) noexcept -> void
{
  m_verticalMove.SetValues(verticalMoveMin, verticalMoveMax);
}

inline auto LSysGeometry::SetRotateDegreesAdjustNumSteps(const uint32_t numSteps) noexcept -> void
{
  m_rotateDegreesAdjust.SetNumSteps(numSteps);
}

inline auto LSysGeometry::SetSpinDegreesAdjustNumSteps(const uint32_t numSteps) noexcept -> void
{
  m_spinDegreesAdjust.SetNumSteps(numSteps);
}

inline auto LSysGeometry::SetVerticalMoveNumSteps(uint32_t numSteps) noexcept -> void
{
  m_verticalMove.SetNumSteps(numSteps);
}

inline auto LSysGeometry::SetYScaleNumSteps(uint32_t numSteps) noexcept -> void
{
  m_yScaleAdjust.SetNumSteps(numSteps);
}

inline auto LSysGeometry::GetVerticalMove() const noexcept -> const UTILS::IncrementedValue<float>&
{
  return m_verticalMove;
}

} // namespace GOOM::VISUAL_FX::L_SYSTEM
