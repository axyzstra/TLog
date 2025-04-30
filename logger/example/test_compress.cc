#include <string.h>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <string>

#include "zlib_compress.h"
#include "zstd_compress.h"

int main() {
  logger::ZlibCompress zip;
  // logger::ZstdCompress zip;
  zip.ResetStream();
  const char* data =
      "This is a test string for zlib compression.This is a test string for zlib compression.This is a test string for "
      "zlib compression.";
  size_t dataSize = strlen(data);
  size_t out_size = zip.CompressedBound(dataSize);
  char* destData = new char[dataSize];

  out_size = zip.Compress(data, dataSize, destData, out_size);
  std::cout << "未压缩 / 压缩 = " << dataSize << "/" << out_size << " = " << static_cast<double>(dataSize) / out_size
            << std::endl;

  std::string decompress_str = zip.Decompress(destData, out_size);
  std::cout << "解压后数据：" << decompress_str << std::endl;

  delete[] destData;
  return 0;
}