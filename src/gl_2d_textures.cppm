module;

#ifdef TARGET_DARWIN
#define GL_SILENCE_DEPRECATION
#endif

#include "goom_gl.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <span>
#include <stdexcept>

export module Goom.GoomVisualization:Gl2dTextures;

import Goom.GlCaller;
import Goom.GoomVisualization.GlUtils;
import Goom.Lib.GoomUtils;
import :GlslProgram;

export namespace GOOM::OPENGL
{

template<typename CppTextureType,
         uint32_t NumTextures,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
class Gl2DTexture
{
public:
  explicit Gl2DTexture(const GlCaller& glCaller);

  auto Setup(uint32_t textureIndex,
             const char* textureShaderName,
             int32_t textureImageUnit,
             int32_t textureWidth,
             int32_t textureHeight) -> void;

  auto DeleteBuffers() -> void;

  [[nodiscard]] auto GetMappedBuffer(size_t pboIndex) noexcept -> std::span<CppTextureType>;

  [[nodiscard]] auto GetTextureName(size_t textureIndex) const noexcept -> GLuint;

  auto ZeroTextures() -> void;
  auto SetTextureWrapType(GLint textureWrapType) noexcept -> void;
  auto BindTextures(GlslProgram& program) -> void;
  auto BindTexture(GlslProgram& program, uint32_t textureIndex) -> void;
  auto CopyMappedBufferToTexture(size_t pboIndex, size_t textureIndex) -> void;

private:
  static constexpr GLenum TEXTURE_UNIT = GL_TEXTURE0 + TextureLocation;

  const GlCaller* m_gl;
  std::array<const char*, NumTextures> m_textureShaderNames{};
  std::array<int32_t, NumTextures> m_textureImageUnits{};
  int32_t m_textureWidth{};
  int32_t m_textureHeight{};
  GLint m_textureWrapType = GL_MIRRORED_REPEAT;
  size_t m_buffSize{};
  std::array<GLuint, NumTextures> m_textureNames{};
  auto AllocateBuffers() -> void;

  struct PboBuffers
  {
    std::array<GLuint, NumPbos> ids{};
    std::array<CppTextureType*, NumPbos> mappedBuffers{};
  };
  PboBuffers m_pboBuffers{};
  auto AllocatePboBuffers() -> void;
  auto DeletePboBuffers() -> void;
  auto CopyPboBufferToBoundTexture(size_t pboIndex) -> void;
};

} // namespace GOOM::OPENGL

