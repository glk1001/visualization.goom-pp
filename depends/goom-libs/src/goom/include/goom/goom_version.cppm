module;

#include <cstdint>
#include <string>

export module Goom.Lib.GoomVersion;

export namespace GOOM
{

inline constexpr auto* GOOM_VER_NAME        = "Goom++ Library";
inline constexpr auto* GOOM_VER_DESCRIPTION = "Goom++ Initial Release";
inline constexpr auto GOOM_VER_MAJOR        = 1;
inline constexpr auto GOOM_VER_MINOR        = 0;
inline constexpr auto GOOM_VER_REVISION     = 0;

struct VersionInfoInt
{
  int32_t verMajor;
  int32_t verMinor;
  int32_t verRevision;
};

[[nodiscard]] auto GetFullVersionStr() -> std::string;

[[nodiscard]] inline auto GetVersionInfo() -> VersionInfoInt
{
  return {.verMajor = GOOM_VER_MAJOR, .verMinor = GOOM_VER_MINOR, .verRevision = GOOM_VER_REVISION};
}

} // namespace GOOM
