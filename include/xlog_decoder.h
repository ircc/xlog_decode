// Copyright (c) 2023-2024 xlog_decode contributors
// Licensed under the MIT License
//
// xlog_decoder.h - XLOG格式解码器类

#ifndef XLOG_DECODE_XLOG_DECODER_H_
#define XLOG_DECODE_XLOG_DECODER_H_

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "xlog_constants.h"

namespace xlog_decode {

// XlogDecoder类处理XLOG格式文件的解码
class XlogDecoder {
 public:
  XlogDecoder();
  ~XlogDecoder();

  // 禁用拷贝和赋值
  XlogDecoder(const XlogDecoder&) = delete;
  XlogDecoder& operator=(const XlogDecoder&) = delete;

  // 检查文件是否为有效的XLOG文件
  static bool IsXlogFile(const std::string& file_path);

  // 检查文件是否为有效的XLOG v2格式（ZLIB压缩）
  static bool IsMarsXlogV2(const std::string& file_path);

  // 检查文件是否为有效的XLOG v3格式（ZSTD压缩）
  static bool IsMarsXlogV3(const std::string& file_path);

  // 检查文件是否为标准ZIP文件
  static bool IsZipFile(const std::string& file_path);

  // 解码单个XLOG文件
  bool DecodeFile(const std::string& input_file,
                  const std::string& output_file,
                  bool skip_error_blocks = true);

  // 根据输入文件名生成输出文件名
  static std::string GenerateOutputFilename(const std::string& input_file);

 private:
  // 解析Mars XLOG格式文件
  bool ParseMarsXlogFile(const std::string& input_file,
                         const std::string& output_file,
                         bool skip_error_blocks);

  // 解码ZIP格式文件
  bool DecodeZipFile(const std::string& input_file,
                     const std::string& output_file);

  // 解码单个XLOG数据块
  int32_t DecodeBlock(const std::vector<uint8_t>& buffer,
                      int32_t offset,
                      std::vector<uint8_t>& output_buffer,
                      bool skip_error_blocks);

  // 检查缓冲区是否包含有效的XLOG数据
  std::pair<bool, std::string> IsValidLogBuffer(
      const std::vector<uint8_t>& buffer,
      int32_t offset,
      int32_t count);

  // 查找有效XLOG块的起始位置
  int32_t FindLogStartPosition(const std::vector<uint8_t>& buffer,
                               int32_t count);

  // 解压ZLIB压缩数据
  bool DecompressZlib(const uint8_t* input_data,
                      size_t input_size,
                      std::vector<uint8_t>& output_buffer);

  // 解压ZSTD压缩数据
  bool DecompressZstd(const uint8_t* input_data,
                      size_t input_size,
                      std::vector<uint8_t>& output_buffer);

  // 用于日志连续性检查的全局序列号
  uint16_t last_seq_ = 0;
};

}  // namespace xlog_decode

#endif  // XLOG_DECODE_XLOG_DECODER_H_