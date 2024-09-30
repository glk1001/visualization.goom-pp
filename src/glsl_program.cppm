module;

#ifdef WIN32
#pragma warning(disable : 4290)
#endif

#include "goom_gl.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <glm/ext/matrix_float3x3.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

export module Goom.GoomVisualization:GlslProgram;

import Goom.Lib.AssertUtils;

export namespace GOOM::OPENGL
{

class GlslProgramException : public std::runtime_error
{
public:
  explicit GlslProgramException(const std::string& msg) : std::runtime_error(msg) {}
};

class GlslProgram
{
public:
  GlslProgram();
  GlslProgram(const GlslProgram&) = delete;
  GlslProgram(GlslProgram&&)      = delete;
  ~GlslProgram();
  auto operator=(const GlslProgram&) -> GlslProgram& = delete;
  auto operator=(GlslProgram&&) -> GlslProgram&      = delete;

  auto DeleteProgram() -> void;

  auto CompileShader(const std::string& fileName) -> void;
  auto LinkShader() -> void;
  auto ValidateShader() const -> void;

  auto Use() const -> void;
  [[nodiscard]] auto IsInUse() const noexcept -> bool;

  auto SetUniform(const std::string_view& name, int x, int y, int z) -> void;
  auto SetUniform(const std::string_view& name, float x, float y, float z) -> void;
  auto SetUniform(const std::string_view& name, const glm::ivec2& vec) -> void;
  auto SetUniform(const std::string_view& name, const glm::ivec3& vec) -> void;
  auto SetUniform(const std::string_view& name, const glm::ivec4& vec) -> void;
  auto SetUniform(const std::string_view& name, const glm::vec2& vec) -> void;
  auto SetUniform(const std::string_view& name, const glm::vec3& vec) -> void;
  auto SetUniform(const std::string_view& name, const glm::vec4& vec) -> void;
  auto SetUniform(const std::string_view& name, const glm::mat3& mat) -> void;
  auto SetUniform(const std::string_view& name, const glm::mat4& mat) -> void;
  auto SetUniform(const std::string_view& name, int val) -> void;
  auto SetUniform(const std::string_view& name, GLuint val) -> void;
  auto SetUniform(const std::string_view& name, bool val) -> void;
  auto SetUniform(const std::string_view& name, float val) -> void;
  auto SetUniform(const std::string_view& name, const std::vector<int>& val) -> void;
  auto SetUniform(const std::string_view& name, const std::vector<float>& val) -> void;

private:
  GLuint m_handle = 0;
  bool m_linked   = false;

  std::unordered_map<std::string, int> m_uniformLocations;
  auto FindUniformLocations() -> void;
  [[nodiscard]] auto GetUniformLocation(const std::string_view& name) -> GLint;

  auto CompileShader(const std::string& filename, GLenum type) -> void;
  auto CompileShader(const std::string& source, GLenum type, const std::string& filename) -> void;
  [[nodiscard]] auto IsLinked() const noexcept -> bool;
  auto DetachAndDeleteShaderObjects() const -> void;

  [[nodiscard]] static auto FileExists(const std::string_view& filename) -> bool;
  [[nodiscard]] static auto GetExtension(const std::string_view& filename) -> std::string;
};

} // namespace GOOM::OPENGL

