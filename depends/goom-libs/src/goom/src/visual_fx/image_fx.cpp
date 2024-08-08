module;

//#undef NO_LOGGING

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

module Goom.VisualFx.ImageFx;

import Goom.Color.ColorAdjustment;
import Goom.Color.ColorMaps;
import Goom.Color.ColorUtils;
import Goom.Color.RandomColorMaps;
import Goom.Color.RandomColorMapsGroups;
import Goom.Draw.GoomDrawBase;
import Goom.Draw.ShaperDrawers.PixelDrawer;
import Goom.Utils.Graphics.ImageBitmaps;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Utils.Math.TValues;
import Goom.VisualFx.FxHelper;
import Goom.VisualFx.FxUtils;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomConfigPaths;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomPaths;
import Goom.Lib.GoomUtils;
import Goom.Lib.Point2d;
import Goom.Lib.SPimpl;
import Goom.PluginInfo;

namespace GOOM::VISUAL_FX
{

using COLOR::ColorAdjustment;
using COLOR::ColorMapPtrWrapper;
using COLOR::ColorMaps;
using COLOR::GetBrighterColor;
using COLOR::GetUnweightedRandomColorMaps;
using COLOR::WeightedRandomColorMaps;
using DRAW::MultiplePixels;
using DRAW::SHAPE_DRAWERS::PixelDrawer;
using FX_UTILS::RandomPixelBlender;
using UTILS::Parallel;
using UTILS::GRAPHICS::ImageBitmap;
using UTILS::MATH::FULL_CIRCLE_RANGE;
using UTILS::MATH::HALF;
using UTILS::MATH::I_HALF;
using UTILS::MATH::NumberRange;
using UTILS::MATH::Sq;
using UTILS::MATH::TValue;
using UTILS::MATH::TWO_PI;

static constexpr auto CHUNK_WIDTH  = 2;
static constexpr auto CHUNK_HEIGHT = 2;

using ChunkPixels = std::array<std::array<Pixel, CHUNK_WIDTH>, CHUNK_HEIGHT>;

class ChunkedImage
{
public:
  ChunkedImage(std::shared_ptr<ImageBitmap> image, const PluginInfo& goomInfo) noexcept;

  struct ImageChunk
  {
    Point2dInt finalPosition;
    ChunkPixels pixels;
  };

  [[nodiscard]] auto GetNumChunks() const -> size_t;
  [[nodiscard]] auto GetImageChunk(size_t i) const -> const ImageChunk&;

  [[nodiscard]] auto GetStartPosition(size_t i) const -> const Point2dInt&;
  auto SetStartPosition(size_t i, const Point2dInt& pos) -> void;

private:
  using ImageAsChunks = std::vector<ImageChunk>;
  std::shared_ptr<ImageBitmap> m_image;
  ImageAsChunks m_imageAsChunks;
  std::vector<Point2dInt> m_startPositions;
  [[nodiscard]] static auto SplitImageIntoChunks(const ImageBitmap& imageBitmap,
                                                 const PluginInfo& goomInfo) -> ImageAsChunks;
  static auto SetImageChunkPixels(const ImageBitmap& imageBitmap,
                                  int32_t yImage,
                                  int32_t xImage,
                                  ImageChunk& imageChunk) -> void;
};

class ImageFx::ImageFxImpl
{
public:
  ImageFxImpl(Parallel& parallel,
              FxHelper& fxHelper,
              const std::string& resourcesDirectory) noexcept;

  auto Start() -> void;
  auto Resume() -> void;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void;

  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;
  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>;

  auto ApplyToImageBuffers() -> void;

private:
  Parallel* m_parallel;
  FxHelper* m_fxHelper;
  PixelDrawer m_pixelDrawer;
  std::string m_resourcesDirectory;
  PixelChannelType m_defaultAlpha = DEFAULT_VISUAL_FX_ALPHA;

  int32_t m_availableWidth  = m_fxHelper->GetDimensions().GetIntWidth() - CHUNK_WIDTH;
  int32_t m_availableHeight = m_fxHelper->GetDimensions().GetIntHeight() - CHUNK_HEIGHT;
  Point2dInt m_screenCentre{.x = I_HALF * m_availableWidth, .y = I_HALF * m_availableHeight};
  float m_maxRadius = HALF * static_cast<float>(std::min(m_availableWidth, m_availableHeight));
  [[nodiscard]] auto GetNewRandBrightnessFactor() const -> float;
  float m_randBrightnessFactor{GetNewRandBrightnessFactor()};

