// Copyright (c) 2023-2024 xlog_decode contributors
// Licensed under the MIT License
//
// xlog_constants.h - Defines constants and structures for XLOG format

#ifndef XLOG_DECODE_XLOG_CONSTANTS_H_
#define XLOG_DECODE_XLOG_CONSTANTS_H_

#include <cstdint>

namespace xlog_decode {

// Magic number constants for XLOG format
enum MagicNumbers : uint8_t {
  MAGIC_NO_COMPRESS_START = 0x03,
  MAGIC_NO_COMPRESS_START1 = 0x06,
  MAGIC_NO_COMPRESS_NO_CRYPT_START = 0x08,
  MAGIC_COMPRESS_START = 0x04,
  MAGIC_COMPRESS_START1 = 0x05,
  MAGIC_COMPRESS_START2 = 0x07,
  MAGIC_COMPRESS_NO_CRYPT_START = 0x09,
  MAGIC_SYNC_ZSTD_START = 0x0A,
  MAGIC_SYNC_NO_CRYPT_ZSTD_START = 0x0B,
  MAGIC_ASYNC_ZSTD_START = 0x0C,
  MAGIC_ASYNC_NO_CRYPT_ZSTD_START = 0x0D,
  MAGIC_END = 0x00
};

// XLOG file header structure
#pragma pack(push, 1)
struct XlogHeader {
  uint8_t start;       // Magic number indicating format
  uint16_t seq;        // Sequence number
  uint8_t begin_hour;  // Begin hour
  uint8_t end_hour;    // End hour
  uint32_t length;     // Length of the data block
  uint8_t crypt[64];   // Cryptographic data (if any)
};
#pragma pack(pop)

// Calculate header length based on the magic number
inline uint32_t GetHeaderLen(uint8_t magic) {
  if (magic == MAGIC_NO_COMPRESS_START || magic == MAGIC_COMPRESS_START ||
      magic == MAGIC_COMPRESS_START1) {
    return 1 + 2 + 1 + 1 + 4 + 4;  // 13 bytes for old format
  } else {
    return 1 + 2 + 1 + 1 + 4 + 64;  // 73 bytes for new format
  }
}

// Calculate trailer length
inline uint32_t GetTrailerLen() {
  return sizeof(uint8_t);  // MAGIC_END is 1 byte
}

// Supported file extensions
inline const char* kXlogFileExt = ".xlog";
inline const char* kMmapFileExt = ".mmap3";

}  // namespace xlog_decode

#endif  // XLOG_DECODE_XLOG_CONSTANTS_H_