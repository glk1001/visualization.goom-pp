module;

#include <array>
#include <chrono>
#include <ctime>
#include <string>

export module Goom.Utils.DateUtils;

export namespace GOOM::UTILS
{

[[nodiscard]] auto GetCurrentDateTimeAsString() noexcept -> std::string;

[[nodiscard]] auto GetStandardDateTimeString(
    const std::string& otherFormatDateTime, const std::string& otherFormat) noexcept -> std::string;

[[nodiscard]] auto GetSteadyClockAsString(
    const std::chrono::steady_clock::time_point& timePoint) noexcept -> std::string;

} // namespace GOOM::UTILS

module :private;

namespace GOOM::UTILS
{

auto GetTimeTAsString(std::time_t timeT) noexcept -> std::string;
auto GetSteadyClockAsTimeT(std::chrono::steady_clock::time_point t) noexcept -> time_t;

auto GetCurrentDateTimeAsString() noexcept -> std::string
{
  return GetSteadyClockAsString(std::chrono::steady_clock::now());
}

auto GetSteadyClockAsString(const std::chrono::steady_clock::time_point& timePoint) noexcept
    -> std::string
{
  const auto timeT = GetSteadyClockAsTimeT(timePoint);
  return GetTimeTAsString(timeT);
}

#ifdef _MSC_VER
auto GetStandardDateTimeString(const std::string& otherFormatDateTime,
                               [[maybe_unused]] const std::string& otherFormat) noexcept
    -> std::string
{
  // 'strptime' is not supported by Microsoft.
  return otherFormatDateTime;
}
#else
auto GetStandardDateTimeString(const std::string& otherFormatDateTime,
                               const std::string& otherFormat) noexcept -> std::string
{
  auto timeTm = tm{};
  // NOLINTNEXTLINE(misc-include-cleaner): Too hard for time headers.
  ::strptime(otherFormatDateTime.c_str(), otherFormat.c_str(), &timeTm);
  timeTm.tm_isdst  = -1; // check for daylight savings
  const auto timeT = ::mktime(&timeTm);
  return GetTimeTAsString(timeT);
}
#endif

auto GetTimeTAsString(const std::time_t timeT) noexcept -> std::string
{
  static constexpr auto BUFF_SIZE = 100U;
  auto buff                       = tm{};
#ifdef _MSC_VER
  ::localtime_s(&buff, &timeT);
  if (auto str = std::array<char, BUFF_SIZE>{};
      std::strftime(str.data(), BUFF_SIZE, "%Y-%m-%d_%H-%M-%S", &buff))
  {
    return std::string{str.data()};
  }
#else
  if (auto str = std::array<char, BUFF_SIZE>{};
      // NOLINTNEXTLINE(misc-include-cleaner): Too hard for time headers.
      std::strftime(str.data(), BUFF_SIZE, "%Y-%m-%d_%H-%M-%S", ::localtime_r(&timeT, &buff)))
  {
    return std::string{str.data()};
  }
#endif

  return "TIME_ERROR";
}

auto GetSteadyClockAsTimeT(const std::chrono::steady_clock::time_point t) noexcept -> time_t
{
  const auto steadyClockDiff = t - std::chrono::steady_clock::now();
  const auto millisDiff = std::chrono::duration_cast<std::chrono::milliseconds>(steadyClockDiff);
  return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() + millisDiff);
}

} // namespace GOOM::UTILS