  WeightedRandomColorMaps m_colorMaps{
      GetUnweightedRandomColorMaps(m_fxHelper->GetGoomRand(), m_defaultAlpha)};
  ColorMapPtrWrapper m_currentColorMap{GetRandomColorMap()};
  [[nodiscard]] auto GetRandomColorMap() const -> ColorMapPtrWrapper;
  bool m_pixelColorIsDominant                    = false;
  static constexpr float DEFAULT_BRIGHTNESS_BASE = 0.2F;
  float m_brightnessBase                         = DEFAULT_BRIGHTNESS_BASE;

  static constexpr auto ADD_WEIGHT           = 20.0F;
  static constexpr auto DARKEN_ONLY_WEIGHT   = 0.0F;
  static constexpr auto LIGHTEN_ONLY_WEIGHT  = 0.0F;
  static constexpr auto LUMA_MIX_WEIGHT      = 5.0F;
  static constexpr auto MULTIPLY_WEIGHT      = 0.0F;
  static constexpr auto ALPHA_WEIGHT         = 20.0F;
  static constexpr auto ALPHA_AND_ADD_WEIGHT = 20.0F;
  // clang-format off
  RandomPixelBlender m_pixelBlender{
      m_fxHelper->GetGoomRand(),
      {
          {.key=RandomPixelBlender::PixelBlendType::ADD,           .weight=ADD_WEIGHT},
          {.key=RandomPixelBlender::PixelBlendType::DARKEN_ONLY,   .weight=DARKEN_ONLY_WEIGHT},
          {.key=RandomPixelBlender::PixelBlendType::LIGHTEN_ONLY,  .weight=LIGHTEN_ONLY_WEIGHT},
          {.key=RandomPixelBlender::PixelBlendType::LUMA_MIX,      .weight=LUMA_MIX_WEIGHT},
          {.key=RandomPixelBlender::PixelBlendType::MULTIPLY,      .weight=MULTIPLY_WEIGHT},
          {.key=RandomPixelBlender::PixelBlendType::ALPHA,         .weight=ALPHA_WEIGHT},
          {.key=RandomPixelBlender::PixelBlendType::ALPHA_AND_ADD, .weight=ALPHA_AND_ADD_WEIGHT},
      }
  };
  // clang-format on
  [[nodiscard]] static auto GetAcceptablePixelBlenderParams(
      const PixelBlenderParams& pixelBlenderParams) noexcept -> PixelBlenderParams;
  auto UpdatePixelBlender() noexcept -> void;

  std::vector<std::unique_ptr<ChunkedImage>> m_images;
  ChunkedImage* m_currentImage{};
  static constexpr uint32_t NUM_STEPS    = 400;
  static constexpr uint32_t T_DELAY_TIME = 15;
  TValue m_inOutT{
      {.stepType = TValue::StepType::CONTINUOUS_REPEATABLE, .numSteps = NUM_STEPS},
      {{.t0 = 1.0F, .delayTime = T_DELAY_TIME}}
  };
  float m_inOutTSq = 0.0F;
  Point2dInt m_floatingStartPosition{};
  TValue m_floatingT{
      {.stepType  = TValue::StepType::CONTINUOUS_REVERSIBLE,
       .numSteps  = NUM_STEPS,
       .startingT = 1.0F}
  };
  auto InitImage() -> void;

  auto DrawChunks() -> void;
  [[nodiscard]] auto GetPositionAdjustedBrightness(float brightness,
                                                   const Point2dInt& position) const -> float;
  auto DrawChunk(const Point2dInt& pos, float brightness, const ChunkPixels& pixels) -> void;
  [[nodiscard]] auto GetNextChunkStartPosition(size_t i) const -> Point2dInt;
  [[nodiscard]] auto GetNextChunkPosition(const Point2dInt& nextStartPosition,
                                          const ChunkedImage::ImageChunk& imageChunk) const
      -> Point2dInt;
  [[nodiscard]] auto GetPixelColors(const Pixel& pixelColor,
                                    float brightness) const -> MultiplePixels;
  [[nodiscard]] auto GetMappedColor(const Pixel& pixelColor) const -> Pixel;

  auto UpdateImageStartPositions() -> void;
  auto ResetCurrentImage() -> void;
  auto ResetStartPositions() -> void;

