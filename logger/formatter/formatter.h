#pragma once

#include <memory>

#include "../log_msg.h"

namespace logger {
  class Formatter {
    public:
      virtual ~Formatter() = default;

      virtual void Format(const LogMsg& msg, std::string* dest) = 0;
  };
}