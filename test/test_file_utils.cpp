#include <fstream>
#include <iostream>
#include <string>
#include <vector>


#include "file_utils.h"

using namespace xlog_decode;

// Create a test file
bool create_test_file(const std::string& file_path,
                      const std::string& content) {
  std::ofstream file(file_path);
  if (!file.is_open()) {
    std::cerr << "Failed to create test file: " << file_path << std::endl;
    return false;
  }

  file << content;
  return true;
}

// Test file path functions
void test_file_path_functions() {
  // Test GetFileName
  std::string test_path = "/path/to/file.txt";
  std::string filename = FileUtils::GetFileName(test_path);
  if (filename != "file.txt") {
    std::cerr << "GetFileName test failed, expected: file.txt, actual: "
              << filename << std::endl;
    exit(1);
  }

  // Test GetDirectoryName
  std::string dirname = FileUtils::GetDirectoryName(test_path);
  if (dirname != "/path/to") {
    std::cerr << "GetDirectoryName test failed, expected: /path/to, actual: "
              << dirname << std::endl;
    exit(1);
  }

  // Test GetFileExtension
  std::string ext = FileUtils::GetFileExtension(test_path);
  if (ext != ".txt") {
    std::cerr << "GetFileExtension test failed, expected: .txt, actual: " << ext
              << std::endl;
    exit(1);
  }

  // Test JoinPath
  std::string joined = FileUtils::JoinPath("/path/to", "file.txt");
  if (joined != "/path/to/file.txt") {
    std::cerr << "JoinPath test failed, expected: /path/to/file.txt, actual: "
              << joined << std::endl;
    exit(1);
  }

  std::cout << "File path function tests passed!" << std::endl;
}

// Test file IO functions
void test_file_io_functions() {
  const std::string test_file = "test_io.txt";
  const std::string test_content = "Hello, FileUtils!";

  // Create test file
  if (!create_test_file(test_file, test_content)) {
    std::cerr << "Failed to create test file" << std::endl;
    exit(1);
  }

  // Test ReadFile
  std::vector<uint8_t> buffer;
  bool read_result = FileUtils::ReadFile(test_file, buffer);
  if (!read_result) {
    std::cerr << "ReadFile test failed, cannot read file" << std::endl;
    exit(1);
  }

  std::string content(buffer.begin(), buffer.end());
  if (content != test_content) {
    std::cerr << "ReadFile test failed, expected: " << test_content
              << ", actual: " << content << std::endl;
    exit(1);
  }

  // Test WriteFile
  const std::string new_test_file = "test_io_write.txt";
  std::string new_content = "Test write content";
  std::vector<uint8_t> write_buffer(new_content.begin(), new_content.end());

  bool write_result = FileUtils::WriteFile(new_test_file, write_buffer);
  if (!write_result) {
    std::cerr << "WriteFile test failed, cannot write file" << std::endl;
    exit(1);
  }

  // Verify written content
  std::vector<uint8_t> verify_buffer;
  FileUtils::ReadFile(new_test_file, verify_buffer);
  std::string verify_content(verify_buffer.begin(), verify_buffer.end());

  if (verify_content != new_content) {
    std::cerr << "WriteFile test failed, expected: " << new_content
              << ", actual: " << verify_content << std::endl;
    exit(1);
  }

  // Test DeleteFile
  bool delete_result = FileUtils::DeleteFile(test_file);
  if (!delete_result) {
    std::cerr << "DeleteFile test failed, cannot delete file" << std::endl;
    exit(1);
  }

  delete_result = FileUtils::DeleteFile(new_test_file);
  if (!delete_result) {
    std::cerr << "DeleteFile test failed, cannot delete second file"
              << std::endl;
    exit(1);
  }

  std::cout << "File IO function tests passed!" << std::endl;
}

int main() {
  std::cout << "Starting FileUtils tests..." << std::endl;

  test_file_path_functions();
  test_file_io_functions();

  std::cout << "All tests passed!" << std::endl;
  return 0;
}