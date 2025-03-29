// Copyright (c) 2023-2024 xlog_decode contributors
// Licensed under the MIT License
//
// file_utils.h - 文件处理工具

#ifndef XLOG_DECODE_FILE_UTILS_H_
#define XLOG_DECODE_FILE_UTILS_H_

#include <cstdint>
#include <string>
#include <vector>

namespace xlog_decode {

// FileUtils类提供一组文件操作工具
class FileUtils {
 public:
  // 检查路径是否存在
  static bool PathExists(const std::string& path);

  // 检查路径是否为目录
  static bool IsDirectory(const std::string& path);

  // 检查文件是否有特定扩展名
  static bool HasExtension(const std::string& file_path,
                           const std::string& extension);

  // 获取文件扩展名
  static std::string GetFileExtension(const std::string& file_path);

  // 获取不含路径的文件名
  static std::string GetFileName(const std::string& file_path);

  // 获取路径的目录部分
  static std::string GetDirectoryName(const std::string& file_path);

  // 连接路径组件
  static std::string JoinPath(const std::string& dir, const std::string& file);

  // 将文件读入字节向量
  static bool ReadFile(const std::string& file_path,
                       std::vector<uint8_t>& buffer);

  // 将字节向量写入文件
  static bool WriteFile(const std::string& file_path,
                        const std::vector<uint8_t>& buffer);

  // 获取文件大小（字节）
  static uint64_t GetFileSize(const std::string& file_path);

  // 扫描目录中具有特定扩展名的文件（可递归）
  static std::vector<std::string> ScanDirectory(
      const std::string& dir_path,
      const std::vector<std::string>& extensions,
      bool recurse = false);

  // 在目录中查找所有已解码文件（带_.log扩展名）
  static std::vector<std::string> FindDecodedFiles(const std::string& dir_path,
                                                   bool recurse = false);

  // 删除文件
  static bool DeleteFile(const std::string& file_path);

  // 文件和目录操作
  static bool FileExists(const std::string& file_path);
  static bool CreateDirectory(const std::string& directory_path);

  // 其他实用方法
  static std::string GetCurrentDirectory();
  static std::vector<std::string> ListFilesInDirectory(
      const std::string& directory_path);
};

}  // namespace xlog_decode

#endif  // XLOG_DECODE_FILE_UTILS_H_