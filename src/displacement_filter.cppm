module;

#include "goom/debug_with_println.h"

//#define SAVE_FILTER_BUFFERS

#include "goom/goom_logger.h"
#include "goom_gl.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <functional>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float4.hpp>
#include <span>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef SAVE_FILTER_BUFFERS
#include <fstream>
#include <print>
#endif

namespace GOOM
{
class GoomLogger;
}

export module Goom.GoomVisualization:DisplacementFilter;

import Goom.FilterFx.GpuFilterEffects.GpuZoomFilterEffect;
import Goom.FilterFx.GpuFilterEffects.None;
import Goom.FilterFx.FilterModes;
import Goom.FilterFx.FilterSettings;
import Goom.FilterFx.FilterSettingsService;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.EnumUtils;
import Goom.Lib.AssertUtils;
import Goom.Lib.FrameData;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;
import Goom.GlCaller;
import :Gl2dTextures;
import :GlRenderTypes;
import :GlslProgram;
import :GlslShaderFile;
import :Scene;

using GOOM::FILTER_FX::FilterSettingsService;
using GOOM::FILTER_FX::GpuZoomFilterMode;
using GOOM::FILTER_FX::TextureWrapType;
using GOOM::FILTER_FX::GPU_FILTER_EFFECTS::None;
using GOOM::UTILS::EnumMap;
using GOOM::UTILS::EnumToString;
using GOOM::UTILS::NUM;

static constexpr auto DEBUG_GPU_FILTERS                  = 0;
static constexpr auto DEBUG_GPU_FILTERS_RECT_INNER_WIDTH = 50;
static constexpr auto DEBUG_GPU_FILTERS_RECT_OUTER_WIDTH = 100;

export namespace GOOM::OPENGL
{

class DisplacementFilter : public IScene
{
public:
  static constexpr auto NUM_PBOS = 3U;
  using FilterPosBuffersXY       = Point2dFlt;

  DisplacementFilter(GoomLogger& goomLogger,
                     const std::string& shaderDir,
                     const TextureBufferDimensions& textureBufferDimensions) noexcept;

  auto SetZeroPrevFrameTMix(bool useZeroPrevFrameTMix) -> void;

  auto InitScene() -> void override;
  auto Resize(const WindowDimensions& windowDimensions) noexcept -> void override;
  auto DestroyScene() -> void override;

  [[nodiscard]] auto GetShaderDir() const noexcept -> const std::string&;
  [[nodiscard]] auto GetBrightnessAdjust() const noexcept -> float;
  auto SetBrightnessAdjust(float value) -> void;

  auto Render() -> void override;

  [[nodiscard]] auto GetFrameData(size_t pboIndex) noexcept -> FrameData&;
  auto UpdateFrameData(size_t pboIndex) noexcept -> void;
  using RequestNextFrameDataFunc = std::function<bool()>;
  auto SetRequestNextFrameDataFunc(
      const RequestNextFrameDataFunc& requestNextFrameDataFunc) noexcept -> void;
  using ReleaseCurrentFrameDataFunc = std::function<void(size_t slot)>;
  auto SetReleaseCurrentFrameDataFunc(
      const ReleaseCurrentFrameDataFunc& releaseCurrentFrameDataFunc) noexcept -> void;

protected:
  static constexpr auto DEFAULT_GAMMA = 2.0F;

  static constexpr auto* UNIFORM_SRCE_DEST_LERP_FACTOR     = "u_srceDestLerpFactor";
  static constexpr auto* UNIFORM_BRIGHTNESS                = "u_brightness";
  static constexpr auto* UNIFORM_BRIGHTNESS_ADJUST         = "u_brightnessAdjust";
  static constexpr auto* UNIFORM_HUE_SHIFT                 = "u_hueShift";
  static constexpr auto* UNIFORM_CHROMA_FACTOR             = "u_chromaFactor";
  static constexpr auto* UNIFORM_BASE_COLOR_MULTIPLIER     = "u_baseColorMultiplier";
  static constexpr auto* UNIFORM_PREV_FRAME_T_MIX          = "u_prevFrameTMix";
  static constexpr auto* UNIFORM_LUMINANCE_PARAMS          = "u_params";
  static constexpr auto* UNIFORM_GAMMA                     = "u_gamma";
  static constexpr auto* UNIFORM_RESET_SRCE_FILTER_POS     = "u_resetSrceFilterPosBuffers";
  static constexpr auto* UNIFORM_POS1_POS2_MIX_FREQ        = "u_pos1Pos2MixFreq";
  static constexpr auto* UNIFORM_TIME                      = "u_time";
  static constexpr auto* UNIFORM_GPU_SRCE_FILTER_MODE      = "u_gpuSrceFilterMode";
  static constexpr auto* UNIFORM_GPU_DEST_FILTER_MODE      = "u_gpuDestFilterMode";
  static constexpr auto* UNIFORM_GPU_SRCE_DEST_LERP_FACTOR = "u_gpuSrceDestFilterLerpFactor";
  static constexpr auto* UNIFORM_GPU_FILTER_LERP_FACTOR    = "u_gpuFilterLerpFactor";
  static constexpr auto* UNIFORM_GPU_MIDPOINT              = "u_gpuFilterMidpoint";
  static constexpr auto* UNIFORM_GPU_MAX_ZOOM_ADJUSTMENT   = "u_gpuMaxZoomAdjustment";

  // IMPORTANT - To make proper use of HDR (which is why we're using RGBA16), we
  //             must use a floating point internal format.
  static constexpr auto FILTER_BUFF_TEX_INTERNAL_FORMAT = GL_RGBA16F;
  static constexpr auto FILTER_POS_TEX_INTERNAL_FORMAT  = GL_RG32F;
  static constexpr auto IMAGE_TEX_INTERNAL_FORMAT       = FILTER_BUFF_TEX_INTERNAL_FORMAT;

  static constexpr auto FILTER_BUFF_TEX_FORMAT = GL_RGBA;
  static constexpr auto FILTER_POS_TEX_FORMAT  = GL_RG;
  static constexpr auto IMAGE_TEX_FORMAT       = FILTER_BUFF_TEX_FORMAT;

  // Following must match 'filterSrcePos' types in FrameData.
  static constexpr auto FILTER_BUFF_TEX_PIXEL_TYPE = GL_UNSIGNED_SHORT;
  static constexpr auto FILTER_POS_TEX_PIXEL_TYPE  = GL_FLOAT;
  static constexpr auto IMAGE_TEX_PIXEL_TYPE       = FILTER_BUFF_TEX_PIXEL_TYPE;

  [[nodiscard]] auto GetBuffSize() const noexcept -> size_t;
  auto BindMainColorsBuffTexture() -> void;
  [[nodiscard]] auto GetLumAverage() const -> float;

  static constexpr auto PASS1_VERTEX_SHADER   = "filter.vert";
  static constexpr auto PASS1_FRAGMENT_SHADER = "pass1_update_filter_buff1_and_buff3.frag";
  virtual auto Pass1UpdateFilterBuff1AndBuff3() noexcept -> void;

  [[nodiscard]] auto GetCurrentFrameData() const noexcept -> const FrameData&;
  [[nodiscard]] auto GetGl() noexcept -> GlCaller&;

private:
  GoomLogger* m_goomLogger;
  GlCaller m_gl;
  std::string m_shaderDir;
  size_t m_buffSize;
  float m_aspectRatio;
  float m_brightnessAdjust    = 1.0F;
  bool m_useZeroPrevFrameTMix = false;
  GLuint m_renderToTextureFbo{};
  GLuint m_renderTextureName{};
  bool m_receivedFrameData = false;
  GLsync m_renderSync{};
  auto DoTheDraw() const -> void;
  auto WaitForRenderSync() noexcept -> void;

  GpuFilterEffectData m_gpuFilterEffectData = GetInitialGpuFilterEffectData();
  [[nodiscard]] auto GetInitialGpuFilterEffectData() const noexcept -> GpuFilterEffectData;
  [[nodiscard]] auto GetCentreZoomMidpoint() const noexcept -> Point2dFlt;
  [[nodiscard]] static auto GetGlTextureWrapType(TextureWrapType textureWrapType) noexcept -> GLint;
  size_t m_currentPboIndex = 0U;
  std::vector<FrameData> m_frameDataArray;
  using IGpuParams = FILTER_FX::GPU_FILTER_EFFECTS::IGpuParams;
  IGpuParams::SetterFuncs m_pass1SetterFuncs;
  [[nodiscard]] auto GetPass1SetterFuncs() noexcept -> IGpuParams::SetterFuncs;
  auto InitFrameDataArrayPointers(std::vector<FrameData>& frameDataArray) noexcept -> void;
  auto InitFrameDataArray() noexcept -> void;
  auto InitFrameDataArrayToGl() -> void;
  static auto InitMiscData(MiscData& miscData) noexcept -> void;
  static auto InitImageArrays(ImageArrays& imageArrays) noexcept -> void;
  auto InitFilterPosArrays(FilterPosArrays& filterPosArrays) noexcept -> void;

