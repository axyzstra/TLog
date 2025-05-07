#include "effective_formatter.h"
#include "effective_msg.pb.h"
#include "sys_util.h"

namespace logger {
void EffectiveFormatter::Format(const LogMsg& msg, std::string* dest) {
  EffectiveMsg eff_msg;
  eff_msg.set_level(static_cast<int32_t>(msg.level));
  eff_msg.set_timestamp(
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
          .count());
  eff_msg.set_pid(GetProcessId());
  eff_msg.set_tid(GetThreadId());
  eff_msg.set_line(msg.location.line);
  eff_msg.set_file_name(msg.location.file_name.data());
  eff_msg.set_func_name(msg.location.func_name.data());
  eff_msg.set_log_info(msg.message.data());
  size_t len = eff_msg.ByteSizeLong();
  dest->resize(len);
  eff_msg.SerializeToArray(dest->data(), len);
}
}  // namespace logger