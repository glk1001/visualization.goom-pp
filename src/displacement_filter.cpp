//#define SAVE_FILTER_BUFFERS
#undef NO_LOGGING

#include "displacement_filter.h"

#include "filter_fx/normalized_coords.h"
#include "gl_render_types.h"
#include "gl_utils.h"
#include "glsl_program.h"
#include "glsl_shader_file.h"
#include "goom/frame_data.h"
#include "goom/goom_config.h"
#include "goom/goom_logger.h"
#include "goom/goom_types.h"
#include "goom/point2d.h"
#include "scene.h"

#include <GL/gl.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format> // NOLINT: Waiting to use C++20.
#include <glm/ext/vector_float4.hpp>
#include <span> // NOLINT: Waiting to use C++20.
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef SAVE_FILTER_BUFFERS
#include <fstream>
#endif

namespace fs = std::filesystem;

// TODO(glk) - Need to pass goomLogger
//std_fmt::println("{}", __LINE__);

namespace GOOM::OPENGL
{

using GOOM::GoomLogger;
using GOOM::FILTER_FX::NormalizedCoords;

namespace
{
// NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
auto CopyBuffer(const std_spn::span<const Point2dFlt> srce, std_spn::span<Point2dFlt> dest) noexcept
    -> void
{
  std::copy(srce.begin(), srce.end(), dest.begin());
}

// TODO(glk) - Move this into goom filters?
auto InitFilterPosBuffer(const Dimensions& dimensions,
                         // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
                         std_spn::span<Point2dFlt> tranBufferFlt) noexcept -> void
{
  Expects(dimensions.GetSize() == tranBufferFlt.size());

  static constexpr auto MIN_COORD   = -2.0F;
  static constexpr auto MAX_COORD   = +2.0F;
  static constexpr auto COORD_WIDTH = MAX_COORD - MIN_COORD;
  const float xRatioScreenToNormalizedCoord =
      COORD_WIDTH / static_cast<float>(dimensions.GetWidth());
  const float yRatioScreenToNormalizedCoord =
      COORD_WIDTH / static_cast<float>(dimensions.GetWidth());

  const auto getNormalizedCoords = [&xRatioScreenToNormalizedCoord, &yRatioScreenToNormalizedCoord](
                                       const float x, const float y) noexcept -> Point2dFlt
  {
    return {MIN_COORD + (xRatioScreenToNormalizedCoord * x),
            MIN_COORD + (yRatioScreenToNormalizedCoord * y)};
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
    m_shaderDir{shaderDir},
    m_buffSize{static_cast<size_t>(GetWidth()) * static_cast<size_t>(GetHeight())},
    m_aspectRatio{static_cast<float>(GetWidth()) / static_cast<float>(GetHeight())},
    m_frameDataArray(NUM_PBOS)
{
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

auto DisplacementFilter::DestroyScene() noexcept -> void
{
  GlCall(glDeleteTextures(1, &m_renderTextureName));
  GlCall(glDeleteFramebuffers(1, &m_renderToTextureFbo));

  m_glFilterPosBuffers.filterSrcePosTexture.DeleteBuffers();
  m_glFilterPosBuffers.filterDestPosTexture.DeleteBuffers();
  m_glImageBuffers.mainImageTexture.DeleteBuffers();
  m_glImageBuffers.lowImageTexture.DeleteBuffers();
  m_glFilterBuffers.filterBuff1Texture.DeleteBuffers();
  m_glFilterBuffers.filterBuff2Texture.DeleteBuffers();
  m_glFilterBuffers.filterBuff3Texture.DeleteBuffers();

  m_programPass1UpdateFilterBuff1AndBuff3.DeleteProgram();
  m_programPass2FilterBuff1LuminanceHistogram.DeleteProgram();
  m_programPass3FilterBuff1LuminanceAverage.DeleteProgram();
  m_programPass4ResetFilterBuff2AndOutputBuff3.DeleteProgram();
}

auto DisplacementFilter::InitFrameDataArray() noexcept -> void
{
  for (auto& frameData : m_frameDataArray)
  {
    InitMiscData(frameData.miscData);
    InitImageArrays(frameData.imageArrays);
    InitFilterPosArrays(frameData.filterPosArrays);
  }
}

auto DisplacementFilter::InitMiscData(GOOM::MiscData& miscData) noexcept -> void
{
  miscData.filterPosBuffersLerpFactor = 0.0F;
  miscData.brightness                 = 1.0F;
  miscData.chromaFactor               = 1.0F;
  miscData.baseColorMultiplier        = 1.0F;
}

auto DisplacementFilter::InitImageArrays(GOOM::ImageArrays& imageArrays) noexcept -> void
{
  imageArrays.mainImagePixelBufferNeedsUpdating = false;
  imageArrays.lowImagePixelBufferNeedsUpdating  = false;
}

auto DisplacementFilter::InitFilterPosArrays(GOOM::FilterPosArrays& filterPosArrays) noexcept
    -> void
{
  filterPosArrays.filterDestPosNeedsUpdating = false;

  InitFilterPosBuffer({static_cast<uint32_t>(GetWidth()), static_cast<uint32_t>(GetHeight())},
                      m_glFilterPosBuffers.filterSrcePosTexture.GetMappedBuffer(0));

  for (auto i = 1U; i < NUM_FILTER_POS_TEXTURES; ++i)
  {
    CopyBuffer(m_glFilterPosBuffers.filterSrcePosTexture.GetMappedBuffer(0),
               m_glFilterPosBuffers.filterSrcePosTexture.GetMappedBuffer(i));
  }
  for (auto i = 0U; i < NUM_PBOS; ++i)
  {
    CopyBuffer(m_glFilterPosBuffers.filterSrcePosTexture.GetMappedBuffer(0),
               m_glFilterPosBuffers.filterDestPosTexture.GetMappedBuffer(i));
  }

  const auto posBufferLen = static_cast<size_t>(GetWidth()) * static_cast<size_t>(GetHeight());
  for (auto& previousFilterDestPosBuffer : m_glFilterPosBuffers.activeFilterDestPosBuffers)
  {
    previousFilterDestPosBuffer.resize(posBufferLen);
    previousFilterDestPosBuffer.assign(
        m_glFilterPosBuffers.filterDestPosTexture.GetMappedBuffer(0).begin(),
        m_glFilterPosBuffers.filterDestPosTexture.GetMappedBuffer(0).end());
  }
}

auto DisplacementFilter::InitFrameDataArrayToGl() noexcept -> void
{
  for (auto i = 0U; i < NUM_FILTER_POS_TEXTURES; ++i)
  {
    m_glFilterPosBuffers.filterSrcePosTexture.CopyMappedBufferToTexture(i, i);
    m_glFilterPosBuffers.filterDestPosTexture.CopyMappedBufferToTexture(0, i);
  }
}

auto DisplacementFilter::Resize(const WindowDimensions& windowDimensions) noexcept -> void
{
  SetFramebufferDimensions(windowDimensions);
}

auto DisplacementFilter::SetupRenderToTextureFBO() noexcept -> void
{
  // Generate and bind the FBO.
  GlCall(glGenFramebuffers(1, &m_renderToTextureFbo));
  GlCall(glBindFramebuffer(GL_FRAMEBUFFER, m_renderToTextureFbo));

  // Create the texture object.
  GlCall(glGenTextures(1, &m_renderTextureName));
  GlCall(glBindTexture(GL_TEXTURE_2D, m_renderTextureName));
  GlCall(
      glTexStorage2D(GL_TEXTURE_2D, 1, FILTER_BUFF_TEX_INTERNAL_FORMAT, GetWidth(), GetHeight()));

  // Bind the texture to the FBO.
  GlCall(glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderTextureName, 0));

  // Set the targets for the fragment output variables.
  const auto drawBuffers = std::array<GLenum, 1>{GL_COLOR_ATTACHMENT0};
  GlCall(glDrawBuffers(1, drawBuffers.data()));

  // Unbind the FBO, and revert to default framebuffer.
  GlCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

auto DisplacementFilter::SetupScreenBuffers() noexcept -> void
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
  static constexpr auto NUM_ARRAY_BUFFERS = 2U;
  std::array<uint32_t, NUM_ARRAY_BUFFERS> handle{};
  GlCall(glGenBuffers(NUM_ARRAY_BUFFERS, handle.data()));

  GlCall(glBindBuffer(GL_ARRAY_BUFFER, handle[0]));
  GlCall(glBufferData(GL_ARRAY_BUFFER,
                      static_cast<size_t>(COMPONENTS_PER_VERTEX * NUM_VERTICES) * sizeof(float),
                      VERTICES.data(),
                      GL_STATIC_DRAW));

  GlCall(glBindBuffer(GL_ARRAY_BUFFER, handle[1]));
  GlCall(glBufferData(GL_ARRAY_BUFFER,
                      static_cast<size_t>(COMPONENTS_PER_VERTEX * NUM_VERTICES) * sizeof(float),
                      TEX_COORDS.data(),
                      GL_STATIC_DRAW));

  // TODO(glk) - Use 4.4 OpenGL - see cookbook
  // Setup the vertex and texture array objects.
  GlCall(glGenVertexArrays(1, &m_fsQuad));
  GlCall(glBindVertexArray(m_fsQuad));

  GlCall(glBindBuffer(GL_ARRAY_BUFFER, handle[0]));
  GlCall(glVertexAttribPointer(0, COMPONENTS_PER_VERTEX, GL_FLOAT, GL_FALSE, 0, nullptr));
  GlCall(glEnableVertexAttribArray(0)); // Vertex position

  GlCall(glBindBuffer(GL_ARRAY_BUFFER, handle[1]));
  GlCall(glVertexAttribPointer(1, COMPONENTS_PER_VERTEX, GL_FLOAT, GL_FALSE, 0, nullptr));
  GlCall(glEnableVertexAttribArray(1)); // Texture coordinates

  GlCall(glBindVertexArray(0));
}

auto DisplacementFilter::CompileAndLinkShaders() -> void
{
  const auto shaderMacros = std::unordered_map<std::string, std::string>{
      {   "FILTER_BUFF1_IMAGE_UNIT",       std::to_string(FILTER_BUFF1_IMAGE_UNIT)},
      {   "FILTER_BUFF2_IMAGE_UNIT",       std::to_string(FILTER_BUFF2_IMAGE_UNIT)},
      {   "FILTER_BUFF3_IMAGE_UNIT",       std::to_string(FILTER_BUFF3_IMAGE_UNIT)},
      {        "LUM_AVG_IMAGE_UNIT",            std::to_string(LUM_AVG_IMAGE_UNIT)},
      {"LUM_HISTOGRAM_BUFFER_INDEX",    std::to_string(LUM_HISTOGRAM_BUFFER_INDEX)},
      {              "ASPECT_RATIO",                 std::to_string(m_aspectRatio)},
      {      "FILTER_POS_MIN_COORD",   std::to_string(NormalizedCoords::MIN_COORD)},
      {    "FILTER_POS_COORD_WIDTH", std::to_string(NormalizedCoords::COORD_WIDTH)},
  };

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

auto DisplacementFilter::CompileShaderFile(GlslProgram& program,
                                           const std::string& filepath,
                                           const ShaderMacros& shaderMacros) -> void
{
  static constexpr auto* INCLUDE_DIR = "";

  const auto tempDir        = fs::temp_directory_path().string();
  const auto filename       = fs::path(filepath).filename().string();
  const auto tempShaderFile = tempDir + "/" + filename;

  const auto shaderFile = GlslShaderFile{filepath, shaderMacros, INCLUDE_DIR};
  shaderFile.WriteToFile(tempShaderFile);
  if (not fs::exists(tempShaderFile))
  {
    // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
    throw std::runtime_error(std_fmt::format("Could not find output file '{}'", tempShaderFile));
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
  GlCall(glViewport(0, 0, GetWidth(), GetHeight()));

  Pass1UpdateFilterBuff1AndBuff3();

  Pass2FilterBuff3LuminanceHistogram();

  Pass3FilterBuff3LuminanceAverage();

  Pass4UpdateFilterBuff2AndOutputBuff3();

  Pass5OutputToScreen();

  WaitForRenderSync();

  GlCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

auto DisplacementFilter::UpdateFrameData(const size_t pboIndex) noexcept -> void
{
  m_currentPboIndex = pboIndex;
}

auto DisplacementFilter::Pass1UpdateFilterBuff1AndBuff3() noexcept -> void
{
  m_receivedFrameData = m_requestNextFrameData();
  if (m_receivedFrameData)
  {
    m_renderSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  }

  UpdateSrceFilterPosBufferToGl(m_currentPboIndex);
  UpdateImageBuffersToGl(m_currentPboIndex);

  m_programPass1UpdateFilterBuff1AndBuff3.Use();

  UpdatePass1MiscDataToGl(m_currentPboIndex);
  UpdateDestFilterPosBufferToGl(m_currentPboIndex);

  BindGlFilterBuffer2();
  BindGlImageBuffers();
  BindGlFilterPosBuffers();

  GlCall(glBindFramebuffer(GL_FRAMEBUFFER, m_renderToTextureFbo));
  GlCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

  // Render the full-screen quad
  GlCall(glBindVertexArray(m_fsQuad));
  GlCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, NUM_VERTICES));

  GlCall(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));

#ifdef SAVE_FILTER_BUFFERS
  SaveFilterBuffersAfterPass1();
#endif

  UpdateCurrentFilterPosTextureIndex();
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

auto DisplacementFilter::Pass4UpdateFilterBuff2AndOutputBuff3() noexcept -> void
{
  m_programPass4ResetFilterBuff2AndOutputBuff3.Use();

  UpdatePass4MiscDataToGl(m_currentPboIndex);

  GlCall(glBindFramebuffer(GL_FRAMEBUFFER, m_renderToTextureFbo));
  GlCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

  // Render the full-screen quad
  GlCall(glBindVertexArray(m_fsQuad));
  GlCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, NUM_VERTICES));

  GlCall(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));

#ifdef SAVE_FILTER_BUFFERS
  SaveFilterBuffersAfterPass4();
#endif
}

auto DisplacementFilter::Pass5OutputToScreen() noexcept -> void
{
  GlCall(glViewport(0, 0, GetFramebufferWidth(), GetFramebufferHeight()));

  GlCall(glBlitNamedFramebuffer(m_renderToTextureFbo,
                                0, // default framebuffer
                                0, // source rectangle
                                0,
                                GetWidth(),
                                GetHeight(),
                                0, // destination rectangle
                                0,
                                GetFramebufferWidth(),
                                GetFramebufferHeight(),
                                GL_COLOR_BUFFER_BIT,
                                GL_LINEAR));
}

auto DisplacementFilter::Pass2FilterBuff3LuminanceHistogram() noexcept -> void
{
  m_programPass2FilterBuff1LuminanceHistogram.Use();

  GlCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_histogramBufferName));
  GlCall(glClearBufferData(
      GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr));

