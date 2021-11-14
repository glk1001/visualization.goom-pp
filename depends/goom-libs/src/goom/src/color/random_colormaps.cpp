#include "random_colormaps.h"

#include "color_data/colormap_enums.h"
#include "colormaps.h"
#include "goom/goom_graphic.h"
#include "goom/logging_control.h"
#include "utils/enumutils.h"
#include "utils/goom_rand_base.h"
//#undef NO_LOGGING
#include "goom/logging.h"

#include <format>
#include <memory>
#include <set>
#include <stdexcept>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace COLOR
{
#else
namespace GOOM::COLOR
{
#endif

using COLOR_DATA::ColorMapName;
using UTILS::IGoomRand;
using UTILS::NUM;
using UTILS::Weights;

#if __cplusplus <= 201402L
const float RandomColorMaps::MIN_ROTATION_POINT = 0.1F;
const float RandomColorMaps::MAX_ROTATION_POINT = 0.9F;
const float RandomColorMaps::MIN_SATURATION = 0.1F;
const float RandomColorMaps::MAX_SATURATION = 1.0F;
const float RandomColorMaps::MIN_LIGHTNESS = 0.1F;
const float RandomColorMaps::MAX_LIGHTNESS = 1.0F;
#endif

const std::set<RandomColorMaps::ColorMapTypes> RandomColorMaps::EMPTY{};
const std::set<RandomColorMaps::ColorMapTypes> RandomColorMaps::ALL{
    ColorMapTypes::ROTATED_T,
    ColorMapTypes::SHADES,
};

auto RandomColorMaps::GetRandomColorMapName() const -> COLOR_DATA::ColorMapName
{
  return static_cast<ColorMapName>(m_goomRand.GetRandInRange(0U, GetNumColorMapNames()));
}

auto RandomColorMaps::GetRandomColorMapName(const ColorMapGroup cmg) const
    -> COLOR_DATA::ColorMapName
{
  const ColorMapNames& colorMapNames = GetColorMapNames(cmg);
  return colorMapNames[m_goomRand.GetRandInRange(0U, static_cast<uint32_t>(colorMapNames.size()))];
}

auto RandomColorMaps::GetRandomColorMap() const -> const IColorMap&
{
  return GetColorMap(GetRandomColorMapName());
}

auto RandomColorMaps::GetRandomColorMap(const ColorMapGroup cmg) const -> const IColorMap&
{
  return GetColorMap(GetRandomColorMapName(cmg));
}

auto RandomColorMaps::GetRandomColorMapPtr(const std::set<ColorMapTypes>& types) const
    -> std::shared_ptr<const IColorMap>
{
  return GetRandomColorMapPtr(GetColorMapPtr(GetRandomColorMapName()), types);
}

auto RandomColorMaps::GetRandomColorMapPtr(const COLOR_DATA::ColorMapName colorMapName,
                                           const std::set<ColorMapTypes>& types) const
    -> std::shared_ptr<const IColorMap>
{
  return GetRandomColorMapPtr(GetColorMapPtr(colorMapName), types);
}

auto RandomColorMaps::GetRandomColorMapPtr(const ColorMapGroup colorMapGroup,
                                           const std::set<ColorMapTypes>& types) const
    -> std::shared_ptr<const IColorMap>
{
  return GetRandomColorMapPtr(GetColorMapPtr(GetRandomColorMapName(colorMapGroup)), types);
}

auto RandomColorMaps::GetRandomColorMapPtr(const std::shared_ptr<const IColorMap>& colorMap,
                                           const std::set<ColorMapTypes>& types) const
    -> std::shared_ptr<const IColorMap>
{
  if (types.empty())
  {
    return colorMap;
  }

  std::shared_ptr<const IColorMap> newColorMap = colorMap;

#if __cplusplus <= 201703L
  if (types.find(ColorMapTypes::ROTATED_T) != types.end())
#else
  if (types.contains(ColorMapTypes::ROTATED_T))
#endif
  {
    newColorMap = GetRandomRotatedColorMapPtr(newColorMap);
  }
#if __cplusplus <= 201703L
  if (types.find(ColorMapTypes::SHADES) != types.end())
#else
  if (types.contains(ColorMapTypes::SHADES))
#endif
  {
    newColorMap = GetRandomTintedColorMapPtr(newColorMap);
  }

  return colorMap;
}

auto RandomColorMaps::GetRandomRotatedColorMapPtr() const -> std::shared_ptr<const IColorMap>
{
  return GetRotatedColorMapPtr(GetRandomColorMapName(),
                               m_goomRand.GetRandInRange(m_minRotationPoint, m_maxRotationPoint));
}

auto RandomColorMaps::GetRandomRotatedColorMapPtr(const COLOR_DATA::ColorMapName colorMapName) const
    -> std::shared_ptr<const IColorMap>
{
  return GetRotatedColorMapPtr(colorMapName,
                               m_goomRand.GetRandInRange(m_minRotationPoint, m_maxRotationPoint));
}

auto RandomColorMaps::GetRandomRotatedColorMapPtr(const ColorMapGroup colorMapGroup) const
    -> std::shared_ptr<const IColorMap>
{
  return GetRotatedColorMapPtr(GetRandomColorMapName(colorMapGroup),
                               m_goomRand.GetRandInRange(m_minRotationPoint, m_maxRotationPoint));
}

auto RandomColorMaps::GetRandomRotatedColorMapPtr(
    const std::shared_ptr<const IColorMap>& colorMap) const -> std::shared_ptr<const IColorMap>
{
  return GetRotatedColorMapPtr(colorMap,
                               m_goomRand.GetRandInRange(m_minRotationPoint, m_maxRotationPoint));
}

auto RandomColorMaps::GetRandomTintedColorMapPtr() const -> std::shared_ptr<const IColorMap>
{
  return GetTintedColorMapPtr(GetRandomColorMapName(),
                              m_goomRand.GetRandInRange(m_minSaturation, m_maxSaturation),
                              m_goomRand.GetRandInRange(m_minLightness, m_maxLightness));
}

auto RandomColorMaps::GetRandomTintedColorMapPtr(const ColorMapName colorMapName) const
    -> std::shared_ptr<const IColorMap>
{
  return GetTintedColorMapPtr(colorMapName,
                              m_goomRand.GetRandInRange(m_minSaturation, m_maxSaturation),
                              m_goomRand.GetRandInRange(m_minLightness, m_maxLightness));
}

auto RandomColorMaps::GetRandomTintedColorMapPtr(const ColorMapGroup colorMapGroup) const
    -> std::shared_ptr<const IColorMap>
{
  return GetTintedColorMapPtr(GetRandomColorMapName(colorMapGroup),
                              m_goomRand.GetRandInRange(m_minSaturation, m_maxSaturation),
                              m_goomRand.GetRandInRange(m_minLightness, m_maxLightness));
}

auto RandomColorMaps::GetRandomTintedColorMapPtr(
    const std::shared_ptr<const IColorMap>& colorMap) const -> std::shared_ptr<const IColorMap>
{
  return GetTintedColorMapPtr(colorMap, m_goomRand.GetRandInRange(m_minSaturation, m_maxSaturation),
                              m_goomRand.GetRandInRange(m_minLightness, m_maxLightness));
}

auto RandomColorMaps::GetRandomGroup() const -> ColorMapGroup
{
  return static_cast<ColorMapGroup>(m_goomRand.GetRandInRange(0U, NUM<ColorMapGroup>));
}

auto RandomColorMaps::GetRandomColor(const IColorMap& colorMap, const float t0, const float t1)
    -> Pixel
{
  return colorMap.GetColor(m_goomRand.GetRandInRange(t0, t1));
}

auto RandomColorMaps::GetMinRotationPoint() const -> float
{
  return m_minRotationPoint;
}

auto RandomColorMaps::GetMaxRotationPoint() const -> float
{
  return m_maxRotationPoint;
}

void RandomColorMaps::SetRotationPointLimits(const float minRotationPoint,
                                             const float maxRotationPoint)
{
  if (minRotationPoint < MIN_ROTATION_POINT)
  {
    throw std::logic_error(
        std20::format("minRotationPoint {} < {}", minRotationPoint, MIN_ROTATION_POINT));
  }
  if (maxRotationPoint > MAX_ROTATION_POINT)
  {
    throw std::logic_error(
        std20::format("maxRotationPoint {} < {}", maxRotationPoint, MAX_ROTATION_POINT));
  }
  if (minRotationPoint > maxRotationPoint)
  {
    throw std::logic_error(std20::format("minRotationPoint {} > maxRotationPoint {}",
                                         minRotationPoint, maxRotationPoint));
  }

  m_minRotationPoint = minRotationPoint;
  m_maxRotationPoint = maxRotationPoint;
}

auto RandomColorMaps::GetMinSaturation() const -> float
{
  return m_minSaturation;
}

auto RandomColorMaps::GetMaxSaturation() const -> float
{
  return m_maxSaturation;
}

void RandomColorMaps::SetSaturationLimits(const float minSaturation, const float maxSaturation)
{
  if (minSaturation < MIN_SATURATION)
  {
    throw std::logic_error(std20::format("minSaturation {} < {}", minSaturation, MIN_SATURATION));
  }
  if (maxSaturation > MAX_SATURATION)
  {
    throw std::logic_error(std20::format("maxSaturation {} < {}", maxSaturation, MAX_SATURATION));
  }
  if (minSaturation > maxSaturation)
  {
    throw std::logic_error(
        std20::format("minSaturation {} > maxSaturation {}", minSaturation, maxSaturation));
  }

  m_minSaturation = minSaturation;
  m_maxSaturation = maxSaturation;
}

auto RandomColorMaps::GetMinLightness() const -> float
{
  return m_minLightness;
}

auto RandomColorMaps::GetMaxLightness() const -> float
{
  return m_maxLightness;
}

void RandomColorMaps::SetLightnessLimits(const float minLightness, const float maxLightness)
{
  if (minLightness < MIN_LIGHTNESS)
  {
    throw std::logic_error(std20::format("minLightness {} < {}", minLightness, MIN_LIGHTNESS));
  }
  if (maxLightness > MAX_LIGHTNESS)
  {
    throw std::logic_error(std20::format("maxLightness {} < {}", maxLightness, MAX_LIGHTNESS));
  }
  if (minLightness > maxLightness)
  {
    throw std::logic_error(
        std20::format("minLightness {} > maxLightness {}", minLightness, maxLightness));
  }

  m_minLightness = minLightness;
  m_maxLightness = maxLightness;
}

WeightedColorMaps::WeightedColorMaps(IGoomRand& goomRand, const Weights<ColorMapGroup>& weights)
  : RandomColorMaps{goomRand}, m_weights{weights}, m_weightsActive{true}
{
}

auto WeightedColorMaps::GetRandomGroup() const -> ColorMapGroup
{
  if (!m_weightsActive)
  {
    return RandomColorMaps::GetRandomGroup();
  }

  return m_weights.GetRandomWeighted();
}

auto GetAllMapsUnweighted(IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>
{
  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::ALL, 1},
          }}
  );
  // clang-format on
}

