#pragma once

#include "goom/spimpl.h"
#include "goom_visual_fx.h"

#include <memory>
#include <string>

namespace GOOM
{
class PluginInfo;

namespace COLOR
{
class RandomColorMaps;
} // namespace COLOR

namespace DRAW
{
class IGoomDraw;
} // namespace DRAW

namespace UTILS
{
class Parallel;
} // namespace UTILS

namespace VISUAL_FX
{

class ImageFx : public IVisualFx
{
public:
  ImageFx(UTILS::Parallel& parallel,
          DRAW::IGoomDraw& draw,
          const PluginInfo& goomInfo,
          const std::string& resourcesDirectory) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string override;

  void SetWeightedColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps);

  void Start() override;

  void ApplyMultiple();

  void Finish() override;

private:
  class ImageFxImpl;
  spimpl::unique_impl_ptr<ImageFxImpl> m_fxImpl;
};

} // namespace VISUAL_FX
} // namespace GOOM