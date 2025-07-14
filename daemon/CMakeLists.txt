# 版本要求
cmake_minimum_required(VERSION 3.20)
project(crfzitd CXX C)

# --- 1. 添加第三方库 ---
# nlohmann/json for JSON manipulation
include_directories("third_party/nlohmann_json/include")

# --- 2. 定义可执行文件 ---
# 我们先只创建 main.cpp 和 uds_server.cpp/h
add_executable(crfzitd
    cpp/main.cpp
    cpp/uds_server.cpp
)

# --- 3. 链接库 ---
# log 库用于 Android Logcat 输出
target_link_libraries(crfzitd PRIVATE log)

# --- 4. 设置 C++ 标准 ---
set_target_properties(crfzitd PROPERTIES CXX_STANDARD 17)

# --- 5. 安装指令 (可选，但良好实践) ---
# 这会让编译后的文件被放置到 Magisk 模块目录
install(TARGETS crfzitd DESTINATION .)