auto GetAllStandardMaps(IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>
{
  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL, 1},
              {ColorMapGroup::SEQUENTIAL,                      1},
              {ColorMapGroup::SEQUENTIAL2,                     1},
              {ColorMapGroup::CYCLIC,                          1},
              {ColorMapGroup::DIVERGING,                       1},
              {ColorMapGroup::DIVERGING_BLACK,                 1},
              {ColorMapGroup::QUALITATIVE,                     1},
              {ColorMapGroup::MISC,                            1},
          }
      }
  );
  // clang-format on
}

auto GetAllSlimMaps(IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>
{
  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM, 1},
              {ColorMapGroup::SEQUENTIAL_SLIM,                      1},
              {ColorMapGroup::SEQUENTIAL2_SLIM,                     1},
              {ColorMapGroup::CYCLIC_SLIM,                          1},
              {ColorMapGroup::DIVERGING_SLIM,                       1},
              {ColorMapGroup::DIVERGING_BLACK_SLIM,                 1},
              {ColorMapGroup::QUALITATIVE_SLIM,                     1},
              {ColorMapGroup::MISC_SLIM,                            1},
          }
      }
  );
  // clang-format on
}

auto GetMostlySequentialStandardMaps(IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>
{
  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL, 1},
              {ColorMapGroup::SEQUENTIAL,                      1},
              {ColorMapGroup::SEQUENTIAL2,                     1},
              {ColorMapGroup::CYCLIC,                          0},
              {ColorMapGroup::DIVERGING,                       0},
              {ColorMapGroup::DIVERGING_BLACK,                 0},
              {ColorMapGroup::QUALITATIVE,                     1},
              {ColorMapGroup::MISC,                            1},
          }
      }
  );
  // clang-format on
}

