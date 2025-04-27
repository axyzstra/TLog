#include <memory>


#include "sink.h"
#include "console_sink.h"
#include "formatter.h"
#include "default_formatter.h"
#include "log_handle.h"

int main() {
  std::shared_ptr<logger::LogSink> sink = std::make_shared<logger::ConsoleSink>();
  logger::LogHandle handle(sink);
  std::string str = "test";
  logger::SourceLocation loc = logger::SourceLocation("test_handle.cc", 14, "main");
  handle.Log(logger::LogLevel::kInfo, loc, str);
  return 0;
}