  auto CopyTextureData(GLuint srceTextureName, GLuint destTextureName) const -> void;

  GLuint m_fsQuad{};
  static constexpr GLuint COMPONENTS_PER_VERTEX     = 2;
  static constexpr int32_t NUM_VERTICES_IN_TRIANGLE = 3;
  static constexpr int32_t NUM_TRIANGLES            = 2;
  static constexpr int32_t NUM_VERTICES             = NUM_TRIANGLES * NUM_VERTICES_IN_TRIANGLE;

  auto CompileAndLinkShaders() -> void;
  using ShaderMacros = std::unordered_map<std::string, std::string>;
  static auto CompileShaderFile(GlslProgram& program,
                                const std::string& filepath,
                                const ShaderMacros& shaderMacros) -> void;
  [[nodiscard]] auto GetShaderFilepath(const std::string& filename) const noexcept -> std::string;
  auto SetupRenderToTextureFBO() -> void;
  auto SetupScreenBuffers() -> void;
  static auto SetupGlSettings() -> void;
  auto SetupGlData() -> void;
  auto InitTextureBuffers() -> void;
  auto SetupGlLumComputeData() noexcept -> void;
  RequestNextFrameDataFunc m_requestNextFrameData;
  ReleaseCurrentFrameDataFunc m_releaseCurrentFrameData;
  auto UpdatePass1MiscDataToGl(size_t pboIndex) noexcept -> void;
  auto UpdatePass4MiscDataToGl(size_t pboIndex) noexcept -> void;
  auto UpdateCurrentDestFilterPosBufferToGl() noexcept -> void;
  auto UpdateImageBuffersToGl(size_t pboIndex) -> void;
  auto UpdatePass1GpuFilterEffectDataToGl() noexcept -> void;

  GlslProgram m_programPass1UpdateFilterBuff1AndBuff3;

  GlslProgram m_programPass2FilterBuff1LuminanceHistogram;
  static constexpr auto PASS2_SHADER = "pass2_lum_histogram.comp";
  auto Pass2FilterBuff3LuminanceHistogram() noexcept -> void;

  GlslProgram m_programPass3FilterBuff1LuminanceAverage;
  static constexpr auto PASS3_SHADER = "pass3_lum_avg.comp";
  auto Pass3FilterBuff3LuminanceAverage() noexcept -> void;

  GlslProgram m_programPass4ResetFilterBuff2AndOutputBuff3;
  static constexpr auto PASS4_VERTEX_SHADER   = "filter.vert";
  static constexpr auto PASS4_FRAGMENT_SHADER = "pass4_reset_filter_buff2_and_output_buff3.frag";
  auto Pass4UpdateFilterBuff2AndOutputBuff3() noexcept -> void;

  auto Pass5OutputToScreen() -> void;

  static constexpr auto NUM_FILTER_BUFF_TEXTURES  = 1;
  static constexpr auto FILTER_BUFF1_TEX_LOCATION = 0;
  static constexpr auto FILTER_BUFF2_TEX_LOCATION =
      FILTER_BUFF1_TEX_LOCATION + NUM_FILTER_BUFF_TEXTURES;
  static constexpr auto FILTER_BUFF3_TEX_LOCATION =
      FILTER_BUFF2_TEX_LOCATION + NUM_FILTER_BUFF_TEXTURES;

  // NOTE: FILTER_SRCE_POS and FILTER_DEST_POS have two textures each
  static constexpr auto NUM_FILTER_POS_TEXTURES = 2;
  static constexpr auto FILTER_SRCE_POS_TEX_LOCATION =
      FILTER_BUFF3_TEX_LOCATION + NUM_FILTER_BUFF_TEXTURES;
  static constexpr auto FILTER_DEST_POS_TEX_LOCATION =
      FILTER_SRCE_POS_TEX_LOCATION + NUM_FILTER_POS_TEXTURES;

  static constexpr auto NUM_IMAGE_TEXTURES = 1;
  static constexpr auto MAIN_IMAGE_TEX_LOCATION =
      FILTER_DEST_POS_TEX_LOCATION + NUM_FILTER_POS_TEXTURES;
  static constexpr auto LOW_IMAGE_TEX_LOCATION = MAIN_IMAGE_TEX_LOCATION + NUM_IMAGE_TEXTURES;

  static constexpr auto LUM_AVG_TEX_LOCATION = LOW_IMAGE_TEX_LOCATION + NUM_IMAGE_TEXTURES;

  static constexpr auto NULL_TEXTURE_NAME                    = "";
  static constexpr auto MAIN_COLORS_IMAGE_TEXTURE_NAME       = "tex_mainColorImage";
  static constexpr auto LOW_COLORS_IMAGE_TEXTURE_NAME        = "tex_lowColorImage";
  static constexpr auto PERSISTENT_COLORS_IMAGE_TEXTURE_NAME = "tex_persistentColorsImage";

  static constexpr auto NULL_IMAGE_UNIT                   = -1;
  static constexpr auto LOW_COLORS_BUFF_IMAGE_UNIT        = 0;
  static constexpr auto PERSISTENT_COLORS_BUFF_IMAGE_UNIT = 1;
  static constexpr auto MAIN_COLORS_BUFF_IMAGE_UNIT       = 2;
  static constexpr auto LUM_AVG_IMAGE_UNIT                = 3;
  static constexpr auto FILTER_SRCE_POS_IMAGE_UNITS       = std::array{4, 5};
  static constexpr auto FILTER_DEST_POS_IMAGE_UNITS       = std::array{6, 7};

  GLuint m_histogramBufferName{};
  static constexpr auto HISTOGRAM_BUFFER_LENGTH    = 256U;
  static constexpr auto LUM_AVG_GROUP_SIZE         = 256U;
  static constexpr auto LUM_HISTOGRAM_BUFFER_INDEX = 3U;
  static constexpr auto LUM_AVG_TEX_UNIT           = GL_TEXTURE0 + LUM_AVG_TEX_LOCATION;
  auto SetLumHistogramParams() noexcept -> void;
  auto SetupGlLumHistogramBuffer() -> void;
  GLuint m_lumAverageDataTextureName{};
  auto SetLumAverageParams(float frameTime) noexcept -> void;
  auto SetupGlLumAverageData() -> void;

  // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
  struct GlFilterPosBuffers
  {
    explicit GlFilterPosBuffers(const GlCaller& glCaller)
      : filterSrcePosTexture{glCaller}, filterDestPosTexture{glCaller}
    {
    }
    Gl2DTexture<FilterPosBuffersXY,
                NUM_FILTER_POS_TEXTURES,
                FILTER_SRCE_POS_TEX_LOCATION,
                FILTER_POS_TEX_FORMAT,
                FILTER_POS_TEX_INTERNAL_FORMAT,
                FILTER_POS_TEX_PIXEL_TYPE,
                0>
        filterSrcePosTexture;
    Gl2DTexture<FilterPosBuffersXY,
                NUM_FILTER_POS_TEXTURES,
                FILTER_DEST_POS_TEX_LOCATION,
                FILTER_POS_TEX_FORMAT,
                FILTER_POS_TEX_INTERNAL_FORMAT,
                FILTER_POS_TEX_PIXEL_TYPE,
                NUM_PBOS>
        filterDestPosTexture;
    size_t numActiveTextures         = NUM_FILTER_POS_TEXTURES;
    size_t currentActiveTextureIndex = 0;
  };
  // NOLINTEND(misc-non-private-member-variables-in-classes)
  auto RotateCurrentFilterPosTextureIndex() noexcept -> void;
  GlFilterPosBuffers m_glFilterPosBuffers{m_gl};
  auto SetupGlFilterPosBuffers() -> void;

  // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
  struct GlFilterBuffers
  {
    explicit GlFilterBuffers(const GlCaller& glCaller)
      : lowColorsBuffTexture{glCaller},
        mainColorsBuffTexture{glCaller},
        persistedColorsBuffTexture{glCaller}
    {
    }
    Gl2DTexture<PixelIntType,
                NUM_FILTER_BUFF_TEXTURES,
                FILTER_BUFF1_TEX_LOCATION,
                FILTER_BUFF_TEX_FORMAT,
                FILTER_BUFF_TEX_INTERNAL_FORMAT,
                FILTER_BUFF_TEX_PIXEL_TYPE,
                0>
        lowColorsBuffTexture;
    Gl2DTexture<PixelIntType,
                NUM_FILTER_BUFF_TEXTURES,
                FILTER_BUFF3_TEX_LOCATION,
                FILTER_BUFF_TEX_FORMAT,
                FILTER_BUFF_TEX_INTERNAL_FORMAT,
                FILTER_BUFF_TEX_PIXEL_TYPE,
                0>
        mainColorsBuffTexture;
    Gl2DTexture<PixelIntType,
                NUM_FILTER_BUFF_TEXTURES,
                FILTER_BUFF2_TEX_LOCATION,
                FILTER_BUFF_TEX_FORMAT,
                FILTER_BUFF_TEX_INTERNAL_FORMAT,
                FILTER_BUFF_TEX_PIXEL_TYPE,
                0>
        persistedColorsBuffTexture;
  };
  // NOLINTEND(misc-non-private-member-variables-in-classes)
  GlFilterBuffers m_glFilterBuffers{m_gl};
  auto SetupGlFilterBuffers() -> void;
  auto BindGlFilterBuffer2() -> void;

  // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
  struct GlImageBuffers
  {
    explicit GlImageBuffers(const GlCaller& glCaller)
      : mainImageTexture{glCaller}, lowImageTexture{glCaller}
    {
    }
    Gl2DTexture<Pixel,
                NUM_IMAGE_TEXTURES,
                MAIN_IMAGE_TEX_LOCATION,
                IMAGE_TEX_FORMAT,
                IMAGE_TEX_INTERNAL_FORMAT,
                IMAGE_TEX_PIXEL_TYPE,
                NUM_PBOS>
        mainImageTexture;
    Gl2DTexture<Pixel,
                NUM_IMAGE_TEXTURES,
                LOW_IMAGE_TEX_LOCATION,
                IMAGE_TEX_FORMAT,
                IMAGE_TEX_INTERNAL_FORMAT,
                IMAGE_TEX_PIXEL_TYPE,
                NUM_PBOS>
        lowImageTexture;
  };
  // NOLINTEND(misc-non-private-member-variables-in-classes)
  GlImageBuffers m_glImageBuffers{m_gl};
  auto SetupGlImageBuffers() -> void;
  auto BindGlImageBuffers() -> void;

#ifdef SAVE_FILTER_BUFFERS
  uint32_t m_pass1SaveNum = 0U;
  uint32_t m_pass4SaveNum = 0U;
  auto SaveGlBuffersAfterPass1() -> void;
  auto SaveGlBuffersAfterPass4() -> void;

  auto SaveFilterBuffersAfterPass1() -> void;
  auto SaveFilterPosBuffersAfterPass1() -> void;
  auto SaveFilterBuffersAfterPass4() -> void;

  auto SavePixelBuffer(const std::string& filename,
                       std::span<Pixel> buffer,
                       float lumAverage = 0.0F) const -> void;
  auto SaveFilterPosBuffer(const std::string& filename, uint32_t textureIndex) -> void;
  auto SaveFilterPosBuffer(const std::string& filename, std::span<FilterPosBuffersXY> buffer) const
      -> void;
#endif
};

} // namespace GOOM::OPENGL

