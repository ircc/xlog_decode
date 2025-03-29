// Copyright (c) 2023-2024 xlog_decode contributors
// Licensed under the MIT License
//
// xlog_decoder.cpp - XlogDecoder类的实现

#include "xlog_decoder.h"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

// 恢复zlib.h的引用
#include <zlib.h>
// 添加zstd.h引用
#include <zstd.h>

#include "file_utils.h"
#include "xlog_constants.h"

namespace xlog_decode {

namespace {
// 预定义的解压缩缓冲区块大小
constexpr size_t kChunkSize = 1024;
}  // namespace

XlogDecoder::XlogDecoder() : last_seq_(0) {}

XlogDecoder::~XlogDecoder() = default;

bool XlogDecoder::IsXlogFile(const std::string& file_path) {
  return FileUtils::HasExtension(file_path, kXlogFileExt) ||
         FileUtils::HasExtension(file_path, kMmapFileExt);
}

bool XlogDecoder::IsMarsXlogV2(const std::string& file_path) {
  try {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
      return false;
    }

    uint8_t magic;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));

    return (magic == MAGIC_NO_COMPRESS_START ||
            magic == MAGIC_NO_COMPRESS_START1 ||
            magic == MAGIC_COMPRESS_START || magic == MAGIC_COMPRESS_START1 ||
            magic == MAGIC_COMPRESS_START2 ||
            magic == MAGIC_NO_COMPRESS_NO_CRYPT_START ||
            magic == MAGIC_COMPRESS_NO_CRYPT_START);
  } catch (const std::exception&) {
    return false;
  }
}

bool XlogDecoder::IsMarsXlogV3(const std::string& file_path) {
  try {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
      return false;
    }

    uint8_t magic;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));

    return (magic == MAGIC_SYNC_ZSTD_START ||
            magic == MAGIC_SYNC_NO_CRYPT_ZSTD_START ||
            magic == MAGIC_ASYNC_ZSTD_START ||
            magic == MAGIC_ASYNC_NO_CRYPT_ZSTD_START);
  } catch (const std::exception&) {
    return false;
  }
}

bool XlogDecoder::IsZipFile(const std::string& file_path) {
  try {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
      return false;
    }

    // ZIP签名是'PK\x03\x04'
    char signature[4];
    file.read(signature, sizeof(signature));

    return (signature[0] == 'P' && signature[1] == 'K' &&
            signature[2] == 0x03 && signature[3] == 0x04);
  } catch (const std::exception&) {
    return false;
  }
}

std::string XlogDecoder::GenerateOutputFilename(const std::string& input_file) {
  // 获取原文件所在目录
  std::string dir_name = FileUtils::GetDirectoryName(input_file);
  // 获取原文件名（带扩展名）
  std::string base_filename = FileUtils::GetFileName(input_file);
  // 生成新文件路径：原目录/原文件名_.log
  return FileUtils::JoinPath(dir_name, base_filename + "_.log");
}

bool XlogDecoder::DecodeFile(const std::string& input_file,
                             const std::string& output_file,
                             bool skip_error_blocks) {
  if (!FileUtils::PathExists(input_file)) {
    std::cerr << "File does not exist: " << input_file << std::endl;
    return false;
  }

  // 重置序列计数器
  last_seq_ = 0;

  // 确定文件类型并调用相应的解码器
  if (IsMarsXlogV2(input_file) || IsMarsXlogV3(input_file)) {
    return ParseMarsXlogFile(input_file, output_file, skip_error_blocks);
  } else if (IsZipFile(input_file)) {
    return DecodeZipFile(input_file, output_file);
  } else {
    return ParseMarsXlogFile(input_file, output_file, skip_error_blocks);
  }
}