  auto UpdateFloatingStartPositions() -> void;
  auto SetNewFloatingStartPosition() -> void;
  [[nodiscard]] auto GetChunkFloatingStartPosition(size_t i) const -> Point2dInt;

  static constexpr float GAMMA = 1.0F;
  ColorAdjustment m_colorAdjust{{.gamma = GAMMA}};
};

ImageFx::ImageFx(Parallel& parallel,
                 FxHelper& fxHelper,
                 const std::string& resourcesDirectory) noexcept
  : m_pimpl{spimpl::make_unique_impl<ImageFxImpl>(parallel, fxHelper, resourcesDirectory)}
{
}

auto ImageFx::GetFxName() const noexcept -> std::string
{
  return "image";
}

auto ImageFx::Start() noexcept -> void
{
  m_pimpl->Start();
}

auto ImageFx::Finish() noexcept -> void
{
  // nothing to do
}

auto ImageFx::Resume() noexcept -> void
{
  m_pimpl->Resume();
}

auto ImageFx::ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pimpl->ChangePixelBlender(pixelBlenderParams);
}

auto ImageFx::SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  m_pimpl->SetWeightedColorMaps(weightedColorMaps);
}

auto ImageFx::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  return m_pimpl->GetCurrentColorMapsNames();
}

auto ImageFx::ApplyToImageBuffers() noexcept -> void
{
  m_pimpl->ApplyToImageBuffers();
}

ImageFx::ImageFxImpl::ImageFxImpl(Parallel& parallel,
                                  FxHelper& fxHelper,
                                  const std::string& resourcesDirectory) noexcept
  : m_parallel{&parallel},
    m_fxHelper{&fxHelper},
    m_pixelDrawer{fxHelper.GetDraw()},
    m_resourcesDirectory{resourcesDirectory}
{
}

inline auto ImageFx::ImageFxImpl::GetRandomColorMap() const -> ColorMapPtrWrapper
{
  return m_colorMaps.GetRandomColorMap(m_colorMaps.GetRandomGroup());
}

inline auto ImageFx::ImageFxImpl::ChangePixelBlender(
    const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pixelBlender.SetPixelBlendType(GetAcceptablePixelBlenderParams(pixelBlenderParams));
}

auto ImageFx::ImageFxImpl::GetAcceptablePixelBlenderParams(
    const PixelBlenderParams& pixelBlenderParams) noexcept -> PixelBlenderParams
{
  if (pixelBlenderParams.useRandomBlender)
  {
    return pixelBlenderParams;
  }
  if ((pixelBlenderParams.forceBlenderType == RandomPixelBlender::PixelBlendType::DARKEN_ONLY) or
      (pixelBlenderParams.forceBlenderType == RandomPixelBlender::PixelBlendType::MULTIPLY))
  {
    return {false, RandomPixelBlender::PixelBlendType::ADD};
  }
  return pixelBlenderParams;
}

inline auto ImageFx::ImageFxImpl::GetCurrentColorMapsNames() const noexcept
    -> std::vector<std::string>
{
  return {m_colorMaps.GetColorMapsName()};
}

inline auto ImageFx::ImageFxImpl::SetWeightedColorMaps(
    const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  m_colorMaps            = WeightedRandomColorMaps{weightedColorMaps.mainColorMaps, m_defaultAlpha};
  m_pixelColorIsDominant = m_fxHelper->GetGoomRand().ProbabilityOf<0.0F>(); // TODO(glk): ??
  m_randBrightnessFactor = GetNewRandBrightnessFactor();
}

inline auto ImageFx::ImageFxImpl::GetNewRandBrightnessFactor() const -> float
{
  static constexpr auto FACTOR_RANGE = NumberRange{0.5F, 2.0F};
  const auto maxRadiusSq             = Sq(m_maxRadius);

  return 1.0F / (m_fxHelper->GetGoomRand().GetRandInRange<FACTOR_RANGE>() * maxRadiusSq);
}

inline auto ImageFx::ImageFxImpl::Resume() -> void
{
  static constexpr auto MIN_BRIGHTNESS = 0.1F;
  const auto brightnessFactor =
      m_fxHelper->GetGoomRand().GetRandInRange<NumberRange{MIN_BRIGHTNESS, 1.0F}>();
  m_brightnessBase = brightnessFactor * DEFAULT_BRIGHTNESS_BASE;
}

inline auto ImageFx::ImageFxImpl::Start() -> void
{
  InitImage();
  ResetCurrentImage();
  ResetStartPositions();
  SetNewFloatingStartPosition();
}