namespace GOOM::OPENGL
{

namespace
{

// NOLINTNEXTLINE(cert-err58-cpp)
const std::unordered_map<std::string, int32_t> EXTENSIONS = {
    {     ".vert",          GL_VERTEX_SHADER},
    {"_vert.glsl",          GL_VERTEX_SHADER},
    {".vert.glsl",          GL_VERTEX_SHADER},
    {     ".geom",        GL_GEOMETRY_SHADER},
    {".geom.glsl",        GL_GEOMETRY_SHADER},
    {      ".tcs",    GL_TESS_CONTROL_SHADER},
    { ".tcs.glsl",    GL_TESS_CONTROL_SHADER},
    {      ".tes", GL_TESS_EVALUATION_SHADER},
    { ".tes.glsl", GL_TESS_EVALUATION_SHADER},
    {     ".frag",        GL_FRAGMENT_SHADER},
    {"_frag.glsl",        GL_FRAGMENT_SHADER},
    {".frag.glsl",        GL_FRAGMENT_SHADER},
    {     ".comp",         GL_COMPUTE_SHADER},
    {  ".cs.glsl",         GL_COMPUTE_SHADER}
};

} // namespace

GlslProgram::GlslProgram() = default;

GlslProgram::~GlslProgram()
{
  DeleteProgram();
}

auto GlslProgram::DeleteProgram() -> void
{
  if (m_handle == 0)
  {
    return;
  }
  DetachAndDeleteShaderObjects();
  glDeleteProgram(m_handle);
  m_handle = 0;
}

auto GlslProgram::DetachAndDeleteShaderObjects() const -> void
{
  // Detach and delete the shader objects (if they are not already removed).
  auto numShaders = GLint{};
  glGetProgramiv(m_handle, GL_ATTACHED_SHADERS, &numShaders);

  auto shaderNames = std::vector<GLuint>(static_cast<GLuint>(numShaders));
  glGetAttachedShaders(m_handle, numShaders, nullptr, shaderNames.data());

  for (const auto shader : shaderNames)
  {
    glDetachShader(m_handle, shader);
    glDeleteShader(shader);
  }
}

auto GlslProgram::CompileShader(const std::string& fileName) -> void
{

  // Check the file name's extension to determine the shader type
  const auto ext = GetExtension(fileName);
  auto type      = GL_VERTEX_SHADER;
  if (const auto it = EXTENSIONS.find(ext); it != EXTENSIONS.end())
  {
    type = it->second;
  }
  else
  {
    throw GlslProgramException("Unrecognized extension: " + ext);
  }

  // Pass the discovered shader type along
  CompileShader(fileName, static_cast<GLenum>(type));
}

auto GlslProgram::GetExtension(const std::string_view& filename) -> std::string
{
  const auto nameStr = std::string{filename};

  if (const auto dotLoc = nameStr.find_last_of('.'); dotLoc != std::string::npos)
  {
    auto ext = nameStr.substr(dotLoc);
    if (ext != ".glsl")
    {
      return ext;
    }

    auto loc = nameStr.find_last_of('.', dotLoc - 1);
    if (loc == std::string::npos)
    {
      loc = nameStr.find_last_of('_', dotLoc - 1);
    }
    if (loc != std::string::npos)
    {
      return nameStr.substr(loc);
    }
  }

  return "";
}

auto GlslProgram::CompileShader(const std::string& filename, const GLenum type) -> void
{
  if (not FileExists(filename))
  {
    throw GlslProgramException(std::string{"Shader: "} + filename + " not found.");
  }

  if (m_handle <= 0)
  {
    m_handle = glCreateProgram();
    if (m_handle == 0)
    {
      throw GlslProgramException("Unable to create shader program.");
    }
  }

  auto inFile = std::ifstream{filename, std::ios::in};
  if (not inFile)
  {
    throw GlslProgramException(std::string{"Unable to open: "} + filename);
  }

  // Get file contents
  std::stringstream code;
  code << inFile.rdbuf();
  inFile.close();

  CompileShader(code.str(), type, filename);
}

auto GlslProgram::CompileShader(const std::string& source,
                                const GLenum type,
                                const std::string& filename) -> void
{
  if (m_handle <= 0)
  {
    m_handle = glCreateProgram();
    if (m_handle == 0)
    {
      throw GlslProgramException("Unable to create shader program.");
    }
  }

  const auto shaderHandle = glCreateShader(type);
  const auto* cCode       = source.c_str();
  glShaderSource(shaderHandle, 1, &cCode, nullptr);
  glCompileShader(shaderHandle);

  // Check for errors
  auto result = int{};
  glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &result);
  if (GL_FALSE == result)
  {
    // Compile failed, get log
    auto msg = std::string{};
    if (not filename.empty())
    {
      msg = filename + ": shader compilation failed\n";
    }
    else
    {
      msg = "Shader compilation failed.\n";
    }

    auto length = 0;
    glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &length);
    if (length > 0)
    {
      auto log     = std::string(static_cast<size_t>(length), ' ');
      auto written = 0;
      glGetShaderInfoLog(shaderHandle, length, &written, log.data());
      msg += log;
    }
    throw GlslProgramException(msg);
  }

  // Compile succeeded, attach shader
  glAttachShader(m_handle, shaderHandle);
}

auto GlslProgram::LinkShader() -> void
{
  if (m_linked)
  {
    return;
  }
  if (m_handle <= 0)
  {
    throw GlslProgramException("Program has not been compiled.");
  }

  glLinkProgram(m_handle);

  auto status = 0;
  glGetProgramiv(m_handle, GL_LINK_STATUS, &status);

  auto errString = std::string{};
  if (GL_FALSE == status)
  {
    // Store log and return false
    auto length = 0;
    glGetProgramiv(m_handle, GL_INFO_LOG_LENGTH, &length);
    errString += "Program LinkShader failed:\n";
    if (length > 0)
    {
      auto log     = std::string(static_cast<size_t>(length), ' ');
      auto written = 0;
      glGetProgramInfoLog(m_handle, length, &written, log.data());
      errString += log;
    }
  }
  else
  {
    FindUniformLocations();
    m_linked = true;
  }

  DetachAndDeleteShaderObjects();

  if (GL_FALSE == status)
  {
    throw GlslProgramException(errString);
  }
}

