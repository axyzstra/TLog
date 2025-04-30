#pragma once
#include <ctime>
#include <iostream>

// 获取当前平台下的 页面 大小
size_t GetPageSize();

void LocalTime(std::tm* tm, std::time_t* now);

size_t GetProcessId();

size_t GetThreadId();