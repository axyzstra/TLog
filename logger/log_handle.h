#pragma once

#include <memory>
#include <atomic>
#include <initializer_list>
#include <vector>

#include "log_common.h"
#include "sinks/sink.h"

namespace logger {
  class LogSink;
  using LogSinkPtr = std::shared_ptr<LogSink>;
  using LogSinkPtrList = std::initializer_list<LogSinkPtr>;
  class LogHandle {
    public:
      explicit LogHandle(LogSinkPtr sink);
      explicit LogHandle(LogSinkPtrList sinks);

      // template <typename It>
      // LogHandle(It begin, It end) : LogHandle(LogSinkPtrList(begin, end)) {}


      ~LogHandle() = default;
      LogHandle(const LogHandle&) = delete;
      LogHandle& operator=(const LogHandle&) = delete;

      void SetLevel(LogLevel level);
      LogLevel GetLevel() const;

      void Log(LogLevel level, SourceLocation loc, StringView message);
    
    protected:
      bool ShouldLog_(LogLevel level) const noexcept {
        return level >= level_ && !sinks_.empty();
      }

      void Log_(const LogMsg& log_msg);

    private:
      std::atomic<LogLevel> level_;
      std::vector<LogSinkPtr> sinks_;
  };
}