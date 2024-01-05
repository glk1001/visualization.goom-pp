#pragma once

// IMPORTANT - THIS CODE IS GENERATED BY:
//
//  'create_colormaps.py'
//
// DO NOT EDIT!!

#include "color_map_enums.h"
#include "vivid/types.h"

#include <array>
#include <vector>

namespace GOOM::COLOR::COLOR_DATA
{

// array of raw maps matching elements of enum 'ColorMapName'
struct ColorNamePair
{
  ColorMapName colorMapName;
  const std::vector<vivid::srgb_t>* vividArray;
};

extern const std::array<ColorNamePair, NUM_COLOR_MAP_ENUMS> ALL_MAPS;

extern const std::vector<ColorMapName> PERC_UNIF_SEQUENTIAL_MAPS;
extern const std::vector<ColorMapName> SEQUENTIAL_MAPS;
extern const std::vector<ColorMapName> SEQUENTIAL2_MAPS;
extern const std::vector<ColorMapName> DIVERGING_MAPS;
extern const std::vector<ColorMapName> DIVERGING_BLACK_MAPS;
extern const std::vector<ColorMapName> QUALITATIVE_MAPS;
extern const std::vector<ColorMapName> MISC_MAPS;
extern const std::vector<ColorMapName> CYCLIC_MAPS;
extern const std::vector<ColorMapName> PERC_UNIF_SEQUENTIAL_SLIM_MAPS;
extern const std::vector<ColorMapName> SEQUENTIAL_SLIM_MAPS;
extern const std::vector<ColorMapName> SEQUENTIAL2_SLIM_MAPS;
extern const std::vector<ColorMapName> DIVERGING_SLIM_MAPS;
extern const std::vector<ColorMapName> DIVERGING_BLACK_SLIM_MAPS;
extern const std::vector<ColorMapName> QUALITATIVE_SLIM_MAPS;
extern const std::vector<ColorMapName> MISC_SLIM_MAPS;
extern const std::vector<ColorMapName> CYCLIC_SLIM_MAPS;

} // namespace GOOM::COLOR::COLOR_DATA
