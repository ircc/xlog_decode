#include <iostream>
#include <string>
#include <vector>

#include "../include/file_utils.h"

using namespace xlog_decode;

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
  std::cout << "XLog Decoder File Utils Test" << std::endl;

  try {
    test_file_utils();
  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }

  std::cout << "All tests completed!" << std::endl;
  return 0;
}