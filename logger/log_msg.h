#pragma once

#include "log_common.h"

namespace logger {
  

  /// @brief 日志信息结构体：保存日志位置、等级、内容
  struct LogMsg {
    LogMsg() = default;
    LogMsg(SourceLocation loc, LogLevel lvl, StringView msg);
    LogMsg(LogLevel lvl, StringView msg);

    LogMsg(const LogMsg& other) = default;
    LogMsg& operator=(const LogMsg& other) = default;

    SourceLocation location;
    LogLevel level;
    StringView message;
  };
}