// Copyright (c) 2023-2024 xlog_decode contributors
// Licensed under the MIT License
//
// main.cpp - xlog_decode工具的主入口点

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "file_utils.h"
#include "xlog_constants.h"
#include "xlog_decoder.h"

// 版本信息现在由构建系统通过XLOG_DECODE_VERSION宏提供

using namespace xlog_decode;

// 获取程序版本
const std::string GetVersion() {
  return "1.0.0";  // 硬编码版本，直到宏问题修复
}

// 打印程序用法
void PrintUsage() {
  std::cout << "xlog_decode - A tool for decoding XLOG format log files\n";
  std::cout << "Version: " << GetVersion() << "\n\n";
  std::cout << "Usage:\n";
  std::cout << "  xlog_decode <command> [options] <path>\n\n";
  std::cout << "Commands:\n";
  std::cout
      << "  decode   - Decode one or more XLOG files (recursive by default)\n";
  std::cout << "  clean    - Delete all decoded files in a directory "
               "(recursive by default)\n";
  std::cout << "  help     - Show this help information\n\n";
  std::cout << "Options:\n";
  std::cout << "  --no-recursive    - Disable recursive processing\n";
  std::cout << "  --keep-errors     - Don't skip blocks with errors during "
               "decoding\n";
  std::cout << "  --version         - Show version information\n\n";
  std::cout << "Examples:\n";
  std::cout
      << "  xlog_decode help                        - Show help information\n";
  std::cout
      << "  xlog_decode decode path/to/file.xlog    - Decode a single file\n";
  std::cout << "  xlog_decode decode path/to/dir          - Decode all XLOG "
               "files in directory and subdirectories\n";
  std::cout << "  xlog_decode decode --no-recursive path/to/dir - Decode XLOG "
               "files only in the top directory\n";
  std::cout << "  xlog_decode clean path/to/dir           - Delete all decoded "
               "files in directory and subdirectories\n";
}

// 解码单个文件
bool DecodeFile(const std::string& file_path, bool skip_error_blocks) {
  try {
    xlog_decode::XlogDecoder decoder;
    std::string output_file =
        xlog_decode::XlogDecoder::GenerateOutputFilename(file_path);

    // 获取输入文件大小
    auto input_file_size = xlog_decode::FileUtils::GetFileSize(file_path);
    double input_size_mb = static_cast<double>(input_file_size) / (1024 * 1024);

    // 添加时间测量
    auto start_time = std::chrono::high_resolution_clock::now();

    bool result = decoder.DecodeFile(file_path, output_file, skip_error_blocks);

    // 计算经过时间
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);

    if (result) {
      // 获取输出文件大小
      auto output_file_size = xlog_decode::FileUtils::GetFileSize(output_file);
      double output_size_mb =
          static_cast<double>(output_file_size) / (1024 * 1024);

      std::cout << output_file << " (cost: " << duration.count() << "ms, "
                << "size: " << std::fixed << std::setprecision(2)
                << input_size_mb << "MB -> " << output_size_mb << "MB)"
                << std::endl;
      return true;
    } else {
      std::cerr << "Failed to decode file: " << file_path
                << " (cost: " << duration.count() << "ms, "
                << "size: " << std::fixed << std::setprecision(2)
                << input_size_mb << "MB)" << std::endl;
      return false;
    }
  } catch (const std::exception& e) {
    std::cerr << "Error decoding file: " << e.what() << std::endl;
    return false;
  }
}

// 处理解码命令
int ProcessDecodeCommand(const std::vector<std::string>& args) {
  if (args.empty()) {
    std::cerr << "Error: Missing path argument for decode command\n\n";
    PrintUsage();
    return 1;
  }

  bool recursive = true;  // 默认启用递归
  bool skip_error_blocks = true;
  std::string path;

  // 解析选项
  for (size_t i = 0; i < args.size(); ++i) {
    if (args[i] == "--no-recursive") {
      recursive = false;  // 禁用递归搜索的选项
    } else if (args[i] == "--keep-errors") {
      skip_error_blocks = false;
    } else if (path.empty()) {
      path = args[i];
    }
  }

  if (path.empty()) {
    std::cerr << "Error: Missing path argument for decode command\n\n";
    PrintUsage();
    return 1;
  }

  if (!xlog_decode::FileUtils::PathExists(path)) {
    std::cerr << "Error: Path does not exist: " << path << std::endl;
    return 1;
  }

  if (xlog_decode::FileUtils::IsDirectory(path)) {
    // 处理目录
    std::vector<std::string> extensions = {kXlogFileExt, kMmapFileExt};

    std::cout << "Searching for XLOG files"
              << (recursive ? " (recursively)" : "") << "..." << std::endl;
    std::vector<std::string> files =
        xlog_decode::FileUtils::ScanDirectory(path, extensions, recursive);

    if (files.empty()) {
      std::cout << "No XLOG files found in the specified directory"
                << std::endl;
      return 0;
    }

    std::cout << "Found " << files.size() << " XLOG files, starting decode..."
              << std::endl;
    int success_count = 0;
    for (const auto& file : files) {
      if (DecodeFile(file, skip_error_blocks)) {
        success_count++;
      }
    }

    std::cout << "Decoded " << success_count << " out of " << files.size()
              << " files" << std::endl;
    return (success_count > 0) ? 0 : 1;
  } else {
    // 处理单个文件
    if (!xlog_decode::XlogDecoder::IsXlogFile(path)) {
      std::cerr << "Warning: File does not have a recognized XLOG extension: "
                << path << std::endl;
      std::cout << "Attempting to decode anyway..." << std::endl;
    }

    return DecodeFile(path, skip_error_blocks) ? 0 : 1;
  }
}

