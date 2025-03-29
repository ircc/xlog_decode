#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "file_utils.h"

using namespace xlog_decode;

// 创建一个测试文件
bool create_test_file(const std::string& file_path, const std::string& content) {
    std::ofstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "无法创建测试文件: " << file_path << std::endl;
        return false;
    }

    file << content;
    return true;
}

// 测试文件路径函数
void test_file_path_functions() {
    // 测试GetFileName
    std::string test_path = "/path/to/file.txt";
    std::string filename = FileUtils::GetFileName(test_path);
    if (filename != "file.txt") {
        std::cerr << "GetFileName测试失败，预期: file.txt, 实际: " << filename << std::endl;
        exit(1);
    }

    // 测试GetDirectoryName
    std::string dirname = FileUtils::GetDirectoryName(test_path);
    if (dirname != "/path/to") {
        std::cerr << "GetDirectoryName测试失败，预期: /path/to, 实际: " << dirname << std::endl;
        exit(1);
    }

    // 测试GetFileExtension
    std::string ext = FileUtils::GetFileExtension(test_path);
    if (ext != ".txt") {
        std::cerr << "GetFileExtension测试失败，预期: .txt, 实际: " << ext << std::endl;
        exit(1);
    }

    // 测试JoinPath
    std::string joined = FileUtils::JoinPath("/path/to", "file.txt");
    if (joined != "/path/to/file.txt") {
        std::cerr << "JoinPath测试失败，预期: /path/to/file.txt, 实际: " << joined << std::endl;
        exit(1);
    }

    std::cout << "文件路径函数测试通过！" << std::endl;
}

// 测试文件读写函数
void test_file_io_functions() {
    const std::string test_file = "test_io.txt";
    const std::string test_content = "Hello, FileUtils!";

    // 创建测试文件
    if (!create_test_file(test_file, test_content)) {
        std::cerr << "创建测试文件失败" << std::endl;
        exit(1);
    }

    // 测试ReadFile
    std::vector<uint8_t> buffer;
    bool read_result = FileUtils::ReadFile(test_file, buffer);
    if (!read_result) {
        std::cerr << "ReadFile测试失败，无法读取文件" << std::endl;
        exit(1);
    }

    std::string content(buffer.begin(), buffer.end());
    if (content != test_content) {
        std::cerr << "ReadFile测试失败，预期: " << test_content << ", 实际: " << content << std::endl;
        exit(1);
    }

    // 测试WriteFile
    const std::string new_test_file = "test_io_write.txt";
    std::string new_content = "写入测试内容";
    std::vector<uint8_t> write_buffer(new_content.begin(), new_content.end());

    bool write_result = FileUtils::WriteFile(new_test_file, write_buffer);
    if (!write_result) {
        std::cerr << "WriteFile测试失败，无法写入文件" << std::endl;
        exit(1);
    }

    // 验证写入的内容
    std::vector<uint8_t> verify_buffer;
    FileUtils::ReadFile(new_test_file, verify_buffer);
    std::string verify_content(verify_buffer.begin(), verify_buffer.end());

    if (verify_content != new_content) {
        std::cerr << "WriteFile测试失败，预期: " << new_content << ", 实际: " << verify_content << std::endl;
        exit(1);
    }

    // 测试DeleteFile
    bool delete_result = FileUtils::DeleteFile(test_file);
    if (!delete_result) {
        std::cerr << "DeleteFile测试失败，无法删除文件" << std::endl;
        exit(1);
    }

    delete_result = FileUtils::DeleteFile(new_test_file);
    if (!delete_result) {
        std::cerr << "DeleteFile测试失败，无法删除第二个文件" << std::endl;
        exit(1);
    }

    std::cout << "文件IO函数测试通过！" << std::endl;
}

int main() {
    std::cout << "开始运行FileUtils测试..." << std::endl;

    test_file_path_functions();
    test_file_io_functions();

    std::cout << "所有测试通过!" << std::endl;
    return 0;
}