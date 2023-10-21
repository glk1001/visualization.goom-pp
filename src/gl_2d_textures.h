#pragma once

#ifdef TARGET_DARWIN
#define GL_SILENCE_DEPRECATION
#endif

#include "gl_utils.h"
#include "glsl_program.h"
#include "goom/goom_utils.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <format> // NOLINT: Waiting to use C++20.
#include <span> // NOLINT: Waiting to use C++20.
#include <stdexcept>

namespace GOOM::OPENGL
{

template<typename CppTextureType,
         int32_t TextureImageUint,
         int32_t TextureLocation,
         uint32_t NumTextures,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
class Gl2DTexture
{
public:
  Gl2DTexture() = default;

  auto Setup(uint32_t textureIndex,
             const char* textureShaderName,
             int32_t textureWidth,
             int32_t textureHeight) -> void;

  auto DeleteBuffers() -> void;

  [[nodiscard]] auto GetTextureName() const noexcept -> GLuint;
  // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
  [[nodiscard]] auto GetMappedBuffer(size_t pboIndex) noexcept -> std_spn::span<CppTextureType>;

  [[nodiscard]] auto GetCurrentTextureIndex() const noexcept { return m_currentTextureIndex; }
  auto RotateCurrentTextureName() noexcept -> void;

  auto ZeroTextureData() noexcept -> void;
  auto CopyMappedBufferToTexture(size_t pboIndex) noexcept -> void;
  auto BindTexture(GlslProgram& program) noexcept -> void;

private:
  static constexpr GLenum TEXTURE_UNIT = GL_TEXTURE0 + TextureLocation;

  std::array<const char*, NumTextures> m_textureShaderNames{};
  int32_t m_textureWidth{};
  int32_t m_textureHeight{};
  size_t m_buffSize{};
  std::array<GLuint, NumTextures> m_textureNames{};
  uint32_t m_currentTextureIndex = 0U;
  auto AllocateBuffers() -> void;

  struct PboBuffers
  {
    std::array<GLuint, NumPbos> ids{};
    std::array<CppTextureType*, NumPbos> mappedBuffers{};
  };
  PboBuffers m_pboBuffers{};
  auto AllocatePboBuffers() -> void;
  auto DeletePboBuffers() -> void;
  auto CopyPboBufferToTexture(size_t pboIndex) noexcept -> void;
};

template<typename CppTextureType,
         int32_t TextureImageUint,
         int32_t TextureLocation,
         uint32_t NumTextures,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 TextureImageUint,
                 TextureLocation,
                 NumTextures,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 NumPbos>::Setup(const uint32_t textureIndex,
                                 const char* const textureShaderName,
                                 // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
                                 const int32_t textureWidth,
                                 const int32_t textureHeight) -> void
{
  m_currentTextureIndex = textureIndex;
  //m_currentTextureIndex = 0;

  m_textureShaderNames.at(textureIndex) = textureShaderName;
  m_textureWidth                        = textureWidth;
  m_textureHeight                       = textureHeight;
  m_buffSize = static_cast<size_t>(m_textureWidth) * static_cast<size_t>(m_textureHeight);

  GlCall(glGenTextures(1, &(m_textureNames.at(textureIndex))));
  GlCall(glActiveTexture(TEXTURE_UNIT + static_cast<GLenum>(textureIndex)));
  GlCall(glBindTexture(GL_TEXTURE_2D, m_textureNames.at(textureIndex)));
  GlCall(glTexStorage2D(GL_TEXTURE_2D, 1, TextureInternalFormat, m_textureWidth, m_textureHeight));
  GlCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
  GlCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
  GlCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT));
  GlCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));

  if constexpr (TextureImageUint != -1)
  {
    GlCall(glBindImageTexture(TextureImageUint,
                              m_textureNames.at(textureIndex),
                              0,
                              GL_FALSE,
                              0,
                              GL_READ_WRITE,
                              TextureInternalFormat));
  }

  if (0 == textureIndex)
  {
    AllocateBuffers();
  }
}

template<typename CppTextureType,
         int32_t TextureImageUint,
         int32_t TextureLocation,
         uint32_t NumTextures,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 TextureImageUint,
                 TextureLocation,
                 NumTextures,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 NumPbos>::RotateCurrentTextureName() noexcept -> void
{
  ++m_currentTextureIndex;
  if (m_currentTextureIndex >= m_textureNames.size())
  {
    m_currentTextureIndex = 0;
  }
  //m_currentTextureIndex = 0;
}

template<typename CppTextureType,
         int32_t TextureImageUint,
         int32_t TextureLocation,
         uint32_t NumTextures,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 TextureImageUint,
                 TextureLocation,
                 NumTextures,
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
         int32_t TextureImageUint,
         int32_t TextureLocation,
         uint32_t NumTextures,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 TextureImageUint,
                 TextureLocation,
                 NumTextures,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 NumPbos>::DeleteBuffers() -> void
{
  if constexpr (0 == NumPbos)
  {
    std::for_each(begin(m_textureNames),
                  end(m_textureNames),
                  [](auto& textureName) { GlCall(glDeleteTextures(1, &textureName)); });
    return;
  }

  DeletePboBuffers();

  std::for_each(begin(m_textureNames),
                end(m_textureNames),
                [](auto& textureName) { GlCall(glDeleteTextures(1, &textureName)); });
}

template<typename CppTextureType,
         int32_t TextureImageUint,
         int32_t TextureLocation,
         uint32_t NumTextures,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 TextureImageUint,
                 TextureLocation,
                 NumTextures,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 NumPbos>::BindTexture(GlslProgram& program) noexcept -> void
{
  for (auto i = 0U; i < m_textureNames.size(); ++i)
  {
    program.SetUniform(m_textureShaderNames.at(i), TextureLocation + static_cast<int32_t>(i));

    GlCall(glActiveTexture(TEXTURE_UNIT + static_cast<GLenum>(i)));
    GlCall(glBindTexture(GL_TEXTURE_2D, m_textureNames.at(i)));
  }
}