  GlCall(glDispatchCompute(
      static_cast<GLuint>(GetWidth() / 16), static_cast<GLuint>(GetHeight() / 16), 1));
  GlCall(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
}

auto DisplacementFilter::Pass3FilterBuff3LuminanceAverage() noexcept -> void
{
  m_programPass3FilterBuff1LuminanceAverage.Use();

  GlCall(glDispatchCompute(LUM_AVG_GROUP_SIZE, 1, 1));
  GlCall(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
}

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

auto DisplacementFilter::GetLumAverage() const noexcept -> float
{
  GlCall(glBindTexture(GL_TEXTURE_2D, m_lumAverageDataTextureName));

  auto lumAverage = 0.0F;
  GlCall(glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, &lumAverage));

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
  m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(
      UNIFORM_LERP_FACTOR, m_frameDataArray.at(pboIndex).miscData.filterPosBuffersLerpFactor);
  m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(
      UNIFORM_BASE_COLOR_MULTIPLIER, m_frameDataArray.at(pboIndex).miscData.baseColorMultiplier);
  m_programPass1UpdateFilterBuff1AndBuff3.SetUniform(
      UNIFORM_TIME, static_cast<uint32_t>(m_frameDataArray.at(pboIndex).miscData.goomTime));
}

auto DisplacementFilter::UpdatePass4MiscDataToGl(const size_t pboIndex) noexcept -> void
{
  m_programPass4ResetFilterBuff2AndOutputBuff3.SetUniform(
      UNIFORM_BRIGHTNESS, m_frameDataArray.at(pboIndex).miscData.brightness);
  m_programPass4ResetFilterBuff2AndOutputBuff3.SetUniform(UNIFORM_BRIGHTNESS_ADJUST,
                                                          m_brightnessAdjust);
  m_programPass4ResetFilterBuff2AndOutputBuff3.SetUniform(
      UNIFORM_HUE_SHIFT, m_frameDataArray.at(pboIndex).miscData.hueShift);
  m_programPass4ResetFilterBuff2AndOutputBuff3.SetUniform(
      UNIFORM_CHROMA_FACTOR, m_frameDataArray.at(pboIndex).miscData.chromaFactor);
}

auto DisplacementFilter::UpdateSrceFilterPosBufferToGl(const size_t pboIndex) noexcept -> void
{
  if (not m_frameDataArray.at(pboIndex).filterPosArrays.filterDestPosNeedsUpdating)
  {
    return;
  }

  const auto lerpFactor = m_frameDataArray.at(pboIndex).miscData.filterPosBuffersLerpFactor;

  for (auto i = 0U; i < m_glFilterPosBuffers.numActiveTextures; ++i)
  {
    UpdateSrceFilterPosBufferAndTexture(lerpFactor, i);
  }

  m_frameDataArray.at(pboIndex).miscData.filterPosBuffersLerpFactor = 0.0;
}

auto DisplacementFilter::UpdateSrceFilterPosBufferAndTexture(
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    const float lerpFactor,
    const size_t buffIndex) noexcept -> void
{
  auto filterSrcePosBuffer = m_glFilterPosBuffers.filterSrcePosTexture.GetMappedBuffer(buffIndex);
  const auto& filterDestPosBuffer = m_glFilterPosBuffers.activeFilterDestPosBuffers.at(buffIndex);

  std::transform(filterDestPosBuffer.cbegin(),
                 filterDestPosBuffer.cend(),
                 filterSrcePosBuffer.begin(),
                 filterSrcePosBuffer.begin(),
                 [&lerpFactor](const Point2dFlt& destPos, const Point2dFlt& srcePos)
                 { return lerp(srcePos, destPos, lerpFactor); });

  m_glFilterPosBuffers.filterSrcePosTexture.CopyMappedBufferToTexture(buffIndex, buffIndex);
}

auto DisplacementFilter::UpdateDestFilterPosBufferToGl(const size_t pboIndex) noexcept -> void
{
  if (not m_frameDataArray.at(pboIndex).filterPosArrays.filterDestPosNeedsUpdating)
  {
    return;
  }

  auto& activeFilterDestPosBuffer = m_glFilterPosBuffers.activeFilterDestPosBuffers.at(
      m_glFilterPosBuffers.currentActiveTextureIndex);

  activeFilterDestPosBuffer.assign(
      m_glFilterPosBuffers.filterDestPosTexture.GetMappedBuffer(pboIndex).begin(),
      m_glFilterPosBuffers.filterDestPosTexture.GetMappedBuffer(pboIndex).end());

  m_glFilterPosBuffers.filterDestPosTexture.CopyMappedBufferToTexture(
      pboIndex, m_glFilterPosBuffers.currentActiveTextureIndex);
}

auto DisplacementFilter::UpdateCurrentFilterPosTextureIndex() noexcept -> void
{
  if (not m_frameDataArray.at(m_currentPboIndex).filterPosArrays.filterDestPosNeedsUpdating)
  {
    return;
  }

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
                                         const GLuint destTextureName) const noexcept -> void
{
  GlCall(glCopyImageSubData(srceTextureName,
                            GL_TEXTURE_2D,
                            0,
                            0,
                            0,
                            0,
                            destTextureName,
                            GL_TEXTURE_2D,
                            0,
                            0,
                            0,
                            0,
                            GetWidth(),
                            GetHeight(),
                            1));
}

auto DisplacementFilter::UpdateImageBuffersToGl(const size_t pboIndex) noexcept -> void
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

auto DisplacementFilter::BindGlFilterPosBuffers() noexcept -> void
{
  m_glFilterPosBuffers.filterSrcePosTexture.BindTextures(m_programPass1UpdateFilterBuff1AndBuff3);
  m_glFilterPosBuffers.filterDestPosTexture.BindTextures(m_programPass1UpdateFilterBuff1AndBuff3);
}

auto DisplacementFilter::BindGlFilterBuffer2() noexcept -> void
{
  m_glFilterBuffers.filterBuff2Texture.BindTextures(m_programPass1UpdateFilterBuff1AndBuff3);
}

auto DisplacementFilter::BindGlImageBuffers() noexcept -> void
{
  m_glImageBuffers.mainImageTexture.BindTextures(m_programPass1UpdateFilterBuff1AndBuff3);
  m_glImageBuffers.lowImageTexture.BindTextures(m_programPass1UpdateFilterBuff1AndBuff3);
}

auto DisplacementFilter::SetupGlFilterBuffers() -> void
{
  m_glFilterBuffers.filterBuff1Texture.Setup(
      0, FILTER_BUFF1_TEX_SHADER_NAME, GetWidth(), GetHeight());
  m_glFilterBuffers.filterBuff2Texture.Setup(
      0, FILTER_BUFF2_TEX_SHADER_NAME, GetWidth(), GetHeight());
  m_glFilterBuffers.filterBuff3Texture.Setup(
      0, FILTER_BUFF3_TEX_SHADER_NAME, GetWidth(), GetHeight());
}

auto DisplacementFilter::SetupGlFilterPosBuffers() -> void
{
  for (auto i = 0U; i < NUM_FILTER_POS_TEXTURES; ++i)
  {
    m_glFilterPosBuffers.filterSrcePosTexture.Setup(
        i, FILTER_SRCE_POS_TEX_SHADER_NAMES.at(i), GetWidth(), GetHeight());
    m_glFilterPosBuffers.filterDestPosTexture.Setup(
        i, FILTER_DEST_POS_TEX_SHADER_NAMES.at(i), GetWidth(), GetHeight());
  }
}

auto DisplacementFilter::SetupGlImageBuffers() -> void
{
  m_glImageBuffers.mainImageTexture.Setup(0, MAIN_IMAGE_TEX_SHADER_NAME, GetWidth(), GetHeight());
  m_glImageBuffers.lowImageTexture.Setup(0, LOW_IMAGE_TEX_SHADER_NAME, GetWidth(), GetHeight());
}

auto DisplacementFilter::SetupGlLumHistogramBuffer() noexcept -> void
{
  GlCall(glGenBuffers(1, &m_histogramBufferName));
  GlCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_histogramBufferName));
  GlCall(glBufferData(GL_SHADER_STORAGE_BUFFER,
                      HISTOGRAM_BUFFER_LENGTH * sizeof(uint32_t),
                      nullptr,
                      GL_DYNAMIC_COPY));
  GlCall(glBindBufferBase(
      GL_SHADER_STORAGE_BUFFER, LUM_HISTOGRAM_BUFFER_INDEX, m_histogramBufferName));
  GlCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
}