bool XlogDecoder::ParseMarsXlogFile(const std::string& input_file,
                                    const std::string& output_file,
                                    bool skip_error_blocks) {
  try {
    // 将整个输入文件读入缓冲区
    std::vector<uint8_t> buffer;
    if (!FileUtils::ReadFile(input_file, buffer)) {
      std::cerr << "Failed to read input file: " << input_file << std::endl;
      return false;
    }

    if (buffer.empty()) {
      std::cerr << "Input file is empty: " << input_file << std::endl;
      return false;
    }

    // 查找有效块的可能起始位置
    std::vector<int32_t> start_positions;
    start_positions.push_back(0);  // 总是从开始处尝试

    // 添加其他潜在的起始位置
    for (size_t i = 1; i < buffer.size(); ++i) {
      uint8_t magic = buffer[i];
      if (magic == MAGIC_NO_COMPRESS_START ||
          magic == MAGIC_NO_COMPRESS_START1 || magic == MAGIC_COMPRESS_START ||
          magic == MAGIC_COMPRESS_START1 || magic == MAGIC_COMPRESS_START2 ||
          magic == MAGIC_NO_COMPRESS_NO_CRYPT_START ||
          magic == MAGIC_COMPRESS_NO_CRYPT_START ||
          magic == MAGIC_SYNC_ZSTD_START ||
          magic == MAGIC_SYNC_NO_CRYPT_ZSTD_START ||
          magic == MAGIC_ASYNC_ZSTD_START ||
          magic == MAGIC_ASYNC_NO_CRYPT_ZSTD_START) {
        start_positions.push_back(static_cast<int32_t>(i));
      }
    }

    // 从每个可能的起始位置尝试解码
    std::vector<uint8_t> output_buffer;
    bool success = false;

    for (int32_t start_pos : start_positions) {
      try {
        int32_t current_pos = start_pos;
        std::vector<uint8_t> temp_buffer;

        while (current_pos >= 0 &&
               static_cast<size_t>(current_pos) < buffer.size()) {
          current_pos =
              DecodeBlock(buffer, current_pos, temp_buffer, skip_error_blocks);
          if (current_pos < 0) {
            break;
          }
        }

        if (!temp_buffer.empty()) {
          output_buffer = std::move(temp_buffer);
          success = true;
          break;
        }
      } catch (const std::exception&) {
        // 尝试下一个起始位置
        continue;
      }
    }

    if (!success) {
      std::cerr << "No valid log data found in file: " << input_file
                << std::endl;
      return false;
    }

    if (output_buffer.empty()) {
      std::cerr << "No valid log content decoded from: " << input_file
                << std::endl;
      return false;
    }

    // 将解码后的数据写入输出文件
    if (!FileUtils::WriteFile(output_file, output_buffer)) {
      std::cerr << "Failed to write output file: " << output_file << std::endl;
      return false;
    }

    return true;
  } catch (const std::exception& e) {
    std::cerr << "Error decoding file: " << e.what() << std::endl;
    return false;
  }
}

bool XlogDecoder::DecodeZipFile(const std::string& input_file,
                                const std::string& output_file) {
  // 注意：这只是一个占位符。要实际实现，你需要使用ZIP库
  // 比如libzip、zlib或minizip来处理ZIP文件。
  std::cerr << "ZIP decoding is not implemented in this version" << std::endl;
  return false;
}

std::pair<bool, std::string> XlogDecoder::IsValidLogBuffer(
    const std::vector<uint8_t>& buffer,
    int32_t offset,
    int32_t count) {
  int32_t current_offset = offset;
  int32_t remaining_count = count;

  while (true) {
    if (static_cast<size_t>(current_offset) == buffer.size()) {
      return {true, ""};
    }

    uint8_t magic_start = buffer[current_offset];
    uint32_t crypt_key_len = 0;

    if (magic_start == MAGIC_NO_COMPRESS_START ||
        magic_start == MAGIC_COMPRESS_START ||
        magic_start == MAGIC_COMPRESS_START1) {
      crypt_key_len = 4;
    } else if (magic_start == MAGIC_COMPRESS_START2 ||
               magic_start == MAGIC_NO_COMPRESS_START1 ||
               magic_start == MAGIC_NO_COMPRESS_NO_CRYPT_START ||
               magic_start == MAGIC_COMPRESS_NO_CRYPT_START ||
               magic_start == MAGIC_SYNC_ZSTD_START ||
               magic_start == MAGIC_SYNC_NO_CRYPT_ZSTD_START ||
               magic_start == MAGIC_ASYNC_ZSTD_START ||
               magic_start == MAGIC_ASYNC_NO_CRYPT_ZSTD_START) {
      crypt_key_len = 64;
    } else {
      std::ostringstream oss;
      oss << "buffer[" << current_offset
          << "]:" << static_cast<int>(magic_start) << " != MAGIC_NUM_START";
      return {false, oss.str()};
    }

    uint32_t header_len = 1 + 2 + 1 + 1 + 4 + crypt_key_len;

    if (static_cast<size_t>(current_offset + header_len + 1 + 1) >
        buffer.size()) {
      std::ostringstream oss;
      oss << "offset:" << (current_offset + header_len + 1 + 1)
          << " > buffer size:" << buffer.size();
      return {false, oss.str()};
    }

    // 从头部提取长度字段
    uint32_t length = 0;
    std::memcpy(&length,
                &buffer[current_offset + header_len - 4 - crypt_key_len],
                sizeof(length));

    if (static_cast<size_t>(current_offset + header_len + length + 1) >
        buffer.size()) {
      std::ostringstream oss;
      oss << "log length:" << length << ", end pos "
          << (current_offset + header_len + length + 1)
          << " > buffer size:" << buffer.size();
      return {false, oss.str()};
    }

    if (buffer[current_offset + header_len + length] != MAGIC_END) {
      std::ostringstream oss;
      oss << "log length:" << length << ", buffer["
          << (current_offset + header_len + length) << "]:"
          << static_cast<int>(buffer[current_offset + header_len + length])
          << " != MAGIC_END";
      return {false, oss.str()};
    }

    // 递减计数器并更新当前偏移量
    remaining_count--;
    if (remaining_count <= 0) {
      return {true, ""};
    }

    current_offset = current_offset + header_len + length + 1;
  }
}

