cmake_minimum_required(VERSION 3.20)
project(crfzitd CXX C)

# --- 1. 构建我们自己提供的 sqlite3 库 ---
set(SQLITE3_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlite3)
add_library(sqlite3_lib STATIC ${SQLITE3_SRC_DIR}/sqlite3.c)
target_include_directories(sqlite3_lib PUBLIC ${SQLITE3_SRC_DIR})


# --- 2. 构建 SQLiteCpp 库 ---
set(SQLITECPP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlitecpp/src)
add_library(sqlitecpp_lib STATIC
    ${SQLITECPP_SRC_DIR}/Database.cpp
    ${SQLITECPP_SRC_DIR}/Statement.cpp
    ${SQLITECPP_SRC_DIR}/Column.cpp
    ${SQLITECPP_SRC_DIR}/Backup.cpp
)
target_include_directories(sqlitecpp_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlitecpp/include)
target_link_libraries(sqlitecpp_lib PUBLIC sqlite3_lib)


# --- 3. 定义最终的可执行文件 ---
add_executable(crfzitd
    cpp/main.cpp
    cpp/uds_server.cpp
    cpp/db_manager.cpp
    cpp/state_manager.cpp
    cpp/policy_engine.cpp
)
target_include_directories(crfzitd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann_json/include)


# --- 4. 链接最终的可执行文件 ---
# 在现代 NDK (libc++) 中，filesystem 已包含在标准库内，无需链接 -lstdc++fs
target_link_libraries(crfzitd
    PRIVATE
    sqlitecpp_lib
    log
)

# --- 5. 设置 C++ 标准 ---
set_target_properties(crfzitd PROPERTIES CXX_STANDARD 17)