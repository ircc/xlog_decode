#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "file_utils.h"
#include "xlog_constants.h"
#include "xlog_decoder.h"

using namespace xlog_decode;

// Test file extension detection
void test_file_extensions() {
  assert(XlogDecoder::IsXlogFile("test.xlog") == true);
  assert(XlogDecoder::IsXlogFile("test.mmap3") == true);
  assert(XlogDecoder::IsXlogFile("test.txt") == false);

  std::cout << "File extension detection tests passed" << std::endl;
}

// Test output filename generation
void test_output_filename_generation() {
  assert(XlogDecoder::GenerateOutputFilename("test.xlog") == "test_.log");
  assert(XlogDecoder::GenerateOutputFilename("test.mmap3") == "test_.log");
  assert(XlogDecoder::GenerateOutputFilename("test.txt") == "test.txt_.log");
  assert(XlogDecoder::GenerateOutputFilename("/path/to/test.xlog") ==
         "/path/to/test_.log");

  std::cout << "Output filename generation tests passed" << std::endl;
}

// Create a simple test xlog file
bool create_test_xlog_file(const std::string& filename) {
  // Create a simple xlog file structure
  std::vector<uint8_t> buffer;

  // Add file header - MAGIC_NO_COMPRESS_START
  buffer.push_back(MAGIC_NO_COMPRESS_START);

  // Sequence number (16 bit)
  buffer.push_back(0x01);
  buffer.push_back(0x00);

  // Begin hour and end hour
  buffer.push_back(0x0A);  // Begin hour
  buffer.push_back(0x0B);  // End hour

  // Data length (32 bit) - we will use 13 bytes of test data
  buffer.push_back(0x0D);
  buffer.push_back(0x00);
  buffer.push_back(0x00);
  buffer.push_back(0x00);

  // Encryption data (for MAGIC_NO_COMPRESS_START, 4 bytes)
  buffer.push_back(0x00);
  buffer.push_back(0x00);
  buffer.push_back(0x00);
  buffer.push_back(0x00);

  // Data section - plain text "Hello, world!"
  const char* test_data = "Hello, world!";
  for (int i = 0; i < 13; i++) {
    buffer.push_back(test_data[i]);
  }

  // End marker
  buffer.push_back(MAGIC_END);

  // Write to file
  return FileUtils::WriteFile(filename, buffer);
}

// Test decoding functionality
void test_decoding() {
  const std::string test_file = "test.xlog";
  const std::string output_file = "test_.log";

  // Create test file
  bool created = create_test_xlog_file(test_file);
  assert(created);

  // Decode test file
  XlogDecoder decoder;
  bool result = decoder.DecodeFile(test_file, output_file, true);
  assert(result);

  // Verify decode result
  std::vector<uint8_t> decoded_data;
  bool read_result = FileUtils::ReadFile(output_file, decoded_data);
  assert(read_result);

  // Verify content is "Hello, world!"
  std::string decoded_text(decoded_data.begin(), decoded_data.end());
  assert(decoded_text == "Hello, world!");

  // Clean up test files
  FileUtils::DeleteFile(test_file);
  FileUtils::DeleteFile(output_file);

  std::cout << "Decoding functionality tests passed" << std::endl;
}

// Main function
int main() {
  std::cout << "Starting xlog_decoder tests..." << std::endl;

  // Run tests
  test_file_extensions();
  test_output_filename_generation();
  test_decoding();

  std::cout << "All tests passed!" << std::endl;
  return 0;
}