template<typename CppTextureType,
         int32_t TextureImageUint,
         int32_t TextureLocation,
         uint32_t NumTextures,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
inline auto Gl2DTexture<CppTextureType,
                        TextureImageUint,
                        TextureLocation,
                        NumTextures,
                        TextureFormat,
                        TextureInternalFormat,
                        TexturePixelType,
                        NumPbos>::GetTextureName() const noexcept -> GLuint
{
  return m_textureNames.at(m_currentTextureIndex);
}

template<typename CppTextureType,
         int32_t TextureImageUint,
         int32_t TextureLocation,
         uint32_t NumTextures,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
inline auto Gl2DTexture<CppTextureType,
                        TextureImageUint,
                        TextureLocation,
                        NumTextures,
                        TextureFormat,
                        TextureInternalFormat,
                        TexturePixelType,
                        NumPbos>::GetMappedBuffer(const size_t pboIndex) noexcept
    // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
    -> std_spn::span<CppTextureType>
{
  // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
  return std_spn::span<CppTextureType>{m_pboBuffers.mappedBuffers.at(pboIndex), m_buffSize};
}

template<typename CppTextureType,
         int32_t TextureImageUint,
         int32_t TextureLocation,
         uint32_t NumTextures,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
inline auto Gl2DTexture<CppTextureType,
                        TextureImageUint,
                        TextureLocation,
                        NumTextures,
                        TextureFormat,
                        TextureInternalFormat,
                        TexturePixelType,
                        NumPbos>::ZeroTextureData() noexcept -> void
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
  std::for_each(begin(m_textureNames),
                end(m_textureNames),
                [](const auto textureName) {
                  GlCall(glClearTexImage(textureName, 0, TextureFormat, TexturePixelType, nullptr));
                });
}

template<typename CppTextureType,
         int32_t TextureImageUint,
         int32_t TextureLocation,
         uint32_t NumTextures,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
inline auto Gl2DTexture<CppTextureType,
                        TextureImageUint,
                        TextureLocation,
                        NumTextures,
                        TextureFormat,
                        TextureInternalFormat,
                        TexturePixelType,
                        NumPbos>::CopyMappedBufferToTexture(const size_t pboIndex) noexcept -> void
{
  GlCall(glBindTexture(GL_TEXTURE_2D, m_textureNames.at(m_currentTextureIndex)));

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

  CopyPboBufferToTexture(pboIndex);
}

template<typename CppTextureType,
         int32_t TextureImageUint,
         int32_t TextureLocation,
         uint32_t NumTextures,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 TextureImageUint,
                 TextureLocation,
                 NumTextures,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 NumPbos>::AllocatePboBuffers() -> void
{
  GlCall(glGenBuffers(NumPbos, m_pboBuffers.ids.data()));

  for (auto i = 0U; i < NumPbos; ++i)
  {
    GlCall(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboBuffers.ids.at(i)));
    GlCall(glBufferStorage(GL_PIXEL_UNPACK_BUFFER,
                           static_cast<GLsizeiptr>(m_buffSize * sizeof(CppTextureType)),
                           nullptr,
                           GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT |
                               GL_MAP_COHERENT_BIT | GL_CLIENT_STORAGE_BIT));

    m_pboBuffers.mappedBuffers.at(i) = ptr_cast<CppTextureType*>(
        glMapBufferRange(GL_PIXEL_UNPACK_BUFFER,
                         0,
                         static_cast<GLsizeiptr>(m_buffSize * sizeof(CppTextureType)),
                         GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));
    if (nullptr == m_pboBuffers.mappedBuffers.at(i))
    {
      // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
      throw std::runtime_error(std_fmt::format("Could not allocate mapped buffer for pbo {}.", i));
    }

    GlCall(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
  }
}

template<typename CppTextureType,
         int32_t TextureImageUint,
         int32_t TextureLocation,
         uint32_t NumTextures,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 TextureImageUint,
                 TextureLocation,
                 NumTextures,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 NumPbos>::DeletePboBuffers() -> void
{
  for (auto i = 0U; i < NumPbos; ++i)
  {
    GlCall(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboBuffers.ids.at(i)));
    GlCall(glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER));
    GlCall(glDeleteBuffers(1, &m_pboBuffers.ids.at(i)));
  }
  GlCall(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
}

template<typename CppTextureType,
         int32_t TextureImageUint,
         int32_t TextureLocation,
         uint32_t NumTextures,
         GLenum TextureFormat,
         GLenum TextureInternalFormat,
         GLenum TexturePixelType,
         uint32_t NumPbos>
auto Gl2DTexture<CppTextureType,
                 TextureImageUint,
                 TextureLocation,
                 NumTextures,
                 TextureFormat,
                 TextureInternalFormat,
                 TexturePixelType,
                 NumPbos>::CopyPboBufferToTexture(const size_t pboIndex) noexcept -> void
{
  GlCall(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboBuffers.ids.at(pboIndex)));

  static constexpr GLint LEVEL    = 0;
  static constexpr GLint X_OFFSET = 0;
  static constexpr GLint Y_OFFSET = 0;
  GlCall(glTexSubImage2D(GL_TEXTURE_2D,
                         LEVEL,
                         X_OFFSET,
                         Y_OFFSET,
                         m_textureWidth,
                         m_textureHeight,
                         TextureFormat,
                         TexturePixelType,
                         nullptr));

  GlCall(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
}

} // namespace GOOM::OPENGL
