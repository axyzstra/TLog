#include <iostream>
#include "sys_util.h"
#include "file_util.h"

int main() {
  std::cout << GetPageSize() << std::endl;
  logger::fs::fpath file_path = "/home/axyz/usr/logger/logger/utils/CMakeLists.txt";
  std::cout << logger::fs::GetFileSize(file_path) << std::endl;
  return 0;
}