#pragma once

#include "goom/spimpl.h"
#include "goom_visual_fx.h"

#include <memory>

namespace GOOM
{

namespace COLOR
{
class RandomColorMaps;
}

namespace VISUAL_FX
{
class FxHelper;

class ShapesFx : public IVisualFx
{
public:
  static constexpr uint32_t NUM_SHAPE_GROUPS = 5;

  explicit ShapesFx(const FxHelper& fxHelper) noexcept;

  [[nodiscard]] auto GetFxName() const noexcept -> std::string override;

  auto Start() noexcept -> void override;
  auto Finish() noexcept -> void override;

  auto SetWeightedMainColorMaps(size_t shapeGroupNum,
                                std::shared_ptr<COLOR::RandomColorMaps> weightedMaps) noexcept
      -> void;
  auto SetWeightedLowColorMaps(size_t shapeGroupNum,
                               std::shared_ptr<COLOR::RandomColorMaps> weightedMaps) noexcept
      -> void;
  auto SetWeightedInnerColorMaps(size_t shapeGroupNum,
                                 std::shared_ptr<COLOR::RandomColorMaps> weightedMaps) noexcept
      -> void;

  auto ApplyMultiple() noexcept -> void;

private:
  class ShapesFxImpl;
  spimpl::unique_impl_ptr<ShapesFxImpl> m_fxImpl;
};

} // namespace VISUAL_FX
} // namespace GOOM