#pragma once

#include "../goom_visual_fx.h"
#include "draw/goom_draw.h"
#include "goom_plugin_info.h"
#include "line_drawer_manager.h"
#include "lsys_colors.h"
#include "lsys_draw.h"
#include "lsys_geom.h"
#include "lsys_paths.h"
#include "point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/t_values.h"
#include "utils/timer.h"

#include <cstdint>
#include <lsys/graphics_generator.h>
#include <lsys/interpret.h>
#include <lsys/l_sys_model.h>
#include <lsys/list.h>
#include <lsys/module.h>
#include <lsys/parsed_model.h>
#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX::L_SYSTEM
{

class LSystem
{
public:
  struct LSystemFile
  {
    const char* filename{};
    struct Overrides
    {
      uint32_t minNumLSysCopies;
      uint32_t maxNumLSysCopies;
      uint32_t minMaxGen;
      uint32_t maxMaxGen;
      float lineWidthFactor;
      float expandBounds;
      uint32_t minNumRotateDegreeSteps;
      uint32_t maxNumRotateDegreeSteps;
      uint32_t minNumSpinDegreeSteps;
      uint32_t maxNumSpinDegreeSteps;
      float xScale;
      float yScale;
      float verticalMoveMin;
      float verticalMoveMax;
      float minDefaultLineWidthFactor;
      float maxDefaultLineWidthFactor;
      float minDefaultDistanceFactor;
      float maxDefaultDistanceFactor;
      float minDefaultTurnAngleInDegreesFactor;
      float maxDefaultTurnAngleInDegreesFactor;
      float probabilityOfSimpleColors;
      float probabilityOfNoise;
      struct NamedArg
      {
        const char* name;
        float min;
        float max;
      };
      std::vector<NamedArg> namedArgs;
    };
    Overrides overrides;
  };

  LSystem(DRAW::IGoomDraw& draw,
          const PluginInfo& goomInfo,
          const UTILS::MATH::IGoomRand& goomRand,
          const std::string& lSystemDirectory,
          const LSystemFile& lSystemFile) noexcept;

  auto ChangeColors() noexcept -> void;

  [[nodiscard]] auto GetPathStart() const noexcept -> const Point2dInt&;
  auto SetPathStart(const Point2dInt& pathStart) noexcept -> void;
  [[nodiscard]] auto GetPathTarget() const noexcept -> const Point2dInt&;
  auto SetPathTarget(const Point2dInt& pathTarget) noexcept -> void;

  auto Start() noexcept -> void;
  auto Update() noexcept -> void;
  auto DrawLSystem() noexcept -> void;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  LineDrawerManager m_lineDrawerManager;
  auto SwitchLineDrawers() -> void;

  static constexpr auto DEFAULT_BOUNDS_EXPAND_FACTOR = 2.0F;
  struct LSysModelSet
  {
    ::LSYS::Properties lSysProperties{};
    LSystemFile::Overrides lSysOverrides{};
    float lSystemXScale = 1.0F;
    float lSystemYScale = 1.0F;
    std::unique_ptr<::LSYS::LSysModel> lSysModel{};
  };
  std::unique_ptr<::LSYS::List<::LSYS::Module>> m_lSysModuleList{};
  LSysModelSet m_lSysModelSet;
  [[nodiscard]] static auto GetLSysModelSet(const PluginInfo& goomInfo,
                                            const std::string& lSysDirectory,
                                            const LSystemFile& lSystemFile) -> LSysModelSet;
  [[nodiscard]] static auto GetLSystemFilename(const std::string& lSystemDirectory,
                                               const LSystemFile& lSystemFile) noexcept
      -> std::string;
  [[nodiscard]] static auto GetBoundsFilename(const std::string& lSystemDirectory,
                                              const LSystemFile& lSystemFile) noexcept
      -> std::string;
  struct BoundingBox2d
  {
    GOOM::Point2dFlt min;
    GOOM::Point2dFlt max;
  };
  [[nodiscard]] static auto GetBoundingBox2d(float expandBounds,
                                             const ::LSYS::BoundingBox3d& boundingBox3d) noexcept
      -> BoundingBox2d;

  LSysColors m_lSysColors{*m_goomRand};
  LSysGeometry m_lSysGeometry;
  LSysDraw m_lSysDraw;

  LSysPath m_lSysPath{*m_goomRand};
  static constexpr auto MIN_PATH_NUM_STEPS = 50U;
  static constexpr auto MAX_PATH_NUM_STEPS = 200U;

  ::LSYS::GraphicsGenerator::DrawFuncs m_drawFuncs = GetLSysDrawFuncs();
  [[nodiscard]] auto GetLSysDrawFuncs() noexcept -> ::LSYS::GraphicsGenerator::DrawFuncs;
  static constexpr const auto* GENERATOR_NAME = "On the Fly";
  std::unique_ptr<::LSYS::GraphicsGenerator> m_LSysGenerator =
      std::make_unique<::LSYS::GraphicsGenerator>(GENERATOR_NAME, m_drawFuncs);
  std::unique_ptr<::LSYS::Interpreter> m_lSysInterpreter{};

  static constexpr auto BRIGHTNESS_ON_TIME         = 25U;
  static constexpr auto BRIGHTNESS_OFF_TIME        = 5U;
  static constexpr auto BRIGHTNESS_FAILED_ON_TIME  = 1U;
  static constexpr auto BRIGHTNESS_FAILED_OFF_TIME = 1U;
  static constexpr auto ON_BRIGHTNESS              = 2.0F;
  static constexpr auto OFF_BRIGHTNESS             = 1.0F;
  UTILS::OnOffTimer m_brightnessOnOffTimer{
      {BRIGHTNESS_ON_TIME,
       BRIGHTNESS_FAILED_ON_TIME, BRIGHTNESS_OFF_TIME,
       BRIGHTNESS_FAILED_OFF_TIME}
  };
  auto StartBrightnessTimer() noexcept -> void;

  static constexpr auto MIN_Y_SCALE_ADJUST_STEPS = 50U;
  static constexpr auto MAX_Y_SCALE_ADJUST_STEPS = 200U;
  static constexpr auto MIN_VERTICAL_MOVE_STEPS  = 50U;
  static constexpr auto MAX_VERTICAL_MOVE_STEPS  = 150U;

  uint32_t m_maxGen                              = 1U;
  static constexpr auto TIME_TO_KEEP_INTERPRETER = 500U;
  UTILS::Timer m_timeForThisLSysInterpreter{TIME_TO_KEEP_INTERPRETER};
  auto UpdateLSysModel() noexcept -> void;
  auto InitNextLSysInterpreter() -> void;
  static constexpr auto DEFAULT_NUM_INTERPRETER_PARAMS_STEPS = 100U;
  UTILS::IncrementedValue<::LSYS::Interpreter::DefaultParams> m_defaultInterpreterParams{
      UTILS::TValue::StepType::SINGLE_CYCLE, DEFAULT_NUM_INTERPRETER_PARAMS_STEPS};
  auto UpdateInterpreterParams() noexcept -> void;
  auto SetNewDefaultInterpreterParams() noexcept -> void;
  [[nodiscard]] auto GetRandomDefaultInterpreterParams() const noexcept
      -> ::LSYS::Interpreter::DefaultParams;
  auto ResetModelNamedArgs() -> void;
  auto RegenerateModuleList() noexcept -> void;
  auto RestartLSysInterpreter() noexcept -> void;
  auto ResetLSysParams() noexcept -> void;

  auto DrawLSystemBatch() noexcept -> void;
  auto IncrementTs() noexcept -> void;
};

inline auto LSystem::ChangeColors() noexcept -> void
{
  m_lSysColors.ChangeColors();
  SwitchLineDrawers();
}

inline auto LSystem::SwitchLineDrawers() -> void
{
  if (m_goomRand->ProbabilityOf(m_lSysModelSet.lSysOverrides.probabilityOfNoise))
  {
    m_lineDrawerManager.SwitchLineDrawers();
  }
  else
  {
    m_lineDrawerManager.SwitchLineDrawers(LineDrawerManager::SwitchLineDrawerType::NONE);
  }

  m_lSysDraw.SetLineDrawer(m_lineDrawerManager.GetLineDrawer());
}

inline auto LSystem::GetPathStart() const noexcept -> const Point2dInt&
{
  return m_lSysPath.GetPathStart();
}

inline auto LSystem::SetPathStart(const Point2dInt& pathStart) noexcept -> void
{
  m_lSysPath.SetPathStart(pathStart);
}

inline auto LSystem::GetPathTarget() const noexcept -> const Point2dInt&
{
  return m_lSysPath.GetPathTarget();
}

inline auto LSystem::SetPathTarget(const Point2dInt& pathTarget) noexcept -> void
{
  m_lSysPath.SetPathTarget(pathTarget);
}

} // namespace GOOM::VISUAL_FX::L_SYSTEM
