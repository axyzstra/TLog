#pragma once

#include "formatter/formatter.h"
#include "log_msg.h"

namespace logger {
  class LogSink {
    public:
      virtual ~LogSink() = default;
      
      virtual void Log(const LogMsg&) = 0;

      virtual void SetFormatter(std::unique_ptr<Formatter> formatter) = 0;

      virtual void Flush() {}
  };
}