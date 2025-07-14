cmake_minimum_required(VERSION 3.20)
project(crfzitd CXX C)

# --- 1. 添加第三方库 ---
# nlohmann/json
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann_json/include)

# SQLiteCpp
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlitecpp/include)
set(SQLITECPP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlitecpp/src)
add_library(sqlitecpp_lib STATIC
    ${SQLITECPP_SRC_DIR}/Database.cpp
    ${SQLITECPP_SRC_DIR}/Statement.cpp
    ${SQLITECPP_SRC_DIR}/Column.cpp
    ${SQLITECPP_SRC_DIR}/Backup.cpp
    ${SQLITECPP_SRC_DIR}/Assertion.cpp
)

# --- 2. 定义可执行文件 ---
add_executable(crfzitd
    cpp/main.cpp
    cpp/uds_server.cpp
    cpp/db_manager.cpp
    cpp/state_manager.cpp
    cpp/policy_engine.cpp
    # cpp/request_router.cpp # 下一阶段再启用
)

# --- 3. 链接库 ---
# find_library 会在 NDK 的 sysroot 中查找
find_library(LOG_LIBRARY log)
find_library(SQLITE3_LIBRARY sqlite3)

target_link_libraries(crfzitd
    PRIVATE
    sqlitecpp_lib       # 链接我们自己编译的库
    ${SQLITE3_LIBRARY}  # 链接系统 sqlite3
    ${LOG_LIBRARY}      # 链接系统 log
    # C++17 filesystem 需要链接这个
    stdc++fs
)

# --- 4. 设置 C++ 标准 ---
set_target_properties(crfzitd PROPERTIES CXX_STANDARD 17)