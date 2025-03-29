// Copyright (c) 2023-2024 xlog_decode contributors
// Licensed under the MIT License
//
// file_utils.cpp - Implementation of the FileUtils class

#include "../include/file_utils.h"

// Standard library headers
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
  // Check if file has the specified extension
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

  // Get file size
  file.seekg(0, std::ios::end);
  std::streamsize file_size = file.tellg();
  file.seekg(0, std::ios::beg);

  // Resize buffer and read file content
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

  // Get all files in the directory
  std::vector<std::string> files = ListFilesInDirectory(dir_path);

  // Process all files
  for (const auto& file_path : files) {
    if (IsDirectory(file_path)) {
      // If directory and recursive scanning is enabled, scan subdirectory
      if (recurse) {
        std::vector<std::string> sub_dir_files =
            ScanDirectory(file_path, extensions, recurse);
        result.insert(result.end(), sub_dir_files.begin(), sub_dir_files.end());
      }
    } else {
      // For files, check the extension
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
  // Decoded files have the extension .xlog_.log
  const std::string kDecodedFileExt = "_.log";
  std::vector<std::string> result;

  if (!PathExists(dir_path) || !IsDirectory(dir_path)) {
    std::cerr << "Path is not a valid directory: " << dir_path << std::endl;
    return result;
  }

  // Get all files in the directory
  std::vector<std::string> files = ListFilesInDirectory(dir_path);

  // Process all files
  for (const auto& file_path : files) {
    if (IsDirectory(file_path)) {
      // If directory and recursive scanning is enabled, scan subdirectory
      if (recurse) {
        std::vector<std::string> sub_dir_files =
            FindDecodedFiles(file_path, recurse);
        result.insert(result.end(), sub_dir_files.begin(), sub_dir_files.end());
      }
    } else {
      // For files, check if name ends with "_.log"
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
  // For nested directories, create recursively
  if (directory_path.empty()) {
    return false;
  }

  // Check if directory already exists
  if (FileExists(directory_path) && IsDirectory(directory_path)) {
    return true;
  }

  // Get parent directory
  std::string parent = GetDirectoryName(directory_path);
  if (!parent.empty() && !FileExists(parent)) {
    // Create parent directory first
    if (!CreateDirectory(parent)) {
      return false;
    }
  }

  // Create current directory
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
  // On Windows, use popen to execute dir command
  std::string cmd = "dir /b \"" + directory_path + "\" 2>nul";
  FILE* pipe = _popen(cmd.c_str(), "r");
  if (!pipe) {
    return files;
  }

  char buffer[1024];
  while (fgets(buffer, sizeof(buffer), pipe)) {
    // Remove line break
    size_t len = strlen(buffer);
    if (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r')) {
      buffer[len - 1] = 0;
    }
    if (len > 1 && (buffer[len - 2] == '\r')) {
      buffer[len - 2] = 0;
    }

    // Skip "." and ".."
    if (strcmp(buffer, ".") != 0 && strcmp(buffer, "..") != 0) {
      files.push_back(JoinPath(directory_path, buffer));
    }
  }

  _pclose(pipe);
#else
  // On POSIX platforms, use dirent
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

}  // namespace xlog_decode