#include <gtest/gtest.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include "effective_msg.pb.h"
#include "log_msg.h"

using namespace logger;

TEST(EffectiveMsgTest, SerializationRoundTrip) {
  EffectiveMsg original;
  original.set_level(static_cast<int32_t>(LogLevel::kInfo));
  original.set_timestamp(std::chrono::system_clock::now().time_since_epoch().count());
  original.set_pid(getpid());
  original.set_tid(syscall(SYS_gettid));
  original.set_line(__LINE__);
  original.set_file_name(__FILE__);
  original.set_func_name(__func__);
  original.set_log_info("Test log message");

  // 序列化
  std::string serialized;
  ASSERT_TRUE(original.SerializeToString(&serialized));

  // 反序列化
  EffectiveMsg parsed;
  ASSERT_TRUE(parsed.ParseFromString(serialized));

  EXPECT_EQ(original.level(), parsed.level());
  EXPECT_EQ(original.timestamp(), parsed.timestamp());
  EXPECT_EQ(original.file_name(), parsed.file_name());
  EXPECT_EQ(original.pid(), parsed.pid());
  EXPECT_EQ(original.tid(), parsed.tid());
  EXPECT_EQ(original.file_name(), parsed.file_name());
  EXPECT_EQ(original.func_name(), parsed.func_name());
  EXPECT_EQ(original.log_info(), parsed.log_info());
}