int32_t XlogDecoder::FindLogStartPosition(const std::vector<uint8_t>& buffer,
                                          int32_t count) {
  int32_t offset = 0;

  while (static_cast<size_t>(offset) < buffer.size()) {
    // 检查所有可能的魔数值
    uint8_t value = buffer[offset];
    if (value == MAGIC_NO_COMPRESS_START || value == MAGIC_NO_COMPRESS_START1 ||
        value == MAGIC_COMPRESS_START || value == MAGIC_COMPRESS_START1 ||
        value == MAGIC_COMPRESS_START2 ||
        value == MAGIC_NO_COMPRESS_NO_CRYPT_START ||
        value == MAGIC_COMPRESS_NO_CRYPT_START ||
        value == MAGIC_SYNC_ZSTD_START ||
        value == MAGIC_SYNC_NO_CRYPT_ZSTD_START ||
        value == MAGIC_ASYNC_ZSTD_START ||
        value == MAGIC_ASYNC_NO_CRYPT_ZSTD_START) {
      // 尝试验证日志缓冲区
      try {
        auto result = IsValidLogBuffer(buffer, offset, count);
        if (result.first) {
          return offset;
        }
      } catch (...) {
        // 忽略异常并继续搜索
      }
    }

    offset++;
  }

  // 如果我们已经搜索完整个缓冲区但没有找到有效的起始位置，
  // 返回0以尝试从头开始解析
  return 0;
}

