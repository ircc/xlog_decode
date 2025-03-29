// Copyright (c) 2023-2024 xlog_decode contributors
// Licensed under the MIT License
//
// file_utils.h - File handling utilities

#ifndef XLOG_DECODE_FILE_UTILS_H_
#define XLOG_DECODE_FILE_UTILS_H_

#include <cstdint>
#include <string>
#include <vector>

namespace xlog_decode {

// FileUtils class provides a set of utilities for file operations
class FileUtils {
 public:
  // Check if a path exists
  static bool PathExists(const std::string& path);

  // Check if a path is a directory
  static bool IsDirectory(const std::string& path);

  // Check if a file has a specific extension
  static bool HasExtension(const std::string& file_path,
                           const std::string& extension);

  // Get the file extension
  static std::string GetFileExtension(const std::string& file_path);

  // Get the filename without path
  static std::string GetFileName(const std::string& file_path);

  // Get the directory part of a path
  static std::string GetDirectoryName(const std::string& file_path);

  // Joins path components
  static std::string JoinPath(const std::string& dir, const std::string& file);

  // Read a file into a byte vector
  static bool ReadFile(const std::string& file_path,
                       std::vector<uint8_t>& buffer);

  // Write a byte vector to a file
  static bool WriteFile(const std::string& file_path,
                        const std::vector<uint8_t>& buffer);

  // Scan a directory for files with a specific extension (recursively if
  // requested)
  static std::vector<std::string> ScanDirectory(
      const std::string& dir_path,
      const std::vector<std::string>& extensions,
      bool recurse = false);

  // Find all decoded files (with _.log extension) in a directory
  static std::vector<std::string> FindDecodedFiles(const std::string& dir_path,
                                                   bool recurse = false);

  // Delete a file
  static bool DeleteFile(const std::string& file_path);

  // File and directory operations
  static bool FileExists(const std::string& file_path);
  static bool CreateDirectory(const std::string& directory_path);

  // Other utility methods
  static std::string GetCurrentDirectory();
  static std::vector<std::string> ListFilesInDirectory(
      const std::string& directory_path);
};

}  // namespace xlog_decode

#endif  // XLOG_DECODE_FILE_UTILS_H_