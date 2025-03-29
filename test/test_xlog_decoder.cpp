#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "file_utils.h"
#include "xlog_constants.h"
#include "xlog_decoder.h"

using namespace xlog_decode;

// 测试文件扩展名检测
void test_file_extensions() {
  assert(XlogDecoder::IsXlogFile("test.xlog") == true);
  assert(XlogDecoder::IsXlogFile("test.mmap3") == true);
  assert(XlogDecoder::IsXlogFile("test.txt") == false);

  std::cout << "文件扩展名检测测试通过" << std::endl;
}

// 测试输出文件名生成
void test_output_filename_generation() {
  assert(XlogDecoder::GenerateOutputFilename("test.xlog") == "test_.log");
  assert(XlogDecoder::GenerateOutputFilename("test.mmap3") == "test_.log");
  assert(XlogDecoder::GenerateOutputFilename("test.txt") == "test.txt_.log");
  assert(XlogDecoder::GenerateOutputFilename("/path/to/test.xlog") ==
         "/path/to/test_.log");

  std::cout << "输出文件名生成测试通过" << std::endl;
}

// 创建一个简单的测试xlog文件
bool create_test_xlog_file(const std::string& filename) {
  // 创建一个简单的xlog文件结构
  std::vector<uint8_t> buffer;

  // 添加文件头 - MAGIC_NO_COMPRESS_START
  buffer.push_back(MAGIC_NO_COMPRESS_START);

  // 序列号 (16位)
  buffer.push_back(0x01);
  buffer.push_back(0x00);

  // 开始小时和结束小时
  buffer.push_back(0x0A);  // 开始小时
  buffer.push_back(0x0B);  // 结束小时

  // 数据长度 (32位) - 我们将使用13字节的测试数据
  buffer.push_back(0x0D);
  buffer.push_back(0x00);
  buffer.push_back(0x00);
  buffer.push_back(0x00);

  // 加密数据 (对于MAGIC_NO_COMPRESS_START, 4字节)
  buffer.push_back(0x00);
  buffer.push_back(0x00);
  buffer.push_back(0x00);
  buffer.push_back(0x00);

  // 数据部分 - 纯文本 "Hello, world!"
  const char* test_data = "Hello, world!";
  for (int i = 0; i < 13; i++) {
    buffer.push_back(test_data[i]);
  }

  // 结束标记
  buffer.push_back(MAGIC_END);

  // 写入文件
  return FileUtils::WriteFile(filename, buffer);
}

// 测试解码功能
void test_decoding() {
  const std::string test_file = "test.xlog";
  const std::string output_file = "test_.log";

  // 创建测试文件
  bool created = create_test_xlog_file(test_file);
  assert(created);

  // 解码测试文件
  XlogDecoder decoder;
  bool result = decoder.DecodeFile(test_file, output_file, true);
  assert(result);

  // 验证解码结果
  std::vector<uint8_t> decoded_data;
  bool read_result = FileUtils::ReadFile(output_file, decoded_data);
  assert(read_result);

  // 验证内容为 "Hello, world!"
  std::string decoded_text(decoded_data.begin(), decoded_data.end());
  assert(decoded_text == "Hello, world!");

  // 清理测试文件
  FileUtils::DeleteFile(test_file);
  FileUtils::DeleteFile(output_file);

  std::cout << "解码功能测试通过" << std::endl;
}

// 主函数
int main() {
  std::cout << "开始运行xlog_decoder测试..." << std::endl;

  // 运行测试
  test_file_extensions();
  test_output_filename_generation();
  test_decoding();

  std::cout << "所有测试通过!" << std::endl;
  return 0;
}