namespace GOOM::OPENGL
{

inline auto DisplacementFilter::SetZeroPrevFrameTMix(const bool useZeroPrevFrameTMix) -> void
{
  m_useZeroPrevFrameTMix = useZeroPrevFrameTMix;
}

inline auto DisplacementFilter::GetShaderDir() const noexcept -> const std::string&
{
  return m_shaderDir;
}

inline auto DisplacementFilter::GetFrameData(const size_t pboIndex) noexcept -> FrameData&
{
  return m_frameDataArray.at(pboIndex);
}

inline auto DisplacementFilter::GetCurrentFrameData() const noexcept -> const FrameData&
{
  return m_frameDataArray.at(m_currentPboIndex);
}

inline auto DisplacementFilter::GetGl() noexcept -> GlCaller&
{
  return m_gl;
}

inline auto DisplacementFilter::GetBrightnessAdjust() const noexcept -> float
{
  return m_brightnessAdjust;
}

inline auto DisplacementFilter::SetBrightnessAdjust(const float value) -> void
{
  m_brightnessAdjust = value;
}

inline auto DisplacementFilter::SetRequestNextFrameDataFunc(
    const RequestNextFrameDataFunc& requestNextFrameDataFunc) noexcept -> void
{
  m_requestNextFrameData = requestNextFrameDataFunc;
}

inline auto DisplacementFilter::SetReleaseCurrentFrameDataFunc(
    const ReleaseCurrentFrameDataFunc& releaseCurrentFrameDataFunc) noexcept -> void
{
  m_releaseCurrentFrameData = releaseCurrentFrameDataFunc;
}

inline auto DisplacementFilter::GetBuffSize() const noexcept -> size_t
{
  return m_buffSize;
}

inline auto DisplacementFilter::BindMainColorsBuffTexture() -> void
{
  m_glFilterBuffers.mainColorsBuffTexture.BindTextures(m_programPass1UpdateFilterBuff1AndBuff3);
}

namespace fs = std::filesystem;

// TODO(glk) - Need to pass goomLogger

namespace
{
auto CopyBuffer(const std::span<const Point2dFlt> srce, std::span<Point2dFlt> dest) noexcept -> void
{
  std::ranges::copy(srce, dest.begin());
}

// TODO(glk) - Move this into goom filters?
auto InitFilterPosBuffer(const Dimensions& dimensions, std::span<Point2dFlt> tranBufferFlt) noexcept
    -> void
{
  Expects(dimensions.GetSize() == tranBufferFlt.size());

  static constexpr auto MIN_COORD   = MIN_NORMALIZED_COORD;
  static constexpr auto COORD_WIDTH = NORMALIZED_COORD_WIDTH;
  const float xRatioScreenToNormalizedCoord =
      COORD_WIDTH / static_cast<float>(dimensions.GetWidth());
  const float yRatioScreenToNormalizedCoord =
      COORD_WIDTH / static_cast<float>(dimensions.GetWidth());

  const auto getNormalizedCoords = [&xRatioScreenToNormalizedCoord, &yRatioScreenToNormalizedCoord](
                                       const float x, const float y) noexcept -> Point2dFlt
  {
    return {.x = MIN_COORD + (xRatioScreenToNormalizedCoord * x),
            .y = MIN_COORD + (yRatioScreenToNormalizedCoord * y)};
  };

  for (auto y = 0U; y < dimensions.GetHeight(); ++y)
  {
    const auto yIndex = static_cast<size_t>(y) * static_cast<size_t>(dimensions.GetWidth());
    for (auto x = 0U; x < dimensions.GetWidth(); ++x)
    {
      const auto index = yIndex + static_cast<size_t>(x);

      const auto identityXY =
          getNormalizedCoords(0.5F + static_cast<float>(x), 0.5F + static_cast<float>(y));

      tranBufferFlt[index] = identityXY;
    }
  }
}

} // namespace

DisplacementFilter::DisplacementFilter(
    GoomLogger& goomLogger,
    const std::string& shaderDir,
    const TextureBufferDimensions& textureBufferDimensions) noexcept
  : IScene{textureBufferDimensions},
    m_goomLogger{&goomLogger},
    m_gl{goomLogger},
    m_shaderDir{shaderDir},
    m_buffSize{static_cast<size_t>(GetWidth()) * static_cast<size_t>(GetHeight())},
    m_aspectRatio{static_cast<float>(GetWidth()) / static_cast<float>(GetHeight())},
    m_frameDataArray(NUM_PBOS),
    m_pass1SetterFuncs{GetPass1SetterFuncs()}
{
}

auto DisplacementFilter::GetPass1SetterFuncs() noexcept -> IGpuParams::SetterFuncs
{
  return {
      .setFloat = [this](const std::string_view& uniformName, const float value)
      { m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(uniformName, value); },
      .setInt = [this](const std::string_view& uniformName, const int32_t value)
      { m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(uniformName, value); },
      .setBool = [this](const std::string_view& uniformName, const bool value)
      { m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(uniformName, value); },
      .setFltVector = [this](const std::string_view& uniformName, const std::vector<float> value)
      { m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(uniformName, value); },
      .setIntVector = [this](const std::string_view& uniformName, const std::vector<int32_t> value)
      { m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(uniformName, value); },
  };
}

auto DisplacementFilter::InitScene() -> void
{
  CompileAndLinkShaders();

  SetupRenderToTextureFBO();

  SetupScreenBuffers();

  SetupGlData();

  SetupGlLumComputeData();

  InitTextureBuffers();

  InitFrameDataArray();

  InitFrameDataArrayToGl();
}

auto DisplacementFilter::DestroyScene() -> void
{
  m_gl.Call()(glDeleteTextures, 1, &m_renderTextureName);
  m_gl.Call()(glDeleteFramebuffers, 1, &m_renderToTextureFbo);

  m_glFilterPosBuffers.filterSrcePosTexture.DeleteBuffers();
  m_glFilterPosBuffers.filterDestPosTexture.DeleteBuffers();
  m_glImageBuffers.mainImageTexture.DeleteBuffers();
  m_glImageBuffers.lowImageTexture.DeleteBuffers();
  m_glFilterBuffers.lowColorsBuffTexture.DeleteBuffers();
  m_glFilterBuffers.persistedColorsBuffTexture.DeleteBuffers();
  m_glFilterBuffers.mainColorsBuffTexture.DeleteBuffers();

  m_programPass1UpdateFilterBuff1AndBuff3.DeleteProgram();
  m_programPass2FilterBuff1LuminanceHistogram.DeleteProgram();
  m_programPass3FilterBuff1LuminanceAverage.DeleteProgram();
  m_programPass4ResetFilterBuff2AndOutputBuff3.DeleteProgram();
}

auto DisplacementFilter::InitFrameDataArray() noexcept -> void
{
  m_gpuFilterEffectData = GetInitialGpuFilterEffectData();

  for (auto& frameData : m_frameDataArray)
  {
    frameData.gpuFilterEffectData = &m_gpuFilterEffectData;

    InitMiscData(frameData.miscData);
    InitImageArrays(frameData.imageArrays);
    InitFilterPosArrays(frameData.filterPosArrays);
  }
};

auto DisplacementFilter::GetInitialGpuFilterEffectData() const noexcept -> GpuFilterEffectData
{
  static constexpr auto CENTRE_MIDPOINT = Point2dFlt{};

  return {
      .filterNeedsUpdating = false,
      .srceFilterMode      = GpuZoomFilterMode::GPU_NONE_MODE,
      .destFilterMode      = GpuZoomFilterMode::GPU_NONE_MODE,
      .srceFilterParams    = &None::GetEmptyGpuParams(),
      .destFilterParams    = &None::GetEmptyGpuParams(),
      .filterTimingInfo    = {.startTime = 0.0F, .maxTime = 1.0F},
      .srceDestLerpFactor  = 1.0F,
      .gpuLerpFactor       = 0.0F,
      .midpoint            = CENTRE_MIDPOINT,
      .maxZoomAdjustment   = FilterSettingsService::DEFAULT_MAX_ZOOM_ADJUSTMENT,
  };
}

auto DisplacementFilter::InitMiscData(MiscData& miscData) noexcept -> void
{
  miscData.brightness          = 1.0F;
  miscData.chromaFactor        = 1.0F;
  miscData.baseColorMultiplier = 1.0F;
  miscData.gamma               = DEFAULT_GAMMA;
}

auto DisplacementFilter::InitImageArrays(ImageArrays& imageArrays) noexcept -> void
{
  imageArrays.mainImagePixelBufferNeedsUpdating = false;
  imageArrays.lowImagePixelBufferNeedsUpdating  = false;
}

auto DisplacementFilter::InitFilterPosArrays(FilterPosArrays& filterPosArrays) noexcept -> void
{
  filterPosArrays.filterPosBuffersLerpFactor = 0.0F;

  filterPosArrays.filterPos1Pos2FreqMixFreq  = FilterPosArrays::DEFAULT_POS1_POS2_MIX_FREQ;
  filterPosArrays.filterDestPosNeedsUpdating = false;

  InitFilterPosBuffer({static_cast<uint32_t>(GetWidth()), static_cast<uint32_t>(GetHeight())},
                      m_glFilterPosBuffers.filterDestPosTexture.GetMappedBuffer(0));

  //  filterPosArrays.maxSqDistance = 0.0F;

  for (auto i = 1U; i < NUM_PBOS; ++i)
  {
    CopyBuffer(m_glFilterPosBuffers.filterDestPosTexture.GetMappedBuffer(0),
               m_glFilterPosBuffers.filterDestPosTexture.GetMappedBuffer(i));
  }
}

auto DisplacementFilter::InitFrameDataArrayToGl() -> void
{
  for (auto i = 0U; i < NUM_FILTER_POS_TEXTURES; ++i)
  {
    m_glFilterPosBuffers.filterDestPosTexture.CopyMappedBufferToTexture(0, i);

    // Make sure copy buffer to texture has completed before using the texture.
    glFinish();

    CopyTextureData(m_glFilterPosBuffers.filterDestPosTexture.GetTextureName(i),
                    m_glFilterPosBuffers.filterSrcePosTexture.GetTextureName(i));
  }
}

auto DisplacementFilter::Resize(const WindowDimensions& windowDimensions) noexcept -> void
{
  SetFramebufferDimensions(windowDimensions);
}

auto DisplacementFilter::SetupRenderToTextureFBO() -> void
{
  // Generate and bind the FBO.
  m_gl.Call()(glGenFramebuffers, 1, &m_renderToTextureFbo);
  m_gl.Call()(glBindFramebuffer, static_cast<GLenum>(GL_FRAMEBUFFER), m_renderToTextureFbo);

  // Create the texture object.
  m_gl.Call()(glGenTextures, 1, &m_renderTextureName);
  m_gl.Call()(glBindTexture, static_cast<GLenum>(GL_TEXTURE_2D), m_renderTextureName);
  m_gl.Call()(glTexStorage2D,
              static_cast<GLenum>(GL_TEXTURE_2D),
              1,
              static_cast<GLenum>(FILTER_BUFF_TEX_INTERNAL_FORMAT),
              GetWidth(),
              GetHeight());

  // Bind the texture to the FBO.
  m_gl.Call()(glFramebufferTexture2D,
              static_cast<GLenum>(GL_FRAMEBUFFER),
              static_cast<GLenum>(GL_COLOR_ATTACHMENT0),
              static_cast<GLenum>(GL_TEXTURE_2D),
              m_renderTextureName,
              0);

  // Set the targets for the fragment output variables.
  const auto drawBuffers = std::array<GLenum, 1>{GL_COLOR_ATTACHMENT0};
  m_gl.Call()(glDrawBuffers, 1, drawBuffers.data());

  // Unbind the FBO, and revert to default framebuffer.
  m_gl.Call()(glBindFramebuffer, static_cast<GLenum>(GL_FRAMEBUFFER), 0U);
}

auto DisplacementFilter::SetupScreenBuffers() -> void
{
  // Setup the vertex and texture coordinate arrays for a full-screen quad (2 triangles).
  static constexpr auto X0 = -1.0F;
  static constexpr auto X1 = +1.0F;
  static constexpr auto Y0 = -1.0F;
  static constexpr auto Y1 = +1.0F;
  // Note: Larger Y at bottom of quad.
  static constexpr auto VERTICES =
      std::array<GLfloat, static_cast<size_t>(COMPONENTS_PER_VERTEX * NUM_VERTICES)>{
          X0,
          Y1, // bottom left
          X0,
          Y0, // top left
          X1,
          Y1, // bottom right
          X1,
          Y1, // bottom right
          X1,
          Y0, // top right
          X0,
          Y0, // top left
      };
  static constexpr auto TEX_COORDS =
      std::array<GLfloat, static_cast<size_t>(COMPONENTS_PER_VERTEX * NUM_VERTICES)>{
          0.0F,
          0.0F, // bottom left
          0.0F,
          1.0F, // top left
          1.0F,
          0.0F, // bottom right
          1.0F,
          0.0F, // bottom right
          1.0F,
          1.0F, // top right
          0.0F,
          1.0F, // top left
      };

  // Setup the vertex and texture array buffers.
  static constexpr auto NUM_ARRAY_BUFFERS = 2;
  std::array<uint32_t, NUM_ARRAY_BUFFERS> handle{};
  m_gl.Call()(glGenBuffers, NUM_ARRAY_BUFFERS, handle.data());

  m_gl.Call()(glBindBuffer, static_cast<GLenum>(GL_ARRAY_BUFFER), handle[0]);
  m_gl.Call()(glBufferData,
              static_cast<GLenum>(GL_ARRAY_BUFFER),
              static_cast<GLsizeiptr>(static_cast<size_t>(COMPONENTS_PER_VERTEX * NUM_VERTICES) *
                                      sizeof(float)),
              VERTICES.data(),
              static_cast<GLenum>(GL_STATIC_DRAW));

  m_gl.Call()(glBindBuffer, static_cast<GLenum>(GL_ARRAY_BUFFER), handle[1]);
  m_gl.Call()(glBufferData,
              static_cast<GLenum>(GL_ARRAY_BUFFER),
              static_cast<GLsizeiptr>(static_cast<size_t>(COMPONENTS_PER_VERTEX * NUM_VERTICES) *
                                      sizeof(float)),
              TEX_COORDS.data(),
              static_cast<GLenum>(GL_STATIC_DRAW));

  // TODO(glk) - Use 4.4 OpenGL - see cookbook
  // Setup the vertex and texture array objects.
  m_gl.Call()(glGenVertexArrays, 1, &m_fsQuad);
  m_gl.Call()(glBindVertexArray, m_fsQuad);

  m_gl.Call()(glBindBuffer, static_cast<GLenum>(GL_ARRAY_BUFFER), handle[0]);
  m_gl.Call()(glVertexAttribPointer,
              0U,
              static_cast<GLint>(COMPONENTS_PER_VERTEX),
              static_cast<GLenum>(GL_FLOAT),
              static_cast<GLboolean>(GL_FALSE),
              0,
              nullptr);
  m_gl.Call()(glEnableVertexAttribArray, 0U); // Vertex position

  m_gl.Call()(glBindBuffer, static_cast<GLenum>(GL_ARRAY_BUFFER), handle[1]);
  m_gl.Call()(glVertexAttribPointer,
              1U,
              static_cast<GLint>(COMPONENTS_PER_VERTEX),
              static_cast<GLenum>(GL_FLOAT),
              static_cast<GLboolean>(GL_FALSE),
              0,
              nullptr);
  m_gl.Call()(glEnableVertexAttribArray, 1U); // Texture coordinates

  m_gl.Call()(glBindVertexArray, 0U);
}

auto DisplacementFilter::CompileAndLinkShaders() -> void
{
  static constexpr auto MIN_ZOOM_ADJUSTMENT = FilterSettingsService::MIN_ZOOM_ADJUSTMENT;

  auto shaderMacros = std::unordered_map<std::string, std::string>{
      {        "LOW_COLORS_BUFF_IMAGE_UNIT",         std::to_string(LOW_COLORS_BUFF_IMAGE_UNIT)},
      {       "MAIN_COLORS_BUFF_IMAGE_UNIT",        std::to_string(MAIN_COLORS_BUFF_IMAGE_UNIT)},
      { "PERSISTENT_COLORS_BUFF_IMAGE_UNIT",  std::to_string(PERSISTENT_COLORS_BUFF_IMAGE_UNIT)},
      {                "LUM_AVG_IMAGE_UNIT",                 std::to_string(LUM_AVG_IMAGE_UNIT)},
      {       "FILTER_SRCE_POS_IMAGE_UNIT1",  std::to_string(FILTER_SRCE_POS_IMAGE_UNITS.at(0))},
      {       "FILTER_SRCE_POS_IMAGE_UNIT2",  std::to_string(FILTER_SRCE_POS_IMAGE_UNITS.at(1))},
      {       "FILTER_DEST_POS_IMAGE_UNIT1",  std::to_string(FILTER_DEST_POS_IMAGE_UNITS.at(0))},
      {       "FILTER_DEST_POS_IMAGE_UNIT2",  std::to_string(FILTER_DEST_POS_IMAGE_UNITS.at(1))},
      {        "LUM_HISTOGRAM_BUFFER_INDEX",         std::to_string(LUM_HISTOGRAM_BUFFER_INDEX)},
      {                             "WIDTH",                         std::to_string(GetWidth())},
      {                            "HEIGHT",                        std::to_string(GetHeight())},
      {                      "ASPECT_RATIO",                      std::to_string(m_aspectRatio)},
      {              "FILTER_POS_MIN_COORD",               std::to_string(MIN_NORMALIZED_COORD)},
      {            "FILTER_POS_COORD_WIDTH",             std::to_string(NORMALIZED_COORD_WIDTH)},
      {    "FILTER_POS_MIN_ZOOM_ADJUSTMENT",                std::to_string(MIN_ZOOM_ADJUSTMENT)},
      {                 "DEBUG_GPU_FILTERS",                  std::to_string(DEBUG_GPU_FILTERS)},
      {"DEBUG_GPU_FILTERS_RECT_INNER_WIDTH", std::to_string(DEBUG_GPU_FILTERS_RECT_INNER_WIDTH)},
      {"DEBUG_GPU_FILTERS_RECT_OUTER_WIDTH", std::to_string(DEBUG_GPU_FILTERS_RECT_OUTER_WIDTH)},
  };

  for (auto i = 0U; i < NUM<GpuZoomFilterMode>; ++i)
  {
    const auto enumName = EnumToString(static_cast<GpuZoomFilterMode>(i));
    shaderMacros.emplace(std::pair{enumName, std::to_string(i)});
  }

  try
  {
    CompileShaderFile(m_programPass1UpdateFilterBuff1AndBuff3,
                      GetShaderFilepath(PASS1_VERTEX_SHADER),
                      shaderMacros);
    CompileShaderFile(m_programPass1UpdateFilterBuff1AndBuff3,
                      GetShaderFilepath(PASS1_FRAGMENT_SHADER),
                      shaderMacros);
    m_programPass1UpdateFilterBuff1AndBuff3.LinkShader();

    CompileShaderFile(
        m_programPass2FilterBuff1LuminanceHistogram, GetShaderFilepath(PASS2_SHADER), shaderMacros);
    m_programPass2FilterBuff1LuminanceHistogram.LinkShader();

    CompileShaderFile(
        m_programPass3FilterBuff1LuminanceAverage, GetShaderFilepath(PASS3_SHADER), shaderMacros);
    m_programPass3FilterBuff1LuminanceAverage.LinkShader();

    CompileShaderFile(m_programPass4ResetFilterBuff2AndOutputBuff3,
                      GetShaderFilepath(PASS4_VERTEX_SHADER),
                      shaderMacros);
    CompileShaderFile(m_programPass4ResetFilterBuff2AndOutputBuff3,
                      GetShaderFilepath(PASS4_FRAGMENT_SHADER),
                      shaderMacros);
    m_programPass4ResetFilterBuff2AndOutputBuff3.LinkShader();
  }
  catch (GlslProgramException& e)
  {
    throw std::runtime_error{std::string{"Compile fail: "} + e.what()};
  }
}

auto DisplacementFilter::GetShaderFilepath(const std::string& filename) const noexcept
    -> std::string
{
  return m_shaderDir + "/" + filename;
}

namespace
{

[[nodiscard]] inline auto GetTempShaderDir() noexcept -> std::string
{
  if constexpr (DEBUG_GPU_FILTERS == 0)
  {
    return fs::temp_directory_path().string();
  }
  else
  {
    return std::string{"/home/greg/Prj/workdir"};
  }
}

} // namespace

auto DisplacementFilter::CompileShaderFile(GlslProgram& program,
                                           const std::string& filepath,
                                           const ShaderMacros& shaderMacros) -> void
{
  static constexpr auto* INCLUDE_DIR = "";

  const auto tempDir        = GetTempShaderDir();
  const auto filename       = fs::path(filepath).filename().string();
  const auto tempShaderFile = tempDir + "/" + filename;

  const auto shaderFile = GlslShaderFile{filepath, shaderMacros, INCLUDE_DIR};
  shaderFile.WriteToFile(tempShaderFile);
  if (not fs::exists(tempShaderFile))
  {
    throw std::runtime_error(std::format("Could not find output file '{}'", tempShaderFile));
  }

  program.CompileShader(tempShaderFile);
}

auto DisplacementFilter::SetupGlData() -> void
{
  SetupGlSettings();
  SetupGlFilterBuffers();
  SetupGlFilterPosBuffers();
  SetupGlImageBuffers();

  InitFrameDataArrayPointers(m_frameDataArray);
}

auto DisplacementFilter::SetupGlSettings() -> void
{
  glDisable(GL_BLEND);
}

auto DisplacementFilter::Render() -> void
{
  m_gl.Call()(glViewport, 0, 0, GetWidth(), GetHeight());

  Pass1UpdateFilterBuff1AndBuff3();

  Pass2FilterBuff3LuminanceHistogram();

  Pass3FilterBuff3LuminanceAverage();

  Pass4UpdateFilterBuff2AndOutputBuff3();

  Pass5OutputToScreen();

  WaitForRenderSync();

  m_gl.Call()(glBindFramebuffer, static_cast<GLenum>(GL_FRAMEBUFFER), 0U);

  UpdateCurrentDestFilterPosBufferToGl();
}

auto DisplacementFilter::UpdateFrameData(const size_t pboIndex) noexcept -> void
{
  m_currentPboIndex = pboIndex;
}

// NOLINTNEXTLINE(bugprone-exception-escape): Not sure what clang-tidy is on about
auto DisplacementFilter::Pass1UpdateFilterBuff1AndBuff3() noexcept -> void
{
  m_receivedFrameData = m_requestNextFrameData();
  if (m_receivedFrameData)
  {
    m_renderSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  }

  UpdateImageBuffersToGl(m_currentPboIndex);

  m_programPass1UpdateFilterBuff1AndBuff3.Use();

  UpdatePass1MiscDataToGl(m_currentPboIndex);
  UpdatePass1GpuFilterEffectDataToGl();

  BindGlFilterBuffer2();
  BindGlImageBuffers();

  DoTheDraw();

#ifdef SAVE_FILTER_BUFFERS
  SaveGlBuffersAfterPass1();
#endif
}

auto DisplacementFilter::DoTheDraw() const -> void
{
  m_gl.Call()(glBindFramebuffer, static_cast<GLenum>(GL_FRAMEBUFFER), m_renderToTextureFbo);
  m_gl.Call()(glClear, static_cast<GLbitfield>(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

  // Render the full-screen quad
  m_gl.Call()(glBindVertexArray, m_fsQuad);
  m_gl.Call()(glDrawArrays, static_cast<GLenum>(GL_TRIANGLE_STRIP), 0, NUM_VERTICES);

  m_gl.Call()(glMemoryBarrier, static_cast<GLenum>(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
}

auto DisplacementFilter::WaitForRenderSync() noexcept -> void
{
  if (not m_receivedFrameData)
  {
    return;
  }

  static constexpr auto TIMEOUT_NANOSECONDS = 50 * 1000 * 1000;

  if (const auto result =
          glClientWaitSync(m_renderSync, GL_SYNC_FLUSH_COMMANDS_BIT, TIMEOUT_NANOSECONDS);
      GL_TIMEOUT_EXPIRED == result)
  {
    LogError(*m_goomLogger, "GL fence did not finish before timeout.");
  }
  else if (GL_WAIT_FAILED == result)
  {
    LogError(*m_goomLogger, "A GL fence error occurred.");
  }

  m_releaseCurrentFrameData(m_currentPboIndex);
}

// NOLINTNEXTLINE(bugprone-exception-escape): Not sure what clang-tidy is on about
auto DisplacementFilter::Pass4UpdateFilterBuff2AndOutputBuff3() noexcept -> void
{
  m_programPass4ResetFilterBuff2AndOutputBuff3.Use();

  UpdatePass4MiscDataToGl(m_currentPboIndex);

  m_gl.Call()(glBindFramebuffer, static_cast<GLenum>(GL_FRAMEBUFFER), m_renderToTextureFbo);
  m_gl.Call()(glClear, static_cast<GLbitfield>(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

  // Render the full-screen quad
  m_gl.Call()(glBindVertexArray, m_fsQuad);
  m_gl.Call()(glDrawArrays, static_cast<GLenum>(GL_TRIANGLE_STRIP), 0, NUM_VERTICES);

  m_gl.Call()(glMemoryBarrier, static_cast<GLenum>(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));

#ifdef SAVE_FILTER_BUFFERS
  SaveGlBuffersAfterPass4();
#endif
}

auto DisplacementFilter::Pass5OutputToScreen() -> void
{
  m_gl.Call()(glViewport, 0, 0, GetFramebufferWidth(), GetFramebufferHeight());

  m_gl.Call()(glBlitNamedFramebuffer,
              m_renderToTextureFbo,
              0U, // default framebuffer
              0, // source rectangle
              0,
              GetWidth(),
              GetHeight(),
              0, // destination rectangle
              0,
              GetFramebufferWidth(),
              GetFramebufferHeight(),
              static_cast<GLbitfield>(GL_COLOR_BUFFER_BIT),
              static_cast<GLenum>(GL_LINEAR));
}

// NOLINTNEXTLINE(bugprone-exception-escape): Not sure what clang-tidy is on about
auto DisplacementFilter::Pass2FilterBuff3LuminanceHistogram() noexcept -> void
{
  m_programPass2FilterBuff1LuminanceHistogram.Use();

  m_gl.Call()(glBindBuffer, static_cast<GLenum>(GL_SHADER_STORAGE_BUFFER), m_histogramBufferName);
  m_gl.Call()(glClearBufferData,
              static_cast<GLenum>(GL_SHADER_STORAGE_BUFFER),
              static_cast<GLenum>(GL_R32UI),
              static_cast<GLenum>(GL_RED_INTEGER),
              static_cast<GLenum>(GL_UNSIGNED_INT),
              nullptr);

  static constexpr auto BLOCK_SIZE = 16;
  m_gl.Call()(glDispatchCompute,
              static_cast<GLuint>(GetWidth() / BLOCK_SIZE),
              static_cast<GLuint>(GetHeight() / BLOCK_SIZE),
              1U);
  m_gl.Call()(glMemoryBarrier, static_cast<GLenum>(GL_SHADER_STORAGE_BARRIER_BIT));
}

// NOLINTNEXTLINE(bugprone-exception-escape): Not sure what clang-tidy is on about
auto DisplacementFilter::Pass3FilterBuff3LuminanceAverage() noexcept -> void
{
  m_programPass3FilterBuff1LuminanceAverage.Use();

  m_gl.Call()(glDispatchCompute, LUM_AVG_GROUP_SIZE, 1U, 1U);
  m_gl.Call()(
      glMemoryBarrier,
      static_cast<GLbitfield>(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
}

// NOLINTNEXTLINE(bugprone-exception-escape): Not sure what clang-tidy is on about
auto DisplacementFilter::SetupGlLumComputeData() noexcept -> void
{
  SetupGlLumHistogramBuffer();
  m_programPass2FilterBuff1LuminanceHistogram.Use();
  SetLumHistogramParams();

  SetupGlLumAverageData();
  m_programPass3FilterBuff1LuminanceAverage.Use();
  static constexpr auto LUM_AVG_TIME_COEFF = 2.0F;
  SetLumAverageParams(LUM_AVG_TIME_COEFF);
}

auto DisplacementFilter::SetLumHistogramParams() noexcept -> void
{
  static constexpr auto MIN_LOG_LUM = -9.0F;
  static constexpr auto MAX_LOG_LUM = +3.5F;
  static_assert((MAX_LOG_LUM - MIN_LOG_LUM) > 0.0F);

  const auto histogramParams = glm::vec4{MIN_LOG_LUM,
                                         1.0F / (MAX_LOG_LUM - MIN_LOG_LUM),
                                         static_cast<float>(GetWidth()),
                                         static_cast<float>(GetHeight())};

  m_programPass2FilterBuff1LuminanceHistogram.SetUniform(UNIFORM_LUMINANCE_PARAMS, histogramParams);
}

auto DisplacementFilter::SetLumAverageParams(const float frameTime) noexcept -> void
{
  static constexpr auto MIN_LOG_LUM = -8.0F;
  static constexpr auto MAX_LOG_LUM = +3.5F;

  static constexpr auto TAU = 1.1F;
  const auto timeCoeff      = std::clamp(1.0F - std::exp(-frameTime * TAU), 0.0F, 1.0F);

  const auto lumAverageParams =
      glm::vec4{MIN_LOG_LUM, MAX_LOG_LUM - MIN_LOG_LUM, timeCoeff, static_cast<float>(m_buffSize)};

  m_programPass3FilterBuff1LuminanceAverage.SetUniform(UNIFORM_LUMINANCE_PARAMS, lumAverageParams);
}

auto DisplacementFilter::GetLumAverage() const -> float
{
  m_gl.Call()(glBindTexture, static_cast<GLenum>(GL_TEXTURE_2D), m_lumAverageDataTextureName);

  auto lumAverage = 0.0F;
  m_gl.Call()(glGetTexImage,
              static_cast<GLenum>(GL_TEXTURE_2D),
              0,
              static_cast<GLenum>(GL_RED),
              static_cast<GLenum>(GL_FLOAT),
              &lumAverage);

  return lumAverage;
}

auto DisplacementFilter::InitFrameDataArrayPointers(std::vector<FrameData>& frameDataArray) noexcept
    -> void
{
  for (auto i = 0U; i < NUM_PBOS; ++i)
  {
    frameDataArray.at(i).filterPosArrays.filterDestPos =
        m_glFilterPosBuffers.filterDestPosTexture.GetMappedBuffer(i);

    frameDataArray.at(i).imageArrays.mainImagePixelBuffer.SetPixelBuffer(
        m_glImageBuffers.mainImageTexture.GetMappedBuffer(i),
        Dimensions{static_cast<uint32_t>(GetWidth()), static_cast<uint32_t>(GetHeight())});
    frameDataArray.at(i).imageArrays.lowImagePixelBuffer.SetPixelBuffer(
        m_glImageBuffers.lowImageTexture.GetMappedBuffer(i),
        Dimensions{static_cast<uint32_t>(GetWidth()), static_cast<uint32_t>(GetHeight())});
  }
}

auto DisplacementFilter::UpdatePass1MiscDataToGl(const size_t pboIndex) noexcept -> void
{
  const auto& filterPosArrays = m_frameDataArray.at(pboIndex).filterPosArrays;
  m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(UNIFORM_SRCE_DEST_LERP_FACTOR,
                                                     filterPosArrays.filterPosBuffersLerpFactor);
  m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(UNIFORM_RESET_SRCE_FILTER_POS,
                                                     filterPosArrays.filterDestPosNeedsUpdating);
  m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(UNIFORM_POS1_POS2_MIX_FREQ,
                                                     filterPosArrays.filterPos1Pos2FreqMixFreq);

  const auto& miscData = m_frameDataArray.at(pboIndex).miscData;
  m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(UNIFORM_BASE_COLOR_MULTIPLIER,
                                                     miscData.baseColorMultiplier);
  m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(
      UNIFORM_PREV_FRAME_T_MIX, m_useZeroPrevFrameTMix ? 0.0F : miscData.prevFrameTMix);
  m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(UNIFORM_TIME,
                                                     static_cast<float>(miscData.goomTime));
  m_glFilterBuffers.persistedColorsBuffTexture.SetTextureWrapType(
      GetGlTextureWrapType(miscData.textureWrapType));
}

auto DisplacementFilter::GetGlTextureWrapType(const TextureWrapType textureWrapType) noexcept
    -> GLint
{
  static constexpr auto GL_TEXTURE_WRAPS = EnumMap<TextureWrapType, GLint>{{{
      {TextureWrapType::REPEAT, GL_REPEAT},
      {TextureWrapType::MIRRORED_REPEAT, GL_MIRRORED_REPEAT},
      {TextureWrapType::CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE},
      {TextureWrapType::MIRRORED_CLAMP_TO_EDGE, GL_MIRROR_CLAMP_TO_EDGE},
  }}};

  return GL_TEXTURE_WRAPS[textureWrapType];
}

auto DisplacementFilter::UpdatePass1GpuFilterEffectDataToGl() noexcept -> void
{
  m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(UNIFORM_GPU_FILTER_LERP_FACTOR,
                                                     m_gpuFilterEffectData.gpuLerpFactor);
  m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(UNIFORM_GPU_SRCE_DEST_LERP_FACTOR,
                                                     m_gpuFilterEffectData.srceDestLerpFactor);
  m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(
      UNIFORM_GPU_MIDPOINT,
      glm::vec2{m_gpuFilterEffectData.midpoint.x, m_gpuFilterEffectData.midpoint.y});
  m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(UNIFORM_GPU_MAX_ZOOM_ADJUSTMENT,
                                                     m_gpuFilterEffectData.maxZoomAdjustment);

#ifdef DEBUG_WITH_PRINTLN
  std::println("UpdatePass1GpuFilterEffectDataToGl: gpuLerpFactor = {}",
               m_gpuFilterEffectData.gpuLerpFactor);
  std::println("  filterNeedsUpdating = {}", m_gpuFilterEffectData.filterNeedsUpdating);
  std::println("  srceFilterMode = {}", EnumToString(m_gpuFilterEffectData.srceFilterMode));
  std::println("  destFilterMode = {}", EnumToString(m_gpuFilterEffectData.destFilterMode));
  std::println("  srceDestLerpFactor = {}", m_gpuFilterEffectData.srceDestLerpFactor);
#endif

  if (not m_gpuFilterEffectData.filterNeedsUpdating)
  {
    return;
  }

  m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(
      UNIFORM_GPU_SRCE_FILTER_MODE, static_cast<uint32_t>(m_gpuFilterEffectData.srceFilterMode));
  m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(
      UNIFORM_GPU_DEST_FILTER_MODE, static_cast<uint32_t>(m_gpuFilterEffectData.destFilterMode));

  m_gpuFilterEffectData.srceFilterParams->OutputGpuParams(m_gpuFilterEffectData.filterTimingInfo,
                                                          m_pass1SetterFuncs);
  m_gpuFilterEffectData.destFilterParams->OutputGpuParams(m_gpuFilterEffectData.filterTimingInfo,
                                                          m_pass1SetterFuncs);

  m_gpuFilterEffectData.filterNeedsUpdating = false;
}

auto DisplacementFilter::UpdatePass4MiscDataToGl(const size_t pboIndex) noexcept -> void
{
  const auto& miscData = m_frameDataArray.at(pboIndex).miscData;

  m_programPass4ResetFilterBuff2AndOutputBuff3.SetUniform(UNIFORM_BRIGHTNESS, miscData.brightness);
  m_programPass4ResetFilterBuff2AndOutputBuff3.SetUniform(UNIFORM_BRIGHTNESS_ADJUST,
                                                          m_brightnessAdjust);
  m_programPass4ResetFilterBuff2AndOutputBuff3.SetUniform(UNIFORM_HUE_SHIFT, miscData.hueShift);
  m_programPass4ResetFilterBuff2AndOutputBuff3.SetUniform(UNIFORM_CHROMA_FACTOR,
                                                          miscData.chromaFactor);
  m_programPass4ResetFilterBuff2AndOutputBuff3.SetUniform(UNIFORM_GAMMA, miscData.gamma);
}

// NOLINTNEXTLINE(bugprone-exception-escape): Not sure what clang-tidy is on about
auto DisplacementFilter::UpdateCurrentDestFilterPosBufferToGl() noexcept -> void
{
  if (not m_frameDataArray.at(m_currentPboIndex).filterPosArrays.filterDestPosNeedsUpdating)
  {
    return;
  }

  m_programPass1UpdateFilterBuff1AndBuff3.Use();
  m_glFilterPosBuffers.filterDestPosTexture.CopyMappedBufferToTexture(
      m_currentPboIndex, m_glFilterPosBuffers.currentActiveTextureIndex);

  RotateCurrentFilterPosTextureIndex();
}

auto DisplacementFilter::RotateCurrentFilterPosTextureIndex() noexcept -> void
{
  ++m_glFilterPosBuffers.currentActiveTextureIndex;
  if (m_glFilterPosBuffers.currentActiveTextureIndex >= m_glFilterPosBuffers.numActiveTextures)
  {
    m_glFilterPosBuffers.currentActiveTextureIndex = 0;
  }
}

auto DisplacementFilter::CopyTextureData(const GLuint srceTextureName,
                                         const GLuint destTextureName) const -> void
{
  m_gl.Call()(glCopyImageSubData,
              srceTextureName,
              static_cast<GLenum>(GL_TEXTURE_2D),
              0,
              0,
              0,
              0,
              destTextureName,
              static_cast<GLenum>(GL_TEXTURE_2D),
              0,
              0,
              0,
              0,
              GetWidth(),
              GetHeight(),
              1);
}

auto DisplacementFilter::UpdateImageBuffersToGl(const size_t pboIndex) -> void
{
  if (m_frameDataArray.at(pboIndex).imageArrays.mainImagePixelBufferNeedsUpdating)
  {
    m_glImageBuffers.mainImageTexture.CopyMappedBufferToTexture(pboIndex, 0);
  }
  if (m_frameDataArray.at(pboIndex).imageArrays.lowImagePixelBufferNeedsUpdating)
  {
    m_glImageBuffers.lowImageTexture.CopyMappedBufferToTexture(pboIndex, 0);
  }
}

auto DisplacementFilter::BindGlFilterBuffer2() -> void
{
  m_glFilterBuffers.persistedColorsBuffTexture.BindTextures(
      m_programPass1UpdateFilterBuff1AndBuff3);
}

auto DisplacementFilter::BindGlImageBuffers() -> void
{
  m_glImageBuffers.mainImageTexture.BindTextures(m_programPass1UpdateFilterBuff1AndBuff3);
  m_glImageBuffers.lowImageTexture.BindTextures(m_programPass1UpdateFilterBuff1AndBuff3);
}

auto DisplacementFilter::SetupGlFilterBuffers() -> void
{
  m_glFilterBuffers.persistedColorsBuffTexture.Setup(0,
                                                     PERSISTENT_COLORS_IMAGE_TEXTURE_NAME,
                                                     PERSISTENT_COLORS_BUFF_IMAGE_UNIT,
                                                     GetWidth(),
                                                     GetHeight());
  m_glFilterBuffers.lowColorsBuffTexture.Setup(
      0, NULL_TEXTURE_NAME, LOW_COLORS_BUFF_IMAGE_UNIT, GetWidth(), GetHeight());
  m_glFilterBuffers.mainColorsBuffTexture.Setup(
      0, NULL_TEXTURE_NAME, MAIN_COLORS_BUFF_IMAGE_UNIT, GetWidth(), GetHeight());
}

auto DisplacementFilter::SetupGlFilterPosBuffers() -> void
{
  for (auto i = 0U; i < NUM_FILTER_POS_TEXTURES; ++i)
  {
    m_glFilterPosBuffers.filterSrcePosTexture.Setup(
        i, NULL_TEXTURE_NAME, FILTER_SRCE_POS_IMAGE_UNITS.at(i), GetWidth(), GetHeight());
    m_glFilterPosBuffers.filterDestPosTexture.Setup(
        i, NULL_TEXTURE_NAME, FILTER_DEST_POS_IMAGE_UNITS.at(i), GetWidth(), GetHeight());
  }
}

auto DisplacementFilter::SetupGlImageBuffers() -> void
{
  m_glImageBuffers.mainImageTexture.Setup(
      0, MAIN_COLORS_IMAGE_TEXTURE_NAME, NULL_IMAGE_UNIT, GetWidth(), GetHeight());
  m_glImageBuffers.lowImageTexture.Setup(
      0, LOW_COLORS_IMAGE_TEXTURE_NAME, NULL_IMAGE_UNIT, GetWidth(), GetHeight());
}

auto DisplacementFilter::SetupGlLumHistogramBuffer() -> void
{
  m_gl.Call()(glGenBuffers, 1, &m_histogramBufferName);
  m_gl.Call()(glBindBuffer, static_cast<GLenum>(GL_SHADER_STORAGE_BUFFER), m_histogramBufferName);
  m_gl.Call()(glBufferData,
              static_cast<GLenum>(GL_SHADER_STORAGE_BUFFER),
              static_cast<GLsizeiptr>(HISTOGRAM_BUFFER_LENGTH * sizeof(uint32_t)),
              nullptr,
              static_cast<GLenum>(GL_DYNAMIC_COPY));
  m_gl.Call()(glBindBufferBase,
              static_cast<GLenum>(GL_SHADER_STORAGE_BUFFER),
              LUM_HISTOGRAM_BUFFER_INDEX,
              m_histogramBufferName);
  m_gl.Call()(glBindBuffer, static_cast<GLenum>(GL_SHADER_STORAGE_BUFFER), 0U);
}

auto DisplacementFilter::SetupGlLumAverageData() -> void
{
  m_gl.Call()(glGenTextures, 1, &m_lumAverageDataTextureName);
  m_gl.Call()(glActiveTexture, static_cast<GLenum>(LUM_AVG_TEX_UNIT));
  m_gl.Call()(glBindTexture, static_cast<GLenum>(GL_TEXTURE_2D), m_lumAverageDataTextureName);
  m_gl.Call()(
      glTexStorage2D, static_cast<GLenum>(GL_TEXTURE_2D), 1, static_cast<GLenum>(GL_R16F), 1, 1);
  m_gl.Call()(glBindImageTexture,
              static_cast<GLenum>(LUM_AVG_IMAGE_UNIT),
              m_lumAverageDataTextureName,
              0,
              static_cast<GLboolean>(GL_FALSE),
              0,
              static_cast<GLenum>(GL_READ_WRITE),
              static_cast<GLenum>(GL_R16F));

  const auto initialData = 0.5F;
  m_gl.Call()(glBindTexture, static_cast<GLenum>(GL_TEXTURE_2D), m_lumAverageDataTextureName);
  m_gl.Call()(glTexSubImage2D,
              static_cast<GLenum>(GL_TEXTURE_2D),
              0,
              0,
              0,
              1,
              1,
              static_cast<GLenum>(GL_RED),
              static_cast<GLenum>(GL_FLOAT),
              &initialData);
}

auto DisplacementFilter::InitTextureBuffers() -> void
{
  m_glFilterBuffers.lowColorsBuffTexture.ZeroTextures();
  m_glFilterBuffers.persistedColorsBuffTexture.ZeroTextures();
  m_glFilterBuffers.mainColorsBuffTexture.ZeroTextures();

  m_glImageBuffers.mainImageTexture.ZeroTextures();
  m_glImageBuffers.lowImageTexture.ZeroTextures();

  m_gl.Call()(glMemoryBarrier, static_cast<GLenum>(GL_TEXTURE_UPDATE_BARRIER_BIT));
}

#ifdef SAVE_FILTER_BUFFERS
// TODO(glk) - Use formatted GoomPoint2dBufferSaver
static constexpr auto SAVE_ROOT_DIR = "/home/greg/.kodi/filter_buffers";
//static constexpr auto SAVE_ROOT_DIR = "/home/greg/Prj/workdir/filter_buffers";

auto DisplacementFilter::SaveGlBuffersAfterPass1() -> void
{
  ++m_pass1SaveNum;

  SaveFilterBuffersAfterPass1();
  SaveFilterPosBuffersAfterPass1();
}

auto DisplacementFilter::SaveGlBuffersAfterPass4() -> void
{
  ++m_pass4SaveNum;

  SaveFilterBuffersAfterPass4();
}

auto DisplacementFilter::SaveFilterBuffersAfterPass1() -> void
{
  auto filterBuffer = std::vector<Pixel>(m_buffSize);
  m_glFilterBuffers.lowColorsBuffTexture.BindTextures(m_programPass1UpdateFilterBuff1AndBuff3);
  m_gl.Call()(glGetTexImage,
              static_cast<GLenum>(GL_TEXTURE_2D),
              0,
              static_cast<GLenum>(GL_RGBA),
              static_cast<GLenum>(FILTER_BUFF_TEX_PIXEL_TYPE),
              filterBuffer.data());

  const auto filename = std::format("{}/filter_buffer1_{:04d}.txt", SAVE_ROOT_DIR, m_pass1SaveNum);

  SavePixelBuffer(filename, filterBuffer);
}

auto DisplacementFilter::SaveFilterPosBuffersAfterPass1() -> void
{
  for (auto i = 0U; i < NUM_FILTER_POS_TEXTURES; ++i)
  {
    const auto filename =
        std::format("{}/filter_pos_buffer{}_{:04d}.txt", SAVE_ROOT_DIR, i, m_pass1SaveNum);

    SaveFilterPosBuffer(filename, i);
  }
}

auto DisplacementFilter::SaveFilterBuffersAfterPass4() -> void
{
  const auto lumAverage = GetLumAverage();

  auto filterBuffer = std::vector<Pixel>(m_buffSize);
  m_glFilterBuffers.mainColorsBuffTexture.BindTextures(
      m_programPass4ResetFilterBuff2AndOutputBuff3);
  m_gl.Call()(glGetTexImage,
              static_cast<GLenum>(GL_TEXTURE_2D),
              0,
              static_cast<GLenum>(GL_RGBA),
              static_cast<GLenum>(FILTER_BUFF_TEX_PIXEL_TYPE),
              filterBuffer.data());

  const auto filename = std::format("{}/filter_buffer3_{:04d}.txt", SAVE_ROOT_DIR, m_pass4SaveNum);

  SavePixelBuffer(filename, filterBuffer, lumAverage);
}

auto DisplacementFilter::SaveFilterPosBuffer(const std::string& filename,
                                             const uint32_t textureIndex) -> void
{
  auto filterBuffer = std::vector<FilterPosBuffersXY>(m_buffSize);
  m_glFilterPosBuffers.filterSrcePosTexture.BindTexture(m_programPass1UpdateFilterBuff1AndBuff3,
                                                        textureIndex);
  m_gl.Call()(glGetTexImage,
              static_cast<GLenum>(GL_TEXTURE_2D),
              0,
              static_cast<GLenum>(GL_RG),
              static_cast<GLenum>(FILTER_POS_TEX_PIXEL_TYPE),
              filterBuffer.data());
  SaveFilterPosBuffer(filename, filterBuffer);
}

auto DisplacementFilter::SaveFilterPosBuffer(const std::string& filename,
                                             std::span<FilterPosBuffersXY> buffer) const -> void
{
  auto file = std::ofstream{filename};
  if (not file.good())
  {
    std::println(stderr, "ERROR: Could not open file '{}'.", filename);
    return;
  }

  auto index = 0U;
  for (auto y = 0; y < GetHeight(); ++y)
  {
    for (auto x = 0; x < GetWidth(); ++x)
    {
      const auto pos = buffer[index];
      file << std::format("[{:4d} {:4d}]  {:6.2f}, {:6.2f}\n", x, y, pos.x, pos.y);
    }
    ++index;
  }
}

auto DisplacementFilter::SavePixelBuffer(const std::string& filename,
                                         std::span<Pixel> buffer,
                                         const float lumAverage) const -> void
{
  auto file = std::ofstream{filename};
  if (not file.good())
  {
    std::println(stderr, "ERROR: Could not open file '{}'.", filename);
    return;
  }

  const auto linearScale   = 0.18F; // MAYBE brightness ??
  const auto finalExposure = linearScale / (lumAverage + 0.0001F);
  file << std::format("LumAverage    = {:.3f}.\n", lumAverage);
  file << std::format("FinalExposure = {:.3f}.\n\n", finalExposure);

  auto index = 0U;
  for (auto y = 0; y < GetHeight(); ++y)
  {
    for (auto x = 0; x < GetWidth(); ++x)
    {
      const auto pixel = buffer[index];
      if (static constexpr auto CUT_OFF = 256U;
          (pixel.R() > CUT_OFF) or (pixel.G() > CUT_OFF) or (pixel.B() > CUT_OFF))
      {
        file << std::format("[{:4d} {:4d}]  {:6d}, {:6d}, {:6d}, {:6d}\n",
                            x,
                            y,
                            pixel.R(),
                            pixel.G(),
                            pixel.B(),
                            pixel.A());
      }
      ++index;
    }
  }
}
#endif

} // namespace GOOM::OPENGL
