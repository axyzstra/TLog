#include "context.h"

namespace logger {
Executor* Context::GetExecutor() {
  return executor_.get();
}
Context::Context() : executor_(std::make_unique<Executor>()) {}
}  // namespace logger