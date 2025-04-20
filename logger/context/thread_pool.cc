#include "thread_pool.h"

namespace logger {
  ThreadPool::ThreadInfo::~ThreadInfo() {
    if (ptr && ptr->joinable()) {
      ptr->join();
    }
  }

  void ThreadPool::AddThread() {
    // 新增线程的逻辑，循环查看任务队列，若有任务则拿出来执行
    auto call = [this]() {
      Task task;
      while (true) {
        // lock
        {
          std::unique_lock<std::mutex> lock(task_mutex_);
          // 若线程池关闭或任务队列为空则阻塞
          this->task_cv_.wait(lock, [this]() {
            return this->is_shutdown_ || !this->tasks_.empty();
          });
          if (this->is_shutdown_) {
            break;
          }
          if (this->tasks_.empty()) {
            break;
          }
          task = std::move(this->tasks_.front());
          this->tasks_.pop();
        } // lock
        if (task) {
          try {
            task();
          } catch (const std::exception& e) {
            std::cout << "出现异常:" << e.what() << std::endl;
          }
        }
      }
    };
    // 创建线程执行以上任务
    ThreadInfoPtr info_ptr = std::make_shared<ThreadInfo>();
    info_ptr->ptr = std::make_shared<std::thread>(call);
    this->worker_threads_.emplace_back(std::move(info_ptr));
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
}