# /CRFzit/daemon/cpp/CMakeLists.txt

cmake_minimum_required(VERSION 3.20)

project(crfzitd CXX C)

# --- 基本配置 ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 目标：crfzitd 可执行文件 ---
add_executable(crfzitd main.cpp)

# --- Rust 库集成 ---
# 我们需要告诉CMake去哪里找Rust编译产物
# Rust的编译将由我们的外部脚本 `remote_build.sh` 控制
# 这里我们假设它已经被编译好了

# Rust库的路径 (相对于构建目录)
set(RUST_LIB_DIR ${CMAKE_SOURCE_DIR}/../rust/target/aarch64-linux-android/release)
# cxx生成的头文件路径
set(RUST_CXX_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../rust/target/cxxbridge/crfzit/daemon/rust/src/lib.rs.h)

# 添加Rust库的头文件目录
target_include_directories(crfzitd PRIVATE ${CMAKE_SOURCE_DIR}/../rust/)
target_include_directories(crfzitd PRIVATE ${CMAKE_BINARY_DIR}) # For generated headers

# 链接Rust静态库
find_library(CRFZIT_RUST_LIB
    NAMES crfzit_core_logic
    PATHS ${RUST_LIB_DIR}
    NO_DEFAULT_PATH)

if(NOT CRFZIT_RUST_LIB)
    message(FATAL_ERROR "crfzit_core_logic static library not found! Ensure Rust part is built first.")
endif()

target_link_libraries(crfzitd PRIVATE ${CRFZIT_RUST_LIB})

# --- gRPC 和 Protobuf 集成 ---
# 我们将使用CMake的FetchContent从源码构建gRPC，以确保与NDK的兼容性
include(FetchContent)

# protobuf
FetchContent_Declare(
    protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG        v25.3 # 使用一个稳定版本
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(protobuf)

# gRPC
FetchContent_Declare(
    grpc
    GIT_REPOSITORY https://github.com/grpc/grpc.git
    GIT_TAG        v1.62.2 # 使用一个稳定版本
    GIT_SHALLOW    TRUE
)
set(gRPC_BUILD_CSHARP_EXT OFF CACHE BOOL "" FORCE) # 禁用不需要的语言
set(gRPC_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(grpc)

# --- 从 .proto 文件生成代码 ---
# 找到protoc编译器
find_program(PROTOC_COMPILER protoc)
get_target_property(GRPC_CPP_PLUGIN gRPC::grpc_cpp_plugin LOCATION)

set(PROTO_SRC_DIR ${CMAKE_SOURCE_DIR}/../../ipc/proto)
set(PROTO_DEST_DIR ${CMAKE_BINARY_DIR}/generated-sources)
file(MAKE_DIRECTORY ${PROTO_DEST_DIR})

file(GLOB PROTO_FILES "${PROTO_SRC_DIR}/*.proto")

# 添加一个自定义命令来生成 .pb.cc, .pb.h, .grpc.pb.cc, .grpc.pb.h
add_custom_command(
    OUTPUT
        ${PROTO_DEST_DIR}/crfzit.pb.cc
        ${PROTO_DEST_DIR}/crfzit.pb.h
        ${PROTO_DEST_DIR}/crfzit.grpc.pb.cc
        ${PROTO_DEST_DIR}/crfzit.grpc.pb.h
    COMMAND ${PROTOC_COMPILER}
        -I ${PROTO_SRC_DIR}
        --cpp_out=${PROTO_DEST_DIR}
        --grpc_out=${PROTO_DEST_DIR}
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
        ${PROTO_FILES}
    DEPENDS ${PROTO_FILES}
    COMMENT "Generating gRPC/Protobuf C++ sources"
)
add_custom_target(GenerateProtoSources DEPENDS ${PROTO_DEST_DIR}/crfzit.pb.cc)
add_dependencies(crfzitd GenerateProtoSources)

# 将生成的源文件添加到目标
target_sources(crfzitd PRIVATE
    ${PROTO_DEST_DIR}/crfzit.pb.cc
    ${PROTO_DEST_DIR}/crfzit.grpc.pb.cc
)
# 将生成的头文件目录包含进来
target_include_directories(crfzitd PRIVATE ${PROTO_DEST_DIR})

# --- 链接 gRPC 和其他依赖 ---
target_link_libraries(crfzitd PRIVATE
    gRPC::grpc++
    protobuf::libprotobuf
)

# Android平台链接时需要的额外库
target_link_libraries(crfzitd PRIVATE log dl)

# --- 安装指令 ---
# 定义安装路径，这样构建后可以轻松找到可执行文件
install(TARGETS crfzitd DESTINATION bin)