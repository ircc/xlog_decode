# xlog_decode

[![Build Status](https://github.com/username/xlog_decode/workflows/Build/badge.svg)](https://github.com/username/xlog_decode/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
![Version](https://img.shields.io/badge/version-1.0.0-blue)

一个用于解码XLOG格式日志文件的跨平台命令行工具。

## 功能介绍

xlog_decode 是一个轻量级、高效的命令行工具，专门用于解码XLOG格式的日志文件。

主要功能:
- 支持解码单个XLOG格式文件（.xlog和.mmap3后缀）
- 支持递归解码目录中的所有XLOG文件（默认启用）
- 支持跳过错误数据块，提高解码成功率
- 支持清理已解码文件（默认递归处理）
- 显示每个文件解码前后的大小和处理时间
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
xlog_decode - XLOG格式日志文件解码工具
版本: 1.0.0

用法:
  xlog_decode <命令> [选项] <路径>

命令:
  decode   - 解码一个或多个XLOG文件（默认递归处理）
  clean    - 删除目录中所有已解码文件（默认递归处理）
  help     - 显示帮助信息

选项:
  --no-recursive    - 禁用递归处理
  --keep-errors     - 解码时不跳过错误数据块
  --version         - 显示版本信息

示例:
  xlog_decode help                        - 显示帮助信息
  xlog_decode decode path/to/file.xlog    - 解码单个文件
  xlog_decode decode path/to/dir          - 递归解码目录中所有XLOG文件
  xlog_decode decode --no-recursive path/to/dir - 只解码目录中的XLOG文件，不包括子目录
  xlog_decode clean path/to/dir           - 递归删除目录中所有已解码文件
```

### 命令详解

#### 帮助与版本信息

显示帮助信息:
```
xlog_decode help
xlog_decode --help
```

显示版本信息:
```
xlog_decode --version
```

#### 解码命令

1. 解码单个文件:
   ```
   xlog_decode decode /path/to/logfile.xlog
   ```

2. 递归解码目录中的所有文件（默认行为）:
   ```
   xlog_decode decode /path/to/logs/
   ```

3. 只解码目录中的文件，不包括子目录:
   ```
   xlog_decode decode --no-recursive /path/to/logs/
   ```

4. 解码时不跳过错误数据块:
   ```
   xlog_decode decode --keep-errors /path/to/logfile.xlog
   ```

#### 清理命令

1. 删除目录中所有已解码文件（默认递归处理）:
   ```
   xlog_decode clean /path/to/logs/
   ```

2. 只删除目录中的已解码文件，不包括子目录:
   ```
   xlog_decode clean --no-recursive /path/to/logs/
   ```

## 开发指南

### 环境要求

- xmake 构建系统 (2.7.0+)
- 支持 C++17 的编译器

### 获取源代码

```bash
git clone https://github.com/username/xlog_decode.git
cd xlog_decode
```

### 构建项目

1. 配置项目:

   ```bash
   xmake config -m release
   ```

2. 编译项目:

   ```bash
   xmake
   ```

3. 运行测试:

   ```bash
   xmake run test_file_utils
   xmake run test_xlog_decoder
   ```

4. 安装程序（可选）:

   ```bash
   xmake install -o /usr/local/bin
   ```

### 多平台构建

xmake支持跨平台编译：

- Windows:
  ```bash
  xmake f -p windows -a x64 -m release
  xmake
  ```

- macOS:
  ```bash
  xmake f -p macosx -a x86_64 -m release  # Intel 芯片
  # 或
  xmake f -p macosx -a arm64 -m release   # Apple Silicon
  xmake
  ```

- Linux:
  ```bash
  xmake f -p linux -a x86_64 -m release
  xmake
  ```

### 目录结构

```
xlog_decode/
├── .github/workflows/   # GitHub Actions 工作流
├── include/             # 头文件
├── src/                 # 源文件
├── third_party/         # 第三方库
│   └── zlib/            # zlib 压缩库
├── test/                # 测试文件
├── docs/                # 文档
├── .gitignore           # Git 忽略文件
├── xmake.lua            # xmake 构建配置
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