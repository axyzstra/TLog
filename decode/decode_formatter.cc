#include <chrono>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "decode_formatter.h"

class FlagFormatter {
 public:
  FlagFormatter() = default;
  virtual ~FlagFormatter() = default;
  virtual void Format(const EffectiveMsg& flag, std::string& dest) = 0;
};

static const std::vector<std::string> LOG_LEVEL = {"V", "D", "I", "W", "E", "F"};

static void CompilePattern(const std::string& pattern);
static void HandleFlag(char flag);

class LogLevelFormatter final : public FlagFormatter {
 public:
  LogLevelFormatter() = default;
  ~LogLevelFormatter() = default;
  void Format(const EffectiveMsg& flag, std::string& dest) override {
    auto index = flag.level();
    if (0 <= index && index <= 5) {
      dest.append(LOG_LEVEL[index]);
    } else {
      dest.append("U");
    }
  }
};

std::string MillisecondsToDateString(long long milliseconds) {
  std::chrono::system_clock::time_point tp =
      std::chrono::system_clock::time_point(std::chrono::milliseconds(milliseconds));
  std::time_t time_tt = std::chrono::system_clock::to_time_t(tp);
  std::tm* timeinfo = std::localtime(&time_tt);
  std::ostringstream oss;
  oss << std::put_time(timeinfo, "%Y-%m-%d %H:%M:%S");
  return oss.str();
}

class TimestampDateFormatter final : public FlagFormatter {
 public:
  TimestampDateFormatter() = default;
  ~TimestampDateFormatter() = default;
  void Format(const EffectiveMsg& flag, std::string& dest) override {
    dest.append(MillisecondsToDateString(flag.timestamp()));
  }
};

class TimestampSecondsFormatter final : public FlagFormatter {
 public:
  TimestampSecondsFormatter() = default;
  ~TimestampSecondsFormatter() = default;
  void Format(const EffectiveMsg& flag, std::string& dest) override {
    dest.append(std::to_string(flag.timestamp() / 1000));
  }
};

class TimestampMillisecondsFormatter final : public FlagFormatter {
 public:
  TimestampMillisecondsFormatter() = default;
  ~TimestampMillisecondsFormatter() = default;
  void Format(const EffectiveMsg& flag, std::string& dest) override { dest.append(std::to_string(flag.timestamp())); }
};

class ThreadIdFormatter final : public FlagFormatter {
 public:
  ThreadIdFormatter() = default;
  ~ThreadIdFormatter() = default;
  void Format(const EffectiveMsg& flag, std::string& dest) override { dest.append(std::to_string(flag.tid())); }
};

class ProcessIdFormatter final : public FlagFormatter {
 public:
  ProcessIdFormatter() = default;
  ~ProcessIdFormatter() override = default;

  void Format(const EffectiveMsg& flag, std::string& dest) override { dest.append(std::to_string(flag.pid())); }
};

class LineFormatter final : public FlagFormatter {
 public:
  LineFormatter() = default;
  ~LineFormatter() = default;

  void Format(const EffectiveMsg& flag, std::string& dest) override { dest.append(std::to_string(flag.line())); }
};

class FileNameFormatter final : public FlagFormatter {
 public:
  FileNameFormatter() = default;
  ~FileNameFormatter() = default;

  void Format(const EffectiveMsg& flag, std::string& dest) override { dest.append(flag.file_name()); }
};

class FuncNameFormatter final : public FlagFormatter {
 public:
  FuncNameFormatter() = default;
  ~FuncNameFormatter() = default;

  void Format(const EffectiveMsg& flag, std::string& dest) override { dest.append(flag.func_name()); }
};

class LogInfoFormatter final : public FlagFormatter {
 public:
  LogInfoFormatter() = default;
  ~LogInfoFormatter() = default;

  void Format(const EffectiveMsg& flag, std::string& dest) override { dest.append(flag.log_info()); }
};

