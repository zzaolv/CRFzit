cmake_minimum_required(VERSION 3.20)
project(crfzitd CXX C)

# --- 1. 构建我们自己提供的 sqlite3 库 ---
# 设置源码目录变量
set(SQLITE3_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlite3)
# 将 sqlite3.c 编译成一个静态库，名为 sqlite3_lib
add_library(sqlite3_lib STATIC ${SQLITE3_SRC_DIR}/sqlite3.c)
# 告诉 CMake，任何链接了 sqlite3_lib 的目标，都应该自动包含它的头文件目录
target_include_directories(sqlite3_lib PUBLIC ${SQLITE3_SRC_DIR})


# --- 2. 构建 SQLiteCpp 库 ---
# 设置源码目录变量
set(SQLITECPP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlitecpp/src)
# 将 SQLiteCpp 的源文件编译成一个静态库
add_library(sqlitecpp_lib STATIC
    ${SQLITECPP_SRC_DIR}/Database.cpp
    ${SQLITECPP_SRC_DIR}/Statement.cpp
    ${SQLITECPP_SRC_DIR}/Column.cpp
    ${SQLITECPP_SRC_DIR}/Backup.cpp
)
# 告诉 CMake，sqlitecpp_lib 需要包含它自己的头文件
target_include_directories(sqlitecpp_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlitecpp/include)
# 告诉 CMake，sqlitecpp_lib 需要链接我们上面自己编译的 sqlite3_lib
target_link_libraries(sqlitecpp_lib PUBLIC sqlite3_lib)


# --- 3. 定义最终的可执行文件 ---
add_executable(crfzitd
    cpp/main.cpp
    cpp/uds_server.cpp
    cpp/db_manager.cpp
    cpp/state_manager.cpp
    cpp/policy_engine.cpp
)
# 告诉 crfzitd 需要包含 nlohmann/json 的头文件
target_include_directories(crfzitd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann_json/include)


# --- 4. 链接最终的可执行文件 ---
# 链接 crfzitd 到它需要的所有库
target_link_libraries(crfzitd
    PRIVATE
    sqlitecpp_lib   # sqlitecpp_lib 会自动带来对 sqlite3_lib 的链接
    log             # 链接 Android 系统 log 库
    stdc++fs        # 链接 filesystem 库
)

# --- 5. 设置 C++ 标准 ---
set_target_properties(crfzitd PROPERTIES CXX_STANDARD 17)