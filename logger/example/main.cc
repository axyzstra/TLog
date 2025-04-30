#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

// 假设MmapAux类定义在一个头文件中
#include "mmap_aux.h"  // 替换为你实际的头文件

using namespace logger;

void test_mmap_aux() {
  std::cout << "=== 测试MmapAux类 ===" << std::endl;

  // 创建测试文件路径
  std::filesystem::path test_file = "/home/axyz/usr/logger/logger/example/example.txt";
  std::cout << "测试文件: " << test_file << std::endl;

  // 确保测试文件目录存在
  std::filesystem::create_directories(test_file.parent_path());

  // 如果测试文件已存在则删除
  if (std::filesystem::exists(test_file)) {
    std::filesystem::remove(test_file);
  }

  // 创建一个空文件
  {
    std::ofstream file(test_file);
    if (!file) {
      std::cerr << "无法创建测试文件: " << test_file << std::endl;
      return;
    }
  }

  try {
    // 测试1: 构造函数和初始化
    std::cout << "测试1: 构造函数和初始化" << std::endl;
    MmapAux mmap(test_file);
    assert(mmap.Empty());
    assert(mmap.GetSize() == 0);
    std::cout << "  通过: 构造函数和初始化" << std::endl;

    // 测试2: 向mmap文件推送数据
    std::cout << "测试2: 推送数据" << std::endl;
    std::vector<uint8_t> data1 = {1, 2, 3, 4, 5};
    mmap.Push(data1.data(), data1.size());
    assert(!mmap.Empty());
    assert(mmap.GetSize() == data1.size());

    // 验证数据是否正确写入
    uint8_t* stored_data = mmap.Data();
    for (size_t i = 0; i < data1.size(); i++) {
      assert(stored_data[i] == data1[i]);
    }
    std::cout << "  通过: 推送数据" << std::endl;

    // 测试3: 推送更多数据（应该触发调整大小）
    std::cout << "测试3: 推送更多数据" << std::endl;
    std::vector<uint8_t> data2(1024, 0);
    // 填充随机数据
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);
    for (auto& byte : data2) {
      byte = static_cast<uint8_t>(distrib(gen));
    }

    mmap.Push(data2.data(), data2.size());
    assert(mmap.GetSize() == data1.size() + data2.size());

    // 验证所有数据是否被保留
    stored_data = mmap.Data();
    for (size_t i = 0; i < data1.size(); i++) {
      assert(stored_data[i] == data1[i]);
    }
    for (size_t i = 0; i < data2.size(); i++) {
      assert(stored_data[data1.size() + i] == data2[i]);
    }
    std::cout << "  通过: 推送更多数据" << std::endl;

    // 测试4: 清除数据
    std::cout << "测试4: 清除数据" << std::endl;
    mmap.Clear();
    assert(mmap.Empty());
    assert(mmap.GetSize() == 0);
    std::cout << "  通过: 清除数据" << std::endl;

    // 测试5: 手动调整大小
    std::cout << "测试5: 手动调整大小" << std::endl;
    size_t new_size = 2048;
    mmap.ReSize(new_size);
    assert(mmap.GetSize() == new_size);

    // 填充模式
    stored_data = mmap.Data();
    for (size_t i = 0; i < new_size; i++) {
      stored_data[i] = i % 256;
    }

    // 验证模式
    for (size_t i = 0; i < new_size; i++) {
      assert(stored_data[i] == i % 256);
    }
    std::cout << "  通过: 手动调整大小" << std::endl;

    // 测试6: GetRatio
    std::cout << "测试6: GetRatio" << std::endl;
    double ratio = mmap.GetRatio();
    std::cout << "  内存使用率: " << ratio << std::endl;
    assert(ratio > 0.0 && ratio <= 1.0);
    std::cout << "  通过: GetRatio" << std::endl;

    // 使用相同的文件创建新实例
    MmapAux mmap2(test_file);
    assert(mmap2.GetSize() == new_size);

    // 验证数据是否仍然存在
    stored_data = mmap2.Data();
    for (size_t i = 0; i < new_size; i++) {
      assert(stored_data[i] == i % 256);
    }
    std::cout << "  通过: 实例间的持久性" << std::endl;

  } catch (const std::exception& e) {
    std::cerr << "测试失败，异常: " << e.what() << std::endl;
    assert(false);
  }

  // 清理
  // if (std::filesystem::exists(test_file)) {
  //     std::filesystem::remove(test_file);
  // }

  std::cout << "所有测试通过！" << std::endl;
}

int main() {
  test_mmap_aux();
  return 0;
}