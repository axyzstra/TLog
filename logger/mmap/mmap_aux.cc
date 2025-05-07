#include "mmap_aux.h"

#include <string.h>
#include <fstream>
#include "file_util.h"
#include "sys_util.h"

namespace logger {
// mmap 映射内存初始大小，避免多次扩容
static constexpr size_t kDefaultCapacity = 512 * 1024;
MmapAux::MmapAux(fpath file_path) : file_path_(std::move(file_path)), handle_(nullptr), capacity_(0) {
  if (!std::filesystem::exists(file_path_)) {
    std::filesystem::create_directories(file_path_.parent_path());
    std::ofstream ofs(file_path_, std::ios::out | std::ios::binary);
    ofs.close();
  }
  size_t file_size = fs::GetFileSize(file_path_);
  size_t dst_size = std::max(file_size + sizeof(MmapHeader), kDefaultCapacity);
  Reserve_(dst_size);
  Init_();
}

bool MmapAux::IsValid_() const {
  MmapHeader* header = Header_();
  if (!header) {
    return false;
  }
  return header->magic == MmapHeader::kMagic;
}

void MmapAux::ReSize(size_t new_size) {
  if (!IsValid_()) {
    return;
  }
  Reserve_(new_size + sizeof(MmapHeader));
  Header_()->size = new_size;
}

uint8_t* MmapAux::Data() const {
  if (!IsValid_()) {
    return nullptr;
  }
  return static_cast<uint8_t*>(handle_) + sizeof(MmapHeader);
}

bool MmapAux::Empty() const {
  return GetSize() == 0;
}

double MmapAux::GetRatio() const {
  if (!IsValid_()) {
    return 0.0;
  }
  return static_cast<double>(GetSize()) / (GetCapacity_() - sizeof(MmapHeader));
}

void MmapAux::Push(const void* data, size_t size) {
  if (!IsValid_()) {
    return;
  }
  // 当前文件总大小
  size_t new_size = GetSize() + size;
  Reserve_(new_size + sizeof(MmapHeader));
  memcpy(Data() + GetSize(), data, size);
  Header_()->size = new_size;
}

void MmapAux::Clear() {
  if (!IsValid_()) {
    return;
  }
  Header_()->size = 0;
}

inline size_t MmapAux::GetSize() const {
  if (!IsValid_()) {
    return 0;
  }
  return Header_()->size;
}

void MmapAux::Init_() {
  MmapHeader* header = Header_();
  if (!header) {
    return;
  }
  if (header->magic != MmapHeader::kMagic) {
    header->magic = MmapHeader::kMagic;
    header->size = 0;
  }
}

size_t MmapAux::GetValidCapacity_(size_t size) {
  size_t page_size = GetPageSize();
  return (size + page_size - 1) / page_size * page_size;
}

void MmapAux::Reserve_(size_t new_size) {
  // 若目前分配的内存大于需求的内存
  if (capacity_ >= new_size) {
    return;
  }
  new_size = GetValidCapacity_(new_size);
  // 若目前分配的内存已经达到要求
  if (capacity_ == new_size) {
    return;
  }
  Unmap_();
  TryMap_(new_size);
  capacity_ = new_size;
}

MmapAux::MmapHeader* MmapAux::Header_() const {
  if (!handle_) {
    return nullptr;
  }
  // 当前分配的内存小于 header 大小，表示当前空间是不满足要求
  if (capacity_ < sizeof(MmapHeader)) {
    return nullptr;
  }
  return static_cast<MmapHeader*>(handle_);
}

inline size_t MmapAux::GetCapacity_() const noexcept {
  return capacity_;
}
};  // namespace logger
