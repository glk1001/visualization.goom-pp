#pragma once

#include <format>
#include <functional>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

import Goom.Lib.GoomTypes;

// TODO(glk): fix this
#if defined(__clang_major__) && __clang_major__ >= 20
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdecls-in-multiple-modules"
#endif

namespace GOOM
{

class GoomLogger
{
public:
  enum class LogLevel : UnderlyingEnumType
  {
    DEBUG,
    INFO,
    WARN,
    ERR, // would 'ERROR' but MSVC 17.5 chokes
  };
  using HandlerFunc = std::function<void(const LogLevel, const std::string&)>;

  GoomLogger() noexcept;
  GoomLogger(const GoomLogger&) = delete;
  GoomLogger(GoomLogger&&)      = delete;
  virtual ~GoomLogger() noexcept;
  auto operator=(const GoomLogger&) -> GoomLogger& = delete;
  auto operator=(GoomLogger&&) -> GoomLogger&      = delete;

  auto SetLogFile(const std::string_view& logF) -> void;
  auto AddHandler(const std::string_view& name, const HandlerFunc& handlerFunc) -> void;
  auto SetShowDateTime(bool val) -> void;

  auto Start() -> void;
  auto Stop() -> void;
  auto Flush() -> void;
  auto Suspend() -> void;
  auto Resume() -> void;

  [[nodiscard]] auto IsLogging() const -> bool;
  [[nodiscard]] auto GetFileLogLevel() const -> LogLevel;
  auto SetFileLogLevel(LogLevel lvl) -> void;
  [[nodiscard]] auto GetHandlersLogLevel() const -> LogLevel;
  auto SetHandlersLogLevel(LogLevel lvl) -> void;

  [[nodiscard]] virtual auto CanLog() const -> bool;
  auto Log(LogLevel lvl, int lineNum, const std::string& funcName, const std::string& msg) -> void;
  template<typename... Args>
  auto Log(LogLevel lvl,
           int lineNum,
           const std::string& funcName,
           const std::string& formatStr,
           const Args&... args) -> void;

protected:
  [[nodiscard]] virtual auto GetLogPrefix(LogLevel lvl,
                                          int lineNum,
                                          const std::string& funcName) const -> std::string;

private:
  LogLevel m_cutoffFileLogLevel     = LogLevel::INFO;
  LogLevel m_cutoffHandlersLogLevel = LogLevel::INFO;
  bool m_showDateTime               = true;
  bool m_doLogging                  = false;
  std::string m_logFile{};
  std::vector<std::pair<std::string, HandlerFunc>> m_handlers{};
  std::vector<std::string> m_logEntries{};
  std::mutex m_mutex{};
  auto DoFlush() -> void;
  auto VLog(LogLevel lvl,
            const std::string& funcName,
            int lineNum,
            const std::string& formatStr,
            std::format_args args) -> void;
};

inline auto GoomLogger::SetLogFile(const std::string_view& logF) -> void
{
  m_logFile = logF;
}

inline auto GoomLogger::AddHandler(const std::string_view& name, const HandlerFunc& handlerFunc)
    -> void
{
  for (const auto& handler : m_handlers)
  {
    if (handler.first == name)
    {
      return;
    }
  }
  m_handlers.emplace_back(name, handlerFunc);
}

inline auto GoomLogger::SetShowDateTime(const bool val) -> void
{
  m_showDateTime = val;
}

inline auto GoomLogger::Start() -> void
{
  const auto lock = std::scoped_lock<std::mutex>{m_mutex};
  m_doLogging     = true;
  m_logEntries.clear();
}

inline auto GoomLogger::Stop() -> void
{
  const auto lock = std::scoped_lock<std::mutex>{m_mutex};
  m_doLogging     = false;
  DoFlush();
}

inline auto GoomLogger::Flush() -> void
{
  const auto lock = std::scoped_lock<std::mutex>{m_mutex};
  DoFlush();
}

inline auto GoomLogger::Suspend() -> void
{
  m_doLogging = false;
}

inline auto GoomLogger::Resume() -> void
{
  m_doLogging = true;
}

inline auto GoomLogger::IsLogging() const -> bool
{
  return m_doLogging;
}

inline auto GoomLogger::GetFileLogLevel() const -> GoomLogger::LogLevel
{
  return m_cutoffFileLogLevel;
}

inline auto GoomLogger::SetFileLogLevel(const LogLevel lvl) -> void
{
  m_cutoffFileLogLevel = lvl;
}

inline auto GoomLogger::GetHandlersLogLevel() const -> GoomLogger::LogLevel
{
  return m_cutoffHandlersLogLevel;
}

inline auto GoomLogger::SetHandlersLogLevel(const LogLevel lvl) -> void
{
  m_cutoffHandlersLogLevel = lvl;
}

inline auto GoomLogger::CanLog() const -> bool
{
  return true;
}

template<typename... Args>
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto GoomLogger::Log(const LogLevel lvl,
                     const int lineNum,
                     const std::string& funcName,
                     const std::string& formatStr,
                     const Args&... args) -> void
{
  VLog(lvl, funcName, lineNum, formatStr, std::make_format_args(args...));
}

} // namespace GOOM

#ifdef NO_LOGGING
//#pragma message("Compiling " __FILE__ " with 'NO_LOGGING' = 'ON' - So there will be NO logging.")