auto ImageFx::ImageFxImpl::InitImage() -> void
{
  static const auto s_IMAGE_FILENAMES = std::array{
      "blossoms.jpg",
      "bokeh.jpg",
      "butterfly.jpg",
      "chameleon-tail.jpg",
      "galaxy.jpg",
      "mountain_sunset.png",
      "night-tree.jpg",
      "pattern1.jpg",
      "pattern2.jpg",
      "pattern3.jpg",
      "pattern4.jpg",
      "pattern5.jpg",
      "pretty-flowers.jpg",
  };
  auto randImageIndexes = std::array<size_t, s_IMAGE_FILENAMES.size()>{};
  std::ranges::iota(randImageIndexes, 0);
  m_fxHelper->GetGoomRand().Shuffle(randImageIndexes);

  const auto imageDir              = join_paths(m_resourcesDirectory, IMAGE_FX_DIR);
  static constexpr auto MAX_IMAGES = 5U;
  for (auto i = 0U; i < MAX_IMAGES; ++i)
  {
    const auto imageFilename = join_paths(imageDir, s_IMAGE_FILENAMES.at(randImageIndexes.at(i)));
    m_images.emplace_back(std::make_unique<ChunkedImage>(
        std::make_shared<ImageBitmap>(imageFilename), m_fxHelper->GetGoomInfo()));
  }
}

inline auto ImageFx::ImageFxImpl::ResetCurrentImage() -> void
{
  const auto imageIndexRange = NumberRange{0U, static_cast<uint32_t>(m_images.size() - 1)};

  m_currentImage = m_images[m_fxHelper->GetGoomRand().GetRandInRange(imageIndexRange)].get();
}

auto ImageFx::ImageFxImpl::ResetStartPositions() -> void
{
  static constexpr auto MIN_RADIUS_FRACTION = 0.7F;
  const auto randMaxRadius =
      m_fxHelper->GetGoomRand().GetRandInRange<NumberRange{MIN_RADIUS_FRACTION, 1.0F}>() *
      m_maxRadius;

  const auto numChunks       = m_currentImage->GetNumChunks();
  auto radiusTheta           = 0.0F;
  const auto radiusThetaStep = TWO_PI / static_cast<float>(numChunks);

  for (auto i = 0U; i < numChunks; ++i)
  {
    static constexpr auto THETA_RANGE  = FULL_CIRCLE_RANGE;
    static constexpr auto SMALL_OFFSET = 0.4F;

    const auto maxRadiusAdj =
        (1.0F - (SMALL_OFFSET * (1.0F + std::sin(radiusTheta)))) * randMaxRadius;
    const auto radiusRange = NumberRange{10.0F, maxRadiusAdj};
    const auto radius      = m_fxHelper->GetGoomRand().GetRandInRange(radiusRange);

    const auto theta = m_fxHelper->GetGoomRand().GetRandInRange<THETA_RANGE>();

    const auto startPos =
        m_screenCentre + Vec2dInt{.x = static_cast<int32_t>((std::cos(theta) * radius)),
                                  .y = static_cast<int32_t>((std::sin(theta) * radius))};

    m_currentImage->SetStartPosition(i, startPos);

    radiusTheta += radiusThetaStep;
  }
}

inline auto ImageFx::ImageFxImpl::GetChunkFloatingStartPosition(const size_t i) const -> Point2dInt
{
  static constexpr auto MARGIN              = 20.0F;
  static constexpr auto RADIUS_FACTOR_RANGE = NumberRange{0.025F, 0.5F};
  const auto aRadius = (m_fxHelper->GetGoomRand().GetRandInRange<RADIUS_FACTOR_RANGE>() *
                        static_cast<float>(m_availableWidth)) -
                       MARGIN;
  const auto bRadius = (m_fxHelper->GetGoomRand().GetRandInRange<RADIUS_FACTOR_RANGE>() *
                        static_cast<float>(m_availableHeight)) -
                       MARGIN;
  const auto theta =
      (TWO_PI * static_cast<float>(i)) / static_cast<float>(m_currentImage->GetNumChunks());
  const auto floatingStartPosition =
      m_screenCentre + Vec2dInt{.x = static_cast<int32_t>((std::cos(theta) * aRadius)),
                                .y = static_cast<int32_t>((std::sin(theta) * bRadius))};
  return floatingStartPosition;
}

