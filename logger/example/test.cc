#include "internal_log.h"
#include <iostream>

int main() {
  int key = 42;
  std::string value = "wang";

  LOG_INFO("Hello World, key {}, value {}", key, value);

  return 0;
}