auto DisplacementFilter::SetupGlLumAverageData() noexcept -> void
{
  GlCall(glGenTextures(1, &m_lumAverageDataTextureName));
  GlCall(glActiveTexture(LUM_AVG_TEX_UNIT));
  GlCall(glBindTexture(GL_TEXTURE_2D, m_lumAverageDataTextureName));
  GlCall(glTexStorage2D(GL_TEXTURE_2D, 1, GL_R16F, 1, 1));
  GlCall(glBindImageTexture(
      LUM_AVG_IMAGE_UNIT, m_lumAverageDataTextureName, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R16F));

  const auto initialData = 0.5F;
  GlCall(glBindTexture(GL_TEXTURE_2D, m_lumAverageDataTextureName));
  GlCall(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RED, GL_FLOAT, &initialData));
}

auto DisplacementFilter::InitTextureBuffers() noexcept -> void
{
  m_glFilterBuffers.filterBuff1Texture.ZeroTextures();
  m_glFilterBuffers.filterBuff2Texture.ZeroTextures();
  m_glFilterBuffers.filterBuff3Texture.ZeroTextures();

  m_glImageBuffers.mainImageTexture.ZeroTextures();
  m_glImageBuffers.lowImageTexture.ZeroTextures();

  GlCall(glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT));
}

