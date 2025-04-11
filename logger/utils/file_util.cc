#include "file_util.h"


namespace logger {
  namespace fs {
    size_t GetFileSize(const fpath& file_path) {
      return std::filesystem::file_size(file_path);
    }
  };
};