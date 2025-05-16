#include <memory>
#include <thread>
#include "effective_formatter.h"
#include "effective_sink.h"
#include "log_handle.h"
#include "log_variadic_handle.h"
#include "logger.h"
#include "timer_counter.h"

std::string GenerateRandomString(int length) {
  std::string str;
  str.reserve(length);
  for (int i = 0; i < length; ++i) {
    str.push_back('a' + rand() % 26);
  }
  return str;
}

int main() {
  logger::EffectiveSink::Conf conf;
  conf.dir = "logger";
  conf.prefix = "loggerdemo";
  conf.pub_key =
      "04827405069030E26A211C973C8710E6FBE79B5CAA364AC111FB171311902277537F8852EADD17EB339EB7CD0BA2490A58CDED2C702DFC1E"
      "FC7EDB544B869F039C";
  // private key FAA5BBE9017C96BF641D19D0144661885E831B5DDF52539EF1AB4790C05E665E
  std::shared_ptr<logger::LogSink> effective_sink = std::make_shared<logger::EffectiveSink>(conf);
  std::shared_ptr<logger::VariadicLogHandle> handle = std::make_shared<logger::VariadicLogHandle>(effective_sink);
  EXT_LOGGER_INIT(handle);
  std::string str = GenerateRandomString(2000);
  std::this_thread::sleep_for(std::chrono::seconds(10));
  {
    TIMER_COUNT("百万级日志写入时间统计");
    for (int i = 0; i < 1000000; ++i) {
      EXT_LOG_INFO("hello {}", str);
    }
    effective_sink->Flush();
  }

  return 0;
}