int32_t XlogDecoder::DecodeBlock(const std::vector<uint8_t>& buffer,
                                 int32_t offset,
                                 std::vector<uint8_t>& output_buffer,
                                 bool skip_error_blocks) {
  if (static_cast<size_t>(offset) >= buffer.size()) {
    return -1;
  }

  // 检查这是否是一个有效的日志缓冲区
  auto result = IsValidLogBuffer(buffer, offset, 1);
  if (!result.first) {
    if (skip_error_blocks) {
      int32_t fix_pos = FindLogStartPosition(
          std::vector<uint8_t>(buffer.begin() + offset, buffer.end()), 1);

      if (fix_pos == -1) {
        return -1;
      } else {
        std::string error_msg =
            "[F]xlog_decode error len=" + std::to_string(fix_pos) +
            ", result:" + result.second + "\n";
        output_buffer.insert(output_buffer.end(), error_msg.begin(),
                             error_msg.end());
        offset += fix_pos;
      }
    } else {
      // 不跳过错误块，直接返回错误
      return -1;
    }
  }

  uint8_t magic_start = buffer[offset];
  uint32_t crypt_key_len = 0;

  if (magic_start == MAGIC_NO_COMPRESS_START ||
      magic_start == MAGIC_COMPRESS_START ||
      magic_start == MAGIC_COMPRESS_START1) {
    crypt_key_len = 4;
  } else if (magic_start == MAGIC_COMPRESS_START2 ||
             magic_start == MAGIC_NO_COMPRESS_START1 ||
             magic_start == MAGIC_NO_COMPRESS_NO_CRYPT_START ||
             magic_start == MAGIC_COMPRESS_NO_CRYPT_START ||
             magic_start == MAGIC_SYNC_ZSTD_START ||
             magic_start == MAGIC_SYNC_NO_CRYPT_ZSTD_START ||
             magic_start == MAGIC_ASYNC_ZSTD_START ||
             magic_start == MAGIC_ASYNC_NO_CRYPT_ZSTD_START) {
    crypt_key_len = 64;
  } else {
    std::string error_msg = "in DecodeBuffer buffer[" + std::to_string(offset) +
                            "]:" + std::to_string(magic_start) +
                            " != MAGIC_NUM_START\n";
    output_buffer.insert(output_buffer.end(), error_msg.begin(),
                         error_msg.end());
    return -1;
  }

  uint32_t header_len = 1 + 2 + 1 + 1 + 4 + crypt_key_len;

  // 提取头部字段
  uint32_t length = 0;
  uint16_t seq = 0;
  uint8_t begin_hour = 0;
  uint8_t end_hour = 0;

  std::memcpy(&length, &buffer[offset + header_len - 4 - crypt_key_len],
              sizeof(length));
  std::memcpy(&seq, &buffer[offset + header_len - 4 - crypt_key_len - 2 - 2],
              sizeof(seq));
  begin_hour = buffer[offset + header_len - 4 - crypt_key_len - 1 - 1];
  end_hour = buffer[offset + header_len - 4 - crypt_key_len - 1];

  // 复制主体数据
  std::vector<uint8_t> body_buffer(
      buffer.begin() + offset + header_len,
      buffer.begin() + offset + header_len + length);

  // 检查序列号的连续性
  if (seq != 0 && seq != 1 && last_seq_ != 0 && seq != (last_seq_ + 1)) {
    std::string warning =
        "[F]xlog_decode log seq:" + std::to_string(last_seq_ + 1) + "-" +
        std::to_string(seq - 1) + " is missing\n";
    output_buffer.insert(output_buffer.end(), warning.begin(), warning.end());
  }

  if (seq != 0) {
    last_seq_ = seq;
  }

  try {
    // 处理不同的压缩格式
    if (magic_start == MAGIC_NO_COMPRESS_START1 ||
        magic_start == MAGIC_COMPRESS_START2) {
      // 旧格式 - 无需特殊处理
      output_buffer.insert(output_buffer.end(), body_buffer.begin(),
                           body_buffer.end());
    } else if (magic_start == MAGIC_SYNC_ZSTD_START ||
               magic_start == MAGIC_SYNC_NO_CRYPT_ZSTD_START ||
               magic_start == MAGIC_ASYNC_ZSTD_START ||
               magic_start == MAGIC_ASYNC_NO_CRYPT_ZSTD_START) {
      // ZSTD压缩
      if (!DecompressZstd(body_buffer.data(), body_buffer.size(),
                          output_buffer)) {
        std::string error_msg = "[F]xlog_decode ZSTD decompress error\n";
        output_buffer.insert(output_buffer.end(), error_msg.begin(),
                             error_msg.end());
      }
    } else if (magic_start == MAGIC_COMPRESS_START ||
               magic_start == MAGIC_COMPRESS_NO_CRYPT_START) {
      // ZLIB压缩
      if (!DecompressZlib(body_buffer.data(), body_buffer.size(),
                          output_buffer)) {
        std::string error_msg = "[F]xlog_decode decompress error\n";
        output_buffer.insert(output_buffer.end(), error_msg.begin(),
                             error_msg.end());
      }
    } else if (magic_start == MAGIC_COMPRESS_START1) {
      // 带嵌入长度的特殊格式
      std::vector<uint8_t> decompress_data;
      size_t pos = 0;

      while (pos < body_buffer.size()) {
        if (pos + 2 > body_buffer.size()) {
          break;
        }

        uint16_t single_log_len = 0;
        std::memcpy(&single_log_len, &body_buffer[pos], sizeof(single_log_len));
        pos += 2;

        if (pos + single_log_len > body_buffer.size()) {
          break;
        }

        decompress_data.insert(decompress_data.end(), body_buffer.begin() + pos,
                               body_buffer.begin() + pos + single_log_len);

        pos += single_log_len;
      }

      if (!DecompressZlib(decompress_data.data(), decompress_data.size(),
                          output_buffer)) {
        std::string error_msg = "[F]xlog_decode decompress error\n";
        output_buffer.insert(output_buffer.end(), error_msg.begin(),
                             error_msg.end());
      }
    } else {
      // 无压缩，直接追加数据
      output_buffer.insert(output_buffer.end(), body_buffer.begin(),
                           body_buffer.end());
    }
  } catch (const std::exception& e) {
    std::string error_msg =
        "[F]xlog_decode decompress error: " + std::string(e.what()) + "\n";
    output_buffer.insert(output_buffer.end(), error_msg.begin(),
                         error_msg.end());
  }

  // 返回此块之后的位置
  return offset + header_len + length + 1;
}

