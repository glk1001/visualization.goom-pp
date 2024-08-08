module;

#include <string>

export module Goom.FilterFx.ZoomVector;

import Goom.FilterFx.FilterSettings;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;

export namespace GOOM::FILTER_FX
{

class IZoomVector
{
public:
  IZoomVector() noexcept                             = default;
  IZoomVector(const IZoomVector&) noexcept           = delete;
  IZoomVector(IZoomVector&&) noexcept                = delete;
  virtual ~IZoomVector() noexcept                    = default;
  auto operator=(const IZoomVector&) -> IZoomVector& = delete;
  auto operator=(IZoomVector&&) -> IZoomVector&      = delete;

  virtual auto SetFilterEffectsSettings(const FilterEffectsSettings& filterEffectsSettings) noexcept
      -> void = 0;

  [[nodiscard]] virtual auto GetZoomPoint(const NormalizedCoords& coords) const noexcept
      -> NormalizedCoords = 0;

  [[nodiscard]] virtual auto GetNameValueParams(const std::string& paramGroup) const noexcept
      -> UTILS::NameValuePairs = 0;
};

} // namespace GOOM::FILTER_FX
