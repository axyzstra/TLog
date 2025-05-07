#pragma once

#include <atomic>
#include <filesystem>
#include <memory>
#include <mutex>

#include "compress.h"
#include "crypt.h"
#include "executor.h"
#include "formatter.h"
#include "mmap_aux.h"
#include "sink.h"
#include "space.h"

namespace logger {

namespace detail {
// mmap 块的 header
struct ChunkHeader {
  static constexpr uint64_t kMagic = 0xdeadbeefdada1100;
  uint64_t magic;
  uint64_t size;
  char pub_key[128];

  ChunkHeader() : magic(kMagic), size(0) {}
};

// 单行 mmap 的 header
struct ItemHeader {
  static constexpr uint32_t kMagic = 0xbe5fba11;
  uint32_t magic;
  uint32_t size;

  ItemHeader() : magic(kMagic), size(0) {}
};

}  // namespace detail

class EffectiveSink final : public LogSink {
 public:
  /// @brief 存储日志文件相关的信息
  struct Conf {
    std::filesystem::path dir;  // 日志保存目录
    std::string prefix;         // 日志名前缀 {prefix}_{datatime}.log
    std::string pub_key;
    std::chrono::minutes interval{5};  // 文件存活时间
    megabytes single_size{4};          // 单个文件大小
    megabytes total_size{100};         // 总文件大小
  };

  /// @brief 构造函数，主要完成如下功能：
  /// 1. 初始化 running_tag，此后当前日志文件的操作都在该线程下完成；
  /// 2. 获取 ECDH 私钥、公钥，使用生成的私钥和提供的 server_pub 生成 AES 密钥；
  /// 3. 创建加密对象、压缩对象；
  /// 4. 创建主从 mmap；
  /// 5. 保证主从 mmap 为空；
  /// @param conf
  explicit EffectiveSink(Conf conf);
  ~EffectiveSink() = default;

  void Log(const LogMsg& msg) override;
  void SetFormatter(std::unique_ptr<Formatter> formatter) override;
  void Flush() override;

 private:
  /// @brief 将从 cache 中的内容写入文件
  void CacheToFile_();

  /// @brief 获取日志文件名称
  std::filesystem::path GetFilePath_();

  void SwapCache_() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::swap(master_cache_, slave_cache_);
  }

  /// @brief 当日志文件总大小大于限制值，则删除超时文件
  void ElimateFiles_();

  /// @brief 将内存中的数据 data 写入主 cache 中
  /// @param data
  /// @param size
  void WriteToCache_(const void* data, uint32_t size);

 private:
  Conf conf_;
  std::unique_ptr<Formatter> formatter_;   // 格式化日志信息
  std::unique_ptr<crypt::Crypt> crypt_;    // 用于日志加密
  std::unique_ptr<Compression> compress_;  // 用于日志压缩
  std::unique_ptr<MmapAux> master_cache_;  // 主 cache
  std::unique_ptr<MmapAux> slave_cache_;   // 从 cache
  std::filesystem::path log_file_path_;    // 日志保存路径

  std::string client_pub_key_;

  std::string compressed_buf_;  // 压缩数据存放缓存
  std::string encryped_buf_;    // 加密数据存放缓存

  // 同步机制
  std::mutex mutex_;
  TaskRunnerTag task_runner_;

  std::atomic<bool> is_slave_free_{true};  // 从 cache 是否为空
};
}  // namespace logger