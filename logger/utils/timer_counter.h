#pragma once

#include <chrono>
#include <iostream>
#include <string>

namespace logger {
class TimerCount {
 public:
  TimerCount(std::string_view info) : info_(info), start_(std::chrono::steady_clock::now()) {
    std::cout << "计时开始..." << std::endl;
  }
  ~TimerCount() {
    auto end = std::chrono::steady_clock::now();
    auto diff = end - start_;
    auto time_in_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
    std::cout << info_ << ":" << time_in_milliseconds << " milliseconds" << std::endl;
  }

 private:
  std::string_view info_;
  std::chrono::time_point<std::chrono::steady_clock> start_;
};
}  // namespace logger

#define _TIMER_CONCAT(a, b) a##b

#ifdef ENABLE_TIMING
#define TIMER_COUNT(info) logger::TimerCount _TIMER_CONCAT(timer_count, __LINE__)(info)
#else
#define TIMER_COUNT(info) void(0)
#endif