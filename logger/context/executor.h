#pragma once

#include <functional>
#include <memory>
#include <condition_variable>
#include <unordered_set>
#include <atomic>
#include <mutex>

#include "thread_pool.h"


namespace logger {
  using Task = std::function<void()>;
  using RepeatedTaskId = uint64_t;

  using TaskRunnerTag = uint64_t;
  
  class Executor {
    private:
      class ExecutorTimer {
        private:
          struct InternalS {
            Task task;
            std::chrono::time_point<std::chrono::high_resolution_clock> time_point_;
            RepeatedTaskId repeated_id;

            bool operator<(const InternalS& other) const {
              return time_point_ > other.time_point_;
            }
          };
        
        public:
          ExecutorTimer();
          ~ExecutorTimer();

          ExecutorTimer(const ExecutorTimer&) = delete;
          ExecutorTimer& operator=(const Executor&) = delete;
        
          bool Start();
          void Stop();

          void PostDelayedTask(Task task, const std::chrono::microseconds& delaytime);
          RepeatedTaskId PostRepeatedTask(Task task, const std::chrono::microseconds& delaytime, uint64_t repeat_num);

          void CancelRepeatedTask(RepeatedTaskId task_id);

        private:
          /// @brief 运行当前队列中的延时任务
          void Run_();

          /// @brief 辅助函数，添加重复执行的延时任务
          /// @brief 这个函数也作为一个任务，每次运行则调用一次 task，减少 repeat_num
          /// @param task 
          /// @param delaytime 
          /// @param id 
          /// @param repeat_num 
          void PostRepeatedTask_(Task task, const std::chrono::microseconds& delaytime, RepeatedTaskId id, uint64_t repeat_num);
          // void PostTask_(Task task, std::chrono::microseconds& delaytime,  RepeatedTaskId id, uint64_t repeat_num);
        private:
          std::priority_queue<InternalS> queue_;
          std::unique_ptr<ThreadPool> thread_pool_;
          std::mutex mutex_;
          std::condition_variable cond_;
          std::atomic<bool> running_;

          std::atomic<RepeatedTaskId> repeated_task_id_;
          std::unordered_set<RepeatedTaskId> repeated_task_id_set;
      };

      class ExecutorContext {
        public:
          ExecutorContext();
          ~ExecutorContext() = default;

          ExecutorContext(const ExecutorContext&) = delete;
          ExecutorContext& operator=(const ExecutorContext&) = delete;

          TaskRunnerTag AddTaskRunner(const TaskRunnerTag& tag);

        private:
          using TaskRunner = ThreadPool;
          using TaskRunnerPtr = std::unique_ptr<TaskRunner>;
          friend class Executor;

        private:
          TaskRunner* GetTaskRunner(const TaskRunnerTag& tag);
        private:
          std::atomic<TaskRunnerTag> index;
          // 每个 tag 有一个 runner(即一个 Strand 队列)
          std::unordered_map<TaskRunnerTag, TaskRunnerPtr> task_runner_dict_;
          std::mutex mutex_;
      };


    public:
      Executor();
      ~Executor();

      Executor(const Executor&) = delete;
      Executor& operator=(const Executor&) = delete;

      TaskRunnerTag AddTaskRunner(const TaskRunnerTag& tag);
      
      void PostTask(const TaskRunnerTag& runner_tag, Task task);


      template <typename R, typename P>
      void PostDelayedTask(const TaskRunnerTag& runner_tag, Task task, const std::chrono::duration<R, P>& delaytime) {
        Task func = std::bind(&Executor::PostTask, this, runner_tag, std::move(task));
        executor_timer_->Start();
        executor_timer_->PostRepeatedTask(std::move(func), std::chrono::duration_cast<std::chrono::microseconds>(delaytime));
      }


      template <typename R, typename P>
      RepeatedTaskId PostRepeatedTask(const TaskRunnerTag& runner_tag, Task task, const std::chrono::duration<R, P>& delaytime, uint64_t repeat_num) {
        Task func = std::bind(&Executor::PostTask, this, runner_tag, std::move(task));
        executor_timer_->Start();
        return executor_timer_->PostRepeatedTask(std::move(func), std::chrono::duration_cast<std::chrono::milliseconds>(delaytime), repeat_num);
      }


      template <typename F, typename... Args>
      auto PostTaskAndGetResult(const TaskRunnerTag& runner_tag, F&& f, Args&&... args) -> std::shared_ptr<std::future<decltype(f(args...))>> {
        auto task_runner = executor_context_->GetTaskRunner(runner_tag);
        auto ret = task_runner->RunRetTask(std::forward<F>(f), std::forward<Args>(args)...);
        return ret;
      }

      void CancelRepeatedTask(RepeatedTaskId task_id) {
        executor_timer_->CancelRepeatedTask(task_id);
      }
    private:
      std::unique_ptr<ExecutorContext> executor_context_;
      std::unique_ptr<ExecutorTimer> executor_timer_;
  };
}