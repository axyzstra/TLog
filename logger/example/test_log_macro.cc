#include <memory>
#include "effective_formatter.h"
#include "effective_sink.h"
#include "log_handle.h"
#include "log_variadic_handle.h"
#include "logger.h"

int main() {
  logger::EffectiveSink::Conf conf;
  conf.dir = "logger";
  conf.prefix = "loggerdemo";
  conf.pub_key =
      "04827405069030E26A211C973C8710E6FBE79B5CAA364AC111FB171311902277537F8852EADD17EB339EB7CD0BA2490A58CDED2C702DFC1E"
      "FC7EDB544B869F039C";
  std::shared_ptr<logger::LogSink> effective_sink = std::make_shared<logger::EffectiveSink>(conf);
  std::shared_ptr<logger::VariadicLogHandle> handle = std::make_shared<logger::VariadicLogHandle>(effective_sink);
  EXT_LOGGER_INIT(handle);
  std::string str = "logger system";

  for (int i = 0; i < 1000000; ++i) {
    EXT_LOG_INFO("hello {}", str);
  }

  effective_sink->Flush();

  return 0;
}