auto GlslProgram::FindUniformLocations() -> void
{
  m_uniformLocations.clear();

  auto numUniforms = GLint{0};
#ifdef __APPLE__
  // For OpenGL 4.1, use glGetActiveUniform
  GLint maxLen;
  GLchar* name;

  glGetProgramiv(handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen);
  glGetProgramiv(handle, GL_ACTIVE_UNIFORMS, &numUniforms);

  name = new GLchar[maxLen];
  for (GLuint i = 0; i < numUniforms; ++i)
  {
    GLint size;
    GLenum type;
    GLsizei written;
    glGetActiveUniform(handle, i, maxLen, &written, &size, &type, name);
    GLint location         = glGetUniformLocation(handle, name);
    uniformLocations[name] = glGetUniformLocation(handle, name);
  }
  delete[] name;
#else
  // For OpenGL 4.3 and above, use glGetProgramResource
  glGetProgramInterfaceiv(m_handle, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);
  //std_fmt::println("numUniforms = {}", numUniforms);

  //auto numUniformBlocks = GLint{0};
  //glGetProgramInterfaceiv(m_handle, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &numUniformBlocks);
  //std_fmt::println("numUniformBlocks = {}", numUniformBlocks);

  static constexpr auto NUM_PROPERTIES = 4;
  static constexpr auto PROPERTIES =
      std::array<GLenum, NUM_PROPERTIES>{GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_BLOCK_INDEX};

  for (GLint i = 0; i < numUniforms; ++i)
  {
    static constexpr auto NUM_RESULTS = 4;
    auto results                      = std::array<GLint, NUM_RESULTS>{};
    glGetProgramResourceiv(m_handle,
                           GL_UNIFORM,
                           static_cast<GLuint>(i),
                           NUM_PROPERTIES,
                           PROPERTIES.data(),
                           NUM_RESULTS,
                           nullptr,
                           results.data());

    if (results[3] != -1)
    {
      //std_fmt::println("Skipped uniform {}", i);
      continue; // Skip uniforms in blocks
    }
    const auto nameBufSize = results[0] + 1;
    auto name              = std::vector<char>(static_cast<size_t>(nameBufSize));
    glGetProgramResourceName(
        m_handle, GL_UNIFORM, static_cast<GLuint>(i), nameBufSize, nullptr, name.data());
    m_uniformLocations[name.data()] = results[2];
    //std_fmt::println("Uniform {}, name = {}, value = {}", i, name.data(), results[2]);
  }
#endif
}

auto GlslProgram::Use() const -> void
{
  if ((m_handle <= 0) or (not m_linked))
  {
    throw GlslProgramException("Shader has not been linked.");
  }
  glUseProgram(m_handle);
}

auto GlslProgram::IsInUse() const noexcept -> bool
{
  GLint currentProgram{};
  glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
  return m_handle == static_cast<GLuint>(currentProgram);
}

auto GlslProgram::IsLinked() const noexcept -> bool
{
  return m_linked;
}

auto GlslProgram::SetUniform(const std::string_view& name, const int x, const int y, const int z)
    -> void
{
  const auto loc = GetUniformLocation(name);
  glUniform3i(loc, x, y, z);
}

auto GlslProgram::SetUniform(const std::string_view& name,
                             const float x,
                             const float y,
                             const float z) -> void
{
  const auto loc = GetUniformLocation(name);
  glUniform3f(loc, x, y, z);
}

auto GlslProgram::SetUniform(const std::string_view& name, const glm::ivec2& vec) -> void
{
  const auto loc = GetUniformLocation(name);
  glUniform2i(loc, vec.x, vec.y); // NOLINT: union hard to fix here
}

auto GlslProgram::SetUniform(const std::string_view& name, const glm::ivec3& vec) -> void
{
  SetUniform(name, vec.x, vec.y, vec.z); // NOLINT: union hard to fix here
}

auto GlslProgram::SetUniform(const std::string_view& name, const glm::ivec4& vec) -> void
{
  const auto loc = GetUniformLocation(name);
  glUniform4i(loc, vec.x, vec.y, vec.z, vec.w); // NOLINT: union hard to fix here
}

