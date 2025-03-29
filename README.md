# xlog_decode

[![Build Status](https://github.com/username/xlog_decode/workflows/Build/badge.svg)](https://github.com/username/xlog_decode/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
![Version](https://img.shields.io/badge/version-1.0.0-blue)

一个用于解码XLOG格式日志文件的跨平台命令行工具。

## 功能介绍

xlog_decode 是一个轻量级、高效的命令行工具，专门用于解码XLOG格式的日志文件。

主要功能:
- 支持解码单个XLOG格式文件（.xlog和.mmap3后缀）
- 支持递归解码目录中的所有XLOG文件
- 支持跳过错误数据块，提高解码成功率
- 支持清理已解码文件
- 跨平台支持：Windows、macOS和Linux

## 安装

### 从预编译二进制文件安装

从 [Release 页面](https://github.com/username/xlog_decode/releases) 下载适合您平台的最新版本：

- Windows: `xlog_decode-win-x64.zip`
- macOS Intel: `xlog_decode-macos-x64.zip`
- macOS Apple Silicon: `xlog_decode-macos-arm64.zip`
- Linux: `xlog_decode-linux-x64.zip`

下载后解压，并将可执行文件放入系统路径中即可使用。

### 从源代码构建

请参考下面的[开发指南](#开发指南)。

## 使用方法

xlog_decode 提供了简单直观的命令行界面：

```
xlog_decode - A tool for decoding XLOG format log files
Version: 1.0.0

Usage:
  xlog_decode <command> [options] <path>

Commands:
  decode   - Decode one or more XLOG files
  clean    - Delete all decoded files in a directory

Options:
  -r, --recursive   - Process files recursively in subdirectories
  -k, --keep-errors - Don't skip blocks with errors during decoding

Examples:
  xlog_decode decode path/to/file.xlog        - Decode a single file
  xlog_decode decode -r path/to/dir           - Decode all XLOG files in directory and subdirectories
  xlog_decode clean -r path/to/dir            - Delete all decoded files in directory and subdirectories
```

### 示例

1. 解码单个文件:
   ```
   xlog_decode decode /path/to/logfile.xlog
   ```

2. 递归解码目录中的所有文件:
   ```
   xlog_decode decode -r /path/to/logs/
   ```

3. 删除目录中所有已解码文件:
   ```
   xlog_decode clean -r /path/to/logs/
   ```

## 开发指南

### 环境要求

- GN 构建系统
- 支持 C++17 的编译器
- zlib 库 (项目中已包含)

### 获取源代码

```bash
git clone https://github.com/username/xlog_decode.git
cd xlog_decode
```

### 构建项目

1. 生成构建文件:

   ```bash
   gn gen out/Release --args="is_debug=false"
   ```

2. 编译项目:

   ```bash
   ninja -C out/Release
   ```

### 目录结构

```
xlog_decode/
├── .github/workflows/   # GitHub Actions 工作流
├── include/             # 头文件
├── src/                 # 源文件
├── third_party/         # 第三方库
│   └── zlib/            # zlib 压缩库
├── build/               # 构建脚本和配置
├── test/                # 测试文件
├── docs/                # 文档
├── .gitignore           # Git 忽略文件
├── BUILD.gn             # GN 构建配置
├── LICENSE              # 许可证文件
└── README.md            # 本文件
```

### 贡献代码

1. Fork 项目
2. 创建特性分支 (`git checkout -b feature/amazing-feature`)
3. 提交更改 (`git commit -m 'Add some amazing feature'`)
4. 推送到分支 (`git push origin feature/amazing-feature`)
5. 创建 Pull Request

## 许可证

本项目基于 MIT 许可证发布 - 详情请参阅 [LICENSE](LICENSE) 文件。