auto GetMostlySequentialSlimMaps(IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>
{
  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM, 1},
              {ColorMapGroup::SEQUENTIAL_SLIM,                      1},
              {ColorMapGroup::SEQUENTIAL2_SLIM,                     1},
              {ColorMapGroup::CYCLIC_SLIM,                          0},
              {ColorMapGroup::DIVERGING_SLIM,                       0},
              {ColorMapGroup::DIVERGING_BLACK_SLIM,                 0},
              {ColorMapGroup::QUALITATIVE_SLIM,                     1},
              {ColorMapGroup::MISC_SLIM,                            1},
          }
      }
  );
  // clang-format on
}

auto GetSlightlyDivergingStandardMaps(IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>
{
  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL, 10},
              {ColorMapGroup::SEQUENTIAL,                      10},
              {ColorMapGroup::SEQUENTIAL2,                     10},
              {ColorMapGroup::CYCLIC,                          10},
              {ColorMapGroup::DIVERGING,                       20},
              {ColorMapGroup::DIVERGING_BLACK,                  1},
              {ColorMapGroup::QUALITATIVE,                     10},
              {ColorMapGroup::MISC,                            20},
          }
      }
  );
  // clang-format on
}

auto GetSlightlyDivergingSlimMaps(IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>
{
  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::PERCEPTUALLY_UNIFORM_SEQUENTIAL_SLIM, 10},
              {ColorMapGroup::SEQUENTIAL_SLIM,                      10},
              {ColorMapGroup::SEQUENTIAL2_SLIM,                     10},
              {ColorMapGroup::CYCLIC_SLIM,                          10},
              {ColorMapGroup::DIVERGING_SLIM,                       20},
              {ColorMapGroup::DIVERGING_BLACK_SLIM,                  1},
              {ColorMapGroup::QUALITATIVE_SLIM,                     10},
              {ColorMapGroup::MISC_SLIM,                            20},
          }
      }
  );
  // clang-format on
}

