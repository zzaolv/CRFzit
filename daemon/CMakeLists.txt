cmake_minimum_required(VERSION 3.20)
project(crfzitd CXX C)

# --- 1. 添加第三方库 ---
# nlohmann/json
include_directories("third_party/nlohmann_json/include")

# SQLiteCpp
include_directories("third_party/sqlitecpp/include")
# 将 SQLiteCpp 的源文件编译成一个静态库
add_library(sqlitecpp_lib STATIC
    third_party/sqlitecpp/src/Database.cpp
    third_party/sqlitecpp/src/Statement.cpp
    third_party/sqlitecpp/src/Column.cpp
    third_party/sqlitecpp/src/Backup.cpp
    third_party/sqlitecpp/src/Assertion.cpp
)

# --- 2. 定义可执行文件 ---
# 添加新的源文件
add_executable(crfzitd
    cpp/main.cpp
    cpp/uds_server.cpp
    cpp/db_manager.cpp       # 新增
    cpp/state_manager.cpp    # 新增
    cpp/policy_engine.cpp    # 新增
    cpp/request_router.cpp   # 新增
)

# --- 3. 链接库 ---
# 系统自带的 sqlite3 库
find_library(SQLITE3_LIBRARY sqlite3)
find_library(LOG_LIBRARY log)

target_link_libraries(crfzitd
    PRIVATE
    sqlitecpp_lib       # 链接我们自己编译的库
    ${SQLITE3_LIBRARY}  # 链接系统 sqlite3
    ${LOG_LIBRARY}
)

# --- 4. 设置 C++ 标准 ---
set_target_properties(crfzitd PROPERTIES CXX_STANDARD 17)