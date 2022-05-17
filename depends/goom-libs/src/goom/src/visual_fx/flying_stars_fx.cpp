#include "flying_stars_fx.h"

//#undef NO_LOGGING

#include "color/color_adjustment.h"
#include "color/colormaps.h"
#include "color/colorutils.h"
#include "color/random_colormaps.h"
#include "color/random_colormaps_manager.h"
#include "draw/goom_draw.h"
#include "fx_helper.h"
#include "goom/logging.h"
#include "goom/spimpl.h"
#include "goom_config.h"
#include "goom_plugin_info.h"
#include "point2d.h"
#include "utils/graphics/image_bitmaps.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <map>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace GOOM::VISUAL_FX
{

using COLOR::ColorAdjustment;
using COLOR::GetBrighterColor;
using COLOR::GetColorMultiply;
using COLOR::GetLightenedColor;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using COLOR::RandomColorMapsManager;
using COLOR::COLOR_DATA::ColorMapName;
using DRAW::IGoomDraw;
using STD20::pi;
using UTILS::GRAPHICS::ImageBitmap;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::SqDistance;
using UTILS::MATH::THIRD_PI;
using UTILS::MATH::TWO_PI;
using UTILS::MATH::U_HALF;
using UTILS::MATH::Weights;

static constexpr uint32_t MIN_STAR_AGE = 15;
static constexpr uint32_t MAX_STAR_EXTRA_AGE = 50;

enum class StarModes
{
  NO_FX = 0,
  FIREWORKS,
  RAIN,
  FOUNTAIN,
  _num // unused and must be last
};

struct Star
{
  Point2dFlt pos{};
  Vec2dFlt velocity{};
  Vec2dFlt acceleration{};
  float age = 0.0;
  float vage = 0.0;
  std::shared_ptr<const IColorMap> dominantMainColormap{};
  std::shared_ptr<const IColorMap> dominantLowColormap{};
  std::shared_ptr<const IColorMap> currentMainColorMap{};
  std::shared_ptr<const IColorMap> currentLowColorMap{};
};

class FlyingStarsFx::FlyingStarsImpl
{
public:
  FlyingStarsImpl(const FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps);

  void Start();

  void SetWeightedMainColorMaps(std::shared_ptr<RandomColorMaps> weightedMaps);
  void SetWeightedLowColorMaps(std::shared_ptr<RandomColorMaps> weightedMaps);

  void UpdateBuffers();

  void Finish();

private:
  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const IGoomRand& m_goomRand;
  const int32_t m_halfWidth;
  const int32_t m_halfHeight;
  const float m_xMax;

  RandomColorMapsManager m_colorMapsManager{};
  std::shared_ptr<RandomColorMaps> m_mainColorMaps{};
  std::shared_ptr<RandomColorMaps> m_lowColorMaps{};
  RandomColorMapsManager::ColorMapId m_dominantMainColorMapID;
  RandomColorMapsManager::ColorMapId m_dominantLowColorMapID;
  RandomColorMapsManager::ColorMapId m_mainColorMapID;
  RandomColorMapsManager::ColorMapId m_lowColorMapID;
  bool m_megaColorMode = false;
  [[nodiscard]] auto GetNextMainColorMapName() const -> ColorMapName;
  [[nodiscard]] auto GetNextLowColorMapName() const -> ColorMapName;
  [[nodiscard]] auto GetNextAngleColorMapName() const -> ColorMapName;

  static constexpr float GAMMA = 1.0F / 1.0F;
  const ColorAdjustment m_colorAdjust{GAMMA, ColorAdjustment::INCREASED_CHROMA_FACTOR};
  [[nodiscard]] auto GetColorCorrection(float brightness, const Pixel& color) const -> Pixel;

  static constexpr float MIN_SATURATION = 0.5F;
  static constexpr float MAX_SATURATION = 1.0F;
  static constexpr float MIN_LIGHTNESS = 0.5F;
  static constexpr float MAX_LIGHTNESS = 1.0F;

  ColorMode m_colorMode = ColorMode::MIX_COLORS;
  const Weights<ColorMode> m_colorModeWeights;
  void ChangeColorMode();
  [[nodiscard]] auto GetMixedColors(const Star& star, float t, float brightness)
      -> std::pair<Pixel, Pixel>;

  struct StarColorSet
  {
    Pixel mainColor;
    Pixel lowColor;
    Pixel dominantMainColor;
    Pixel dominantLowColor;
  };
  [[nodiscard]] auto GetMixColors(const Star& star, float t) -> StarColorSet;
  [[nodiscard]] auto GetReversedMixColors(const Star& star, float t) -> StarColorSet;
  [[nodiscard]] auto GetSineMixColors(const Star& star) -> StarColorSet;
  [[nodiscard]] auto GetSimilarLowColors(const Star& star, float t) -> StarColorSet;
  [[nodiscard]] auto GetFinalMixedColors(const StarColorSet& starColorSet,
                                         float t,
                                         float brightness) -> std::pair<Pixel, Pixel>;

  uint32_t m_counter = 0;
  static constexpr uint32_t MAX_COUNT = 100;

  StarModes m_fxMode = StarModes::FIREWORKS;
  static constexpr uint32_t MAX_NUM_STARS = 1024;
  static constexpr uint32_t MIN_NUM_STARS = 100;
  uint32_t m_maxStars = MAX_NUM_STARS;
  std::vector<Star> m_stars{};
  static constexpr float OLD_AGE = 0.95F;
  static constexpr uint32_t DEFAULT_MAX_STAR_AGE = 15;
  uint32_t m_maxStarAge = DEFAULT_MAX_STAR_AGE;

  static constexpr float MIN_MIN_SIDE_WIND = -0.10F;
  static constexpr float MAX_MIN_SIDE_WIND = -0.01F;
  static constexpr float MIN_MAX_SIDE_WIND = +0.01F;
  static constexpr float MAX_MAX_SIDE_WIND = +0.10F;
  static constexpr float DEFAULT_MIN_SIDE_WIND = 0.0F;
  float m_minSideWind = DEFAULT_MIN_SIDE_WIND;
  static constexpr float DEFAULT_MAX_SIDE_WIND = 0.00001F;
  float m_maxSideWind = DEFAULT_MAX_SIDE_WIND;

  static constexpr float MIN_MIN_GRAVITY = +0.005F;
  static constexpr float MAX_MIN_GRAVITY = +0.010F;
  static constexpr float MIN_MAX_GRAVITY = +0.050F;
  static constexpr float MAX_MAX_GRAVITY = +0.090F;
  float m_minGravity = MAX_MIN_GRAVITY;
  float m_maxGravity = MAX_MAX_GRAVITY;

  // For fireworks' largest bombs.
  static constexpr float MIN_AGE = 1.0F - (99.0F / 100.0F);
  // For fireworks' smallest bombs.
  static constexpr float MAX_AGE = 1.0F - (80.0F / 100.0F);

  bool m_useSingleBufferOnly = true;

  const Weights<StarModes> m_starModes;
  void CheckForStarEvents();
  void SoundEventOccurred();
  void UpdateWindAndGravity();
  void ChangeColorMaps();
  void UpdateStarColorMaps(float angle, Star& star);
  static constexpr size_t NUM_SEGMENTS = 20;
  std::array<ColorMapName, NUM_SEGMENTS> m_angleColorMapName{};
  void UpdateAngleColorMapNames();
  static auto GetSegmentNum(float angle) -> size_t;
  [[nodiscard]] auto GetDominantMainColorMapPtr(float angle) const
      -> std::shared_ptr<const IColorMap>;
  [[nodiscard]] auto GetDominantLowColorMapPtr(float angle) const
      -> std::shared_ptr<const IColorMap>;
  [[nodiscard]] auto GetCurrentMainColorMapPtr(float angle) const
      -> std::shared_ptr<const IColorMap>;
  [[nodiscard]] auto GetCurrentLowColorMapPtr(float angle) const
      -> std::shared_ptr<const IColorMap>;
  void DrawStars();
  static void UpdateStar(Star& star);
  [[nodiscard]] auto IsStarDead(const Star& star) const -> bool;
  enum class DrawMode
  {
    CIRCLES,
    LINES,
    DOTS,
    CIRCLES_AND_LINES,
    _num // unused and must be last
  };
  DrawMode m_drawMode = DrawMode::CIRCLES;
  const Weights<DrawMode> m_drawModeWeights;
  void ChangeDrawMode();
  const SmallImageBitmaps& m_smallBitmaps;
  [[nodiscard]] auto GetImageBitmap(size_t size) const -> const ImageBitmap&;
  using DrawFunc = std::function<void(
      Point2dInt point1, Point2dInt point2, uint32_t size, const std::vector<Pixel>& colors)>;
  const std::map<DrawMode, const DrawFunc> m_drawFuncs{};
  [[nodiscard]] auto GetDrawFunc() const -> DrawFunc;
  void DrawStar(const Star& star, float flipSpeed, const DrawFunc& drawFunc);
  void DrawParticleCircle(Point2dInt point1,
                          Point2dInt point2,
                          uint32_t size,
                          const std::vector<Pixel>& colors);
  void DrawParticleLine(Point2dInt point1,
                        Point2dInt point2,
                        uint32_t size,
                        const std::vector<Pixel>& colors);
  void DrawParticleDot(Point2dInt point1,
                       Point2dInt point2,
                       uint32_t size,
                       const std::vector<Pixel>& colors);
  void RemoveDeadStars();

  struct StarModeParams
  {
    Point2dInt pos{};
    float gravityFactor = 1.0F;
    float windFactor = 1.0F;
    float vage = 0.0;
    float radius = 1.0F;
  };
  [[nodiscard]] auto GetStarParams(float defaultRadius, float heightRatio) -> StarModeParams;
  [[nodiscard]] auto GetFireworksStarParams(float defaultRadius) const -> StarModeParams;
  [[nodiscard]] auto GetRainStarParams(float defaultRadius) const -> StarModeParams;
  [[nodiscard]] auto GetFountainStarParams(float defaultRadius) const -> StarModeParams;
  void AddStarBombs(const StarModeParams& starModeParams, size_t maxStarsInBomb);
  [[nodiscard]] auto GetMaxStarsInABomb(float heightRatio) const -> size_t;
  void AddABomb(const Point2dInt& pos, float radius, float vage, float gravity, float sideWind);
  [[nodiscard]] auto GetBombAngle(const Star& star) const -> float;
  [[nodiscard]] auto GetRainBombAngle(const Star& star) const -> float;
  [[nodiscard]] auto GetFireworksBombAngle() const -> float;
  [[nodiscard]] auto GetFountainBombAngle(const Star& star) const -> float;

  static constexpr size_t MIN_DOT_SIZE = 3;
  static constexpr size_t MAX_DOT_SIZE = 5;
  static_assert(MAX_DOT_SIZE <= SmallImageBitmaps::MAX_IMAGE_SIZE, "Max dot size mismatch.");
};

FlyingStarsFx::FlyingStarsFx(const FxHelper& fxHelper,
                             const SmallImageBitmaps& smallBitmaps) noexcept
  : m_fxImpl{spimpl::make_unique_impl<FlyingStarsImpl>(fxHelper, smallBitmaps)}
{
}

void FlyingStarsFx::SetWeightedMainColorMaps(const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_fxImpl->SetWeightedMainColorMaps(weightedMaps);
}

void FlyingStarsFx::SetWeightedLowColorMaps(const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_fxImpl->SetWeightedLowColorMaps(weightedMaps);
}

void FlyingStarsFx::Start()
{
  m_fxImpl->Start();
}

void FlyingStarsFx::Resume()
{
  // nothing to be done
}

void FlyingStarsFx::Suspend()
{
  // nothing to be done
}

void FlyingStarsFx::Finish()
{
  m_fxImpl->Finish();
}

auto FlyingStarsFx::GetFxName() const -> std::string
{
  return "Flying Stars FX";
}

void FlyingStarsFx::ApplyMultiple()
{
  m_fxImpl->UpdateBuffers();
}

// clang-format off
static constexpr float COLOR_MODE_MIX_COLORS_WEIGHT         = 30.0F;
static constexpr float COLOR_MODE_REVERSE_MIX_COLORS_WEIGHT = 15.0F;
static constexpr float COLOR_MODE_SIMILAR_LOW_COLORS_WEIGHT = 10.0F;
static constexpr float COLOR_MODE_SINE_MIX_COLORS_WEIGHT    =  5.0F;

static constexpr float STAR_MODES_NO_FX_WEIGHT     =  11.0F;
static constexpr float STAR_MODES_FIREWORKS_WEIGHT =  10.0F;
static constexpr float STAR_MODES_FOUNTAIN_WEIGHT  =   7.0F;
static constexpr float STAR_MODES_RAIN_WEIGHT      =   7.0F;

static constexpr float DRAW_MODE_DOTS_WEIGHT              = 30.0F;
static constexpr float DRAW_MODE_CIRCLES_WEIGHT           = 20.0F;
static constexpr float DRAW_MODE_LINES_WEIGHT             = 10.0F;
static constexpr float DRAW_MODE_CIRCLES_AND_LINES_WEIGHT = 15.0F;
// clang-format on

FlyingStarsFx::FlyingStarsImpl::FlyingStarsImpl(const FxHelper& fxHelper,
                                                const SmallImageBitmaps& smallBitmaps)
  : m_draw{fxHelper.GetDraw()},
    m_goomInfo{fxHelper.GetGoomInfo()},
    m_goomRand{fxHelper.GetGoomRand()},
    m_halfWidth{static_cast<int32_t>(U_HALF * m_goomInfo.GetScreenInfo().width)},
    m_halfHeight{static_cast<int32_t>(U_HALF * m_goomInfo.GetScreenInfo().height)},
    m_xMax{static_cast<float>(m_goomInfo.GetScreenInfo().width - 1)},
    m_dominantMainColorMapID{m_colorMapsManager.AddDefaultColorMapInfo(m_goomRand)},
    m_dominantLowColorMapID{m_colorMapsManager.AddDefaultColorMapInfo(m_goomRand)},
    m_mainColorMapID{m_colorMapsManager.AddDefaultColorMapInfo(m_goomRand)},
    m_lowColorMapID{m_colorMapsManager.AddDefaultColorMapInfo(m_goomRand)},
    // clang-format off
    m_colorModeWeights{
        m_goomRand,
        {
            { ColorMode::MIX_COLORS,         COLOR_MODE_MIX_COLORS_WEIGHT },
            { ColorMode::REVERSE_MIX_COLORS, COLOR_MODE_REVERSE_MIX_COLORS_WEIGHT },
            { ColorMode::SIMILAR_LOW_COLORS, COLOR_MODE_SIMILAR_LOW_COLORS_WEIGHT },
            { ColorMode::SINE_MIX_COLORS,    COLOR_MODE_SINE_MIX_COLORS_WEIGHT },
        }
    },
    m_starModes{
        m_goomRand,
        {
            {StarModes::NO_FX,     STAR_MODES_NO_FX_WEIGHT},
            {StarModes::FIREWORKS, STAR_MODES_FIREWORKS_WEIGHT},
            {StarModes::FOUNTAIN,  STAR_MODES_FOUNTAIN_WEIGHT},
            {StarModes::RAIN,      STAR_MODES_RAIN_WEIGHT},
        }
    },
    m_drawModeWeights{
        m_goomRand,
        {
            { DrawMode::DOTS,              DRAW_MODE_DOTS_WEIGHT },
            { DrawMode::CIRCLES,           DRAW_MODE_CIRCLES_WEIGHT },
            { DrawMode::LINES,             DRAW_MODE_LINES_WEIGHT },
            { DrawMode::CIRCLES_AND_LINES, DRAW_MODE_CIRCLES_AND_LINES_WEIGHT },
        }
    },
    // clang-format on
    m_smallBitmaps{smallBitmaps},
    m_drawFuncs{
        {DrawMode::CIRCLES,
         [this](const Point2dInt point1,
             const Point2dInt point2,
             const uint32_t size,
             const std::vector<Pixel>& colors) {
           DrawParticleCircle(point1, point2, size, colors);
         }},
        {DrawMode::LINES,
         [this](const Point2dInt point1,
             const Point2dInt point2,
             const uint32_t size,
             const std::vector<Pixel>& colors) { DrawParticleLine(point1, point2, size, colors); }},
        {
            DrawMode::DOTS,
            [this](const Point2dInt point1,
                const Point2dInt point2,
                const uint32_t size,
                const std::vector<Pixel>& colors) {
              DrawParticleDot(point1, point2, size, colors);
            },
        }}
{
  m_stars.reserve(MAX_NUM_STARS);
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetImageBitmap(const size_t size) const
    -> const ImageBitmap&
{
  return m_smallBitmaps.GetImageBitmap(SmallImageBitmaps::ImageNames::CIRCLE,
                                       std::clamp(size, MIN_DOT_SIZE, MAX_DOT_SIZE));
}

inline void FlyingStarsFx::FlyingStarsImpl::SetWeightedMainColorMaps(
    const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_mainColorMaps = weightedMaps;

  m_colorMapsManager.UpdateColorMapInfo(
      m_dominantMainColorMapID,
      {m_mainColorMaps, ColorMapName::_NULL, RandomColorMaps::ALL_COLOR_MAP_TYPES});
  m_colorMapsManager.UpdateColorMapInfo(m_mainColorMapID, {m_mainColorMaps, ColorMapName::_NULL,
                                                           RandomColorMaps::ALL_COLOR_MAP_TYPES});

  m_mainColorMaps->SetSaturationLimits(MIN_SATURATION, MAX_SATURATION);
  m_mainColorMaps->SetLightnessLimits(MIN_LIGHTNESS, MAX_LIGHTNESS);
}

inline void FlyingStarsFx::FlyingStarsImpl::SetWeightedLowColorMaps(
    const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_lowColorMaps = weightedMaps;

  m_colorMapsManager.UpdateColorMapInfo(
      m_dominantLowColorMapID,
      {m_lowColorMaps, ColorMapName::_NULL, RandomColorMaps::ALL_COLOR_MAP_TYPES});
  m_colorMapsManager.UpdateColorMapInfo(
      m_lowColorMapID, {m_lowColorMaps, ColorMapName::_NULL, RandomColorMaps::ALL_COLOR_MAP_TYPES});

  m_lowColorMaps->SetSaturationLimits(MIN_SATURATION, MAX_SATURATION);
  m_lowColorMaps->SetLightnessLimits(MIN_LIGHTNESS, MAX_LIGHTNESS);
}

inline void FlyingStarsFx::FlyingStarsImpl::Start()
{
  // nothing to do
}

void FlyingStarsFx::FlyingStarsImpl::Finish()
{
  // nothing to do
}

inline void FlyingStarsFx::FlyingStarsImpl::UpdateBuffers()
{
  ++m_counter;

  m_maxStars = m_goomRand.GetRandInRange(MIN_NUM_STARS, MAX_NUM_STARS);

  CheckForStarEvents();
  DrawStars();
  RemoveDeadStars();
}

void FlyingStarsFx::FlyingStarsImpl::CheckForStarEvents()
{
  if (m_stars.empty() || (m_goomInfo.GetSoundInfo().GetTimeSinceLastGoom() < 1))
  {
    SoundEventOccurred();
    static constexpr float PROB_NEW_STAR_MODE = 1.0F / 20.0F;
    if (m_goomRand.ProbabilityOf(PROB_NEW_STAR_MODE))
    {
      m_fxMode = m_starModes.GetRandomWeighted();
      ChangeColorMode();
      ChangeDrawMode();
    }
    else if (m_counter > MAX_COUNT)
    {
      m_counter = 0;
      ChangeColorMode();
      ChangeDrawMode();
    }
  }
  // m_fxMode = StarModes::rain;
}

void FlyingStarsFx::FlyingStarsImpl::DrawStars()
{
  const float flipSpeed = m_goomRand.GetRandInRange(0.1F, 10.0F);

  for (auto& star : m_stars)
  {
    UpdateStar(star);

    // Is it a dead particle?
    if (star.age >= static_cast<float>(m_maxStarAge))
    {
      continue;
    }

    DrawStar(star, flipSpeed, GetDrawFunc());
  }
}

inline void FlyingStarsFx::FlyingStarsImpl::ChangeDrawMode()
{
  m_drawMode = m_drawModeWeights.GetRandomWeighted();
}


inline auto FlyingStarsFx::FlyingStarsImpl::GetDrawFunc() const -> DrawFunc
{
  if (m_drawMode != DrawMode::CIRCLES_AND_LINES)
  {
    return m_drawFuncs.at(m_drawMode);
  }
  static constexpr float PROB_CIRCLES = 0.5F;
  return m_goomRand.ProbabilityOf(PROB_CIRCLES) ? m_drawFuncs.at(DrawMode::CIRCLES)
                                                : m_drawFuncs.at(DrawMode::LINES);
}

void FlyingStarsFx::FlyingStarsImpl::DrawStar(const Star& star,
                                              const float flipSpeed,
                                              const DrawFunc& drawFunc)
{
  const float tAge = star.age / static_cast<float>(m_maxStarAge);
  const float ageBrightness = 0.2F + ((0.8F * std::fabs(0.10F - tAge)) / 0.25F);
  const size_t numParts =
      tAge > OLD_AGE ? 4 : (2 + static_cast<size_t>(std::lround((1.0F - tAge) * 2.0F)));

  const Point2dInt point0 = star.pos.ToInt();

  Point2dInt point1 = point0;
  for (size_t j = 1; j <= numParts; ++j)
  {
    const Point2dInt point2 = {
        point0.x -
            static_cast<int32_t>(
                0.5F * (1.0F + std::sin(flipSpeed * star.velocity.x * static_cast<float>(j))) *
                star.velocity.x * static_cast<float>(j)),
        point0.y -
            static_cast<int32_t>(
                0.5F * (1.0F + std::cos(flipSpeed * star.velocity.y * static_cast<float>(j))) *
                star.velocity.y * static_cast<float>(j))};

    const float brightness =
        (3.7F * ageBrightness * static_cast<float>(j)) / static_cast<float>(numParts);
    const auto mixedColors = GetMixedColors(star, tAge, brightness);
    const std::vector<Pixel> colors = {mixedColors.first, mixedColors.second};
    const uint32_t size = tAge < OLD_AGE ? 1 : m_goomRand.GetRandInRange(2U, MAX_DOT_SIZE + 1);

    drawFunc(point1, point2, size, colors);

    point1 = point2;
  }
}

void FlyingStarsFx::FlyingStarsImpl::DrawParticleCircle(const Point2dInt point1,
                                                        [[maybe_unused]] const Point2dInt point2,
                                                        const uint32_t size,
                                                        const std::vector<Pixel>& colors)
{
  if (m_useSingleBufferOnly)
  {
    m_draw.Circle(point1, static_cast<int>(size), colors[0]);
  }
  else
  {
    m_draw.Circle(point1, static_cast<int>(size), colors);
  }
}

void FlyingStarsFx::FlyingStarsImpl::DrawParticleLine(const Point2dInt point1,
                                                      const Point2dInt point2,
                                                      const uint32_t size,
                                                      const std::vector<Pixel>& colors)
{
  if (m_useSingleBufferOnly)
  {
    m_draw.Line(point1, point2, colors[0], static_cast<uint8_t>(size));
  }
  else
  {
    m_draw.Line(point1, point2, colors, static_cast<uint8_t>(size));
  }
}

void FlyingStarsFx::FlyingStarsImpl::DrawParticleDot(const Point2dInt point1,
                                                     [[maybe_unused]] const Point2dInt point2,
                                                     const uint32_t size,
                                                     const std::vector<Pixel>& colors)
{
  const auto getMainColor =
      [&colors]([[maybe_unused]] const size_t x, [[maybe_unused]] const size_t y, const Pixel& bgnd)
  { return GetColorMultiply(bgnd, colors[0]); };
  const auto getLowColor =
      [&colors]([[maybe_unused]] const size_t x, [[maybe_unused]] const size_t y, const Pixel& bgnd)
  { return GetColorMultiply(bgnd, colors[1]); };

  const ImageBitmap& bitmap = GetImageBitmap(size);

  if (m_useSingleBufferOnly)
  {
    m_draw.Bitmap(point1, bitmap, getMainColor);
  }
  else
  {
    const std::vector<IGoomDraw::GetBitmapColorFunc> getColors{getMainColor, getLowColor};
    m_draw.Bitmap(point1, bitmap, getColors);
  }
}

inline void FlyingStarsFx::FlyingStarsImpl::RemoveDeadStars()
{
  const auto isDead = [&](const Star& star) { return IsStarDead(star); };
#if __cplusplus <= 201703L
  m_stars.erase(std::remove_if(m_stars.begin(), m_stars.end(), isDead), m_stars.end());
#else
  const size_t numRemoved = std::erase_if(m_stars, isDead);
#endif
}

inline auto FlyingStarsFx::FlyingStarsImpl::IsStarDead(const Star& star) const -> bool
{
  static constexpr int32_t DEAD_MARGIN = 64;

  if ((star.pos.x < -DEAD_MARGIN) ||
      (star.pos.x > static_cast<float>(m_goomInfo.GetScreenInfo().width + DEAD_MARGIN)))
  {
    return true;
  }
  if ((star.pos.y < -DEAD_MARGIN) ||
      (star.pos.y > static_cast<float>(m_goomInfo.GetScreenInfo().height + DEAD_MARGIN)))
  {
    return true;
  }

  return star.age >= static_cast<float>(this->m_maxStarAge);
}

inline void FlyingStarsFx::FlyingStarsImpl::ChangeColorMaps()
{
  m_colorMapsManager.ChangeColorMapNow(m_dominantMainColorMapID);
  m_colorMapsManager.ChangeColorMapNow(m_dominantLowColorMapID);

  static constexpr float PROB_MEGA_COLOR_MODE = 1.0F / 10.0F;
  m_megaColorMode = m_goomRand.ProbabilityOf(PROB_MEGA_COLOR_MODE);

  m_colorMapsManager.UpdateColorMapName(m_mainColorMapID, GetNextMainColorMapName());
  m_colorMapsManager.UpdateColorMapName(m_lowColorMapID, GetNextLowColorMapName());

  UpdateAngleColorMapNames();
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetNextMainColorMapName() const -> ColorMapName
{
  return m_megaColorMode
             ? ColorMapName::_NULL
             : m_mainColorMaps->GetRandomColorMapName(m_mainColorMaps->GetRandomGroup());
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetNextLowColorMapName() const -> ColorMapName
{
  return m_megaColorMode ? ColorMapName::_NULL
                         : m_lowColorMaps->GetRandomColorMapName(m_lowColorMaps->GetRandomGroup());
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetNextAngleColorMapName() const -> ColorMapName
{
  return m_megaColorMode
             ? m_mainColorMaps->GetRandomColorMapName()
             : m_mainColorMaps->GetRandomColorMapName(m_mainColorMaps->GetRandomGroup());
}

void FlyingStarsFx::FlyingStarsImpl::UpdateStarColorMaps(const float angle, Star& star)
{
  // TODO Get colormap based on current mode.
  if (constexpr float PROB_RANDOM_COLOR_MAPS = 0.5F;
      m_goomRand.ProbabilityOf(PROB_RANDOM_COLOR_MAPS))
  {
    star.dominantMainColormap = m_colorMapsManager.GetColorMapPtr(m_dominantMainColorMapID);
    star.dominantLowColormap = m_colorMapsManager.GetColorMapPtr(m_dominantLowColorMapID);
    star.currentMainColorMap = m_colorMapsManager.GetColorMapPtr(m_mainColorMapID);
    star.currentLowColorMap = m_colorMapsManager.GetColorMapPtr(m_lowColorMapID);
  }
  else
  {
    star.dominantMainColormap = GetDominantMainColorMapPtr(angle);
    star.dominantLowColormap = GetDominantLowColorMapPtr(angle);
    star.currentMainColorMap = GetCurrentMainColorMapPtr(angle);
    star.currentLowColorMap = GetCurrentLowColorMapPtr(angle);
  }

  assert(star.dominantMainColormap);
  assert(star.dominantLowColormap);
  assert(star.currentMainColorMap);
  assert(star.currentLowColorMap);
}

void FlyingStarsFx::FlyingStarsImpl::UpdateAngleColorMapNames()
{
  for (size_t i = 0; i < NUM_SEGMENTS; ++i)
  {
    m_angleColorMapName.at(i) = GetNextAngleColorMapName();
  }
}

auto FlyingStarsFx::FlyingStarsImpl::GetDominantMainColorMapPtr(const float angle) const
    -> std::shared_ptr<const IColorMap>
{
  return std::const_pointer_cast<const IColorMap>(
      m_mainColorMaps->GetColorMapPtr(m_angleColorMapName.at(GetSegmentNum(angle))));
}

auto FlyingStarsFx::FlyingStarsImpl::GetDominantLowColorMapPtr(const float angle) const
    -> std::shared_ptr<const IColorMap>
{
  return std::const_pointer_cast<const IColorMap>(
      m_mainColorMaps->GetColorMapPtr(m_angleColorMapName.at(GetSegmentNum(angle))));
}

auto FlyingStarsFx::FlyingStarsImpl::GetCurrentMainColorMapPtr(const float angle) const
    -> std::shared_ptr<const IColorMap>
{
  return std::const_pointer_cast<const IColorMap>(
      m_mainColorMaps->GetColorMapPtr(m_angleColorMapName.at(GetSegmentNum(angle))));
}

auto FlyingStarsFx::FlyingStarsImpl::GetCurrentLowColorMapPtr(const float angle) const
    -> std::shared_ptr<const IColorMap>
{
  return std::const_pointer_cast<const IColorMap>(
      m_mainColorMaps->GetColorMapPtr(m_angleColorMapName.at(GetSegmentNum(angle))));
}

auto FlyingStarsFx::FlyingStarsImpl::GetSegmentNum(const float angle) -> size_t
{
  const float segmentSize = TWO_PI / static_cast<float>(NUM_SEGMENTS);
  float nextAngle = segmentSize;
  for (size_t i = 0; i < NUM_SEGMENTS; ++i)
  {
    if (angle <= nextAngle)
    {
      return i;
    }
    nextAngle += segmentSize;
  }
  throw std::logic_error("Angle too large.");
}

void FlyingStarsFx::FlyingStarsImpl::ChangeColorMode()
{
  m_colorMode = m_colorModeWeights.GetRandomWeighted();
}

auto FlyingStarsFx::FlyingStarsImpl::GetMixedColors(const Star& star,
                                                    const float t,
                                                    const float brightness)
    -> std::pair<Pixel, Pixel>
{
  StarColorSet starColorSet;

  switch (m_colorMode)
  {
    case ColorMode::SINE_MIX_COLORS:
      starColorSet = GetSineMixColors(star);
      break;
    case ColorMode::MIX_COLORS:
      starColorSet = GetMixColors(star, t);
      break;
    case ColorMode::SIMILAR_LOW_COLORS:
      starColorSet = GetSimilarLowColors(star, t);
      break;
    case ColorMode::REVERSE_MIX_COLORS:
      starColorSet = GetReversedMixColors(star, t);
      break;
    default:
      throw std::logic_error("Unknown ColorMode enum.");
  }

  return GetFinalMixedColors(starColorSet, t, brightness);
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetMixColors(const Star& star, const float t)
    -> StarColorSet
{
  return {star.currentMainColorMap->GetColor(t), star.currentLowColorMap->GetColor(t),
          star.dominantMainColormap->GetColor(t), star.dominantLowColormap->GetColor(t)};
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetReversedMixColors(const Star& star, const float t)
    -> StarColorSet
{
  return GetMixColors(star, 1.0F - t);
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetSimilarLowColors(const Star& star, const float t)
    -> StarColorSet
{
  StarColorSet starColorSet = GetMixColors(star, t);
  starColorSet.dominantLowColor = starColorSet.dominantMainColor;
  return starColorSet;
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetSineMixColors(const Star& star) -> StarColorSet
{
  static constexpr float FREQ = 20.0F;
  static constexpr float T_MIX_FACTOR = 0.5F;
  static constexpr float Z_STEP = 0.1F;
  static float s_z = 0.0F;

  const float tSin = T_MIX_FACTOR * (1.0F + std::sin(FREQ * s_z));

  StarColorSet starColorSet;
  starColorSet.mainColor = star.currentMainColorMap->GetColor(tSin);
  starColorSet.lowColor = star.currentLowColorMap->GetColor(tSin);
  starColorSet.dominantMainColor = star.dominantMainColormap->GetColor(tSin);
  starColorSet.dominantLowColor = star.dominantLowColormap->GetColor(tSin);

  s_z += Z_STEP;

  return starColorSet;
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetFinalMixedColors(const StarColorSet& starColorSet,
                                                                const float t,
                                                                const float brightness)
    -> std::pair<Pixel, Pixel>
{
  static constexpr float MIN_MIX = 0.2F;
  static constexpr float MAX_MIX = 0.8F;
  const float tMix = STD20::lerp(MIN_MIX, MAX_MIX, t);
  const Pixel mixedMainColor =
      GetColorCorrection(brightness, IColorMap::GetColorMix(starColorSet.mainColor,
                                                            starColorSet.dominantMainColor, tMix));
  const Pixel mixedLowColor = GetLightenedColor(
      IColorMap::GetColorMix(starColorSet.lowColor, starColorSet.dominantLowColor, tMix), 10.0F);
  const Pixel remixedLowColor =
      m_colorMode == ColorMode::SIMILAR_LOW_COLORS
          ? mixedLowColor
          : GetColorCorrection(brightness,
                               IColorMap::GetColorMix(mixedMainColor, mixedLowColor, 0.4F));

  return {mixedMainColor, remixedLowColor};
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetColorCorrection(float brightness,
                                                               const Pixel& color) const -> Pixel
{
  if constexpr (1.0F == GAMMA)
  {
    return GetBrighterColor(brightness, color);
  }
  else
  {
    return m_colorAdjust.GetAdjustment(brightness, color);
  }
}

/**
 * Met a jour la position et vitesse d'une particule.
 */
inline void FlyingStarsFx::FlyingStarsImpl::UpdateStar(Star& star)
{
  star.pos.Translate(star.velocity);
  star.velocity += star.acceleration;
  star.age += star.vage;
}

/**
 * Ajoute de nouvelles particules au moment d'un evenement sonore.
 */
void FlyingStarsFx::FlyingStarsImpl::SoundEventOccurred()
{
  if (m_fxMode == StarModes::NO_FX)
  {
    return;
  }

  m_maxStarAge = MIN_STAR_AGE + m_goomRand.GetNRand(MAX_STAR_EXTRA_AGE);
  static constexpr float PROB_SINGLE_BUFFER_ONLY = 1.0F / 100.0F;
  m_useSingleBufferOnly = m_goomRand.ProbabilityOf(PROB_SINGLE_BUFFER_ONLY);

  UpdateWindAndGravity();
  ChangeColorMaps();

  // Why 200 ? Because the FX was developed on 320x200.
  static constexpr float WIDTH = 320.0F;
  static constexpr float HEIGHT = 200.0F;
  static constexpr float MIN_HEIGHT = 50.0F;
  const auto heightRatio = static_cast<float>(m_goomInfo.GetScreenInfo().height) / HEIGHT;
  const float defaultRadius = (1.0F + m_goomInfo.GetSoundInfo().GetGoomPower()) *
                              (m_goomRand.GetRandInRange(MIN_HEIGHT, HEIGHT) / WIDTH);

  const StarModeParams starParams = GetStarParams(defaultRadius, heightRatio);
  const size_t maxStarsInBomb = GetMaxStarsInABomb(heightRatio);

  AddStarBombs(starParams, maxStarsInBomb);
}

inline void FlyingStarsFx::FlyingStarsImpl::UpdateWindAndGravity()
{
  if (constexpr float PROB_NEW_WIND_AND_GRAVITY = 1.0F / 10.0F;
      !m_goomRand.ProbabilityOf(PROB_NEW_WIND_AND_GRAVITY))
  {
    return;
  }

  m_minSideWind = m_goomRand.GetRandInRange(MIN_MIN_SIDE_WIND, MAX_MIN_SIDE_WIND);
  m_maxSideWind = m_goomRand.GetRandInRange(MIN_MAX_SIDE_WIND, MAX_MAX_SIDE_WIND);
  m_minGravity = m_goomRand.GetRandInRange(MIN_MIN_GRAVITY, MAX_MIN_GRAVITY);
  m_maxGravity = m_goomRand.GetRandInRange(MIN_MAX_GRAVITY, MAX_MAX_GRAVITY);
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetMaxStarsInABomb(const float heightRatio) const
    -> size_t
{
  const auto maxStarsInBomb = static_cast<size_t>(
      heightRatio * (100.0F + ((m_goomInfo.GetSoundInfo().GetGoomPower() + 1.0F) *
                               m_goomRand.GetRandInRange(0.0F, 150.0F))));

  if (m_goomInfo.GetSoundInfo().GetTimeSinceLastBigGoom() < 1)
  {
    return 2 * maxStarsInBomb;
  }

  return maxStarsInBomb;
}

auto FlyingStarsFx::FlyingStarsImpl::GetStarParams(const float defaultRadius,
                                                   const float heightRatio) -> StarModeParams
{
  static constexpr float STAR_AGE_FACTOR = 2.0F / 3.0F;

  StarModeParams starParams;

  switch (m_fxMode)
  {
    case StarModes::FIREWORKS:
      starParams = GetFireworksStarParams(defaultRadius);
      break;
    case StarModes::RAIN:
      starParams = GetRainStarParams(defaultRadius);
      break;
    case StarModes::FOUNTAIN:
      m_maxStarAge = static_cast<uint32_t>(STAR_AGE_FACTOR * static_cast<float>(m_maxStarAge));
      starParams = GetFountainStarParams(defaultRadius);
      break;
    default:
      throw std::logic_error("Unknown StarModes enum.");
  }

  starParams.radius *= heightRatio;
  if (m_goomInfo.GetSoundInfo().GetTimeSinceLastBigGoom() < 1)
  {
    static constexpr float RADIUS_FACTOR = 1.5F;
    starParams.radius *= RADIUS_FACTOR;
  }

  return starParams;
}

auto FlyingStarsFx::FlyingStarsImpl::GetFireworksStarParams(const float defaultRadius) const
    -> StarModeParams
{
  StarModeParams starParams;

  const auto rsq = static_cast<float>(m_halfHeight * m_halfHeight);
  while (true)
  {
    starParams.pos.x = static_cast<int32_t>(m_goomRand.GetNRand(m_goomInfo.GetScreenInfo().width));
    starParams.pos.y = static_cast<int32_t>(m_goomRand.GetNRand(m_goomInfo.GetScreenInfo().height));
    const float sqDist = SqDistance(static_cast<float>(starParams.pos.x - m_halfWidth),
                                    static_cast<float>(starParams.pos.y - m_halfHeight));
    if (sqDist < rsq)
    {
      break;
    }
  }

  static constexpr float RADIUS_FACTOR = 1.0F;
  static constexpr float INITIAL_WIND_FACTOR = 0.1F;
  static constexpr float INITIAL_GRAVITY_FACTOR = 0.4F;
  starParams.radius = RADIUS_FACTOR * defaultRadius;
  starParams.vage = MAX_AGE * (1.0F - m_goomInfo.GetSoundInfo().GetGoomPower());
  starParams.windFactor = INITIAL_WIND_FACTOR;
  starParams.gravityFactor = INITIAL_GRAVITY_FACTOR;

  return starParams;
}

auto FlyingStarsFx::FlyingStarsImpl::GetRainStarParams(const float defaultRadius) const
    -> StarModeParams
{
  StarModeParams starParams;

  const auto x0 = static_cast<int32_t>(m_goomInfo.GetScreenInfo().width / 25);
  starParams.pos.x =
      m_goomRand.GetRandInRange(x0, static_cast<int32_t>(m_goomInfo.GetScreenInfo().width) - x0);

  static constexpr int32_t MIN_Y = 3;
  static constexpr int32_t MAX_Y = 63;
  starParams.pos.y = -m_goomRand.GetRandInRange(MIN_Y, MAX_Y + 1);

  static constexpr float RADIUS_FACTOR = 1.5F;
  static constexpr float INITIAL_VAGE = 0.002F;
  static constexpr float INITIAL_WIND_FACTOR = 1.0F;
  static constexpr float INITIAL_GRAVITY_FACTOR = 0.4F;
  starParams.radius = RADIUS_FACTOR * defaultRadius;
  starParams.vage = INITIAL_VAGE;
  starParams.windFactor = INITIAL_WIND_FACTOR;
  starParams.gravityFactor = INITIAL_GRAVITY_FACTOR;

  return starParams;
}

auto FlyingStarsFx::FlyingStarsImpl::GetFountainStarParams(const float defaultRadius) const
    -> StarModeParams
{
  StarModeParams starParams;

  const int32_t x0 = m_halfWidth / 5;
  starParams.pos.x = m_goomRand.GetRandInRange(m_halfWidth - x0, m_halfWidth + x0);

  static constexpr uint32_t MIN_Y = 3;
  static constexpr uint32_t MAX_Y = 63;
  starParams.pos.y = static_cast<int32_t>(m_goomInfo.GetScreenInfo().height +
                                          m_goomRand.GetRandInRange(MIN_Y, MAX_Y + 1));

  static constexpr float INITIAL_VAGE = 0.001F;
  static constexpr float INITIAL_WIND_FACTOR = 1.0F;
  static constexpr float INITIAL_GRAVITY_FACTOR = 1.0F;
  starParams.radius = 1.0F + defaultRadius;
  starParams.vage = INITIAL_VAGE;
  starParams.windFactor = INITIAL_WIND_FACTOR;
  starParams.gravityFactor = INITIAL_GRAVITY_FACTOR;

  return starParams;
}

void FlyingStarsFx::FlyingStarsImpl::AddStarBombs(const StarModeParams& starModeParams,
                                                  const size_t maxStarsInBomb)
{
  const float sideWind =
      starModeParams.windFactor * m_goomRand.GetRandInRange(m_minSideWind, m_maxSideWind);
  const float gravity =
      starModeParams.gravityFactor * m_goomRand.GetRandInRange(m_minGravity, m_maxGravity);

  for (size_t i = 0; i < maxStarsInBomb; ++i)
  {
    m_colorMapsManager.ChangeColorMapNow(m_mainColorMapID);
    m_colorMapsManager.ChangeColorMapNow(m_lowColorMapID);
    AddABomb(starModeParams.pos, starModeParams.radius, starModeParams.vage, gravity, sideWind);
  }
}

/**
 * Cree une nouvelle 'bombe', c'est a dire une particule appartenant a une fusee d'artifice.
 */
void FlyingStarsFx::FlyingStarsImpl::AddABomb(const Point2dInt& pos,
                                              const float radius,
                                              const float vage,
                                              const float gravity,
                                              const float sideWind)
{
  if (m_stars.size() >= m_maxStars)
  {
    return;
  }

  Star& star = m_stars.emplace_back();

  star.pos = pos.ToFlt();

  const float bombRadius = radius * m_goomRand.GetRandInRange(0.01F, 2.0F);
  const float bombAngle = GetBombAngle(star);

  static constexpr float RADIUS_OFFSET = -0.2F;
  star.velocity.x = bombRadius * std::cos(bombAngle);
  star.velocity.y = RADIUS_OFFSET + (bombRadius * std::sin(bombAngle));

  star.acceleration.x = sideWind;
  star.acceleration.y = gravity;

  static constexpr float HALF_MAX_AGE = 0.5F * MAX_AGE;
  star.age = m_goomRand.GetRandInRange(MIN_AGE, HALF_MAX_AGE);
  star.vage = std::max(MIN_AGE, vage);

  UpdateStarColorMaps(bombAngle, star);
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetBombAngle(const Star& star) const -> float
{
  switch (m_fxMode)
  {
    case StarModes::FIREWORKS:
      return GetFireworksBombAngle();
    case StarModes::RAIN:
      return GetRainBombAngle(star);
    case StarModes::FOUNTAIN:
      return GetFountainBombAngle(star);
    default:
      throw std::logic_error("Unknown StarModes enum.");
  }
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetFireworksBombAngle() const -> float
{
  static constexpr float MIN_FIREWORKS_ANGLE = 0.0F;
  static constexpr float MAX_FIREWORKS_ANGLE = TWO_PI;

  return m_goomRand.GetRandInRange(MIN_FIREWORKS_ANGLE, MAX_FIREWORKS_ANGLE);
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetFountainBombAngle(const Star& star) const -> float
{
  static constexpr float MIN_FOUNTAIN_ANGLE = pi + 0.1F;
  static constexpr float MAX_MIN_FOUNTAIN_ANGLE = pi + THIRD_PI;
  static constexpr float MAX_FOUNTAIN_ANGLE = TWO_PI - 0.1F;

  const float xFactor = star.pos.x / m_xMax;
  const float minAngle =
      STD20::lerp(MIN_FOUNTAIN_ANGLE, MAX_MIN_FOUNTAIN_ANGLE - 0.1F, 1.0F - xFactor);
  const float maxAngle = STD20::lerp(MAX_MIN_FOUNTAIN_ANGLE + 0.1F, MAX_FOUNTAIN_ANGLE, xFactor);

  return m_goomRand.GetRandInRange(minAngle, maxAngle);
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetRainBombAngle(const Star& star) const -> float
{
  static constexpr float MIN_RAIN_ANGLE = 0.1F;
  static constexpr float MAX_MIN_RAIN_ANGLE = THIRD_PI;
  static constexpr float MAX_RAIN_ANGLE = pi - 0.1F;

  const float xFactor = star.pos.x / m_xMax;
  const float minAngle = STD20::lerp(MIN_RAIN_ANGLE, MAX_MIN_RAIN_ANGLE - 0.1F, 1.0F - xFactor);
  const float maxAngle = STD20::lerp(MAX_MIN_RAIN_ANGLE + 0.1F, MAX_RAIN_ANGLE, xFactor);

  return m_goomRand.GetRandInRange(minAngle, maxAngle);
}

} // namespace GOOM::VISUAL_FX