auto GlslProgram::SetUniform(const std::string_view& name, const glm::vec2& vec) -> void
{
  const auto loc = GetUniformLocation(name);
  glUniform2f(loc, vec.x, vec.y); // NOLINT: union hard to fix here
}

auto GlslProgram::SetUniform(const std::string_view& name, const glm::vec3& vec) -> void
{
  SetUniform(name, vec.x, vec.y, vec.z); // NOLINT: union hard to fix here
}

auto GlslProgram::SetUniform(const std::string_view& name, const glm::vec4& vec) -> void
{
  const auto loc = GetUniformLocation(name);
  glUniform4f(loc, vec.x, vec.y, vec.z, vec.w); // NOLINT: union hard to fix here
}

auto GlslProgram::SetUniform(const std::string_view& name, const glm::mat3& mat) -> void
{
  const auto loc = GetUniformLocation(name);
  glUniformMatrix3fv(loc, 1, GL_FALSE, &mat[0][0]);
}

auto GlslProgram::SetUniform(const std::string_view& name, const glm::mat4& mat) -> void
{
  const auto loc = GetUniformLocation(name);
  glUniformMatrix4fv(loc, 1, GL_FALSE, &mat[0][0]);
}

auto GlslProgram::SetUniform(const std::string_view& name, const int val) -> void
{
  const auto loc = GetUniformLocation(name);
  glUniform1i(loc, val);
}

auto GlslProgram::SetUniform(const std::string_view& name, const GLuint val) -> void
{
  const auto loc = GetUniformLocation(name);
  glUniform1ui(loc, val);
}

auto GlslProgram::SetUniform(const std::string_view& name, const bool val) -> void
{
  const auto loc = GetUniformLocation(name);
  glUniform1i(loc, static_cast<GLint>(val));
}

auto GlslProgram::SetUniform(const std::string_view& name, const float val) -> void
{
  const auto loc = GetUniformLocation(name);
  glUniform1f(loc, val);
}

auto GlslProgram::SetUniform(const std::string_view& name, const std::vector<int>& val) -> void
{
  switch (val.size())
  {
    case 1:
      SetUniform(name, val.front());
      break;
    case 2:
      SetUniform(name, glm::ivec2{val[0], val[1]});
      break;
    case 3:
      SetUniform(name, val[0], val[1], val[2]);
      break;
    case 4:
      SetUniform(name, glm::ivec4{val[0], val[1], val[2], val[3]});
      break;
    default:
      std::unreachable();
  }
}

auto GlslProgram::SetUniform(const std::string_view& name, const std::vector<float>& val) -> void
{
  switch (val.size())
  {
    case 1:
      SetUniform(name, val.front());
      break;
    case 2:
      SetUniform(name, glm::vec2{val[0], val[1]});
      break;
    case 3:
      SetUniform(name, val[0], val[1], val[2]);
      break;
    case 4:
      SetUniform(name, glm::vec4{val[0], val[1], val[2], val[3]});
      break;
    default:
      std::unreachable();
  }
}

auto GlslProgram::ValidateShader() const -> void
{
  if (not IsLinked())
  {
    throw GlslProgramException("Program is not linked");
  }

  auto status = GLint{};
  glValidateProgram(m_handle);
  glGetProgramiv(m_handle, GL_VALIDATE_STATUS, &status);

  if (GL_FALSE == status)
  {
    // Store log and return false
    auto length    = 0;
    auto logString = std::string{};

    glGetProgramiv(m_handle, GL_INFO_LOG_LENGTH, &length);

    if (length > 0)
    {
      auto cLog    = std::vector<char>(static_cast<size_t>(length));
      auto written = 0;
      glGetProgramInfoLog(m_handle, length, &written, cLog.data());
      logString = cLog.data();
    }

    throw GlslProgramException(std::string{"Program failed to ValidateShader\n"} + logString);
  }
}

auto GlslProgram::FileExists(const std::string_view& filename) -> bool
{
  return std::filesystem::exists(filename);
}

auto GlslProgram::GetUniformLocation(const std::string_view& name) -> GLint
{
  const auto nameStr = std::string{name};

  const auto pos = m_uniformLocations.find(nameStr);
  if (pos == m_uniformLocations.end())
  {
    const auto loc              = glGetUniformLocation(m_handle, nameStr.c_str());
    m_uniformLocations[nameStr] = loc;

    Expects(loc != -1, nameStr);

    return loc;
  }

  return pos->second;
}

} // namespace GOOM::OPENGL
