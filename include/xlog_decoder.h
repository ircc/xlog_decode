// Copyright (c) 2023-2024 xlog_decode contributors
// Licensed under the MIT License
//
// xlog_decoder.h - XLOG format decoder class

#ifndef XLOG_DECODE_XLOG_DECODER_H_
#define XLOG_DECODE_XLOG_DECODER_H_

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "xlog_constants.h"

namespace xlog_decode {

// XlogDecoder class handles decoding of XLOG format files
class XlogDecoder {
 public:
  XlogDecoder();
  ~XlogDecoder();

  // Disable copy and assignment
  XlogDecoder(const XlogDecoder&) = delete;
  XlogDecoder& operator=(const XlogDecoder&) = delete;

  // Check if file is a valid XLOG file
  static bool IsXlogFile(const std::string& file_path);

  // Check if file is a valid XLOG v2 format (ZLIB compression)
  static bool IsMarsXlogV2(const std::string& file_path);

  // Check if file is a valid XLOG v3 format (ZSTD compression)
  static bool IsMarsXlogV3(const std::string& file_path);

  // Check if file is a standard ZIP file
  static bool IsZipFile(const std::string& file_path);

  // Decode a single XLOG file
  bool DecodeFile(const std::string& input_file,
                  const std::string& output_file,
                  bool skip_error_blocks = true);

  // Generate output filename based on input filename
  static std::string GenerateOutputFilename(const std::string& input_file);

 private:
  // Parse Mars XLOG formatted file
  bool ParseMarsXlogFile(const std::string& input_file,
                         const std::string& output_file,
                         bool skip_error_blocks);

  // Decode ZIP formatted file
  bool DecodeZipFile(const std::string& input_file,
                     const std::string& output_file);

  // Decode a single block of XLOG data
  int32_t DecodeBlock(const std::vector<uint8_t>& buffer,
                      int32_t offset,
                      std::vector<uint8_t>& output_buffer,
                      bool skip_error_blocks);

  // Check if a buffer contains valid XLOG data
  std::pair<bool, std::string> IsValidLogBuffer(
      const std::vector<uint8_t>& buffer,
      int32_t offset,
      int32_t count);

  // Find the start position of a valid XLOG block
  int32_t FindLogStartPosition(const std::vector<uint8_t>& buffer,
                               int32_t count);

  // Decompress ZLIB compressed data
  bool DecompressZlib(const uint8_t* input_data,
                      size_t input_size,
                      std::vector<uint8_t>& output_buffer);

  // Global sequence number for log continuity checking
  uint16_t last_seq_ = 0;
};

}  // namespace xlog_decode

#endif  // XLOG_DECODE_XLOG_DECODER_H_