auto GetBlueStandardMaps(IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>
{
  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::BLUES, 1},
          }
      }
  );
  // clang-format on
}

auto GetRedStandardMaps(IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>
{
  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::REDS, 1},
          }
      }
  );
  // clang-format on
}

auto GetGreenStandardMaps(IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>
{
  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::GREENS, 1},
          }
      }
  );
  // clang-format on
}

auto GetYellowStandardMaps(IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>
{
  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::YELLOWS, 1},
          }
      }
  );
  // clang-format on
}

auto GetOrangeStandardMaps(IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>
{
  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::ORANGES, 1},
          }
      }
  );
  // clang-format on
}

auto GetPurpleStandardMaps(IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>
{
  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::PURPLES, 1},
          }
      }
  );
  // clang-format on
}

auto GetCitiesStandardMaps(IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>
{
  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::CITIES, 1},
          }
      }
  );
  // clang-format on
}

auto GetSeasonsStandardMaps(IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>
{
  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::SEASONS, 1},
          }
      }
  );
  // clang-format on
}

auto GetHeatStandardMaps(IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>
{
  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::HEAT, 1},
          }
      }
  );
  // clang-format on
}

auto GetColdStandardMaps(IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>
{
  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::COLD, 1},
          }
      }
  );
  // clang-format on
}

auto GetPastelStandardMaps(IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>
{
  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::PASTEL, 1},
          }
      }
  );
  // clang-format on
}

auto GetDivergingBlackStandardMaps(IGoomRand& goomRand) -> std::shared_ptr<RandomColorMaps>
{
  // clang-format off
  return std::make_shared<WeightedColorMaps>(
      goomRand,
      Weights<ColorMapGroup>{
          goomRand,
          {
              {ColorMapGroup::DIVERGING_BLACK, 1},
          }
      }
  );
  // clang-format on
}

#if __cplusplus <= 201402L
} // namespace COLOR
} // namespace GOOM
#else
} // namespace GOOM::COLOR
#endif
