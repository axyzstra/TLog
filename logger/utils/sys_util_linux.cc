#include "sys_util.h"

#include <unistd.h>
// 获取 Linux 下的 页 大小
size_t GetPageSize() {
  return getpagesize();
}