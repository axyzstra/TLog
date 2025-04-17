#include <gtest/gtest.h>
#include "log_common.h"
#include "log_msg.h"


using namespace logger;

class LogMsgTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 测试用的通用数据
        test_loc_ = SourceLocation("/home/axyz/usr/logger/logger/example/test.cc", 42, "test_function");
    }
    
    // 不要在类中直接初始化 LogMsg 成员变量，而是在需要的地方创建实例
    SourceLocation test_loc_;
};

// 测试带位置的构造函数
TEST_F(LogMsgTest, ConstructorWithLocation) {
    LogMsg test_msg(test_loc_, LogLevel::kError, "Error message");
    
    EXPECT_EQ(test_msg.location.file_name, "test.cc");
    EXPECT_EQ(test_msg.location.line, 42);
    EXPECT_EQ(test_msg.location.func_name, "test_function");
    EXPECT_EQ(test_msg.level, LogLevel::kError);
    EXPECT_EQ(test_msg.message, "Error message");
}

// 测试不带位置的构造函数
TEST_F(LogMsgTest, ConstructorWithoutLocation) {
    LogMsg msg(LogLevel::kInfo, "Info message");
    
    EXPECT_TRUE(msg.location.file_name.empty());
    EXPECT_EQ(msg.location.line, 0);
    EXPECT_TRUE(msg.location.func_name.empty());
    EXPECT_EQ(msg.level, LogLevel::kInfo);
    EXPECT_EQ(msg.message, "Info message");
}

// 测试拷贝构造函数
TEST_F(LogMsgTest, CopyConstructor) {
    LogMsg original(test_loc_, LogLevel::kError, "Error message");
    LogMsg copy(original);
    
    EXPECT_EQ(copy.location.file_name, "test.cc");
    EXPECT_EQ(copy.location.line, 42);
    EXPECT_EQ(copy.location.func_name, "test_function");
    EXPECT_EQ(copy.level, LogLevel::kError);
    EXPECT_EQ(copy.message, "Error message");
}

// 测试拷贝赋值运算符
TEST_F(LogMsgTest, CopyAssignment) {
    LogMsg original(test_loc_, LogLevel::kError, "Error message");
    LogMsg copy = original;
    
    EXPECT_EQ(copy.location.file_name, "test.cc");
    EXPECT_EQ(copy.location.line, 42);
    EXPECT_EQ(copy.location.func_name, "test_function");
    EXPECT_EQ(copy.level, LogLevel::kError);
    EXPECT_EQ(copy.message, "Error message");
}

// 测试空消息
TEST_F(LogMsgTest, EmptyMessage) {
    LogMsg empty_msg(LogLevel::kDebug, "");
    EXPECT_TRUE(empty_msg.message.empty());
}

// 测试不同日志级别
TEST_F(LogMsgTest, DifferentLogLevels) {
    LogMsg trace_msg(LogLevel::kTrace, "Trace message");
    EXPECT_EQ(trace_msg.level, LogLevel::kTrace);
    
    LogMsg debug_msg(LogLevel::kDebug, "Debug message");
    EXPECT_EQ(debug_msg.level, LogLevel::kDebug);
    
    LogMsg info_msg(LogLevel::kInfo, "Info message");
    EXPECT_EQ(info_msg.level, LogLevel::kInfo);
    
    LogMsg warn_msg(LogLevel::kWarn, "Warn message");
    EXPECT_EQ(warn_msg.level, LogLevel::kWarn);
    
    LogMsg error_msg(LogLevel::kError, "Error message");
    EXPECT_EQ(error_msg.level, LogLevel::kError);
    
    LogMsg fatal_msg(LogLevel::kFatal, "Fatal message");
    EXPECT_EQ(fatal_msg.level, LogLevel::kFatal);
    
    LogMsg off_msg(LogLevel::kOff, "Off message");
    EXPECT_EQ(off_msg.level, LogLevel::kOff);
}

// 测试空文件名处理
TEST_F(LogMsgTest, EmptyFileName) {
    SourceLocation empty_loc("", 0, "");
    LogMsg empty_loc_msg(empty_loc, LogLevel::kError, "Error");
    
    EXPECT_TRUE(empty_loc_msg.location.file_name.empty());
    EXPECT_TRUE(empty_loc_msg.location.func_name.empty());
    EXPECT_EQ(empty_loc_msg.location.line, 0);
}

// 测试路径处理
TEST_F(LogMsgTest, PathHandling) {
    // Unix风格路径
    SourceLocation unix_loc("/path/to/src/file.cpp", 10, "func");
    LogMsg unix_msg(unix_loc, LogLevel::kInfo, "msg");
    EXPECT_EQ(unix_msg.location.file_name, "file.cpp");
    
    // Windows风格路径
    SourceLocation windows_loc("C:\\path\\to\\src\\file.cpp", 10, "func");
    LogMsg windows_msg(windows_loc, LogLevel::kInfo, "msg");
    EXPECT_EQ(windows_msg.location.file_name, "file.cpp");
}