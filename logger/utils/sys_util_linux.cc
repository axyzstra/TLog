#include "sys_util.h"

#include <unistd.h>
// 获取 Linux 下的 页 大小
size_t GetPageSize() {
  return getpagesize();
}

void LocalTime(std::tm* tm, std::time_t* now) {
  localtime_r(now, tm);
}

size_t GetProcessId() {
  return static_cast<size_t>(::getpid());
}

size_t GetThreadId() {
  return static_cast<size_t>(::gettid());
}