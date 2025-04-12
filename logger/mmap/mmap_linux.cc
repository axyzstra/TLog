/**
 * 由于 linux windows 中 munmap mmap 不一致
 * 因此头文件中的一些成员函数需要分开实现
*/

#include "mmap_aux.h"
#include "defer.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace logger {
  
  bool MmapAux::TryMap_(size_t capacity) {
    // 以读写方式打开一个文件，若不存在则创建，并指定权限为 R W X
    int fd = open(file_path_.string().c_str(), O_RDWR | O_CREAT, S_IRWXU);
    LOG_DEFER {
      if (fd != -1) {
        close(fd);
      }
    };
    // 等价于
    // logger::ExecuteOnScopeExit defer([&](){
    //   if (fd != -1) {
    //     close(fd);
    //   }
    // });

    if (fd == -1) {
      return false;
    } else {
      ftruncate(fd, capacity);
    }
    handle_ = mmap(nullptr, capacity, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    return handle_ != MAP_FAILED;
  }
  
  void MmapAux::Unmap_() {
    if (handle_) {
      munmap(handle_, capacity_);
    }
    handle_ = nullptr;
  }

  // 将内存内容同步到文件
  void MmapAux::Sync_() {
    if (handle_) {
      msync(handle_, capacity_, MS_SYNC);
    }
  }
};