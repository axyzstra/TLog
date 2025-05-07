#include <memory>
#include "effective_formatter.h"
#include "effective_sink.h"
#include "log_handle.h"

int main() {
  logger::EffectiveSink::Conf conf;
  conf.dir = "logger";
  conf.prefix = "loggerdemo";
  conf.pub_key =
      "04827405069030E26A211C973C8710E6FBE79B5CAA364AC111FB171311902277537F8852EADD17EB339EB7CD0BA2490A58CDED2C702DFC1E"
      "FC7EDB544B869F039C";

  std::shared_ptr<logger::LogSink> effective_sink = std::make_shared<logger::EffectiveSink>(conf);
  logger::LogHandle handle({effective_sink});

  auto begin = std::chrono::system_clock::now();
  // logger::SourceLocation srcLoc("test_sink.cc", 18, "main");
  logger::SourceLocation srcLoc(__FILE__, __LINE__, __func__);
  for (int i = 0; i < 1000000; ++i) {
    std::string str = "hello";
    handle.Log(logger::LogLevel::kInfo, srcLoc, str);
  }
  effective_sink->Flush();
  auto end = std::chrono::system_clock::now();
  std::chrono::milliseconds diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
  return 0;
}