#include "thread_pool.h"
#include <iostream>
#include <thread>

using namespace logger;


int main() {
  ThreadPool pool(5);
  pool.Start();

  for (size_t i = 0; i < 10; i++) {
    pool.RunTask([i](){
      std::cout << std::this_thread::get_id() << ": 执行-->" << i << std::endl;
    });
  }

  std::cout << "------------------------" << std::endl;

  for (size_t i = 0; i < 10; i++) {
    auto future = pool.RunRetTask([i](){
      std::cout << std::this_thread::get_id() << ": 执行-->" << i << std::endl;
      return i;
    });
    std::cout << "返回值：" << future->get() << std::endl;
  }

  return 0;
}