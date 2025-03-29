// Copyright (c) 2023-2024 xlog_decode contributors
// Licensed under the MIT License
//
// xlog_constants.h - 定义XLOG格式的常量和结构

#ifndef XLOG_DECODE_XLOG_CONSTANTS_H_
#define XLOG_DECODE_XLOG_CONSTANTS_H_

#include <cstdint>

namespace xlog_decode {

// XLOG格式的魔数常量
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

// XLOG文件头结构
#pragma pack(push, 1)
struct XlogHeader {
  uint8_t start;       // 指示格式的魔数
  uint16_t seq;        // 序列号
  uint8_t begin_hour;  // 开始小时
  uint8_t end_hour;    // 结束小时
  uint32_t length;     // 数据块长度
  uint8_t crypt[64];   // 加密数据（如果有）
};
#pragma pack(pop)

// 根据魔数计算头部长度
inline uint32_t GetHeaderLen(uint8_t magic) {
  if (magic == MAGIC_NO_COMPRESS_START || magic == MAGIC_COMPRESS_START ||
      magic == MAGIC_COMPRESS_START1) {
    return 1 + 2 + 1 + 1 + 4 + 4;  // 旧格式13字节
  } else {
    return 1 + 2 + 1 + 1 + 4 + 64;  // 新格式73字节
  }
}

// 计算尾部长度
inline uint32_t GetTrailerLen() {
  return sizeof(uint8_t);  // MAGIC_END是1字节
}

// 支持的文件扩展名
inline const char* kXlogFileExt = ".xlog";
inline const char* kMmapFileExt = ".mmap3";

}  // namespace xlog_decode

#endif  // XLOG_DECODE_XLOG_CONSTANTS_H_