#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "aes_crypt.h"
#include "compress.h"
#include "crypt.h"
#include "decode_formatter.h"
#include "effective_sink.h"
#include "zstd_compress.h"

#include "effective_msg.pb.h"

using namespace logger;
using namespace logger::crypt;
using namespace logger::detail;

std::unique_ptr<DecodeFormatter> decode_formatter;
std::unique_ptr<Compression> decompress;

void DecodeFile(const std::string& input_file_path, const std::string& pri_key, const std::string& output_file_path);
void AppendDataToFile(const std::string& file_path, const std::string& data);
void DecodeChunkData(char* data,
                     size_t size,
                     const std::string& cli_pub_key,
                     const std::string& svr_pri_key,
                     std::string& output_data);
void DecodeItemData(char* data, size_t size, Crypt* crypt, std::string& output_data);
std::vector<char> ReadFile(const std::string& input_file_path);

void DecodeFile(const std::string& input_file_path, const std::string& pri_key, const std::string& output_file_path) {
  auto input_data = ReadFile(input_file_path);
  if (input_data.size() < sizeof(ChunkHeader)) {
    throw std::runtime_error("DecodeFile: input file is too small");
    return;
  }
  auto chunk_header = reinterpret_cast<ChunkHeader*>(input_data.data());
  if (chunk_header->magic != ChunkHeader::kMagic) {
    throw std::runtime_error("DecodeFile: invalid file magic");
    return;
  }
  size_t offset = 0;
  size_t file_size = input_data.size();
  std::string output_data;
  output_data.reserve(1024 * 1024);
  while (offset < file_size) {
    ChunkHeader* chunk_header = reinterpret_cast<ChunkHeader*>(input_data.data() + offset);
    if (chunk_header->magic != ChunkHeader::kMagic) {
      throw std::runtime_error("DecodeFile: invalid chunk magic");
      return;
    }
    output_data.clear();
    offset += sizeof(ChunkHeader);
    DecodeChunkData(input_data.data() + offset, chunk_header->size, std::string(chunk_header->pub_key), pri_key,
                    output_data);
    offset += chunk_header->size;
    AppendDataToFile(output_file_path, output_data);
  }
}

void AppendDataToFile(const std::string& file_path, const std::string& data) {
  std::ofstream ofs(file_path, std::ios::binary | std::ios::app);
  ofs.write(data.data(), data.size());
}

/// @brief 将 Chunk 中的数据解析到 output_data 中
/// Chunk 中包含多个 Item
/// 解析过程：使用 svr 私钥和 cli 公钥生成 aes 的加解密 密钥
/// 得到密钥后，分别解析每一个 Item
/// @param data
/// @param size
/// @param cli_pub_key
/// @param svr_pri_key
/// @param output_data
void DecodeChunkData(char* data,
                     size_t size,
                     const std::string& cli_pub_key,
                     const std::string& svr_pri_key,
                     std::string& output_data) {
  std::cout << "decode chunk " << size << std::endl;
  std::string svr_pri_key_bin = HexKeyToBinary(svr_pri_key);
  std::string shared_secret = GenECDHSharedSecret(svr_pri_key_bin, cli_pub_key);
  std::unique_ptr<Crypt> crypt = std::make_unique<AESCrypt>(shared_secret);
  size_t offset = 0;
  size_t count = 0;
  while (offset < size) {
    ++count;
    if (count % 1000 == 0) {
      std::cout << "decode item:" << count << std::endl;
    }
    ItemHeader* item_header = reinterpret_cast<ItemHeader*>(data + offset);
    if (item_header->magic != ItemHeader::kMagic) {
      throw std::runtime_error("DecodeChunkData: invalid item magic");
      return;
    }
    offset += sizeof(ItemHeader);
    DecodeItemData(data + offset, item_header->size, crypt.get(), output_data);
    offset += item_header->size;
    output_data.push_back('\n');
  }
}

/// @brief 解析 Item 中的数据，流程 原始数据->解密->解压->格式化->写入缓存
/// @param data
/// @param size
/// @param crypt
/// @param output_data
void DecodeItemData(char* data, size_t size, Crypt* crypt, std::string& output_data) {
  std::string decrypted = crypt->Decrypt(data, size);
  std::string decompressed = decompress->Decompress(decrypted.data(), decrypted.size());
  EffectiveMsg msg;
  msg.ParseFromString(decompressed);
  std::string formatter_logger;
  decode_formatter->Format(msg, formatter_logger);
  output_data.append(formatter_logger);
}

/// @brief 从文件中读取全部字符，将其保存到字符数组中
/// @param input_file_path
/// @return
std::vector<char> ReadFile(const std::string& input_file_path) {
  std::ifstream file(input_file_path, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("ReadFile: open file failed!");
  }
  std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  return buffer;
}

int main() {
  std::string input_file_path = "/home/axyz/usr/logger/logger/example/build/logger/loggerdemo_20250508181829.log";
  std::string pri_key = "FAA5BBE9017C96BF641D19D0144661885E831B5DDF52539EF1AB4790C05E665E";
  std::filesystem::path path(input_file_path);
  std::string file_name = path.filename();
  std::string output_file_path = "/home/axyz/usr/logger/logger/example/build/logger_decode/" + file_name;
  try {
    decode_formatter = std::make_unique<DecodeFormatter>();
    decode_formatter->SetPattern("[%l][%D:%S][%p:%t][%F:%f:%#]%v");
    decompress = std::make_unique<ZstdCompress>();
    DecodeFile(input_file_path, pri_key, output_file_path);
  } catch (const std::exception& e) {
    std::cerr << "Decode failed:" << e.what() << std::endl;
    return 1;
  }
  return 0;
}