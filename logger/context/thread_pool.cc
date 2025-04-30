#include "thread_pool.h"

namespace logger {
ThreadPool::ThreadInfo::~ThreadInfo() {
  if (ptr && ptr->joinable()) {
    ptr->join();
  }
}

void ThreadPool::AddThread() {
  auto call = [this]() {
    Task task;
    while (true) {
      {
        std::unique_lock<std::mutex> lock(task_mutex_);
        // 任务队列不为空时解除阻塞 需要关闭线程池时解除阻塞
        task_cv_.wait(lock, [this]() { return !this->tasks_.empty() || this->is_shutdown_.load(); });
        // 若是因为关闭线程池解除的阻塞，直接结束任务
        if (this->is_shutdown_.load()) {
          return;
        }
        task = std::move(this->tasks_.front());
        this->tasks_.pop();
      }
      task();
    }
  };
  ThreadInfoPtr info_ptr = std::make_shared<ThreadInfo>();
  info_ptr->ptr = std::make_shared<std::thread>(call);
  this->worker_threads_.emplace_back(info_ptr);
}

bool ThreadPool::Start() {
  // 线程池已经启动
  if (is_available_.load()) {
    return false;
  }
  is_available_.store(true);
  for (int i = 0; i < thread_count_; i++) {
    AddThread();
  }
  return true;
}

void ThreadPool::Stop() {
  if (is_available_.load()) {
    is_shutdown_.store(true);
    is_available_.store(false);
    task_cv_.notify_all();
  }
  worker_threads_.clear();
}
}  // namespace logger