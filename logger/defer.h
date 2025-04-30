#pragma once

#include <functional>

namespace logger {
class ExecuteOnScopeExit {
 public:
  ExecuteOnScopeExit(const ExecuteOnScopeExit&) = delete;
  ExecuteOnScopeExit& operator=(const ExecuteOnScopeExit&) = delete;

  template <typename F, typename... Args>
  ExecuteOnScopeExit(F&& f, Args&&... args) {
    func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
  }

  ~ExecuteOnScopeExit() noexcept {
    if (func_) {
      func_();
    }
  }

 private:
  std::function<void()> func_;
};
};  // namespace logger

#define _MAKE_DEFER_(line) logger::ExecuteOnScopeExit defer##line = [&]()

#undef LOG_DEFER
#define LOG_DEFER _MAKE_DEFER_(__LINE__)