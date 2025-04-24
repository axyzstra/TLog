#include <iostream>
#include <string>
#include <functional>
#include <chrono>
#include <thread>

#include "executor.h"
#include "context.h"



int main() {
    std::cout << "--- 开始使用宏提交任务示例 ---" << std::endl;
    // 在使用任务运行器之前，通常需要先创建它。
    // NEW_TASK_RUNNER 宏会调用 EXECUTOR->AddTaskRunner(tag)。
    logger::TaskRunnerTag workerTag = 1;  
    std::cout << "使用 NEW_TASK_RUNNER 宏为标签 '" << workerTag << "' 创建任务运行器..." << std::endl;
    NEW_TASK_RUNNER(workerTag);
    std::cout << "任务运行器创建宏调用完成。" << std::endl;

    // 定义一个简单的无返回值任务
    logger::Task simpleTask = [](){
        std::cout << "    [Task] 这是一个通过 POST_TASK 提交的简单任务。" << std::endl;
    };

    // 使用 POST_TASK 宏提交这个简单任务
    // 这个宏会调用 EXECUTOR->PostTask(runner_tag, task)
    std::cout << "使用 POST_TASK 宏提交一个简单任务到 '" << workerTag << "'..." << std::endl;
    POST_TASK(workerTag, simpleTask);
    std::cout << "简单任务提交宏调用完成。" << std::endl;

    // 使用 WAIT_TASK_IDLE 宏等待特定任务运行器处理完其队列中在其之前提交的任务
    // 这个宏会调用 EXECUTOR->PostTaskAndGetResult(runner_tag, [](){})->wait()
    std::cout << "使用 WAIT_TASK_IDLE 宏等待任务运行器 '" << workerTag << "' 空闲 (等待之前任务完成)..." << std::endl;
    WAIT_TASK_IDLE(workerTag);
    std::cout << "任务运行器 '" << workerTag << "' 已空闲 (WAIT_TASK_IDLE 宏完成)。" << std::endl;

    // 定义一个用于重复执行的任务
    logger::Task repeatedTaskAction = [](){
        std::cout << "    [Repeated Task] 这是一个重复执行的任务实例。" << std::endl;
    };

    // 定义重复任务的参数
    std::chrono::microseconds repeatDelay(500000); // 500 毫秒
    uint64_t numberOfRepeats = 4; // 重复 4 次

    // 使用 POST_REPEATED_TASK 宏提交一个重复任务
    // 这个宏会调用 EXECUTOR->PostRepeatedTask(runner_tag, task, delaytime, repeat_num)
    std::cout << "使用 POST_REPEATED_TASK 宏提交一个重复任务到 '" << workerTag << "'..." << std::endl;
    std::cout << "  延迟: " << std::chrono::duration_cast<std::chrono::milliseconds>(repeatDelay).count() << "ms, 重复次数: " << numberOfRepeats << std::endl;
    logger::RepeatedTaskId taskId = POST_REPEATED_TASK(workerTag, repeatedTaskAction, repeatDelay, numberOfRepeats);
    std::cout << "重复任务提交宏调用完成，返回的任务 ID 为: " << taskId << std::endl;

    // 注意：重复任务会在后台异步执行。主线程不会等待它完成。
    // 你可能需要在此处等待一段时间来观察重复任务的输出
    std::cout << "主线程继续执行，等待一段时间观察重复任务执行...\n";
    std::this_thread::sleep_for(std::chrono::seconds(3)); // 等待 3 秒


    // EXECUTOR->CancelRepeatedTask(taskId);

    std::cout << "--- 宏使用示例结束 ---" << std::endl;

    return 0;
}