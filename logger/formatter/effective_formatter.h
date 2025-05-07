#pragma once

#include "formatter.h"

namespace logger {
class EffectiveFormatter : public Formatter {
 public:
  void Format(const LogMsg& msg, std::string* dest) override;
};
}  // namespace logger