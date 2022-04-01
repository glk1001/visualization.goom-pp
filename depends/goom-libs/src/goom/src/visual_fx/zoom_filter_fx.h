#pragma once

#include "goom/spimpl.h"
#include "goom_visual_fx.h"
#include "utils/name_value_pairs.h"

#include <cstdint>
#include <memory>

namespace GOOM
{

namespace UTILS
{
class Parallel;
}

struct FXBuffSettings;
class PixelBuffer;
class PluginInfo;

namespace VISUAL_FX
{

namespace FILTERS
{
class FilterBufferColorInfo;
class FilterBuffersService;
class FilterColorsService;
struct ZoomFilterBufferSettings;
struct ZoomFilterEffectsSettings;
struct ZoomFilterColorSettings;
}

class ZoomFilterFx : public IVisualFx
{
public:
  ZoomFilterFx() noexcept = delete;
  ZoomFilterFx(UTILS::Parallel& parallel,
               const PluginInfo& goomInfo,
               std::unique_ptr<FILTERS::FilterBuffersService> filterBuffersService,
               std::unique_ptr<FILTERS::FilterColorsService> filterColorsService) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string override;
  void SetBuffSettings(const FXBuffSettings& settings);

  void Start() override;

  [[nodiscard]] auto GetTranLerpFactor() const -> int32_t;

  [[nodiscard]] auto GetFilterEffectsSettings() const
      -> const VISUAL_FX::FILTERS::ZoomFilterEffectsSettings&;
  void UpdateFilterEffectsSettings(
      const VISUAL_FX::FILTERS::ZoomFilterEffectsSettings& filterEffectsSettings);
  void UpdateFilterBufferSettings(
      const VISUAL_FX::FILTERS::ZoomFilterBufferSettings& filterBufferSettings);
  void UpdateFilterColorSettings(
      const VISUAL_FX::FILTERS::ZoomFilterColorSettings& filterColorSettings);

  void ZoomFilterFastRgb(const PixelBuffer& srceBuff, PixelBuffer& destBuff);
  void SetZoomFilterBrightness(float brightness);
  [[nodiscard]] auto GetLastFilterBufferColorInfo() const -> const FILTERS::FilterBufferColorInfo&;
  [[nodiscard]] auto GetLastFilterBufferColorInfo() -> FILTERS::FilterBufferColorInfo&;

  void Finish() override;

  [[nodiscard]] auto GetNameValueParams() const -> UTILS::NameValuePairs;

private:
  class ZoomFilterImpl;
  spimpl::unique_impl_ptr<ZoomFilterImpl> m_fxImpl;
};

} // namespace VISUAL_FX
} // namespace GOOM
