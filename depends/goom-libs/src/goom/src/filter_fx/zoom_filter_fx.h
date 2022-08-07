#pragma once

#include "goom/spimpl.h"
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

namespace FILTER_FX
{

class FilterBufferColorInfo;
class FilterBuffersService;
class FilterColorsService;
struct ZoomFilterBufferSettings;
struct ZoomFilterEffectsSettings;
struct ZoomFilterColorSettings;

class ZoomFilterFx
{
public:
  ZoomFilterFx() noexcept = delete;
  ZoomFilterFx(GOOM::UTILS::Parallel& parallel,
               const PluginInfo& goomInfo,
               std::unique_ptr<FilterBuffersService> filterBuffersService,
               std::unique_ptr<FilterColorsService> filterColorsService) noexcept;

  auto SetBuffSettings(const FXBuffSettings& settings) noexcept -> void;

  auto Start() noexcept -> void;
  auto Finish() noexcept -> void;

  [[nodiscard]] auto GetTranLerpFactor() const noexcept -> int32_t;

  [[nodiscard]] auto GetFilterEffectsSettings() const noexcept -> const ZoomFilterEffectsSettings&;
  auto UpdateFilterEffectsSettings(const ZoomFilterEffectsSettings& filterEffectsSettings) noexcept
      -> void;
  auto UpdateFilterBufferSettings(const ZoomFilterBufferSettings& filterBufferSettings) noexcept
      -> void;
  auto UpdateFilterColorSettings(const ZoomFilterColorSettings& filterColorSettings) noexcept
      -> void;

  auto ZoomFilterFastRgb(const PixelBuffer& srceBuff, PixelBuffer& destBuff) noexcept -> void;
  auto SetZoomFilterBrightness(float brightness) noexcept -> void;
  [[nodiscard]] auto GetLastFilterBufferColorInfo() const noexcept -> const FilterBufferColorInfo&;
  [[nodiscard]] auto GetLastFilterBufferColorInfo() noexcept -> FilterBufferColorInfo&;

  [[nodiscard]] auto GetNameValueParams() const noexcept -> GOOM::UTILS::NameValuePairs;

private:
  class ZoomFilterImpl;
  spimpl::unique_impl_ptr<ZoomFilterImpl> m_pimpl;
};

} // namespace FILTER_FX
} // namespace GOOM
