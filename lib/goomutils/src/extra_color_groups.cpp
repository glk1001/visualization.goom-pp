#include "color_data/extra_color_groups.h"

#include "color_data/colormap_enums.h"

#include <vector>

#if __cplusplus > 201402L
namespace GOOM::UTILS::COLOR_DATA
{
#else
namespace GOOM
{
namespace UTILS
{
namespace COLOR_DATA
{
#endif

// clang-format off
const std::vector<ColorMapName> WES_ANDERSON_MAPS {
    ColorMapName::AQUATIC1_5,
    ColorMapName::AQUATIC2_5,
    ColorMapName::AQUATIC3_5,
    ColorMapName::CAVALCANTI_5,
    ColorMapName::CHEVALIER_4,
    ColorMapName::DARJEELING1_4,
    ColorMapName::DARJEELING2_5,
    ColorMapName::DARJEELING3_5,
    ColorMapName::DARJEELING4_5,
    ColorMapName::FANTASTICFOX1_5,
    ColorMapName::FANTASTICFOX2_5,
    ColorMapName::GRANDBUDAPEST1_4,
    ColorMapName::GRANDBUDAPEST2_4,
    ColorMapName::GRANDBUDAPEST3_6,
    ColorMapName::GRANDBUDAPEST4_5,
    ColorMapName::GRANDBUDAPEST5_5,
    ColorMapName::ISLEOFDOGS1_5,
    ColorMapName::ISLEOFDOGS2_6,
    ColorMapName::ISLEOFDOGS3_4,
    ColorMapName::MOONRISE1_5,
    ColorMapName::MOONRISE2_4,
    ColorMapName::MOONRISE3_4,
    ColorMapName::MOONRISE4_5,
    ColorMapName::MOONRISE5_6,
    ColorMapName::MOONRISE6_5,
    ColorMapName::MOONRISE7_5,
    ColorMapName::MENDL_4,
    ColorMapName::MARGOT1_5,
    ColorMapName::MARGOT2_4,
    ColorMapName::MARGOT3_4,
    ColorMapName::ROYAL1_4,
    ColorMapName::ROYAL2_5,
    ColorMapName::ROYAL3_5,
    ColorMapName::ZISSOU_5,
};
const std::vector<ColorMapName> BLUE_MAPS
{
    ColorMapName::BLUES,
    ColorMapName::BLUES10_2,
    ColorMapName::BLUES10_3,
    ColorMapName::BLUES10_4,
    ColorMapName::BLUES10_5,
    ColorMapName::BLUES10_6,
    ColorMapName::BLUES10_7,
    ColorMapName::BLUES10_8,
    ColorMapName::BLUES10_9,
    ColorMapName::BLUES10_10,
    ColorMapName::BLUES7_2,
    ColorMapName::BLUES7_3,
    ColorMapName::BLUES7_4,
    ColorMapName::BLUES7_5,
    ColorMapName::BLUES7_6,
    ColorMapName::BLUES7_7,
    ColorMapName::BLUES_3,
    ColorMapName::BLUES_4,
    ColorMapName::BLUES_5,
    ColorMapName::BLUES_6,
    ColorMapName::BLUES_7,
    ColorMapName::BLUES_8,
    ColorMapName::BLUES_9,
    ColorMapName::BLUGRN_2,
    ColorMapName::BLUGRN_3,
    ColorMapName::BLUGRN_4,
    ColorMapName::BLUGRN_5,
    ColorMapName::BLUGRN_6,
    ColorMapName::BLUGRN_7,
    ColorMapName::BLUYL_2,
    ColorMapName::BLUYL_3,
    ColorMapName::BLUYL_4,
    ColorMapName::BLUYL_5,
    ColorMapName::BLUYL_6,
    ColorMapName::BLUYL_7,
    ColorMapName::BROWNBLUE10_10,
    ColorMapName::BROWNBLUE12_12,
    ColorMapName::EMRLD_2,
    ColorMapName::EMRLD_3,
    ColorMapName::EMRLD_4,
    ColorMapName::EMRLD_5,
    ColorMapName::EMRLD_6,
    ColorMapName::EMRLD_7,
    ColorMapName::PURP_2,
    ColorMapName::PURP_3,
    ColorMapName::PURP_4,
    ColorMapName::PURP_5,
    ColorMapName::PURP_6,
    ColorMapName::PURP_7,
    ColorMapName::PURPLE_16,
    ColorMapName::PURPLES,
    ColorMapName::PURPLES_3,
    ColorMapName::PURPLES_4,
    ColorMapName::PURPLES_5,
    ColorMapName::PURPLES_6,
    ColorMapName::PURPLES_7,
    ColorMapName::PURPLES_8,
    ColorMapName::PURPLES_9,
    ColorMapName::PURPOR_2,
    ColorMapName::PURPOR_3,
    ColorMapName::PURPOR_4,
    ColorMapName::PURPOR_5,
    ColorMapName::PURPOR_6,
    ColorMapName::PURPOR_7,
    ColorMapName::TEAL_2,
    ColorMapName::TEAL_3,
    ColorMapName::TEAL_4,
    ColorMapName::TEAL_5,
    ColorMapName::TEAL_6,
    ColorMapName::TEAL_7,
    ColorMapName::TEALGRN_2,
    ColorMapName::TEALGRN_3,
    ColorMapName::TEALGRN_4,
    ColorMapName::TEALGRN_5,
    ColorMapName::TEALGRN_6,
    ColorMapName::TEALGRN_7,
    ColorMapName::YLGNBU,
    ColorMapName::YLGNBU_3,
    ColorMapName::YLGNBU_4,
    ColorMapName::YLGNBU_5,
    ColorMapName::YLGNBU_6,
    ColorMapName::YLGNBU_7,
    ColorMapName::YLGNBU_8,
    ColorMapName::YLGNBU_9,
};
const std::vector<ColorMapName> RED_MAPS
{
    ColorMapName::ARMYROSE_7,
    ColorMapName::AUTUMN,
    ColorMapName::BLUEDARKRED12_12,
    ColorMapName::BLUEORANGERED_14,
    ColorMapName::MAGENTA_7,
    ColorMapName::RDBU,
    ColorMapName::RDGY,
    ColorMapName::RDPU,
    ColorMapName::RDYLBU,
    ColorMapName::RDYLGN,
    ColorMapName::REDS,
    ColorMapName::REDOR_7,
    ColorMapName::REDYELLOWBLUE_11,
    ColorMapName::YLORRD,
};
const std::vector<ColorMapName> GREEN_MAPS
{
    ColorMapName::BLUEGREEN_14,
    ColorMapName::BLUGRN_7,
    ColorMapName::BUGN,
    ColorMapName::DARKMINT_7,
    ColorMapName::EMRLD_7,
    ColorMapName::GNBU,
    ColorMapName::GREENMAGENTA_16,
    ColorMapName::GREENORANGE_12,
    ColorMapName::GREENS,
    ColorMapName::MINT_7,
    ColorMapName::PINK_BLACK_GREEN_W3C_,
    ColorMapName::PRGN_11,
    ColorMapName::PUBUGN,
    ColorMapName::RDGY,
    ColorMapName::RDYLGN,
    ColorMapName::RED_BLACK_GREEN,
    ColorMapName::TEALGRN_7,
    ColorMapName::YLGN,
    ColorMapName::YLGNBU,
};
const std::vector<ColorMapName> YELLOW_MAPS
{
    ColorMapName::AGGRNYL_7,
    ColorMapName::BLUYL_7,
    ColorMapName::BRWNYL_7,
    ColorMapName::BURGYL_7,
    ColorMapName::ORYEL_7,
    ColorMapName::PINKYL_7,
    ColorMapName::PIYG,
    ColorMapName::RDYLBU_11,
    ColorMapName::RDYLGN,
    ColorMapName::REDYELLOWBLUE_11,
    ColorMapName::YLGN,
    ColorMapName::YLGNBU,
    ColorMapName::YLORBR,
    ColorMapName::YLORRD,
};
const std::vector<ColorMapName> ORANGE_MAPS
{
    ColorMapName::BLUEDARKORANGE12_12,
    ColorMapName::BLUEDARKORANGE18_18,
    ColorMapName::BLUEORANGE12_12,
    ColorMapName::BLUEORANGE10_10,
    ColorMapName::BLUEORANGE8_8,
    ColorMapName::BLUEORANGERED_14,
    ColorMapName::ORANGES,
    ColorMapName::ORRD,
    ColorMapName::ORYEL_7,
    ColorMapName::PEACH_7,
    ColorMapName::PURPOR_7,
    ColorMapName::RED_BLACK_ORANGE,
    ColorMapName::REDOR_7,
    ColorMapName::SUMMER,
    ColorMapName::SUNSET_7,
    ColorMapName::SUNSETDARK_7,
    ColorMapName::TRAFFICLIGHT_9,
    ColorMapName::YLORBR,
    ColorMapName::YLORRD,
};
const std::vector<ColorMapName> PURPLE_MAPS
{
    ColorMapName::BUPU,
    ColorMapName::PUBU,
    ColorMapName::PUBUGN,
    ColorMapName::PUOR,
    ColorMapName::PURD,
    ColorMapName::PURP_7,
    ColorMapName::PURPLE_16,
    ColorMapName::PURPLEGRAY_12,
    ColorMapName::PURPLES,
    ColorMapName::PURPOR_7,
    ColorMapName::RDPU_9,
};
const std::vector<ColorMapName> CITY_MAPS
{
    ColorMapName::BAMAKO_20,
    ColorMapName::BATLOW_20,
    ColorMapName::BERLIN_20,
    ColorMapName::BILBAO_20,
    ColorMapName::BUDA_20,
    ColorMapName::CORK_20,
    ColorMapName::DAVOS_20,
    ColorMapName::DEVON_20,
    ColorMapName::HAWAII_20,
    ColorMapName::LAJOLLA_20,
    ColorMapName::LAPAZ_20,
    ColorMapName::LISBON_20,
    ColorMapName::OLERON_20,
    ColorMapName::OSLO_20,
    ColorMapName::ROMA_20,
    ColorMapName::TOFINO_20,
    ColorMapName::TOKYO_20,
};
const std::vector<ColorMapName> SEASON_MAPS
{
    ColorMapName::AUTUMN,
    ColorMapName::FALL_7,
    ColorMapName::SPRING,
    ColorMapName::SUMMER,
    ColorMapName::WINTER,
};
const std::vector<ColorMapName> HEAT_MAPS
{
    ColorMapName::AFMHOT,
    ColorMapName::GEYSER_7,
    ColorMapName::GIST_HEAT,
    ColorMapName::HOT,
    ColorMapName::INFERNO,
    ColorMapName::INFERNO_20,
    ColorMapName::JET,
    ColorMapName::MAGMA,
    ColorMapName::MAGMA_20,
    ColorMapName::PLASMA,
    ColorMapName::PLASMA_20,
    ColorMapName::SOLAR_20,
    ColorMapName::SUMMER,
    ColorMapName::THERMAL_20,
    ColorMapName::TROPIC_7,
};
const std::vector<ColorMapName> COLD_MAPS
{
    ColorMapName::BLUEGRAY_2,
    ColorMapName::BLUEGRAY_3,
    ColorMapName::BLUEGRAY_4,
    ColorMapName::BLUEGRAY_5,
    ColorMapName::BLUEGRAY_6,
    ColorMapName::BLUEGRAY_7,
    ColorMapName::BLUEGRAY_8,
    ColorMapName::BLUEGREY_2,
    ColorMapName::BLUEGREY_3,
    ColorMapName::BLUEGREY_4,
    ColorMapName::BLUEGREY_5,
    ColorMapName::BLUEGREY_6,
    ColorMapName::BLUEGREY_7,
    ColorMapName::BLUEGREY_8,
    ColorMapName::COOL,
    ColorMapName::COOLWARM,
    ColorMapName::FALL_2,
    ColorMapName::FALL_3,
    ColorMapName::FALL_4,
    ColorMapName::FALL_5,
    ColorMapName::FALL_6,
    ColorMapName::FALL_7,
    ColorMapName::GRAY,
    ColorMapName::GRAY_3,
    ColorMapName::GRAY_4,
    ColorMapName::GRAY_5,
    ColorMapName::GRAY_6,
    ColorMapName::GRAY_7,
    ColorMapName::GRAY_8,
    ColorMapName::GRAY_9,
    ColorMapName::GRAY_10,
    ColorMapName::GRAY_11,
    ColorMapName::GRAY_12,
    ColorMapName::GRAY_13,
    ColorMapName::GRAY_14,
    ColorMapName::GRAY_15,
    ColorMapName::GRAY_16,
    ColorMapName::GRAY_17,
    ColorMapName::GRAY_18,
    ColorMapName::GRAY_19,
    ColorMapName::GRAY_20,
    ColorMapName::GRAYC_3,
    ColorMapName::GRAYC_4,
    ColorMapName::GRAYC_5,
    ColorMapName::GRAYC_6,
    ColorMapName::GRAYC_7,
    ColorMapName::GRAYC_8,
    ColorMapName::GRAYC_9,
    ColorMapName::GRAYC_10,
    ColorMapName::GRAYC_11,
    ColorMapName::GRAYC_12,
    ColorMapName::GRAYC_13,
    ColorMapName::GRAYC_14,
    ColorMapName::GRAYC_15,
    ColorMapName::GRAYC_16,
    ColorMapName::GRAYC_17,
    ColorMapName::GRAYC_18,
    ColorMapName::GRAYC_19,
    ColorMapName::GRAYC_20,
    ColorMapName::GREYS,
    ColorMapName::GREYS_3,
    ColorMapName::GREYS_4,
    ColorMapName::GREYS_5,
    ColorMapName::GREYS_6,
    ColorMapName::GREYS_7,
    ColorMapName::GREYS_8,
    ColorMapName::GREYS_9,
    ColorMapName::ICE_3,
    ColorMapName::ICE_4,
    ColorMapName::ICE_5,
    ColorMapName::ICE_6,
    ColorMapName::ICE_7,
    ColorMapName::ICE_8,
    ColorMapName::ICE_9,
    ColorMapName::ICE_10,
    ColorMapName::ICE_11,
    ColorMapName::ICE_12,
    ColorMapName::ICE_13,
    ColorMapName::ICE_14,
    ColorMapName::ICE_15,
    ColorMapName::ICE_16,
    ColorMapName::ICE_17,
    ColorMapName::ICE_18,
    ColorMapName::ICE_19,
    ColorMapName::ICE_20,
    ColorMapName::PURPLEGRAY_6,
    ColorMapName::PURPLEGRAY_12,
    ColorMapName::RDGY,
    ColorMapName::RDGY_3,
    ColorMapName::RDGY_4,
    ColorMapName::RDGY_5,
    ColorMapName::RDGY_6,
    ColorMapName::RDGY_7,
    ColorMapName::RDGY_8,
    ColorMapName::RDGY_9,
    ColorMapName::RDGY_10,
    ColorMapName::RDGY_11,
    ColorMapName::TWILIGHT,
    ColorMapName::TWILIGHT_SHIFTED,
};
const std::vector<ColorMapName> PASTEL_MAPS
{
    ColorMapName::PASTEL1,
    ColorMapName::PASTEL1_3,
    ColorMapName::PASTEL1_4,
    ColorMapName::PASTEL1_5,
    ColorMapName::PASTEL1_6,
    ColorMapName::PASTEL1_7,
    ColorMapName::PASTEL1_8,
    ColorMapName::PASTEL2,
    ColorMapName::PASTEL2_3,
    ColorMapName::PASTEL2_4,
    ColorMapName::PASTEL2_5,
    ColorMapName::PASTEL2_6,
    ColorMapName::PASTEL2_7,
    ColorMapName::PASTEL_2,
    ColorMapName::PASTEL_3,
    ColorMapName::PASTEL_4,
    ColorMapName::PASTEL_5,
    ColorMapName::PASTEL_6,
    ColorMapName::PASTEL_7,
    ColorMapName::PASTEL_8,
    ColorMapName::PASTEL_9,
    ColorMapName::PASTEL_10,
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif
