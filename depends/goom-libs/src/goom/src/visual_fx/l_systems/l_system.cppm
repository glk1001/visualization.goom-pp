module;

#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

module Goom.VisualFx.LSystemFx:LSystem;

import LSys.GraphicsGenerator;
import LSys.Interpret;
import LSys.List;
import LSys.LSysModel;
import LSys.Module;
import LSys.ParsedModel;
import LSys.Value;
import LSys.Vector;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.IncrementedValues;
import Goom.Utils.Math.Misc;
import Goom.Utils.Math.TValues;
import Goom.Utils.Timer;
import Goom.VisualFx.FxHelper;
import Goom.VisualFx.VisualFxBase;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomPaths;
import Goom.Lib.GoomUtils;
import Goom.Lib.Point2d;
import Goom.PluginInfo;
import :LineDrawerManager;
import :LSysColors;
import :LSysDraw;
import :LSysGeom;
import :LSysPaths;

using GOOM::UTILS::OnOffTimer;
using GOOM::UTILS::Timer;
using GOOM::UTILS::MATH::IncrementedValue;
using GOOM::UTILS::MATH::NumberRange;
using GOOM::UTILS::MATH::TValue;

using DefaultParams = LSYS::Interpreter::DefaultParams;

template<>
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto lerp(const DefaultParams& val1,
          const DefaultParams& val2,
          const float t) noexcept -> DefaultParams
{
  return {
      std::lerp(val1.turnAngleInDegrees, val2.turnAngleInDegrees, t),
      std::lerp(val1.width, val2.width, t),
      std::lerp(val1.distance, val2.distance, t),
  };
}

template<>
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto Clamped(const DefaultParams& val,
             const DefaultParams& val1,
             const DefaultParams& val2) noexcept -> DefaultParams
{
  using GOOM::UTILS::MATH::UnorderedClamp;

  return {UnorderedClamp(val.turnAngleInDegrees, val1.turnAngleInDegrees, val2.turnAngleInDegrees),
          UnorderedClamp(val.width, val1.width, val2.width),
          UnorderedClamp(val.distance, val1.distance, val2.distance)};
}

template<>
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto GetMatching(const DefaultParams& val,
                 const DefaultParams& val1,
                 const DefaultParams& val2) noexcept -> float
{
  using GOOM::UTILS::MATH::SMALL_FLOAT;

  if (std::fabs(val2.turnAngleInDegrees - val1.turnAngleInDegrees) > SMALL_FLOAT)
  {
    return ((val.turnAngleInDegrees - val1.turnAngleInDegrees) /
            (val2.turnAngleInDegrees - val1.turnAngleInDegrees));
  }
  if (std::fabs(val2.width - val1.width) > SMALL_FLOAT)
  {
    return ((val.width - val1.width) / (val2.width - val1.width));
  }
  if (std::fabs(val2.distance - val1.distance) > SMALL_FLOAT)
  {
    return ((val.distance - val1.distance) / (val2.distance - val1.distance));
  }
  return 0.0F;
}

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

  LSystem(FxHelper& fxHelper,
          const std::string& lSystemDirectory,
          const LSystemFile& lSystemFile,
          PixelChannelType defaultAlpha) noexcept;

  auto ChangeColors() noexcept -> void;

  [[nodiscard]] auto GetPathStart() const noexcept -> const Point2dInt&;
  auto SetPathStart(const Point2dInt& pathStart) noexcept -> void;
  [[nodiscard]] auto GetPathTarget() const noexcept -> const Point2dInt&;
  auto SetPathTarget(const Point2dInt& pathTarget) noexcept -> void;

  auto Start() noexcept -> void;
  auto Update() noexcept -> void;
  auto DrawLSystem() noexcept -> void;

private:
  const FxHelper* m_fxHelper;
  LineDrawerManager m_lineDrawerManager;
  auto SwitchLineDrawers() -> void;

  static constexpr auto DEFAULT_BOUNDS_EXPAND_FACTOR = 2.0F;
  struct LSysModelSet
  {
    ::LSYS::Properties lSysProperties{};
    LSystemFile::Overrides lSysOverrides{};
    float lSystemXScale = 1.0F;
    float lSystemYScale = 1.0F;
    std::unique_ptr<::LSYS::LSysModel> lSysModel;
  };
  std::unique_ptr<::LSYS::List<::LSYS::Module>> m_lSysModuleList;
  LSysModelSet m_lSysModelSet;
  [[nodiscard]] static auto GetLSysModelSet(const PluginInfo& goomInfo,
                                            const std::string& lSysDirectory,
                                            const LSystemFile& lSystemFile) -> LSysModelSet;
  [[nodiscard]] static auto GetLSystemFilename(
      const std::string& lSystemDirectory, const LSystemFile& lSystemFile) noexcept -> std::string;
  [[nodiscard]] static auto GetBoundsFilename(
      const std::string& lSystemDirectory, const LSystemFile& lSystemFile) noexcept -> std::string;
  struct BoundingBox2d
  {
    GOOM::Point2dFlt min;
    GOOM::Point2dFlt max;
  };
  [[nodiscard]] static auto GetBoundingBox2d(
      float expandBounds, const ::LSYS::BoundingBox3d& boundingBox3d) noexcept -> BoundingBox2d;

  LSysColors m_lSysColors;
  LSysGeometry m_lSysGeometry;
  LSysDraw m_lSysDraw;

  LSysPath m_lSysPath{m_fxHelper->GetGoomRand()};
  static constexpr auto PATH_NUM_STEPS_RANGE = NumberRange{50U, 200U};

  ::LSYS::GraphicsGenerator::DrawFuncs m_drawFuncs = GetLSysDrawFuncs();
  [[nodiscard]] auto GetLSysDrawFuncs() noexcept -> ::LSYS::GraphicsGenerator::DrawFuncs;
  static constexpr const auto* GENERATOR_NAME = "On the Fly";
  std::unique_ptr<::LSYS::GraphicsGenerator> m_LSysGenerator =
      std::make_unique<::LSYS::GraphicsGenerator>(GENERATOR_NAME, m_drawFuncs);
  std::unique_ptr<::LSYS::Interpreter> m_lSysInterpreter;

  static constexpr auto BRIGHTNESS_ON_TIME         = 25U;
  static constexpr auto BRIGHTNESS_OFF_TIME        = 5U;
  static constexpr auto BRIGHTNESS_FAILED_ON_TIME  = 1U;
  static constexpr auto BRIGHTNESS_FAILED_OFF_TIME = 1U;
  static constexpr auto ON_BRIGHTNESS              = 2.0F;
  static constexpr auto OFF_BRIGHTNESS             = 1.0F;
  OnOffTimer m_brightnessOnOffTimer{
      m_fxHelper->GetGoomTime(),
      {.numOnCount               = BRIGHTNESS_ON_TIME,
                       .numOnCountAfterFailedOff = BRIGHTNESS_FAILED_ON_TIME,
                       .numOffCount              = BRIGHTNESS_OFF_TIME,
                       .numOffCountAfterFailedOn = BRIGHTNESS_FAILED_OFF_TIME}
  };
  auto StartBrightnessTimer() noexcept -> void;

  static constexpr auto Y_SCALE_ADJUST_STEPS_RANGE = NumberRange{50U, 200U};
  static constexpr auto VERTICAL_MOVE_STEPS_RANGE  = NumberRange{50U, 150U};

  uint32_t m_maxGen                              = 1U;
  static constexpr auto TIME_TO_KEEP_INTERPRETER = 500U;
  Timer m_timeForThisLSysInterpreter{m_fxHelper->GetGoomTime(), TIME_TO_KEEP_INTERPRETER};
  auto UpdateLSysModel() noexcept -> void;
  auto InitNextLSysInterpreter() -> void;
  static constexpr auto DEFAULT_NUM_INTERPRETER_PARAMS_STEPS = 100U;
  IncrementedValue<DefaultParams> m_defaultInterpreterParams{TValue::StepType::SINGLE_CYCLE,
                                                             DEFAULT_NUM_INTERPRETER_PARAMS_STEPS};
  auto UpdateInterpreterParams() noexcept -> void;
  auto SetNewDefaultInterpreterParams() noexcept -> void;
  [[nodiscard]] auto GetRandomDefaultInterpreterParams() const noexcept -> DefaultParams;
  auto ResetModelNamedArgs() -> void;
  auto RegenerateModuleList() noexcept -> void;
  auto RestartLSysInterpreter() noexcept -> void;
  auto ResetLSysParams() noexcept -> void;

  auto DrawLSystemBatch() noexcept -> void;
  auto IncrementTs() noexcept -> void;
};

} // namespace GOOM::VISUAL_FX::L_SYSTEM