// 处理清理命令
int ProcessCleanCommand(const std::vector<std::string>& args) {
  if (args.empty()) {
    std::cerr << "Error: Missing path argument for clean command\n\n";
    PrintUsage();
    return 1;
  }

  bool recursive = true;  // 默认启用递归
  std::string path;

  // 解析选项
  for (size_t i = 0; i < args.size(); ++i) {
    if (args[i] == "--no-recursive") {
      recursive = false;  // 禁用递归模式的选项
    } else if (path.empty()) {
      path = args[i];
    }
  }

  if (path.empty()) {
    std::cerr << "Error: Missing path argument for clean command\n\n";
    PrintUsage();
    return 1;
  }

  if (!xlog_decode::FileUtils::PathExists(path)) {
    std::cerr << "Error: Path does not exist: " << path << std::endl;
    return 1;
  }

  if (!xlog_decode::FileUtils::IsDirectory(path)) {
    std::cerr << "Error: Path must be a directory for clean command: " << path
              << std::endl;
    return 1;
  }

  std::cout << "Searching for decoded files"
            << (recursive ? " (recursively)" : "") << "..." << std::endl;
  std::vector<std::string> files =
      xlog_decode::FileUtils::FindDecodedFiles(path, recursive);

  if (files.empty()) {
    std::cout << "No decoded files found in the specified directory"
              << std::endl;
    return 0;
  }

  std::cout << "Found " << files.size()
            << " decoded files, starting deletion..." << std::endl;
  int deleted_count = 0;
  for (const auto& file : files) {
    std::cout << "Deleting: " << file << std::endl;
    if (xlog_decode::FileUtils::DeleteFile(file)) {
      deleted_count++;
    }
  }

  std::cout << "Deleted " << deleted_count << " out of " << files.size()
            << " decoded files" << std::endl;
  return 0;
}

// 处理帮助命令
int ProcessHelpCommand(const std::vector<std::string>& args) {
  PrintUsage();
  return 0;
}

// 处理版本命令
int ProcessVersionCommand() {
  std::cout << "xlog_decode version " << GetVersion() << std::endl;
  std::cout << "Copyright (c) 2023-2024 xlog_decode contributors" << std::endl;
  std::cout << "Licensed under the MIT License" << std::endl;
  return 0;
}

// 测试文件工具
void TestFileUtils() {
  std::cout << "Testing FileUtils functionality..." << std::endl;

  // 测试路径操作
  std::string test_path = "c:/path/to/file.txt";
  std::cout << "Test path: " << test_path << std::endl;
  std::cout << "Filename: " << FileUtils::GetFileName(test_path) << std::endl;
  std::cout << "Directory: " << FileUtils::GetDirectoryName(test_path)
            << std::endl;
  std::cout << "Extension: " << FileUtils::GetFileExtension(test_path)
            << std::endl;

  // 测试路径连接
  std::string dir = "c:/path/to";
  std::string file = "file.txt";
  std::string joined = FileUtils::JoinPath(dir, file);
  std::cout << "Joined path: " << joined << std::endl;

  // 测试当前目录
  std::cout << "Current directory: " << FileUtils::GetCurrentDirectory()
            << std::endl;

  // 创建测试文件
  std::string test_file = "test.txt";
  std::vector<uint8_t> test_data = {'H', 'e', 'l', 'l', 'o', ',', ' ',
                                    'w', 'o', 'r', 'l', 'd', '!'};
  bool write_success = FileUtils::WriteFile(test_file, test_data);
  std::cout << "Write file: " << (write_success ? "success" : "failed")
            << std::endl;

  // 读取测试文件
  std::vector<uint8_t> read_data;
  bool read_success = FileUtils::ReadFile(test_file, read_data);
  std::cout << "Read file: " << (read_success ? "success" : "failed")
            << std::endl;
  if (read_success) {
    std::string content(read_data.begin(), read_data.end());
    std::cout << "File content: " << content << std::endl;
  }

  // 检查文件是否存在
  std::cout << "File exists: "
            << (FileUtils::FileExists(test_file) ? "yes" : "no") << std::endl;

  // 列出当前目录中的文件
  std::cout << "Files in current directory:" << std::endl;
  std::vector<std::string> files =
      FileUtils::ListFilesInDirectory(FileUtils::GetCurrentDirectory());
  for (const auto& file : files) {
    std::cout << "  " << file << std::endl;
  }

  // 删除测试文件
  bool delete_success = FileUtils::DeleteFile(test_file);
  std::cout << "Delete file: " << (delete_success ? "success" : "failed")
            << std::endl;
}

int main(int argc, char* argv[]) {
  // 开发测试模式
#ifdef XLOG_DECODE_TEST_MODE
  std::cout << "XLog Decoder Test Program" << std::endl;
  try {
    TestFileUtils();
  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }
  std::cout << "All tests completed!" << std::endl;
  return 0;
#endif

  // 命令行模式
  if (argc < 2) {
    std::cerr << "Error: Missing command argument\n\n";
    PrintUsage();
    return 1;
  }

  std::string command = argv[1];
  std::vector<std::string> args;

  // 收集命令行参数
  for (int i = 2; i < argc; ++i) {
    args.push_back(argv[i]);
  }

  // 处理命令
  if (command == "decode") {
    return ProcessDecodeCommand(args);
  } else if (command == "clean") {
    return ProcessCleanCommand(args);
  } else if (command == "help" || command == "--help") {
    return ProcessHelpCommand(args);
  } else if (command == "--version") {
    return ProcessVersionCommand();
  } else {
    std::cerr << "Error: Unknown command '" << command << "'\n\n";
    PrintUsage();
    return 1;
  }
}