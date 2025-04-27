#pragma once

#include <string.h>
#include "formatter.h"

#include "../utils/sys_util.h"

namespace logger {
  class DefaultFormatter : public Formatter {
    public:
      void Format(const LogMsg& msg, std::string* dest) override;
  };
}