// Copyright (c) 2023-2024 xlog_decode contributors
// Licensed under the MIT License
//
// file_utils.cpp - FileUtils类的实现

#include "../include/file_utils.h"

// 标准库头文件
#include <sys/stat.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>

#if defined(_WIN32)
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#define PATH_SEPARATOR "\\"
#else
#include <dirent.h>
#include <unistd.h>
#define PATH_SEPARATOR "/"
#endif

namespace xlog_decode {

bool FileUtils::PathExists(const std::string& path) {
  return FileExists(path);
}

bool FileUtils::IsDirectory(const std::string& path) {
  struct stat buffer;
  if (stat(path.c_str(), &buffer) != 0) {
    return false;
  }
  return (buffer.st_mode & S_IFDIR) != 0;
}

bool FileUtils::HasExtension(const std::string& file_path,
                             const std::string& extension) {
  // 检查文件是否有指定的扩展名
  std::string file_ext = GetFileExtension(file_path);
  return file_ext == extension;
}

std::string FileUtils::GetFileExtension(const std::string& file_path) {
  std::string file_name = GetFileName(file_path);
  size_t last_dot = file_name.find_last_of(".");
  if (last_dot == std::string::npos) {
    return "";
  }
  return file_name.substr(last_dot);
}

std::string FileUtils::GetFileName(const std::string& file_path) {
  size_t last_slash = file_path.find_last_of("/\\");
  if (last_slash == std::string::npos) {
    return file_path;
  }
  return file_path.substr(last_slash + 1);
}

std::string FileUtils::GetDirectoryName(const std::string& file_path) {
  size_t last_slash = file_path.find_last_of("/\\");
  if (last_slash == std::string::npos) {
    return "";
  }
  return file_path.substr(0, last_slash);
}

std::string FileUtils::JoinPath(const std::string& directory,
                                const std::string& file_name) {
  if (directory.empty()) {
    return file_name;
  }

  char last_char = directory[directory.size() - 1];
  if (last_char == '/' || last_char == '\\') {
    return directory + file_name;
  } else {
    return directory + PATH_SEPARATOR + file_name;
  }
}

bool FileUtils::ReadFile(const std::string& file_path,
                         std::vector<uint8_t>& buffer) {
  std::ifstream file(file_path, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Failed to open file: " << file_path << std::endl;
    return false;
  }

  // 获取文件大小
  file.seekg(0, std::ios::end);
  std::streamsize file_size = file.tellg();
  file.seekg(0, std::ios::beg);

  // 调整缓冲区大小并读取文件内容
  buffer.resize(file_size);
  if (!file.read(reinterpret_cast<char*>(buffer.data()), file_size)) {
    std::cerr << "Failed to read file: " << file_path << std::endl;
    return false;
  }

  return true;
}

bool FileUtils::WriteFile(const std::string& file_path,
                          const std::vector<uint8_t>& buffer) {
  std::ofstream file(file_path, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Failed to create file: " << file_path << std::endl;
    return false;
  }

  file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
  if (file.fail()) {
    std::cerr << "Failed to write to file: " << file_path << std::endl;
    return false;
  }

  return true;
}

std::vector<std::string> FileUtils::ScanDirectory(
    const std::string& dir_path,
    const std::vector<std::string>& extensions,
    bool recurse) {
  std::vector<std::string> result;

  if (!PathExists(dir_path) || !IsDirectory(dir_path)) {
    std::cerr << "Path is not a valid directory: " << dir_path << std::endl;
    return result;
  }

  // 获取目录中的所有文件
  std::vector<std::string> files = ListFilesInDirectory(dir_path);

  // 处理所有文件
  for (const auto& file_path : files) {
    if (IsDirectory(file_path)) {
      // 如果是目录且启用了递归扫描，扫描子目录
      if (recurse) {
        std::vector<std::string> sub_dir_files =
            ScanDirectory(file_path, extensions, recurse);
        result.insert(result.end(), sub_dir_files.begin(), sub_dir_files.end());
      }
    } else {
      // 对于文件，检查扩展名
      std::string ext = GetFileExtension(file_path);
      if (std::find(extensions.begin(), extensions.end(), ext) !=
          extensions.end()) {
        result.push_back(file_path);
      }
    }
  }

  return result;
}

std::vector<std::string> FileUtils::FindDecodedFiles(
    const std::string& dir_path,
    bool recurse) {
  // 解码后的文件以"_.log"结尾
  const std::string kDecodedFileExt = "_.log";
  std::vector<std::string> result;

  if (!PathExists(dir_path) || !IsDirectory(dir_path)) {
    std::cerr << "Path is not a valid directory: " << dir_path << std::endl;
    return result;
  }

  // 获取目录中的所有文件
  std::vector<std::string> files = ListFilesInDirectory(dir_path);

  // 处理所有文件
  for (const auto& file_path : files) {
    if (IsDirectory(file_path)) {
      // 如果是目录且启用了递归扫描，扫描子目录
      if (recurse) {
        std::vector<std::string> sub_dir_files =
            FindDecodedFiles(file_path, recurse);
        result.insert(result.end(), sub_dir_files.begin(), sub_dir_files.end());
      }
    } else {
      // 检查文件名是否以"_.log"结尾
      if (file_path.length() >= 5 &&
          file_path.substr(file_path.length() - 5) == kDecodedFileExt) {
        result.push_back(file_path);
      }
    }
  }

  return result;
}

bool FileUtils::DeleteFile(const std::string& file_path) {
  return std::remove(file_path.c_str()) == 0;
}

bool FileUtils::FileExists(const std::string& file_path) {
  struct stat buffer;
  return (stat(file_path.c_str(), &buffer) == 0);
}

bool FileUtils::CreateDirectory(const std::string& directory_path) {
  // 对于嵌套目录，递归创建
  if (directory_path.empty()) {
    return false;
  }

  // 检查目录是否已存在
  if (FileExists(directory_path) && IsDirectory(directory_path)) {
    return true;
  }

  // 获取父目录
  std::string parent = GetDirectoryName(directory_path);
  if (!parent.empty() && !FileExists(parent)) {
    // 先创建父目录
    if (!CreateDirectory(parent)) {
      return false;
    }
  }

  // 创建当前目录
#if defined(_WIN32)
  return _mkdir(directory_path.c_str()) == 0;
#else
  return mkdir(directory_path.c_str(), 0755) == 0;
#endif
}

std::string FileUtils::GetCurrentDirectory() {
  char buffer[1024];
#if defined(_WIN32)
  if (_getcwd(buffer, sizeof(buffer)) != nullptr) {
    return std::string(buffer);
  }
#else
  if (getcwd(buffer, sizeof(buffer)) != nullptr) {
    return std::string(buffer);
  }
#endif
  return "";
}

std::vector<std::string> FileUtils::ListFilesInDirectory(
    const std::string& directory_path) {
  std::vector<std::string> files;

  if (!FileExists(directory_path) || !IsDirectory(directory_path)) {
    return files;
  }

#if defined(_WIN32)
  // 在Windows上，使用popen执行dir命令
  std::string cmd = "dir /b \"" + directory_path + "\" 2>nul";
  FILE* pipe = _popen(cmd.c_str(), "r");
  if (!pipe) {
    return files;
  }

  char buffer[1024];
  while (fgets(buffer, sizeof(buffer), pipe)) {
    // 移除换行符
    size_t len = strlen(buffer);
    if (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r')) {
      buffer[len - 1] = 0;
    }
    if (len > 1 && (buffer[len - 2] == '\r')) {
      buffer[len - 2] = 0;
    }

    // 跳过"."和".."
    if (strcmp(buffer, ".") != 0 && strcmp(buffer, "..") != 0) {
      files.push_back(JoinPath(directory_path, buffer));
    }
  }

  _pclose(pipe);
#else
  // 在POSIX平台上，使用dirent
  DIR* dir = opendir(directory_path.c_str());
  if (dir != nullptr) {
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
      if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
        files.push_back(JoinPath(directory_path, entry->d_name));
      }
    }
    closedir(dir);
  }
#endif

  return files;
}

// 获取文件大小（字节）
uint64_t FileUtils::GetFileSize(const std::string& file_path) {
  try {
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
      return 0;
    }
    return static_cast<uint64_t>(file.tellg());
  } catch (...) {
    return 0;
  }
}

}  // namespace xlog_decode