#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace logger {
class ThreadPool {
 public:
  explicit ThreadPool(uint32_t thread_count) : thread_count_(thread_count), is_shutdown_(false), is_available_(false) {}

  ~ThreadPool() { Stop(); }

  // 线程不可复制
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  bool Start();

  void Stop();

  template <typename F, typename... Args>
  void RunTask(F&& f, Args&&... args) {
    if (is_shutdown_.load() || !is_available_.load()) {
      return;
    }
    auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    {
      std::lock_guard<std::mutex> lock(task_mutex_);
      tasks_.emplace(task);
    }
    task_cv_.notify_all();
  }

  template <typename F, typename... Args>
  auto RunRetTask(F&& f, Args&&... args) -> std::shared_ptr<std::future<decltype(f(args...))>> {
    using RetType = decltype(f(args...));
    if (is_shutdown_.load() || !is_available_.load()) {
      return nullptr;
    }

    auto task =
        std::make_shared<std::packaged_task<RetType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    auto ret = task->get_future();

    {
      std::unique_lock<std::mutex> lock(task_mutex_);
      tasks_.emplace([task]() { (*task)(); });
    }
    task_cv_.notify_all();
    return std::make_shared<std::future<RetType>>(std::move(ret));
  }

 private:
  void AddThread();

 private:
  using Task = std::function<void()>;
  using ThreadPtr = std::shared_ptr<std::thread>;

  // 保存线程相关信息
  struct ThreadInfo {
    ThreadPtr ptr{nullptr};

    ThreadInfo() = default;
    ~ThreadInfo();
  };

  using ThreadInfoPtr = std::shared_ptr<ThreadInfo>;

 private:
  std::vector<ThreadInfoPtr> worker_threads_;
  std::queue<Task> tasks_;

  std::mutex task_mutex_;
  std::condition_variable task_cv_;

  std::atomic<uint32_t> thread_count_;
  std::atomic<bool> is_shutdown_;
  std::atomic<bool> is_available_;
};
}  // namespace logger