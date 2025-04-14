#pragma once

#include <stdint.h>
#include <filesystem>

namespace logger {
  namespace fs {
    using fpath = std::filesystem::path;
    size_t GetFileSize(const fpath& file_path);
  };
}