inline auto ImageFx::ImageFxImpl::SetNewFloatingStartPosition() -> void
{
  m_floatingStartPosition =
      m_screenCentre - Vec2dInt{.x = m_fxHelper->GetGoomRand().GetRandInRange(
                                    NumberRange{CHUNK_WIDTH, m_availableWidth - 1}),
                                .y = m_fxHelper->GetGoomRand().GetRandInRange(
                                    NumberRange{CHUNK_HEIGHT, m_availableHeight - 1})};
}

inline auto ImageFx::ImageFxImpl::ApplyToImageBuffers() -> void
{
  UpdatePixelBlender();

  DrawChunks();

  UpdateFloatingStartPositions();

  UpdateImageStartPositions();
}

inline auto ImageFx::ImageFxImpl::UpdatePixelBlender() noexcept -> void
{
  m_fxHelper->GetDraw().SetPixelBlendFunc(m_pixelBlender.GetCurrentPixelBlendFunc());
  m_pixelBlender.Update();
}

inline auto ImageFx::ImageFxImpl::DrawChunks() -> void
{
  static constexpr auto IN_OUT_FACTOR                  = 0.02F;
  const auto inOutT                                    = m_inOutT();
  static constexpr auto IN_OUT_CLOSE_TO_RESOLVED_IMAGE = 0.99F;
  const auto brightness =
      inOutT > IN_OUT_CLOSE_TO_RESOLVED_IMAGE ? 0.0F : m_brightnessBase + (IN_OUT_FACTOR * inOutT);

  const auto drawChunk = [this, &brightness](const size_t i)
  {
    const auto nextStartPosition  = GetNextChunkStartPosition(i);
    const auto& imageChunk        = m_currentImage->GetImageChunk(i);
    const auto nextChunkPosition  = GetNextChunkPosition(nextStartPosition, imageChunk);
    const auto adjustedBrightness = GetPositionAdjustedBrightness(brightness, nextChunkPosition);
    DrawChunk(nextChunkPosition, adjustedBrightness, imageChunk.pixels);
  };

  m_parallel->ForLoop(m_currentImage->GetNumChunks(), drawChunk);

  /*** 3 times slower
  for (auto i = 0U; i < m_currentImage->GetNumChunks(); ++i)
  {
    drawChunk(i);
  }
   **/
}

inline auto ImageFx::ImageFxImpl::GetPositionAdjustedBrightness(
    const float brightness, const Point2dInt& position) const -> float
{
  return m_randBrightnessFactor * (brightness * static_cast<float>(SqDistanceFromZero(position)));
}

inline auto ImageFx::ImageFxImpl::UpdateFloatingStartPositions() -> void
{
  m_floatingT.Increment();
  if (m_floatingT() <= 0.0F)
  {
    SetNewFloatingStartPosition();
  }
}

inline auto ImageFx::ImageFxImpl::UpdateImageStartPositions() -> void
{
  const auto delayJustFinishing = m_inOutT.DelayJustFinishing();

  m_inOutT.Increment();
  m_inOutTSq = Sq(m_inOutT());

  if (delayJustFinishing)
  {
    ResetCurrentImage();
    ResetStartPositions();
    SetNewFloatingStartPosition();
    m_floatingT.Reset(1.0F);
    m_currentColorMap = GetRandomColorMap();
  }
}

inline auto ImageFx::ImageFxImpl::GetNextChunkStartPosition(const size_t i) const -> Point2dInt
{
  return lerp(m_currentImage->GetStartPosition(i),
              m_floatingStartPosition + ToVec2dInt(GetChunkFloatingStartPosition(i)),
              m_floatingT());
}

inline auto ImageFx::ImageFxImpl::GetNextChunkPosition(
    const Point2dInt& nextStartPosition,
    const ChunkedImage::ImageChunk& imageChunk) const -> Point2dInt
{
  const auto nextChunkPosition = lerp(nextStartPosition, imageChunk.finalPosition, m_inOutT());
  return nextChunkPosition;
}

auto ImageFx::ImageFxImpl::DrawChunk(const Point2dInt& pos,
                                     const float brightness,
                                     const ChunkPixels& pixels) -> void

{
  auto y = pos.y;
  for (auto i = 0U; i < CHUNK_HEIGHT; ++i)
  {
    const auto& pixelRow = pixels.at(i);

    auto x = pos.x;
    for (const auto& xPixel : pixelRow)
    {
      if ((x < 0) || (x >= m_availableWidth))
      {
        continue;
      }
      if ((y < 0) || (y >= m_availableHeight))
      {
        continue;
      }
      const auto pixelColors = GetPixelColors(xPixel, brightness);
      m_pixelDrawer.DrawPixels({.x = x, .y = y}, pixelColors);

      ++x;
    }

    ++y;
  }
}

