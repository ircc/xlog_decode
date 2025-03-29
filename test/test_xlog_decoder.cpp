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

// Main function
int main() {
  std::cout << "Starting xlog_decoder tests..." << std::endl;

  // Run only static method tests
  test_file_extensions();
  test_output_filename_generation();

  std::cout << "All tests passed!" << std::endl;
  return 0;
}