namespace GOOM::VISUAL_FX::L_SYSTEM
{

inline auto LSystem::ChangeColors() noexcept -> void
{
  m_lSysColors.ChangeColors();
  SwitchLineDrawers();
}

inline auto LSystem::SwitchLineDrawers() -> void
{
  if (m_fxHelper->GetGoomRand().ProbabilityOf(m_lSysModelSet.lSysOverrides.probabilityOfNoise))
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

using ::LSYS::BoundingBox3d;
using ::LSYS::GetBoundingBox3d;
using ::LSYS::GetFinalProperties;
using ::LSYS::GraphicsGenerator;
using ::LSYS::Interpreter;
using ::LSYS::List;
using ::LSYS::Module;
using ::LSYS::SetParserDebug;
using ::LSYS::Value;

LSystem::LSystem(FxHelper& fxHelper,
                 const std::string& lSystemDirectory,
                 const LSystemFile& lSystemFile,
                 const PixelChannelType defaultAlpha) noexcept
  : m_fxHelper{&fxHelper},
    m_lineDrawerManager{fxHelper.GetDraw(), fxHelper.GetGoomRand()},
    m_lSysModelSet{GetLSysModelSet(fxHelper.GetGoomInfo(), lSystemDirectory, lSystemFile)},
    m_lSysColors{fxHelper.GetGoomRand(), defaultAlpha},
    m_lSysGeometry{fxHelper.GetGoomRand(),
                   m_lSysModelSet.lSystemXScale * lSystemFile.overrides.xScale,
                   m_lSysModelSet.lSystemYScale * lSystemFile.overrides.yScale},
    m_lSysDraw{m_lSysGeometry, m_lSysColors, lSystemFile.overrides.lineWidthFactor}
{
}

auto LSystem::GetLSysDrawFuncs() noexcept -> GraphicsGenerator::DrawFuncs
{
  const auto drawLine = [this](const ::LSYS::Vector& point1,
                               const ::LSYS::Vector& point2,
                               const uint32_t lSysColor,
                               const float lineWidth)
  {
    m_lSysDraw.DrawLine(lSysColor, point1, point2, lineWidth);
    m_lineDrawerManager.Update();
  };
  const auto drawPolygon = [this](const std::vector<::LSYS::Vector>& polygon,
                                  const uint32_t lSysColor,
                                  const float lineWidth)
  {
    m_lSysDraw.DrawPolygon(lSysColor, polygon, lineWidth);
    m_lineDrawerManager.Update();
  };

  return {drawLine, drawPolygon};
}

auto LSystem::Start() noexcept -> void
{
  SetParserDebug(false);

  InitNextLSysInterpreter();
  RestartLSysInterpreter();

  m_lSysPath.Init();

  StartBrightnessTimer();
}

auto LSystem::StartBrightnessTimer() noexcept -> void
{
  m_brightnessOnOffTimer.Reset();

  m_brightnessOnOffTimer.SetActions({.onAction =
                                         [this]()
                                     {
                                       m_lSysColors.SetGlobalBrightness(ON_BRIGHTNESS);
                                       return true;
                                     },
                                     .offAction =
                                         [this]()
                                     {
                                       m_lSysColors.SetGlobalBrightness(OFF_BRIGHTNESS);
                                       return true;
                                     }});

  m_brightnessOnOffTimer.StartOnTimer();
}

auto LSystem::Update() noexcept -> void
{
  m_lSysGeometry.SetTranslateAdjust(ToVec2dFlt(m_lSysPath.GetNextPathPosition()));

  IncrementTs();

  m_lSysGeometry.UpdateCurrentTransformArray();
}

inline auto LSystem::IncrementTs() noexcept -> void
{
  m_lSysGeometry.IncrementTs();
  m_defaultInterpreterParams.Increment();
  m_lSysPath.IncrementPositionT();
  m_brightnessOnOffTimer.Update();
}

auto LSystem::GetLSysModelSet(const PluginInfo& goomInfo,
                              const std::string& lSysDirectory,
                              const LSystemFile& lSystemFile) -> LSysModelSet
{
  auto lSysModelSet = LSysModelSet{};

  lSysModelSet.lSysOverrides                = lSystemFile.overrides;
  lSysModelSet.lSysProperties.inputFilename = GetLSystemFilename(lSysDirectory, lSystemFile);
  //LogInfo("L-System file = \"{}\".", lSystemFile.filename);

  const auto boundingBox3d = GetBoundingBox3d(GetBoundsFilename(lSysDirectory, lSystemFile));
  const auto boundingBox2d =
      GetBoundingBox2d(lSysModelSet.lSysOverrides.expandBounds, boundingBox3d);
  //LogInfo("boundingBox2d = ({:.2f}, {:.2f}), ({:.2f}, {:.2f})",
  //        boundingBox2d.min.x,
  //        boundingBox2d.min.y,
  //        boundingBox2d.max.x,
  //        boundingBox2d.max.y);

  lSysModelSet.lSystemXScale =
      goomInfo.GetDimensions().GetFltWidth() / (boundingBox2d.max.x - boundingBox2d.min.x);
  // TODO(glk) How to handle scale issues
  lSysModelSet.lSystemYScale =
      goomInfo.GetDimensions().GetFltHeight() / (boundingBox2d.max.y - boundingBox2d.min.y);

  lSysModelSet.lSysModel = GetParsedModel(lSysModelSet.lSysProperties);
  lSysModelSet.lSysProperties =
      GetFinalProperties(lSysModelSet.lSysModel->GetSymbolTable(), lSysModelSet.lSysProperties);
  //LogInfo("L-System properties.maxGen = {}.", lSysModelSet.lSysProperties.maxGen);
  //LogInfo("L-System properties.lineWidth = {}.", lSysModelSet.lSysProperties.lineWidth);
  //LogInfo("L-System properties.lineDistance = {}.", lSysModelSet.lSysProperties.lineDistance);
  //LogInfo("L-System properties.turnAngle = {}.", lSysModelSet.lSysProperties.turnAngle);

  return lSysModelSet;
}

auto LSystem::UpdateLSysModel() noexcept -> void
{
  if (not m_lSysInterpreter->AllDone())
  {
    return;
  }

  //LogInfo("Update L-System model.");

  if (static constexpr auto PROB_NEW_LSYS_INTERPRETER = 0.5F;
      m_timeForThisLSysInterpreter.Finished() and
      m_fxHelper->GetGoomRand().ProbabilityOf<PROB_NEW_LSYS_INTERPRETER>())
  {
    InitNextLSysInterpreter();
  }
  else
  {
    ResetLSysParams();
  }

  RestartLSysInterpreter();
}

auto LSystem::InitNextLSysInterpreter() -> void
{
  //LogInfo("Init next interpreter.");

  m_lSysModuleList = nullptr;

  const auto numLSysCopies = m_fxHelper->GetGoomRand().GetRandInRange(
      NumberRange{m_lSysModelSet.lSysOverrides.minNumLSysCopies,
                  m_lSysModelSet.lSysOverrides.maxNumLSysCopies});

  m_lSysDraw.SetNumLSysCopies(numLSysCopies);
  SwitchLineDrawers();

  m_lSysGeometry.SetNumLSysCopies(numLSysCopies);
  m_lSysGeometry.SetVerticalMoveMaxMin(m_lSysModelSet.lSysOverrides.verticalMoveMin,
                                       m_lSysModelSet.lSysOverrides.verticalMoveMax);
  m_lSysGeometry.SetVerticalMoveNumSteps(
      m_fxHelper->GetGoomRand().GetRandInRange<VERTICAL_MOVE_STEPS_RANGE>());
  m_lSysGeometry.SetYScaleNumSteps(
      m_fxHelper->GetGoomRand().GetRandInRange<Y_SCALE_ADJUST_STEPS_RANGE>());
  m_lSysGeometry.SetRotateDegreesAdjustNumSteps(m_fxHelper->GetGoomRand().GetRandInRange(
      NumberRange{m_lSysModelSet.lSysOverrides.minNumRotateDegreeSteps,
                  m_lSysModelSet.lSysOverrides.maxNumRotateDegreeSteps}));
  m_lSysGeometry.SetSpinDegreesAdjustNumSteps(m_fxHelper->GetGoomRand().GetRandInRange(
      NumberRange{m_lSysModelSet.lSysOverrides.minNumSpinDegreeSteps,
                  m_lSysModelSet.lSysOverrides.maxNumSpinDegreeSteps}));

  m_lSysPath.SetPathNumSteps(m_fxHelper->GetGoomRand().GetRandInRange<PATH_NUM_STEPS_RANGE>());

  m_lSysColors.SetProbabilityOfSimpleColors(m_lSysModelSet.lSysOverrides.probabilityOfSimpleColors);
  m_lSysColors.SetNumColors(numLSysCopies);
  m_lSysColors.ChangeColors();

  m_maxGen = m_fxHelper->GetGoomRand().GetRandInRange(
      NumberRange{m_lSysModelSet.lSysOverrides.minMaxGen, m_lSysModelSet.lSysOverrides.maxMaxGen});

  SetNewDefaultInterpreterParams();
  ResetModelNamedArgs();

  m_lSysInterpreter = std::make_unique<Interpreter>(*m_LSysGenerator);
  m_lSysInterpreter->SetDefaults(m_defaultInterpreterParams());

  m_timeForThisLSysInterpreter.ResetToZero();
}

inline auto LSystem::RegenerateModuleList() noexcept -> void
{
  //LogInfo("Regenerate module list.");

  m_lSysModuleList =
      std::make_unique<List<Module>>(*m_lSysModelSet.lSysModel->GetStartModuleList());

  for (auto gen = 0U; gen < m_maxGen; ++gen)
  {
    m_lSysModuleList = m_lSysModelSet.lSysModel->Generate(m_lSysModuleList.get());
  }

  Ensures(m_lSysModuleList != nullptr);

  //LogInfo("Regenerated module list - num = {}.", m_lSysModuleList->size());
}

inline auto LSystem::RestartLSysInterpreter() noexcept -> void
{
  //LogInfo("Restart interpreter.");
  RegenerateModuleList();
  m_lSysInterpreter->Start(*m_lSysModuleList);
}

inline auto LSystem::UpdateInterpreterParams() noexcept -> void
{
  if (not m_defaultInterpreterParams.GetT().IsStopped())
  {
    return;
  }

  if (static constexpr auto PROB_NEW_DEFAULT_PARAMS = 0.5F;
      m_fxHelper->GetGoomRand().ProbabilityOf<PROB_NEW_DEFAULT_PARAMS>())
  {
    m_defaultInterpreterParams.SetValues(m_defaultInterpreterParams(),
                                         GetRandomDefaultInterpreterParams());
    ResetModelNamedArgs();
  }
  else
  {
    m_defaultInterpreterParams.ReverseValues();
  }

  static constexpr auto RESET_START_T = 0.1F;
  m_defaultInterpreterParams.ResetT(RESET_START_T);
  m_defaultInterpreterParams.Increment();
}

inline auto LSystem::SetNewDefaultInterpreterParams() noexcept -> void
{
  const auto currentDefaultParams = GetRandomDefaultInterpreterParams();
  const auto nextDefaultParams    = GetRandomDefaultInterpreterParams();
  m_defaultInterpreterParams.SetValues(currentDefaultParams, nextDefaultParams);
  m_defaultInterpreterParams.ResetT();
}

inline auto LSystem::GetRandomDefaultInterpreterParams() const noexcept
    -> Interpreter::DefaultParams
{
  return {
      .turnAngleInDegrees = m_lSysModelSet.lSysProperties.turnAngle *
                            m_fxHelper->GetGoomRand().GetRandInRange(NumberRange{
                                m_lSysModelSet.lSysOverrides.minDefaultTurnAngleInDegreesFactor,
                                m_lSysModelSet.lSysOverrides.maxDefaultTurnAngleInDegreesFactor}),
      .width = m_lSysModelSet.lSysProperties.lineWidth *
               m_fxHelper->GetGoomRand().GetRandInRange(
                   NumberRange{m_lSysModelSet.lSysOverrides.minDefaultLineWidthFactor,
                               m_lSysModelSet.lSysOverrides.maxDefaultLineWidthFactor}),
      .distance = m_lSysModelSet.lSysProperties.lineDistance *
                  m_fxHelper->GetGoomRand().GetRandInRange(
                      NumberRange{m_lSysModelSet.lSysOverrides.minDefaultDistanceFactor,
                                  m_lSysModelSet.lSysOverrides.maxDefaultDistanceFactor}),
  };
}

inline auto LSystem::ResetLSysParams() noexcept -> void
{
  //LogInfo("Reset lsys params.");

  if (static constexpr auto PROB_CHANGE_ROTATE_DIRECTION = 0.00001F;
      m_fxHelper->GetGoomRand().ProbabilityOf<PROB_CHANGE_ROTATE_DIRECTION>())
  {
    m_lSysGeometry.ReverseRotateDirection();
  }
  if (static constexpr auto PROB_CHANGE_SPIN_DIRECTION = 0.00001F;
      m_fxHelper->GetGoomRand().ProbabilityOf<PROB_CHANGE_SPIN_DIRECTION>())
  {
    m_lSysGeometry.ReverseSpinDirection();
  }

  UpdateInterpreterParams();

  m_lSysInterpreter->SetDefaults(m_defaultInterpreterParams());
  //LogInfo("L-System default width = {}.", m_defaultInterpreterParams().width);
  //LogInfo("L-System default distance = {}.", m_defaultInterpreterParams().distance);
  //LogInfo("L-System default turnAngleInDegrees = {}.",
  //        m_defaultInterpreterParams().turnAngleInDegrees);
}

inline auto LSystem::GetLSystemFilename(const std::string& lSystemDirectory,
                                        const LSystemFile& lSystemFile) noexcept -> std::string
{
  return join_paths(lSystemDirectory, std::string{lSystemFile.filename} + ".in");
}

inline auto LSystem::GetBoundsFilename(const std::string& lSystemDirectory,
                                       const LSystemFile& lSystemFile) noexcept -> std::string
{
  return join_paths(lSystemDirectory, std::string{lSystemFile.filename} + ".bnds");
}

inline auto LSystem::ResetModelNamedArgs() -> void
{
  if (m_lSysModelSet.lSysOverrides.namedArgs.empty())
  {
    return;
  }

  for (const auto& namedArg : m_lSysModelSet.lSysOverrides.namedArgs)
  {
    m_lSysModelSet.lSysModel->ResetArgument(
        namedArg.name,
        Value{m_fxHelper->GetGoomRand().GetRandInRange(NumberRange{namedArg.min, namedArg.max})});
  }
}

auto LSystem::DrawLSystem() noexcept -> void
{
  //LogInfo("Start L-System interpreted draw. Num modules = {}.", m_lSysModuleList->size());

  DrawLSystemBatch();
  UpdateLSysModel();
}

inline auto LSystem::DrawLSystemBatch() noexcept -> void
{
  /**
  //static constexpr auto NUM = 100U;
  static auto n             = 1;
  static auto subtract      = false;
  static auto counter       = 0U;
  const auto numModules     = n;
  if (counter > 0)
  {
    ++counter;
    if (counter == 100U)
    {
      counter = 0U;
    }
  }
  else if (subtract)
  {
    n -= 10;
    if (n <= 1)
    {
      n = 1;
      subtract = false;
    }
  }
  else
  {
    n += 10;
    if (n >= static_cast<int32_t>(m_lSysModuleList->size()))
    {
      subtract = true;
      counter  = 1U;
    }
  }
  **/
  const auto numModules = static_cast<int32_t>(m_lSysModuleList->size());
  for (auto i = 0; i < numModules; ++i)
  {
    if (m_lSysInterpreter->AllDone())
    {
      break;
    }
    m_lSysInterpreter->InterpretNext();
  }
}

inline auto LSystem::GetBoundingBox2d(const float expandBounds,
                                      const BoundingBox3d& boundingBox3d) noexcept -> BoundingBox2d
{
  auto boundingBox2d =
      BoundingBox2d{.min = Scale(LSysDraw::GetPerspectivePoint(boundingBox3d.min), expandBounds),
                    .max = Scale(LSysDraw::GetPerspectivePoint(boundingBox3d.max), expandBounds)};
  return boundingBox2d;
}

} // namespace GOOM::VISUAL_FX::L_SYSTEM
