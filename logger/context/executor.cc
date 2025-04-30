#include "executor.h"

namespace logger {

Executor::Executor() {
  executor_context_ = std::make_unique<ExecutorContext>();
  executor_timer_ = std::make_unique<ExecutorTimer>();
}

Executor::~Executor() {
  executor_context_.reset();
  executor_timer_.reset();
}

TaskRunnerTag Executor::AddTaskRunner(const TaskRunnerTag& tag) {
  return executor_context_->AddTaskRunner(tag);
}

void Executor::PostTask(const TaskRunnerTag& runner_tag, Task task) {
  auto task_runner = executor_context_->GetTaskRunner(runner_tag);
  task_runner->RunTask(std::move(task));
}

Executor::ExecutorContext::ExecutorContext() : index(0) {}

TaskRunnerTag Executor::ExecutorContext::AddTaskRunner(const TaskRunnerTag& tag) {
  std::lock_guard<std::mutex> lock(mutex_);
  TaskRunnerTag cur_tag = tag;
  // 若当前 tag 的线程池已被添加，则换一个 tag
  while (task_runner_dict_.find(cur_tag) != task_runner_dict_.end()) {
    cur_tag = index++;
  }
  TaskRunnerPtr runner = std::make_unique<TaskRunner>(1);
  runner->Start();
  task_runner_dict_.emplace(cur_tag, std::move(runner));
  return cur_tag;
}

Executor::ExecutorContext::TaskRunner* Executor::ExecutorContext::GetTaskRunner(const TaskRunnerTag& tag) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto ret = task_runner_dict_.find(tag);
  // ret->second 为 unique_ptr，因此返回 .get()
  return ret == task_runner_dict_.end() ? nullptr : ret->second.get();
}

Executor::ExecutorTimer::ExecutorTimer() : repeated_task_id_(0), running_(false) {
  thread_pool_ = std::make_unique<ThreadPool>(1);
}

Executor::ExecutorTimer::~ExecutorTimer() {
  Stop();
}

void Executor::ExecutorTimer::Stop() {
  running_.store(false);
  cond_.notify_all();
  thread_pool_.reset();
}

bool Executor::ExecutorTimer::Start() {
  if (running_.load()) {
    return true;
  }
  running_.store(true);
  bool ret = thread_pool_->Start();
  thread_pool_->RunTask(&ExecutorTimer::Run_, this);
  return ret;
}

void Executor::ExecutorTimer::Run_() {
  while (running_.load()) {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this]() { return !this->queue_.empty(); });
    auto s = queue_.top();
    auto now_time = std::chrono::high_resolution_clock::now();
    if (s.time_point_ > now_time) {
      auto diff = std::chrono::duration_cast<std::chrono::microseconds>(s.time_point_ - now_time);
      cond_.wait_for(lock, diff);
      continue;
    }
    queue_.pop();
    lock.unlock();
    s.task();
  }
}

// 此处可以去掉 move，将参数改为 Task&&，在使用函数的时候进行 move
void Executor::ExecutorTimer::PostDelayedTask(Task task, const std::chrono::microseconds& delaytime) {
  InternalS s;
  s.time_point_ = std::chrono::high_resolution_clock::now() + delaytime;
  s.task = std::move(task);
  {
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.push(s);
    cond_.notify_all();
  }
}
void Executor::ExecutorTimer::CancelRepeatedTask(RepeatedTaskId task_id) {
  repeated_task_id_set.erase(task_id);
}

// void Executor::ExecutorTimer::PostTask_(Task task, std::chrono::microseconds& delaytime, RepeatedTaskId id, uint64_t
// repeat_num) {
//   PostRepeatedTask_(std::move(task), delaytime, repeated_task_id_, repeat_num);
// }

RepeatedTaskId Executor::ExecutorTimer::PostRepeatedTask(Task task,
                                                         const std::chrono::microseconds& delaytime,
                                                         uint64_t repeat_num) {
  RepeatedTaskId id = repeated_task_id_++;
  repeated_task_id_set.insert(id);
  PostRepeatedTask_(std::move(task), delaytime, id, repeat_num);
  return id;
}

//
void Executor::ExecutorTimer::PostRepeatedTask_(Task task,
                                                const std::chrono::microseconds& delaytime,
                                                RepeatedTaskId id,
                                                uint64_t repeat_num) {
  // 若 id 不存在表示已经移除该重复任务，则不需要进行执行
  if (repeated_task_id_set.find(id) == repeated_task_id_set.end()) {
    return;
  }
  // 若执行完毕指定次数则不需进行执行
  if (repeat_num == 0) {
    return;
  }

  task();

  Task func =
      std::bind(&Executor::ExecutorTimer::PostRepeatedTask_, this, std::move(task), delaytime, id, repeat_num - 1);
  InternalS s;
  s.time_point_ = std::chrono::high_resolution_clock::now() + delaytime;
  s.repeated_id = id;
  s.task = std::move(func);
  {
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.push(s);
  }
  cond_.notify_all();
}

}  // namespace logger