module;

#include <cstddef>
#include <string_view>
#include <vector>

export module Goom.Control.GoomStates;

import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRandBase;
import Goom.Lib.GoomTypes;

export namespace GOOM::CONTROL
{

enum class GoomStates : UnderlyingEnumType
{
  CIRCLES_ONLY = 0,
  CIRCLES_IFS,
  CIRCLES_IMAGE,
  CIRCLES_IMAGE_STARS,
  CIRCLES_IMAGE_STARS_L_SYSTEM,
  CIRCLES_LINES,
  CIRCLES_STARS_TUBES,
  CIRCLES_TENTACLES,
  DOTS_IFS,
  DOTS_IFS_RAINDROPS,
  DOTS_IFS_STARS,
  DOTS_IMAGE_RAINDROPS,
  DOTS_IMAGE_RAINDROPS_STARS,
  DOTS_IMAGE_STARS,
  DOTS_IMAGE_STARS_L_SYSTEM,
  DOTS_LINES,
  DOTS_LINES_RAINDROPS_STARS,
  DOTS_LINES_STARS_TENTACLES,
  DOTS_LINES_TENTACLES_TUBES,
  DOTS_LINES_TUBES,
  DOTS_ONLY,
  DOTS_RAINDROPS_STARS,
  DOTS_STARS,
  DOTS_STARS_L_SYSTEM,
  DOTS_STARS_TENTACLES_TUBES,
  DOTS_TENTACLES_TUBES,
  IFS_IMAGE,
  IFS_IMAGE_RAINDROPS_SHAPES,
  IFS_IMAGE_SHAPES,
  IFS_LINES_RAINDROPS_STARS,
  IFS_LINES_STARS,
  IFS_ONLY,
  IFS_PARTICLES,
  IFS_RAINDROPS,
  IFS_RAINDROPS_SHAPES,
  IFS_RAINDROPS_STARS,
  IFS_SHAPES,
  IFS_STARS,
  IFS_STARS_TENTACLES,
  IFS_TENTACLES,
  IFS_TENTACLES_TUBES,
  IFS_TUBES,
  IMAGE_LINES,
  IMAGE_LINES_RAINDROPS,
  IMAGE_LINES_SHAPES,
  IMAGE_LINES_STARS_TENTACLES,
  IMAGE_ONLY,
  IMAGE_RAINDROPS,
  IMAGE_RAINDROPS_SHAPES,
  IMAGE_SHAPES,
  IMAGE_SHAPES_L_SYSTEM,
  IMAGE_SHAPES_STARS,
  IMAGE_SHAPES_TUBES,
  IMAGE_STARS,
  IMAGE_STARS_L_SYSTEM,
  IMAGE_TENTACLES,
  IMAGE_TUBES,
  L_SYSTEM_ONLY,
  LINES_ONLY,
  LINES_PARTICLES,
  LINES_RAINDROPS,
  LINES_SHAPES_STARS,
  LINES_STARS,
  LINES_TENTACLES,
  PARTICLES_ONLY,
  PARTICLES_TENTACLES,
  PARTICLES_TUBES,
  RAINDROPS_ONLY,
  SHAPES_ONLY,
  SHAPES_STARS,
  SHAPES_TUBES,
  STARS_ONLY,
  TENTACLES_ONLY,
  TUBES_ONLY,
};

enum class GoomDrawables : UnderlyingEnumType
{
  CIRCLES = 0,
  DOTS,
  IFS,
  IMAGE,
  L_SYSTEM,
  LINES,
  PARTICLES,
  RAINDROPS,
  SHAPES,
  STARS,
  TENTACLES,
  TUBES,
};

using BuffIntensityRange = UTILS::MATH::NumberRange<float>;

class GoomStateInfo
{
public:
  struct DrawableInfo
  {
    GoomDrawables fx{};
    BuffIntensityRange buffIntensityRange{};
  };
  struct StateInfo
  {
    std::string_view name;
    std::vector<DrawableInfo> drawablesInfo;
  };

  GoomStateInfo() noexcept = delete;

  [[nodiscard]] static auto GetStateInfo(GoomStates goomState) -> const StateInfo&;
  [[nodiscard]] static auto GetBuffIntensityRange(GoomStates goomState, GoomDrawables fx)
      -> BuffIntensityRange;
  [[nodiscard]] static auto IsMultiThreaded(GoomStates goomState) -> bool;

private:
  using StateInfoMap = UTILS::EnumMap<GoomStates, StateInfo>;
  static const StateInfoMap STATE_INFO_MAP;
  [[nodiscard]] static auto GetStateInfoMap() noexcept -> StateInfoMap;
  [[nodiscard]] static auto GetDrawablesInfo(GoomStates goomState) -> std::vector<DrawableInfo>;
};

} // namespace GOOM::CONTROL

namespace GOOM::CONTROL
{

inline auto GoomStateInfo::GetStateInfo(const GoomStates goomState) -> const StateInfo&
{
  return STATE_INFO_MAP[goomState];
}

} // namespace GOOM::CONTROL
