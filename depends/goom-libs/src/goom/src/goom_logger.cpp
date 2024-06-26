#include "goom/goom_logger.h"

#include <format>
#include <fstream>
#include <ios>
#include <iterator>
#include <mutex>
#include <ostream>
#include <stdexcept>
#include <string>

import Goom.Utils.DateUtils;
import Goom.Utils.EnumUtils;
import Goom.Lib.AssertUtils;

namespace GOOM
{

using UTILS::EnumMap;
using UTILS::GetCurrentDateTimeAsString;

static constexpr auto LOG_LEVEL_STR = EnumMap<GoomLogger::LogLevel, const char*>{{{
    {GoomLogger::LogLevel::DEBUG, "Debug"},
    {GoomLogger::LogLevel::INFO, "Info"},
    {GoomLogger::LogLevel::WARN, "Warn"},
    {GoomLogger::LogLevel::ERR, "Error"},
}}};

GoomLogger::GoomLogger() noexcept
{
  SetFileLogLevel(m_cutoffFileLogLevel);
  SetHandlersLogLevel(m_cutoffHandlersLogLevel);
}

GoomLogger::~GoomLogger() noexcept
{
  Expects(not m_doLogging);
}

auto GoomLogger::VLog(const LogLevel lvl,
                      const std::string& funcName,
                      const int lineNum,
                      const std::string& formatStr,
                      const std::format_args args) -> void
{
  std::string buffer;
  // Pass custom argument formatter as a template arg to pass further down.
  std::vformat_to(std::back_inserter(buffer), formatStr, args);
  Log(lvl, lineNum, funcName, std::string(buffer.data(), buffer.size()));
}

auto GoomLogger::Log(const LogLevel lvl,
                     const int lineNum,
                     const std::string& funcName,
                     const std::string& msg) -> void
{
  const auto lock = std::scoped_lock<std::mutex>{m_mutex};
  if ((not m_doLogging) or (not CanLog()))
  {
    return;
  }

  const auto mainMsg = GetLogPrefix(lvl, lineNum, funcName) + ":" + msg;
  const auto logMsg =
      std::string{not m_showDateTime ? mainMsg : ((GetCurrentDateTimeAsString() + ":") + mainMsg)};

  if (lvl >= m_cutoffFileLogLevel)
  {
    m_logEntries.push_back(logMsg);
  }
  if (lvl >= m_cutoffHandlersLogLevel)
  {
    for (const auto& handler : m_handlers)
    {
      handler.second(lvl, logMsg);
    }
  }
}

auto GoomLogger::GetLogPrefix(const LogLevel lvl,
                              const int lineNum,
                              const std::string& funcName) const -> std::string
{
  return std::format("{}:{}:{}", funcName, lineNum, LOG_LEVEL_STR[lvl]);
}

auto GoomLogger::DoFlush() -> void
{
  if (m_logFile.empty())
  {
    return;
  }
  if (m_logEntries.empty())
  {
    return;
  }

  auto logFile = std::ofstream{m_logFile, std::ios::out | std::ios::app};
  if (!logFile.good())
  {
    throw std::runtime_error("Could not open log file \"" + m_logFile + "\".");
  }
  for (const auto& entry : m_logEntries)
  {
    logFile << entry << "\n";
  }

  m_logEntries.clear();
}

} // namespace GOOM
