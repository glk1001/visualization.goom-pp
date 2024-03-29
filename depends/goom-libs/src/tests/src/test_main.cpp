#undef NO_LOGGING

#include "goom/goom_control.h"
#include "goom/goom_logger.h"

#include <iostream>
#include <ostream>
#include <string>

#if __clang_major__ >= 16 // NOLINT: Can't include header for this.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wunsafe-buffer-usage"
#endif
#include <catch2/catch_session.hpp>
#if __clang_major__ >= 16 // NOLINT: Can't include header for this.
#pragma GCC diagnostic pop
#endif

using Catch::Session;
using GOOM::GoomControl;
using GOOM::GoomLogger;

auto main(int argc, char* argv[]) -> int
{
  // global setup...
  auto goomLogger        = GoomControl::MakeGoomLogger();
  const auto fConsoleLog = [](const GoomLogger::LogLevel, const std::string& str)
  { std::clog << str << "\n"; };
  AddLogHandler(*goomLogger, "console-log", fConsoleLog);
  SetLogLevel(*goomLogger, GoomLogger::LogLevel::INFO);
  SetLogLevelForFiles(*goomLogger, GoomLogger::LogLevel::INFO);
  LogStart(*goomLogger);

  LogInfo(*goomLogger, "Start unit tests...");

  const auto result = Session().run(argc, argv);

  // global clean-up...

  LogStop(*goomLogger);

  return result;
}
