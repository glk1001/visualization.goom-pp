#include "lsys_geom.h"

//#undef NO_LOGGING

#include "goom_logger.h"

namespace GOOM::VISUAL_FX::L_SYSTEM
{

using UTILS::TValue;
using UTILS::MATH::DEGREES_360;
using UTILS::MATH::ToRadians;

LSysGeometry::LSysGeometry(const UTILS::MATH::IGoomRand& goomRand,
                           const float xScale,
                           const float yScale) noexcept
  : m_goomRand{&goomRand}, m_xScale{xScale}, m_yScale{yScale}
{
}

auto LSysGeometry::IncrementTs() noexcept -> void
{
  m_rotateDegreesAdjust.Increment();
  m_spinDegreesAdjust.Increment();
  m_yScaleAdjust.Increment();
  m_verticalMove.Increment();
}

auto LSysGeometry::SetNumLSysCopies(const uint32_t numLSysCopies) noexcept -> void
{
  Expects(numLSysCopies > 0U);
  m_numLSysCopies        = numLSysCopies;
  m_transformAdjustArray = GetTransformAdjustArray();
  UpdateCurrentTransformArray();
}

auto LSysGeometry::GetTransformAdjustArray() const noexcept -> std::vector<TransformAdjust>
{
  auto transformAdjustArray = std::vector<TransformAdjust>(m_numLSysCopies);

  auto t = TValue{
      {TValue::StepType::SINGLE_CYCLE, m_numLSysCopies}
  };
  for (auto& transformAdjust : transformAdjustArray)
  {
    static constexpr auto MIN_X_SCALE = 0.8F;
    static constexpr auto MAX_X_SCALE = 1.0F;
    static constexpr auto MIN_Y_SCALE = 0.9F;
    static constexpr auto MAX_Y_SCALE = 1.1F;
    transformAdjust.xScale            = m_goomRand->GetRandInRange(MIN_X_SCALE, MAX_X_SCALE);
    transformAdjust.yScale            = m_goomRand->GetRandInRange(MIN_Y_SCALE, MAX_Y_SCALE);
    transformAdjust.rotateDegrees     = t() * DEGREES_360;
    transformAdjust.translate         = {0.0F, 0.0F};

    t.Increment();
  }

  return transformAdjustArray;
}

auto LSysGeometry::UpdateCurrentTransformArray() noexcept -> void
{
  m_currentTransformArray.resize(m_numLSysCopies);

  for (auto i = 0U; i < m_numLSysCopies; ++i)
  {
    const auto rotateRadians = m_rotateSign * ToRadians(m_transformAdjustArray.at(i).rotateDegrees +
                                                        m_rotateDegreesAdjust());
    const auto spinRadians   = m_spinSign * ToRadians(m_spinDegreesAdjust());

    auto& currentTransform = m_currentTransformArray.at(i);

    currentTransform.xScale = m_xScale * m_transformAdjustArray.at(i).xScale;
    currentTransform.yScale = (m_yScale * m_transformAdjustArray.at(i).yScale) * m_yScaleAdjust();
    currentTransform.verticalMove   = m_verticalMove();
    currentTransform.sinRotateAngle = std::sin(rotateRadians);
    currentTransform.cosRotateAngle = std::cos(rotateRadians);
    currentTransform.sinSpinAngle   = std::sin(spinRadians);
    currentTransform.cosSpinAngle   = std::cos(spinRadians);
    currentTransform.translate      = m_transformAdjustArray.at(i).translate + m_translateAdjust;
  }
}

auto LSysGeometry::GetTransformedPoint(const Point2dFlt& point,
                                       const uint32_t copyNum) const noexcept -> Point2dFlt
{
  auto transformedPoint = point;

  // TODO(glk) Consolidate operations??
  transformedPoint = Scale(transformedPoint,
                           m_currentTransformArray.at(copyNum).xScale,
                           m_currentTransformArray.at(copyNum).yScale);
  transformedPoint = Rotate(transformedPoint,
                            m_currentTransformArray.at(copyNum).sinSpinAngle,
                            m_currentTransformArray.at(copyNum).cosSpinAngle);
  transformedPoint = TranslateY(transformedPoint, m_currentTransformArray.at(copyNum).verticalMove);
  transformedPoint = Rotate(transformedPoint,
                            m_currentTransformArray.at(copyNum).sinRotateAngle,
                            m_currentTransformArray.at(copyNum).cosRotateAngle);
  transformedPoint = Translate(transformedPoint, m_currentTransformArray.at(copyNum).translate);

  return transformedPoint;
}

} // namespace GOOM::VISUAL_FX::L_SYSTEM
