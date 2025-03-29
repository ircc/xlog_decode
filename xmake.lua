-- xmake.lua 配置文件
-- xlog_decode 项目

-- 设置项目信息
set_project("xlog_decode")
set_version("1.0.0")
set_xmakever("2.7.0")
set_languages("c++17")

-- 平台特定配置
if is_plat("windows") then
    add_defines("NOMINMAX", "WIN32_LEAN_AND_MEAN", "NOGDI", "NOUSER")
end

-- 设置构建选项
option("recursive")
    set_default(true)
    set_showmenu(true)
    set_description("支持递归处理子目录")
option_end()

option("keep-errors")
    set_default(false)
    set_showmenu(true)
    set_description("不跳过错误数据块")
option_end()

-- 添加包含目录
add_includedirs("include")

-- 文件工具库
target("file_utils")
    set_kind("static")
    add_files("src/file_utils.cpp")

-- XLog解码器库
target("xlog_decoder")
    set_kind("static")
    add_files("src/xlog_decoder.cpp")
    add_deps("file_utils")

-- 第三方库依赖，仅在文件存在时添加
-- add_includedirs("third_party/zlib")
-- add_files("third_party/zlib/*.c")
-- add_includedirs("third_party/zstd/lib")
-- add_files("third_party/zstd/lib/common/*.c")
-- add_files("third_party/zstd/lib/compress/*.c")
-- add_files("third_party/zstd/lib/decompress/*.c")

-- 主程序
target("xlog_decode")
    set_kind("binary")
    add_files("src/main.cpp")
    add_deps("file_utils", "xlog_decoder")

-- 测试程序
target("test_file_utils")
    set_kind("binary")
    add_files("test/test_file_utils_main.cpp")
    add_deps("file_utils")

target("test_file_utils_2")
    set_kind("binary")
    add_files("test/test_file_utils.cpp")
    add_deps("file_utils")

target("test_xlog_decoder")
    set_kind("binary")
    add_files("test/test_xlog_decoder.cpp")
    add_deps("file_utils", "xlog_decoder")