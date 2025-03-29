// Copyright (c) 2023-2024 xlog_decode contributors
// Licensed under the MIT License
//
// main.cpp - Main entry point for the xlog_decode tool

#include <iostream>
#include <string>
#include <vector>

#include "file_utils.h"
#include "xlog_constants.h"
#include "xlog_decoder.h"

using namespace xlog_decode;

// Print program usage
void PrintUsage() {
  std::cout << "xlog_decode - A tool for decoding XLOG format log files\n";
  std::cout << "Version: 1.0.0\n\n";
  std::cout << "Usage:\n";
  std::cout << "  xlog_decode <command> [options] <path>\n\n";
  std::cout << "Commands:\n";
  std::cout << "  decode   - Decode one or more XLOG files\n";
  std::cout << "  clean    - Delete all decoded files in a directory\n\n";
  std::cout << "Options:\n";
  std::cout
      << "  -r, --recursive   - Process files recursively in subdirectories\n";
  std::cout << "  -k, --keep-errors - Don't skip blocks with errors during "
               "decoding\n\n";
  std::cout << "Examples:\n";
  std::cout << "  xlog_decode decode path/to/file.xlog        - Decode a "
               "single file\n";
  std::cout << "  xlog_decode decode -r path/to/dir           - Decode all "
               "XLOG files in directory and subdirectories\n";
  std::cout << "  xlog_decode clean -r path/to/dir            - Delete all "
               "decoded files in directory and subdirectories\n";
}

// Decode a single file
bool DecodeFile(const std::string& file_path, bool skip_error_blocks) {
  try {
    xlog_decode::XlogDecoder decoder;
    std::string output_file =
        xlog_decode::XlogDecoder::GenerateOutputFilename(file_path);

    std::cout << "Decoding: " << file_path << std::endl;
    if (decoder.DecodeFile(file_path, output_file, skip_error_blocks)) {
      std::cout << "Successfully decoded to: " << output_file << std::endl;
      return true;
    } else {
      std::cerr << "Failed to decode file: " << file_path << std::endl;
      return false;
    }
  } catch (const std::exception& e) {
    std::cerr << "Error decoding file: " << e.what() << std::endl;
    return false;
  }
}

// Process decode command
int ProcessDecodeCommand(const std::vector<std::string>& args) {
  if (args.empty()) {
    std::cerr << "Error: Missing path argument for decode command\n\n";
    PrintUsage();
    return 1;
  }

  bool recursive = false;
  bool skip_error_blocks = true;
  std::string path;

  // Parse options
  for (size_t i = 0; i < args.size(); ++i) {
    if (args[i] == "-r" || args[i] == "--recursive") {
      recursive = true;
    } else if (args[i] == "-k" || args[i] == "--keep-errors") {
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
    // Process directory
    std::vector<std::string> extensions = {kXlogFileExt, kMmapFileExt};

    std::vector<std::string> files =
        xlog_decode::FileUtils::ScanDirectory(path, extensions, recursive);

    if (files.empty()) {
      std::cout << "No XLOG files found in the specified directory"
                << std::endl;
      return 0;
    }

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
    // Process single file
    if (!xlog_decode::XlogDecoder::IsXlogFile(path)) {
      std::cerr << "Warning: File does not have a recognized XLOG extension: "
                << path << std::endl;
      std::cout << "Attempting to decode anyway..." << std::endl;
    }

    return DecodeFile(path, skip_error_blocks) ? 0 : 1;
  }
}

// Process clean command
int ProcessCleanCommand(const std::vector<std::string>& args) {
  if (args.empty()) {
    std::cerr << "Error: Missing path argument for clean command\n\n";
    PrintUsage();
    return 1;
  }

  bool recursive = false;
  std::string path;

  // Parse options
  for (size_t i = 0; i < args.size(); ++i) {
    if (args[i] == "-r" || args[i] == "--recursive") {
      recursive = true;
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

  std::vector<std::string> files =
      xlog_decode::FileUtils::FindDecodedFiles(path, recursive);

  if (files.empty()) {
    std::cout << "No decoded files found in the specified directory"
              << std::endl;
    return 0;
  }

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

void test_file_utils() {
  std::cout << "Testing FileUtils functionality..." << std::endl;

  // Test path operations
  std::string test_path = "c:/path/to/file.txt";
  std::cout << "Test path: " << test_path << std::endl;
  std::cout << "Filename: " << FileUtils::GetFileName(test_path) << std::endl;
  std::cout << "Directory: " << FileUtils::GetDirectoryName(test_path)
            << std::endl;
  std::cout << "Extension: " << FileUtils::GetFileExtension(test_path)
            << std::endl;

  // Test path join
  std::string dir = "c:/path/to";
  std::string file = "file.txt";
  std::string joined = FileUtils::JoinPath(dir, file);
  std::cout << "Joined path: " << joined << std::endl;

  // Test current directory
  std::cout << "Current directory: " << FileUtils::GetCurrentDirectory()
            << std::endl;

  // Create test file
  std::string test_file = "test.txt";
  std::vector<uint8_t> test_data = {'H', 'e', 'l', 'l', 'o', ',', ' ',
                                    'w', 'o', 'r', 'l', 'd', '!'};
  bool write_success = FileUtils::WriteFile(test_file, test_data);
  std::cout << "Write file: " << (write_success ? "success" : "failed")
            << std::endl;

  // Read test file
  std::vector<uint8_t> read_data;
  bool read_success = FileUtils::ReadFile(test_file, read_data);
  std::cout << "Read file: " << (read_success ? "success" : "failed")
            << std::endl;
  if (read_success) {
    std::string content(read_data.begin(), read_data.end());
    std::cout << "File content: " << content << std::endl;
  }

  // Check if file exists
  std::cout << "File exists: "
            << (FileUtils::FileExists(test_file) ? "yes" : "no") << std::endl;

  // List files in current directory
  std::cout << "Files in current directory:" << std::endl;
  std::vector<std::string> files =
      FileUtils::ListFilesInDirectory(FileUtils::GetCurrentDirectory());
  for (const auto& file : files) {
    std::cout << "  " << file << std::endl;
  }

  // Delete test file
  bool delete_success = FileUtils::DeleteFile(test_file);
  std::cout << "Delete file: " << (delete_success ? "success" : "failed")
            << std::endl;
}

int main(int argc, char* argv[]) {
  std::cout << "XLog Decoder Test Program" << std::endl;

  try {
    test_file_utils();
  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }

  std::cout << "All tests completed!" << std::endl;
  return 0;
}