#ifdef SAVE_FILTER_BUFFERS
auto DisplacementFilter::SaveFilterBuffersAfterPass1() -> void
{
  auto filterBuffer = std::vector<GOOM::Pixel>(m_buffSize);
  m_glFilterBuffers.filterBuff1Texture.BindTexture(m_programPass1UpdateFilterBuff1AndBuff3);
  GlCall(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, FILTER_BUFF_TEX_PIXEL_TYPE, filterBuffer.data()));

  static auto s_saveNum = 1U;
  const auto filename =
      std_fmt::format("/home/greg/.kodi/filter_buffers/filter_buffer1_{:04d}.txt", s_saveNum);
  ++s_saveNum;

  SaveFilterBuffer(filename, filterBuffer);
}

auto DisplacementFilter::SaveFilterBuffersAfterPass4() -> void
{
  const auto lumAverage = GetLumAverage();

  auto filterBuffer = std::vector<GOOM::Pixel>(m_buffSize);
  m_glFilterBuffers.filterBuff3Texture.BindTexture(m_programPass4ResetFilterBuff2AndOutputBuff3);
  GlCall(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, FILTER_BUFF_TEX_PIXEL_TYPE, filterBuffer.data()));

  static auto s_saveNum = 1U;
  const auto filename =
      std_fmt::format("/home/greg/.kodi/filter_buffers/filter_buffer3_{:04d}.txt", s_saveNum);
  ++s_saveNum;

  SaveFilterBuffer(filename, filterBuffer, lumAverage);
}

auto DisplacementFilter::SaveFilterBuffer(const std::string& filename,
                                          const std::vector<GOOM::Pixel>& buffer,
                                          const float lumAverage) -> void
{
  auto file = std::ofstream{filename};
  if (not file.good())
  {
    std_fmt::println(stderr, "ERROR: Could not open file '{}'.", filename);
    return;
  }

  const auto linearScale   = 0.18F; // MAYBE brightness ??
  const auto finalExposure = linearScale / (lumAverage + 0.0001F);
  file << std_fmt::format("LumAverage    = {:.3f}.\n", lumAverage);
  file << std_fmt::format("FinalExposure = {:.3f}.\n\n", finalExposure);

  auto index = 0U;
  for (auto y = 0; y < GetHeight(); ++y)
  {
    for (auto x = 0; x < GetWidth(); ++x)
    {
      const auto pixel              = buffer[index];
      static constexpr auto CUT_OFF = 256U;
      if ((pixel.R() > CUT_OFF) or (pixel.G() > CUT_OFF) or (pixel.B() > CUT_OFF))
      {
        file << std_fmt::format("[{:4d} {:4d}]  {:6d}, {:6d}, {:6d}, {:6d}\n",
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
