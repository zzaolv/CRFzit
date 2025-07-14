cmake_minimum_required(VERSION 3.20)
project(crfzitd CXX C)

# --- 1. 添加第三方库 ---
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann_json/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlitecpp/include)

set(SQLITECPP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlitecpp/src)
add_library(sqlitecpp_lib STATIC
    ${SQLITECPP_SRC_DIR}/Database.cpp
    ${SQLITECPP_SRC_DIR}/Statement.cpp
    ${SQLITECPP_SRC_DIR}/Column.cpp
    ${SQLITECPP_SRC_DIR}/Backup.cpp
)

# --- 2. 定义可执行文件 ---
add_executable(crfzitd
    cpp/main.cpp
    cpp/uds_server.cpp
    cpp/db_manager.cpp
    cpp/state_manager.cpp
    cpp/policy_engine.cpp
)

# --- 3. 链接库 ---
# 声明 sqlitecpp_lib 对 sqlite3 的依赖，这会传递头文件路径
target_link_libraries(sqlitecpp_lib PUBLIC sqlite3)

# 链接 crfzitd 到我们需要的库
target_link_libraries(crfzitd
    PRIVATE
    sqlitecpp_lib
    log
    stdc++fs
)

# --- 4. 设置 C++ 标准 ---
set_target_properties(crfzitd PROPERTIES CXX_STANDARD 17)