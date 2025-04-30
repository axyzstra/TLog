#include "log_msg.h"

namespace logger {
LogMsg::LogMsg(SourceLocation loc, LogLevel lvl, StringView msg)
    : location(std::move(loc)), level(lvl), message(std::move(msg)) {}

LogMsg::LogMsg(LogLevel lvl, StringView msg)
    : location(std::move(SourceLocation{})), level(lvl), message(std::move(msg)) {}
}  // namespace logger