#ifndef VISUALIZATION_GOOM_TUBE_FX_H
#define VISUALIZATION_GOOM_TUBE_FX_H

#include "goom_visual_fx.h"
#include "goomutils/spimpl.h"

#include <memory>
#include <string>

namespace GOOM
{

class IGoomDraw;
class PluginInfo;

namespace UTILS
{
class RandomColorMaps;
class SmallImageBitmaps;
} // namespace UTILS

class TubeFx : public IVisualFx
{
public:
  TubeFx() noexcept = delete;
  explicit TubeFx(const IGoomDraw& draw,
                  const std::shared_ptr<const PluginInfo>& goomInfo,
                  const UTILS::SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string override;

  [[nodiscard]] auto GetResourcesDirectory() const -> const std::string& override;
  void SetResourcesDirectory(const std::string& dirName) override;

  void Start() override;

  void Resume() override;
  void Suspend() override;

  void SetWeightedColorMaps(std::shared_ptr<UTILS::RandomColorMaps> weightedMaps);
  void SetWeightedLowColorMaps(std::shared_ptr<UTILS::RandomColorMaps> weightedMaps);

  void ApplyNoDraw();
  void ApplyMultiple();

  void Finish() override;

private:
  bool m_enabled = true;
  class TubeFxImpl;
  spimpl::unique_impl_ptr<TubeFxImpl> m_fxImpl;
};

} // namespace GOOM

#endif /* VISUALIZATION_GOOM_TUBE_FX_H */
