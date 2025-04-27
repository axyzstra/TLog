#include "console_sink.h"

namespace logger {
  ConsoleSink::ConsoleSink() : formatter_(std::make_unique<DefaultFormatter>()) {}

  void ConsoleSink::Log(const LogMsg& msg) {
    std::string buf;
    formatter_->Format(msg, &buf);
    fwrite(buf.data(), 1, buf.size(), stdout);
    fwrite("\n", 1, 1, stdout);
  }

  void ConsoleSink::SetFormatter(std::unique_ptr<Formatter> formatter) {
    formatter_ = std::move(formatter);
  }
}