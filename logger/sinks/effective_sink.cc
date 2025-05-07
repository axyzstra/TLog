#include "effective_sink.h"

#include <cstring>
#include <fstream>

#include "aes_crypt.h"
#include "context.h"
#include "effective_formatter.h"
#include "file_util.h"
#include "internal_log.h"
#include "sys_util.h"
#include "timer_counter.h"
#include "zstd_compress.h"

namespace logger {

EffectiveSink::EffectiveSink(Conf conf) : conf_(std::move(conf)) {
  // 创建新的日志文件，打印必要信息
  LOG_INFO("EffectiveSink: dir = {}, prefix = {}, pub_key = {}, interval = {}, single_size = {}, total_size = {}",
           conf_.dir.string(), conf_.prefix, conf_.pub_key, conf.interval.count(), conf_.single_size.count(),
           conf.total_size.count());
  // 存放日志目录不存在则创建
  if (!std::filesystem::exists(conf_.dir)) {
    std::filesystem::create_directories(conf_.dir);
  }
  task_runner_ = NEW_TASK_RUNNER(123456);
  formatter_ = std::make_unique<EffectiveFormatter>();
  // 获取公钥和私钥
  auto ecdh_key = crypt::GenECDHKey();
  auto client_pri = std::get<0>(ecdh_key);
  client_pub_key_ = std::get<1>(ecdh_key);
  // 将 server 的公钥转为二进制
  std::string server_pub_key_bin = crypt::HexKeyToBinary(conf_.pub_key);
  // server_pub 和自己的私钥生成用于 AES 加密的私钥
  std::string shared_secret = crypt::GenECDHSharedSecret(client_pri, server_pub_key_bin);
  crypt_ = std::make_unique<crypt::AESCrypt>(shared_secret);

  compress_ = std::make_unique<ZstdCompress>();

  master_cache_ = std::make_unique<MmapAux>(conf_.dir / "master_cache");
  slave_cache_ = std::make_unique<MmapAux>(conf_.dir / "slave_cache");
  if (!master_cache_ || !slave_cache_) {
    throw std::runtime_error("EffectiveSink::EffectiveSink: create mmap failed");
  }

  // 保证 slave_cache 为空
  if (!slave_cache_->Empty()) {
    is_slave_free_.store(true);
    POST_TASK(task_runner_, [this]() { CacheToFile_(); });
    WAIT_TASK_IDLE(task_runner_);
  }
  // 保证 master_cache 为空，若不为空则和从 cache 交换，然后和上一步一样
  if (!master_cache_->Empty()) {
    if (is_slave_free_.load()) {
      is_slave_free_.store(false);
      SwapCache_();
    }
    POST_TASK(task_runner_, [this]() { CacheToFile_(); });
    WAIT_TASK_IDLE(task_runner_);
  }
  // 设置日志文件存活时间，每隔一段时间运行一次
  POST_REPEATED_TASK(task_runner_, [this]() { ElimateFiles_(); }, conf_.interval, -1);
}

void EffectiveSink::ElimateFiles_() {
  LOG_INFO("EffectiveSink::ElimateFiles_: start");
  std::vector<std::filesystem::path> files;
  for (auto& p : std::filesystem::directory_iterator(conf_.dir)) {
    if (p.path().extension() == ".log") {
      files.push_back(p.path());
    }
  }

  std::sort(files.begin(), files.end(), [](const std::filesystem::path& lhs, const std::filesystem::path& rhs) {
    return std::filesystem::last_write_time(lhs) > std::filesystem::last_write_time(rhs);
  });

  size_t total_bytes = space_cast<bytes>(conf_.total_size).count();
  size_t used_bytes = 0;
  for (auto& file : files) {
    used_bytes += fs::GetFileSize(file);
    if (used_bytes > total_bytes) {
      LOG_INFO("EffectiveSink::ElimateFiles_: remove file={}", file.string());
      std::filesystem::remove(file);
    }
  }
}

void EffectiveSink::CacheToFile_() {
  if (is_slave_free_.load()) {
    return;
  }
  if (slave_cache_->Empty()) {
    is_slave_free_.store(true);
    return;
  }
  {
    auto file_path = GetFilePath_();
    detail::ChunkHeader chunk_header;
    chunk_header.size = slave_cache_->GetSize();
    memcpy(chunk_header.pub_key, client_pub_key_.data(), client_pub_key_.size());
    std::ofstream ofs(file_path, std::ios::binary | std::ios::app);
    ofs.write(reinterpret_cast<char*>(&chunk_header), sizeof(chunk_header));
    ofs.write(reinterpret_cast<char*>(slave_cache_->Data()), chunk_header.size);
    ofs.close();
  }
  slave_cache_->Clear();
  is_slave_free_.store(true);
}

std::filesystem::path EffectiveSink::GetFilePath_() {
  // 日志格式 {prefix}_{datetime}.log 此处获取日期和时间
  auto GetDateTimePath = [this]() -> std::filesystem::path {
    std::time_t now = std::time(nullptr);
    std::tm tm;
    LocalTime(&tm, &now);
    char time_buf[32] = {0};
    std::strftime(time_buf, sizeof(time_buf), "%Y%m%d%H%M%S", &tm);
    return (conf_.dir / (conf_.prefix + "_" + time_buf));
  };

  if (log_file_path_.empty()) {
    log_file_path_ = GetDateTimePath().string() + ".log";
  } else {
    auto file_size = fs::GetFileSize(log_file_path_);
    bytes single_bytes = space_cast<bytes>(conf_.single_size);
    // 若当前日志文件超过限制大小，则重新创建日志文件
    if (file_size > single_bytes.count()) {
      auto date_time_path = GetDateTimePath().string();
      auto file_path = date_time_path + ".log";
      // 若重新创建的日志文件重复，则加上后缀
      if (std::filesystem::exists(file_path)) {
        int index = 0;
        for (auto& p : std::filesystem::directory_iterator(conf_.dir)) {
          if (p.path().filename().string().find(date_time_path) != std::string::npos) {
            index++;
          }
        }
        log_file_path_ = date_time_path + "_" + std::to_string(index) + ".log";
      } else {
        log_file_path_ = file_path;
      }
    }
  }
  LOG_INFO("EffectiveSink::GetFilePath_: log_file_path={}", log_file_path_.string());
  return log_file_path_;
}

void EffectiveSink::SetFormatter(std::unique_ptr<Formatter> formatter) {
  formatter_ = std::move(formatter);
}

void EffectiveSink::Flush() {
  TIMER_COUNT("Flush Function");
  POST_TASK(task_runner_, [this]() { CacheToFile_(); });
  WAIT_TASK_IDLE(task_runner_);

  if (is_slave_free_.load()) {
    is_slave_free_.store(false);
    SwapCache_();
  }

  POST_TASK(task_runner_, [this]() { CacheToFile_(); });
  WAIT_TASK_IDLE(task_runner_);
}

void EffectiveSink::Log(const LogMsg& msg) {
  static thread_local std::string buf;
  formatter_->Format(msg, &buf);
  // 若主 cache 为空，重置压缩缓冲区，只重置一次以增大压缩比
  if (master_cache_->Empty()) {
    compress_->ResetStream();
  }
  {
    std::lock_guard<std::mutex> lock(mutex_);
    compressed_buf_.reserve(compress_->CompressedBound(buf.size()));
    size_t compressed_size =
        compress_->Compress(buf.data(), buf.size(), compressed_buf_.data(), compressed_buf_.capacity());
    if (compressed_size == 0) {
      LOG_ERROR("EffectiveSink::Log: compress failed");
      return;
    }
    // 开始加密
    encryped_buf_.clear();
    encryped_buf_.reserve(compressed_size + 16);
    crypt_->Encrypt(compressed_buf_.data(), compressed_size, encryped_buf_);
    if (encryped_buf_.empty()) {
      LOG_ERROR("EffectiveSink::Log: encrypt failed");
      return;
    }
    // 将加密后数据写入 cache
    WriteToCache_(encryped_buf_.data(), encryped_buf_.size());
  }
  // 若主 cache 超过 0.8 则和从 cache 交换，交换后将从 cache 写入文件
  if (master_cache_->GetRatio() > 0.8) {
    if (is_slave_free_.load()) {
      is_slave_free_.store(false);
      SwapCache_();
    }
    POST_TASK(task_runner_, [this]() { CacheToFile_(); });
    WAIT_TASK_IDLE(task_runner_);
  }
}
void EffectiveSink::WriteToCache_(const void* data, uint32_t size) {
  detail::ItemHeader item_header;
  item_header.size = size;
  master_cache_->Push(&item_header, sizeof(item_header));
  master_cache_->Push(data, size);
}
}  // namespace logger