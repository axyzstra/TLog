#include "default_formatter.h"

namespace logger {

// [2025-04-27 12:00:00.000] [INFO] [file.cc:123] [pid:tid] message
void DefaultFormatter::Format(const LogMsg& msg, std::string* dest) {
  // 即 Trace Debug Info Warn Error Fatal 六个等级，其对应的宏编号为 0~5
  const std::string kLogLevelStr = "TDIWEF";
  std::time_t now = std::time(nullptr);
  std::tm tm;

  LocalTime(&tm, &now);

  char time_buf[32] = {0};

  std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &tm);
  dest->append("[", 1);
  dest->append(time_buf, strlen(time_buf));
  dest->append("] [", 3);
  dest->append(1, kLogLevelStr[static_cast<int>(msg.level)]);
  dest->append("] [", 3);
  dest->append(msg.location.file_name.data(), msg.location.file_name.size());
  dest->append(":", 1);
  dest->append(std::to_string(msg.location.line));
  dest->append("] [", 3);
  dest->append(std::to_string(GetProcessId()));
  dest->append(":", 1);
  dest->append(std::to_string(GetThreadId()));
  dest->append("] ", 2);
  dest->append(msg.message.data(), msg.message.size());
}
}  // namespace logger