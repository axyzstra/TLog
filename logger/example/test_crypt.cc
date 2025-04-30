#include <iostream>
#include <cstring>
#include "aes_crypt.h"


int main() {
  logger::crypt::AESCrypt aes("0123456789abcdef");
  const char* text = "Hello, World";
  size_t input_size = strlen(text);
  std::string str = "";
  aes.Encrypt(text, input_size, str);
  std::cout << str << std::endl;

  std::string destr = aes.Decrypt(str.c_str(), str.size());
  std::cout << destr << std::endl;
}