inline auto SetLogFile([[maybe_unused]] const GOOM::GoomLogger& logger,
                       [[maybe_unused]] const std::string_view& logF) -> void
{
  // No logging for Release.
}

inline auto AddLogHandler([[maybe_unused]] const GOOM::GoomLogger& logger,
                          [[maybe_unused]] const std::string_view& name,
                          [[maybe_unused]] const GOOM::GoomLogger::HandlerFunc& handlerFunc) -> void
{
  // No logging for Release.
}

inline auto SetShowDateTime([[maybe_unused]] const GOOM::GoomLogger& logger,
                            [[maybe_unused]] const bool val) -> void
{
  // No logging for Release.
}

inline auto LogStart([[maybe_unused]] const GOOM::GoomLogger& logger) -> void
{
  // No logging for Release.
}

inline auto LogStop([[maybe_unused]] const GOOM::GoomLogger& logger) -> void
{
  // No logging for Release.
}

inline auto LogFlush([[maybe_unused]] const GOOM::GoomLogger& logger) -> void
{
  // No logging for Release.
}

inline auto LogSuspend([[maybe_unused]] const GOOM::GoomLogger& logger) -> void
{
  // No logging for Release.
}

inline auto LogResume([[maybe_unused]] const GOOM::GoomLogger& logger) -> void
{
  // No logging for Release.
}

[[nodiscard]] inline auto IsLogging([[maybe_unused]] const GOOM::GoomLogger& logger) -> bool
{
  return false;
}

[[nodiscard]] inline auto GetLogLevel([[maybe_unused]] const GOOM::GoomLogger& logger)
    -> GOOM::GoomLogger::LogLevel
{
  return GOOM::GoomLogger::LogLevel{};
}

inline auto SetLogLevel([[maybe_unused]] const GOOM::GoomLogger& logger,
                        [[maybe_unused]] const GOOM::GoomLogger::LogLevel lvl) -> void
{
  // No logging for Release.
}

inline auto SetLogLevelForFiles([[maybe_unused]] const GOOM::GoomLogger& logger,
                                [[maybe_unused]] const GOOM::GoomLogger::LogLevel lvl) -> void
{
  // No logging for Release.
}

// NOLINTBEGIN: Remove these macros with C++20.
#define LogDebug(logger, ...)
#define LogInfo(logger, ...)
#define LogWarn(logger, ...)
#define LogError(logger, ...)
// NOLINTEND: Remove these macros with C++20.

#else
#pragma message("Compiling " __FILE__ " with 'NO_LOGGING' = 'OFF' - SO THERE WILL BE LOGGING.")

inline auto SetLogFile(GOOM::GoomLogger& logger, const std::string& logF) -> void
{
  logger.SetLogFile(logF);
}

inline auto AddLogHandler(GOOM::GoomLogger& logger,
                          const std::string_view& name,
                          const GOOM::GoomLogger::HandlerFunc& handlerFunc) -> void
{
  logger.AddHandler(name, handlerFunc);
}

inline auto SetShowDateTime(GOOM::GoomLogger& logger, const bool val) -> void
{
  logger.SetShowDateTime(val);
}

inline auto LogStart(GOOM::GoomLogger& logger) -> void
{
  logger.Start();
}

inline auto LogStop(GOOM::GoomLogger& logger) -> void
{
  logger.Stop();
}

inline auto LogFlush(GOOM::GoomLogger& logger) -> void
{
  logger.Flush();
}

inline auto LogSuspend(GOOM::GoomLogger& logger) -> void
{
  logger.Suspend();
}

inline auto LogResume(GOOM::GoomLogger& logger) -> void
{
  logger.Resume();
}

[[nodiscard]] inline auto IsLogging(const GOOM::GoomLogger& logger) -> bool
{
  return logger.IsLogging();
}

[[nodiscard]] inline auto GetLogLevel(const GOOM::GoomLogger& logger) -> GOOM::GoomLogger::LogLevel
{
  return logger.GetHandlersLogLevel();
}

inline auto SetLogLevel(GOOM::GoomLogger& logger, const GOOM::GoomLogger::LogLevel lvl) -> void
{
  logger.SetHandlersLogLevel(lvl);
}

inline auto SetLogLevelForFiles(GOOM::GoomLogger& logger, const GOOM::GoomLogger::LogLevel lvl)
    -> void
{
  logger.SetFileLogLevel(lvl);
}

// NOLINTBEGIN: Remove these macros with C++20.
#define LogDebug(logger, ...) \
  (logger).Log(GOOM::GoomLogger::LogLevel::DEBUG, __LINE__, __func__, __VA_ARGS__)
#define LogInfo(logger, ...) \
  (logger).Log(GOOM::GoomLogger::LogLevel::INFO, __LINE__, __func__, __VA_ARGS__)
#define LogWarn(logger, ...) \
  (logger).Log(GOOM::GoomLogger::LogLevel::WARN, __LINE__, __func__, __VA_ARGS__)
#define LogError(logger, ...) \
  (logger).Log(GOOM::GoomLogger::LogLevel::ERR, __LINE__, __func__, __VA_ARGS__)
// NOLINTEND: Remove these macros with C++20.
#endif

#if defined(__clang_major__) && __clang_major__ >= 20
#pragma GCC diagnostic pop
#endif
