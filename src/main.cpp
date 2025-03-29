// Copyright (c) 2023-2024 xlog_decode contributors
// Licensed under the MIT License
//
// main.cpp - Main entry point for the xlog_decode tool

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "file_utils.h"
#include "xlog_constants.h"
#include "xlog_decoder.h"

// Version information is now provided by the build system via
// XLOG_DECODE_VERSION macro

using namespace xlog_decode;

// Get program version
const std::string GetVersion() {
  return "1.0.0";  // Hardcoded version until macro issue is fixed
}

// Print program usage
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

// Decode a single file
bool DecodeFile(const std::string& file_path, bool skip_error_blocks) {
  try {
    xlog_decode::XlogDecoder decoder;
    std::string output_file =
        xlog_decode::XlogDecoder::GenerateOutputFilename(file_path);

    // Get input file size
    auto input_file_size = xlog_decode::FileUtils::GetFileSize(file_path);
    double input_size_mb = static_cast<double>(input_file_size) / (1024 * 1024);

    // Add timing measurement
    auto start_time = std::chrono::high_resolution_clock::now();

    bool result = decoder.DecodeFile(file_path, output_file, skip_error_blocks);

    // Calculate elapsed time
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);

    if (result) {
      // Get output file size
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

// Process decode command
int ProcessDecodeCommand(const std::vector<std::string>& args) {
  if (args.empty()) {
    std::cerr << "Error: Missing path argument for decode command\n\n";
    PrintUsage();
    return 1;
  }

  bool recursive = true;  // Set recursive to true by default
  bool skip_error_blocks = true;
  std::string path;

  // Parse options
  for (size_t i = 0; i < args.size(); ++i) {
    if (args[i] == "--no-recursive") {
      recursive = false;  // Add option to disable recursive search
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
    // Process directory
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

  bool recursive = true;  // Set recursive to true by default
  std::string path;

  // Parse options
  for (size_t i = 0; i < args.size(); ++i) {
    if (args[i] == "--no-recursive") {
      recursive = false;  // Option to disable recursive mode
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

// Process help command
int ProcessHelpCommand(const std::vector<std::string>& args) {
  PrintUsage();
  return 0;
}

// Process version command
int ProcessVersionCommand() {
  std::cout << "xlog_decode version " << GetVersion() << std::endl;
  std::cout << "Copyright (c) 2023-2024 xlog_decode contributors" << std::endl;
  std::cout << "Licensed under the MIT License" << std::endl;
  return 0;
}

// Test file utilities
void TestFileUtils() {
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
  // Development test mode
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

  // Command line mode
  if (argc < 2) {
    std::cerr << "Error: Missing command argument\n\n";
    PrintUsage();
    return 1;
  }

  std::string command = argv[1];
  std::vector<std::string> args;

  // Collect command line arguments
  for (int i = 2; i < argc; ++i) {
    args.push_back(argv[i]);
  }

  // Process commands
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