namespace GOOM::OPENGL
{

template<typename CppTextureType,
         uint32_t NumTextures,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
Gl2DTexture<CppTextureType,
            NumTextures,
            TextureLocation,
            TextureFormat,
            TextureInternalFormat,
            TexturePixelType,
            NumPbos>::Gl2DTexture(const GlCaller& glCaller)
  : m_gl{&glCaller}
{
}

template<typename CppTextureType,
         uint32_t NumTextures,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 NumTextures,
                 TextureLocation,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 NumPbos>::Setup(const uint32_t textureIndex,
                                 const char* const textureShaderName,
                                 // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
                                 const int32_t textureImageUnit,
                                 const int32_t textureWidth,
                                 const int32_t textureHeight) -> void
{
  m_textureShaderNames.at(textureIndex) = textureShaderName;
  m_textureImageUnits.at(textureIndex)  = textureImageUnit;
  m_textureWidth                        = textureWidth;
  m_textureHeight                       = textureHeight;
  m_buffSize = static_cast<size_t>(m_textureWidth) * static_cast<size_t>(m_textureHeight);

  m_gl->Call()(glGenTextures, 1, &(m_textureNames.at(textureIndex)));
  m_gl->Call()(glActiveTexture, TEXTURE_UNIT + textureIndex);
  m_gl->Call()(glBindTexture, static_cast<GLenum>(GL_TEXTURE_2D), m_textureNames.at(textureIndex));
  m_gl->Call()(glTexStorage2D,
               static_cast<GLenum>(GL_TEXTURE_2D),
               1,
               TextureInternalFormat,
               m_textureWidth,
               m_textureHeight);
  m_gl->Call()(glTexParameteri,
               static_cast<GLenum>(GL_TEXTURE_2D),
               static_cast<GLenum>(GL_TEXTURE_MIN_FILTER),
               GL_LINEAR);
  m_gl->Call()(glTexParameteri,
               static_cast<GLenum>(GL_TEXTURE_2D),
               static_cast<GLenum>(GL_TEXTURE_MAG_FILTER),
               GL_LINEAR);
  m_gl->Call()(glTexParameteri,
               static_cast<GLenum>(GL_TEXTURE_2D),
               static_cast<GLenum>(GL_TEXTURE_WRAP_S),
               m_textureWrapType);
  m_gl->Call()(glTexParameteri,
               static_cast<GLenum>(GL_TEXTURE_2D),
               static_cast<GLenum>(GL_TEXTURE_WRAP_T),
               m_textureWrapType);

  if (m_textureImageUnits.at(textureIndex) != -1)
  {
    m_gl->Call()(glBindImageTexture,
                 static_cast<GLuint>(m_textureImageUnits.at(textureIndex)),
                 m_textureNames.at(textureIndex),
                 0,
                 static_cast<GLboolean>(GL_FALSE),
                 0,
                 static_cast<GLenum>(GL_READ_WRITE),
                 TextureInternalFormat);
  }

  if (0 == textureIndex)
  {
    AllocateBuffers();
  }
}

template<typename CppTextureType,
         uint32_t NumTextures,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 NumTextures,
                 TextureLocation,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 NumPbos>::AllocateBuffers() -> void
{
  if constexpr (0 == NumPbos)
  {
    return;
  }

  AllocatePboBuffers();
}

template<typename CppTextureType,
         uint32_t NumTextures,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 NumTextures,
                 TextureLocation,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 NumPbos>::DeleteBuffers() -> void
{
  if constexpr (0 == NumPbos)
  {
    std::ranges::for_each(m_textureNames,
                          [this](auto& textureName)
                          { m_gl->Call()(glDeleteTextures, 1, &textureName); });
    return;
  }

  DeletePboBuffers();

  std::ranges::for_each(m_textureNames,
                        [this](auto& textureName)
                        { m_gl->Call()(glDeleteTextures, 1, &textureName); });
}

template<typename CppTextureType,
         uint32_t NumTextures,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 NumTextures,
                 TextureLocation,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 NumPbos>::BindTextures(GlslProgram& program) -> void
{
  for (auto i = 0U; i < m_textureNames.size(); ++i)
  {
    BindTexture(program, i);
  }
}

template<typename CppTextureType,
         uint32_t NumTextures,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 NumTextures,
                 TextureLocation,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 NumPbos>::BindTexture(GlslProgram& program, const uint32_t textureIndex) -> void
{
  if (m_textureShaderNames.at(textureIndex)[0] != '\0')
  {
    program.SetUniform(m_textureShaderNames.at(textureIndex),
                       TextureLocation + static_cast<int32_t>(textureIndex));
  }

  m_gl->Call()(glActiveTexture, TEXTURE_UNIT + textureIndex);

  m_gl->Call()(glTexParameteri,
               static_cast<GLenum>(GL_TEXTURE_2D),
               static_cast<GLenum>(GL_TEXTURE_WRAP_S),
               m_textureWrapType);
  m_gl->Call()(glTexParameteri,
               static_cast<GLenum>(GL_TEXTURE_2D),
               static_cast<GLenum>(GL_TEXTURE_WRAP_T),
               m_textureWrapType);

  m_gl->Call()(glBindTexture, static_cast<GLenum>(GL_TEXTURE_2D), m_textureNames.at(textureIndex));
}

template<typename CppTextureType,
         uint32_t NumTextures,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 NumTextures,
                 TextureLocation,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 NumPbos>::GetTextureName(const size_t textureIndex) const noexcept -> GLuint
{
  return m_textureNames.at(textureIndex);
}

template<typename CppTextureType,
         uint32_t NumTextures,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 NumTextures,
                 TextureLocation,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 NumPbos>::GetMappedBuffer(const size_t pboIndex) noexcept
    -> std::span<CppTextureType>
{
  return std::span<CppTextureType>{m_pboBuffers.mappedBuffers.at(pboIndex), m_buffSize};
}

template<typename CppTextureType,
         uint32_t NumTextures,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 NumTextures,
                 TextureLocation,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 NumPbos>::SetTextureWrapType(const GLint textureWrapType) noexcept -> void
{
  m_textureWrapType = textureWrapType;
}

template<typename CppTextureType,
         uint32_t NumTextures,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 NumTextures,
                 TextureLocation,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 NumPbos>::ZeroTextures() -> void
{
  //  const auto zero = int64_t{0xFF000000};
  //  GlCall(glClearTexImage(m_textureName, 0, TextureFormat, TexturePixelType, &zero));
  //  GlCall(glClearTexSubImage(m_textureName,
  //                            0,
  //                            0,
  //                            0,
  //                            0,
  //                            m_textureWidth,
  //                            m_textureHeight,
  //                            TextureFormat,
  //                            TexturePixelType,
  //                            0,
  //                            nullptr));
  std::ranges::for_each(
      m_textureNames,
      [this](const auto textureName)
      { m_gl->Call()(glClearTexImage, textureName, 0, TextureFormat, TexturePixelType, nullptr); });
}

template<typename CppTextureType,
         uint32_t NumTextures,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 NumTextures,
                 TextureLocation,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
                 NumPbos>::CopyMappedBufferToTexture(const size_t pboIndex,
                                                     const size_t textureIndex) -> void
{
  m_gl->Call()(glBindTexture, static_cast<GLenum>(GL_TEXTURE_2D), m_textureNames.at(textureIndex));

  /**
  if constexpr (0 == NumPbos)
  {
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    m_textureWidth,
                    m_textureHeight,
                    TextureFormat,
                    TexturePixelType,
                    buffer);
    return;
  }
   **/

  CopyPboBufferToBoundTexture(pboIndex);
}

template<typename CppTextureType,
         uint32_t NumTextures,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 NumTextures,
                 TextureLocation,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 NumPbos>::AllocatePboBuffers() -> void
{
  m_gl->Call()(glGenBuffers, static_cast<GLsizei>(NumPbos), m_pboBuffers.ids.data());

  for (auto i = 0U; i < NumPbos; ++i)
  {
    m_gl->Call()(glBindBuffer, static_cast<GLenum>(GL_PIXEL_UNPACK_BUFFER), m_pboBuffers.ids.at(i));
    m_gl->Call()(glBufferStorage,
                 static_cast<GLenum>(GL_PIXEL_UNPACK_BUFFER),
                 static_cast<GLsizeiptr>(m_buffSize * sizeof(CppTextureType)),
                 nullptr,
                 static_cast<GLbitfield>(GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT |
                                         GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT |
                                         GL_CLIENT_STORAGE_BIT));

    m_pboBuffers.mappedBuffers.at(i) = ptr_cast<CppTextureType*>(
        glMapBufferRange(GL_PIXEL_UNPACK_BUFFER,
                         0,
                         static_cast<GLsizeiptr>(m_buffSize * sizeof(CppTextureType)),
                         GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));
    if (nullptr == m_pboBuffers.mappedBuffers.at(i))
    {
      throw std::runtime_error(std::format("Could not allocate mapped buffer for pbo {}.", i));
    }

    m_gl->Call()(glBindBuffer, static_cast<GLenum>(GL_PIXEL_UNPACK_BUFFER), 0U);
  }
}

template<typename CppTextureType,
         uint32_t NumTextures,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 NumTextures,
                 TextureLocation,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 NumPbos>::DeletePboBuffers() -> void
{
  for (auto i = 0U; i < NumPbos; ++i)
  {
    m_gl->Call()(glBindBuffer, static_cast<GLenum>(GL_PIXEL_UNPACK_BUFFER), m_pboBuffers.ids.at(i));
    m_gl->Call()(glUnmapBuffer, static_cast<GLenum>(GL_PIXEL_UNPACK_BUFFER));
    m_gl->Call()(glDeleteBuffers, 1, &m_pboBuffers.ids.at(i));
  }
  m_gl->Call()(glBindBuffer, static_cast<GLenum>(GL_PIXEL_UNPACK_BUFFER), 0U);
}

template<typename CppTextureType,
         uint32_t NumTextures,
         int32_t TextureLocation,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 NumTextures,
                 TextureLocation,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 NumPbos>::CopyPboBufferToBoundTexture(const size_t pboIndex) -> void
{
  m_gl->Call()(
      glBindBuffer, static_cast<GLenum>(GL_PIXEL_UNPACK_BUFFER), m_pboBuffers.ids.at(pboIndex));

  static constexpr GLint LEVEL    = 0;
  static constexpr GLint X_OFFSET = 0;
  static constexpr GLint Y_OFFSET = 0;
  m_gl->Call()(glTexSubImage2D,
               static_cast<GLenum>(GL_TEXTURE_2D),
               LEVEL,
               X_OFFSET,
               Y_OFFSET,
               m_textureWidth,
               m_textureHeight,
               TextureFormat,
               TexturePixelType,
               nullptr);

  m_gl->Call()(glBindBuffer, static_cast<GLenum>(GL_PIXEL_UNPACK_BUFFER), 0U);
}

} // namespace GOOM::OPENGL
