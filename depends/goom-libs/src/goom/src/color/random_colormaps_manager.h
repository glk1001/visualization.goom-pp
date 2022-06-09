#pragma once

#include "colormaps.h"
#include "goom_config.h"
#include "random_colormaps.h"
#include "utils/math/goom_rand_base.h"

#include <cstdint>
#include <memory>
#include <set>
#include <vector>

namespace GOOM::COLOR
{

class RandomColorMapsManager
{
public:
  // NOLINTBEGIN
  struct ColorMapInfo
  {
    ~ColorMapInfo() noexcept;
    std::shared_ptr<const RandomColorMaps> colorMaps{};
    COLOR_DATA::ColorMapName colorMapName{COLOR_DATA::ColorMapName::_NULL};
    std::set<RandomColorMaps::ColorMapTypes> types{};
  };
  // NOLINTEND

  class ColorMapId
  {
  public:
    ColorMapId() noexcept = default;
    explicit ColorMapId(int32_t id) noexcept;

    [[nodiscard]] auto IsSet() const noexcept -> bool;
    [[nodiscard]] auto operator()() const noexcept -> size_t;

  private:
    int32_t m_id = -1;
  };

  RandomColorMapsManager() = default;

  [[nodiscard]] auto AddDefaultColorMapInfo(const UTILS::MATH::IGoomRand& goomRand) noexcept
      -> ColorMapId;
  [[nodiscard]] auto AddColorMapInfo(const ColorMapInfo& info) noexcept -> ColorMapId;

  auto UpdateColorMapInfo(ColorMapId id, const ColorMapInfo& info) noexcept -> void;
  auto UpdateColorMapName(ColorMapId id, COLOR_DATA::ColorMapName colorMapName) noexcept -> void;

  auto ChangeAllColorMapsNow() noexcept -> void;
  auto ChangeColorMapNow(ColorMapId id) noexcept -> void;

  [[nodiscard]] auto GetColorMap(ColorMapId id) const noexcept -> const IColorMap&;
  [[nodiscard]] auto GetColorMapPtr(ColorMapId id) const noexcept
      -> std::shared_ptr<const IColorMap>;

private:
  std::vector<ColorMapInfo> m_infoList{};
  std::vector<std::shared_ptr<const IColorMap>> m_colorMaps{};
  auto RandomizeColorMaps(size_t id) noexcept -> void;
};

inline RandomColorMapsManager::ColorMapId::ColorMapId(const int32_t id) noexcept : m_id{id}
{
  Expects(id >= 0);
}

inline auto RandomColorMapsManager::ColorMapId::operator()() const noexcept -> size_t
{
  Expects(m_id >= 0);
  return static_cast<size_t>(m_id);
}

inline auto RandomColorMapsManager::ColorMapId::IsSet() const noexcept -> bool
{
  return m_id != -1;
}

inline auto RandomColorMapsManager::GetColorMap(const ColorMapId id) const noexcept
    -> const IColorMap&
{
  return *m_colorMaps.at(id());
}

inline auto RandomColorMapsManager::GetColorMapPtr(const ColorMapId id) const noexcept
    -> std::shared_ptr<const IColorMap>
{
  return m_colorMaps.at(id());
}

inline auto RandomColorMapsManager::ChangeAllColorMapsNow() noexcept -> void
{
  for (size_t id = 0; id < m_infoList.size(); ++id)
  {
    RandomizeColorMaps(id);
  }
}

inline auto RandomColorMapsManager::ChangeColorMapNow(const ColorMapId id) noexcept -> void
{
  RandomizeColorMaps(id());
}

} // namespace GOOM::COLOR