bool XlogDecoder::DecompressZlib(const uint8_t* input_data,
                                 size_t input_size,
                                 std::vector<uint8_t>& output_buffer) {
  // 恢复zlib解压缩实现
  if (input_size == 0) {
    return true;  // 没有需要解压的数据
  }

  z_stream strm;
  unsigned char out[kChunkSize];

  // 初始化zlib进行解压缩
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;

  int ret = inflateInit2(&strm, -MAX_WBITS);  // 原始deflate格式
  if (ret != Z_OK) {
    return false;
  }

  // 设置输入数据
  strm.avail_in = static_cast<uInt>(input_size);
  strm.next_in =
      const_cast<Bytef*>(input_data);  // 安全的转换，因为zlib不会修改输入

  // 解压直到完成
  do {
    strm.avail_out = kChunkSize;
    strm.next_out = out;

    ret = inflate(&strm, Z_NO_FLUSH);
    if (ret != Z_OK && ret != Z_STREAM_END) {
      inflateEnd(&strm);

      if (ret == Z_NEED_DICT || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
        return false;
      }
    }

    unsigned int have = kChunkSize - strm.avail_out;
    output_buffer.insert(output_buffer.end(), out, out + have);

  } while (strm.avail_out == 0);

  // 清理
  inflateEnd(&strm);
  return ret == Z_STREAM_END || ret == Z_OK;
}

bool XlogDecoder::DecompressZstd(const uint8_t* input_data,
                                 size_t input_size,
                                 std::vector<uint8_t>& output_buffer) {
  if (input_size == 0) {
    return true;  // 没有需要解压的数据
  }

  // 获取解压后的大小
  unsigned long long const frame_content_size =
      ZSTD_getFrameContentSize(input_data, input_size);
  if (frame_content_size == ZSTD_CONTENTSIZE_ERROR ||
      frame_content_size == ZSTD_CONTENTSIZE_UNKNOWN) {
    // 无法确定大小，使用增量解压
    size_t const out_bufsize = ZSTD_DStreamOutSize();
    std::vector<char> out_buffer(out_bufsize);

    ZSTD_DStream* const dstream = ZSTD_createDStream();
    if (dstream == nullptr) {
      return false;
    }

    ZSTD_initDStream(dstream);

    ZSTD_inBuffer input = {input_data, input_size, 0};
    while (input.pos < input.size) {
      ZSTD_outBuffer output = {out_buffer.data(), out_buffer.size(), 0};
      size_t const ret = ZSTD_decompressStream(dstream, &output, &input);
      if (ZSTD_isError(ret)) {
        ZSTD_freeDStream(dstream);
        return false;
      }
      output_buffer.insert(output_buffer.end(), out_buffer.begin(),
                           out_buffer.begin() + output.pos);
    }

    ZSTD_freeDStream(dstream);
    return true;
  } else {
    // 已知解压后大小，直接解压
    std::vector<char> decompress_buffer(frame_content_size);
    size_t const dsize =
        ZSTD_decompress(decompress_buffer.data(), decompress_buffer.size(),
                        input_data, input_size);

    if (ZSTD_isError(dsize)) {
      return false;
    }

    output_buffer.insert(output_buffer.end(), decompress_buffer.begin(),
                         decompress_buffer.begin() + dsize);
    return true;
  }
}

}  // namespace xlog_decode