class AggregateFormatter final : public FlagFormatter {
 public:
  AggregateFormatter() = default;
  ~AggregateFormatter() override = default;
  void AddChar(char ch) { str_.push_back(ch); }
  void Format(const EffectiveMsg& flag, std::string& dest) override { dest.append(str_); }

 private:
  std::string str_;
};

std::vector<std::unique_ptr<FlagFormatter>> flag_formatters_;

/// @brief 逐字符解析 Pattern
/// @param pattern
void ParsePattern(const std::string& pattern) {
  std::unique_ptr<AggregateFormatter> str_formatter;
  flag_formatters_.clear();
  auto end = pattern.end();
  for (auto it = pattern.begin(); it != end; ++it) {
    // 若遇到 % 则表示接下来会遇到 l、D、S、p、t、F、f、#、v
    if (*it == '%') {
      if (str_formatter) {
        flag_formatters_.push_back(std::move(str_formatter));
      }
      ++it;
      if (it == end) {
        break;
      }
      // 由于 % 后面是 l、D、S、p、t、F、f、#、v
      // 因此需要添加对应的 formatter
      HandleFlag(*it);
    } else {
      // 若 str_formatter 已经通过 move 置为 nullptr
      if (!str_formatter) {
        str_formatter = std::make_unique<AggregateFormatter>();
      }
      str_formatter->AddChar(*it);
    }
  }
  if (str_formatter) {
    flag_formatters_.push_back(std::move(str_formatter));
  }
}

void HandleFlag(char flag) {
  switch (flag) {
    case 'l':
      flag_formatters_.push_back(std::make_unique<LogLevelFormatter>());
      break;
    case 'D':
      flag_formatters_.push_back(std::make_unique<TimestampDateFormatter>());
      break;
    case 'S':
      flag_formatters_.push_back(std::make_unique<TimestampSecondsFormatter>());
      break;
    case 'M':
      flag_formatters_.push_back(std::make_unique<TimestampMillisecondsFormatter>());
      break;
    case 'p':
      flag_formatters_.push_back(std::make_unique<ProcessIdFormatter>());
      break;
    case 't':
      flag_formatters_.push_back(std::make_unique<ThreadIdFormatter>());
      break;
    case '#':
      flag_formatters_.push_back(std::make_unique<LineFormatter>());
      break;
    case 'F':
      flag_formatters_.push_back(std::make_unique<FileNameFormatter>());
      break;
    case 'f':
      flag_formatters_.push_back(std::make_unique<FuncNameFormatter>());
      break;
    case 'v':
      flag_formatters_.push_back(std::make_unique<LogInfoFormatter>());
      break;
    default:
      // 处理其他字符，打印时在前面加上 %
      auto formatter = std::make_unique<AggregateFormatter>();
      formatter->AddChar('%');
      formatter->AddChar(flag);
      flag_formatters_.push_back(std::move(formatter));
      break;
  }
}

void DecodeFormatter::SetPattern(const std::string& pattern) {
  ParsePattern(pattern);
}

std::string DefaultPatternMsg(const EffectiveMsg& msg) {
  std::string output_string;
  output_string.append("[");
  output_string.append(std::to_string(msg.level()));
  output_string.append("][");
  output_string.append(std::to_string(msg.timestamp()));
  output_string.append("][");
  output_string.append(std::to_string(msg.pid()));
  output_string.append(":");
  output_string.append(std::to_string(msg.tid()));
  output_string.append("][");
  output_string.append(msg.file_name());
  output_string.append(":");
  output_string.append(msg.func_name());
  output_string.append(":");
  output_string.append(std::to_string(msg.line()));
  output_string.append("]");
  output_string.append(msg.log_info());
  return output_string;
}

void DecodeFormatter::Format(const EffectiveMsg& msg, std::string& dest) {
  if (!flag_formatters_.empty()) {
    for (auto& formatter : flag_formatters_) {
      formatter->Format(msg, dest);
    }
  } else {
    dest.append(DefaultPatternMsg(msg));
  }
  dest.append("\n");
}