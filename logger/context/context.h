#pragma once

#include "executor.h"

#include <memory>

// 去掉了我认为多余的封装类 ExecutorManager 以及封装函数 NewTaskRunner;
namespace logger {
class Context {
 public:
  Executor* GetExecutor();

  static Context* GetInstance() {
    static Context* instance = new Context();
    return instance;
  }

  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;

 private:
  Context();

 private:
  std::unique_ptr<Executor> executor_;
};
}  // namespace logger

#define CONTEXT logger::Context::GetInstance()

#define EXECUTOR CONTEXT->GetExecutor()

#define NEW_TASK_RUNNER(tag) EXECUTOR->AddTaskRunner(tag)

#define POST_TASK(runner_tag, task) EXECUTOR->PostTask(runner_tag, task)

// 执行有返回值
#define WAIT_TASK_IDLE(runner_tag) EXECUTOR->PostTaskAndGetResult(runner_tag, []() {})->wait()

#define POST_REPEATED_TASK(runner_tag, task, delaytime, repeat_num) \
  EXECUTOR->PostRepeatedTask(runner_tag, task, delaytime, repeat_num)

#define CANCEL_REPEATED_TASK(taskId) EXECUTOR->CancelRepeatedTask(taskId)
