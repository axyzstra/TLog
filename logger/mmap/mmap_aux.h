#pragma once
#include <filesystem>
#include <memory>

namespace logger {
class MmapAux {
  using fpath = std::filesystem::path;

 public:
  /// @brief 对 handle_ 分配和文件大小一致的内存大小
  /// @param file_path
  /// @return
  explicit MmapAux(fpath file_path);
  ~MmapAux() = default;

  MmapAux(const MmapAux&) = delete;
  MmapAux& operator=(const MmapAux&) = delete;

  /// @brief 调整内存
  /// @param new_size 内容大小: capacity_ - sizeof(header)
  void ReSize(size_t new_size);

  /// @brief 返回 mmap 映射内存中的数据起始位置(header_ 之后)
  /// @return
  uint8_t* Data() const;

  /// @brief 获取内存空间大小的真正占有大小(不包括 head_)
  /// @return
  size_t GetSize() const;

  /// @brief 将 head->size 置空表示不存放任何数据
  void Clear();

  /// @brief 通过 mmap 映射的内存向文件中写数据
  /// @param data
  /// @param size
  void Push(const void* data, size_t size);

  /// @brief 获取 mmap 映射的内存中的内存占用比例
  /// @return
  double GetRatio() const;

  bool Empty() const;

 private:
  // mmap 的 head
  struct MmapHeader {
    // magic 保证合法性
    static constexpr uint32_t kMagic = 0x3fff;
    uint32_t magic = kMagic;
    uint32_t size;
  };

 private:
  /// @brief 获取满足 new_size 的整数倍 页 的大小
  /// @param new_size
  size_t GetValidCapacity_(size_t new_size);

  /// @brief 重新分配内存空间大小
  /// @param new_size 整个映射内存的大小
  void Reserve_(size_t new_size);

  /// @brief 获取当前内存区的大小
  /// @return capacity_ 即内存区容量大小
  size_t GetCapacity_() const noexcept;

  /// @brief 和文件建立映射
  /// @param capacity 映射空间大小
  /// @return
  bool TryMap_(size_t capacity);

  /// @brief 取消当前内存空间和文件的映射
  void Unmap_();

  /// @brief 将当前内存空间的内容同步到文件
  void Sync_();

  /// @brief 对比 magic 以查看数据是否被篡改
  /// @return
  bool IsValid_() const;

  /// @brief 获取 mmap 空间最开始的空间
  /// @return 返回得到 header 的地址
  MmapHeader* Header_() const;

  /// @brief 初始化内存空间，主要是设置 header 中的值
  void Init_();

 private:
  // mmap 映射的文件路径
  fpath file_path_;
  // mmap 映射的内存地址
  void* handle_;
  // 当前分配的内存容量
  size_t capacity_;
};
};  // namespace logger