inline auto ImageFx::ImageFxImpl::GetPixelColors(const Pixel& pixelColor,
                                                 const float brightness) const -> MultiplePixels
{
  const auto mixedColor =
      ColorMaps::GetColorMix(GetMappedColor(pixelColor), pixelColor, m_inOutTSq);
  const auto color0 = GetBrighterColor(brightness, mixedColor);
  const auto color1 = GetBrighterColor(0.5F * brightness, pixelColor);

  if (m_pixelColorIsDominant)
  {
    return {.color1 = color1, .color2 = color0};
  }

  return {.color1 = color0, .color2 = color1};
}

inline auto ImageFx::ImageFxImpl::GetMappedColor(const Pixel& pixelColor) const -> Pixel
{
  const auto t = (pixelColor.RFlt() + pixelColor.GFlt() + pixelColor.BFlt()) / 3.0F;
  return m_currentColorMap.GetColor(t);
}

ChunkedImage::ChunkedImage(std::shared_ptr<ImageBitmap> image, const PluginInfo& goomInfo) noexcept
  : m_image{std::move(image)},
    m_imageAsChunks{SplitImageIntoChunks(*m_image, goomInfo)},
    m_startPositions(m_imageAsChunks.size())
{
}

inline auto ChunkedImage::GetNumChunks() const -> size_t
{
  return m_imageAsChunks.size();
}

inline auto ChunkedImage::GetImageChunk(const size_t i) const -> const ImageChunk&
{
  return m_imageAsChunks.at(i);
}

inline auto ChunkedImage::GetStartPosition(const size_t i) const -> const Point2dInt&
{
  return m_startPositions.at(i);
}

inline auto ChunkedImage::SetStartPosition(const size_t i, const Point2dInt& pos) -> void
{
  m_startPositions.at(i) = pos;
}

auto ChunkedImage::SplitImageIntoChunks(const ImageBitmap& imageBitmap,
                                        const PluginInfo& goomInfo) -> ImageAsChunks
{
  auto imageAsChunks = ImageAsChunks{};

  const auto centre = goomInfo.GetDimensions().GetCentrePoint();
  const auto x0     = centre.x - static_cast<int32_t>(imageBitmap.GetWidth() / 2);
  const auto y0     = centre.y - static_cast<int32_t>(imageBitmap.GetHeight() / 2);

  Ensures(x0 >= 0);
  Ensures(y0 >= 0);

  const auto numYChunks = static_cast<int32_t>(imageBitmap.GetHeight()) / CHUNK_HEIGHT;
  const auto numXChunks = static_cast<int32_t>(imageBitmap.GetWidth()) / CHUNK_WIDTH;
  auto y                = y0;
  auto yImage           = 0;
  for (auto yChunk = 0; yChunk < numYChunks; ++yChunk)
  {
    auto x      = x0;
    auto xImage = 0;
    for (auto xChunk = 0; xChunk < numXChunks; ++xChunk)
    {
      ImageChunk imageChunk;
      imageChunk.finalPosition = {.x = x, .y = y};
      SetImageChunkPixels(imageBitmap, yImage, xImage, imageChunk);
      imageAsChunks.emplace_back(imageChunk);

      x += CHUNK_WIDTH;
      xImage += CHUNK_WIDTH;
    }
    y += CHUNK_HEIGHT;
    yImage += CHUNK_HEIGHT;
  }

  return imageAsChunks;
}

auto ChunkedImage::SetImageChunkPixels(const ImageBitmap& imageBitmap,
                                       const int32_t yImage,
                                       const int32_t xImage,
                                       ChunkedImage::ImageChunk& imageChunk) -> void
{
  for (auto yPixel = 0U; yPixel < CHUNK_HEIGHT; ++yPixel)
  {
    const auto yImageChunkPos = static_cast<size_t>(yImage) + yPixel;
    auto& pixelRow            = imageChunk.pixels.at(yPixel);
    for (size_t xPixel = 0; xPixel < CHUNK_WIDTH; ++xPixel)
    {
      pixelRow.at(xPixel) = imageBitmap(static_cast<size_t>(xImage) + xPixel, yImageChunkPos);
    }
  }
}

} // namespace GOOM::VISUAL_FX
