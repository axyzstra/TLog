#include "log_handle.h"

namespace logger {
  LogHandle::LogHandle(LogSinkPtr sink) : 
    level_{LogLevel::kInfo},
    sinks_{sink} {}

  LogHandle::LogHandle(LogSinkPtrList sinks) : 
    level_{LogLevel::kInfo}, 
    sinks_{sinks} {}

  void LogHandle::SetLevel(LogLevel level) {
    level_ = level;
  }
  LogLevel LogHandle::GetLevel() const {
    return level_;
  }

  void LogHandle::Log(LogLevel level, SourceLocation loc, StringView message) {
    if (!ShouldLog_(level_)) {
      return;
    }
    LogMsg msg(loc, level, message);
    Log_(msg);
  }

  void LogHandle::Log_(const LogMsg& log_msg) {
    for (auto& sink : sinks_) {
      sink->Log(log_msg);
    }
  }
}