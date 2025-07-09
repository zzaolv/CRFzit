# 文件路径: /CRFzit/daemon/cpp/CMakeLists.txt (全新)

cmake_minimum_required(VERSION 3.20)
project(crfzitd CXX C)

# --- 1. 全局设置 ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build all libraries statically")

# --- 2. 依赖管理 (gRPC 和 libev) ---
include(FetchContent)
set(gRPC_INSTALL OFF CACHE BOOL "Disable gRPC install")
set(protobuf_INSTALL OFF CACHE BOOL "Disable Protobuf install")
set(ABSL_INSTALL OFF CACHE BOOL "Disable Abseil install")

FetchContent_Declare(
    gRPC
    GIT_REPOSITORY https://github.com/grpc/grpc.git
    GIT_TAG        v1.49.2 # 使用经过验证的稳定版本
    GIT_SUBMODULES_RECURSE TRUE
)
set(gRPC_BUILD_TESTS OFF CACHE BOOL "Disable gRPC tests")
set(gRPC_BUILD_CSHARP_EXT OFF CACHE BOOL "Disable gRPC C# extensions")
FetchContent_MakeAvailable(gRPC)

# libev for event loop
# FetchContent_Declare(
#     libev
#     GIT_REPOSITORY https://github.com/enki/libev.git
#     GIT_TAG        master
# )
# set(EV_SRC_DIR ${fetchcontent_SOURCE_DIR_LIBEV})
# FetchContent_MakeAvailable(libev)


# --- 3. Protobuf/gRPC C++ 代码生成 ---
set(DAEMON_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(PROTO_SRC_DIR ${DAEMON_ROOT}/../ipc/proto)
set(PROTO_FILES ${PROTO_SRC_DIR}/crfzit.proto)
set(PROTO_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen/proto)
set(GRPC_CPP_PLUGIN_EXE $<TARGET_FILE:grpc_cpp_plugin>)

add_custom_command(
    OUTPUT ${PROTO_GEN_DIR}/crfzit.pb.cc ${PROTO_GEN_DIR}/crfzit.pb.h ${PROTO_GEN_DIR}/crfzit.grpc.pb.cc ${PROTO_GEN_DIR}/crfzit.grpc.pb.h
    COMMAND protobuf::protoc
        --grpc_out=${PROTO_GEN_DIR} --cpp_out=${PROTO_GEN_DIR} -I ${PROTO_SRC_DIR}
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXE} ${PROTO_FILES}
    DEPENDS ${PROTO_FILES}
)
add_library(crfzit_proto ${PROTO_GEN_DIR}/crfzit.pb.cc ${PROTO_GEN_DIR}/crfzit.grpc.pb.cc)
target_include_directories(crfzit_proto PUBLIC ${PROTO_GEN_DIR})
target_link_libraries(crfzit_proto PUBLIC protobuf::libprotobuf grpc)

# --- 4. Rust 库链接 (不再用 ExternalProject，由外部脚本编译) ---
# 我们将由外部脚本先编译好 Rust 库，这里只负责链接
set(RUST_BUILD_DIR ${DAEMON_ROOT}/rust/target)
set(RUST_LIB_DIR ${RUST_BUILD_DIR}/aarch64-linux-android/release)
find_library(CRFZIT_LOGIC_LIB NAMES crfzit_core_logic PATHS ${RUST_LIB_DIR} REQUIRED)
find_library(CRFZIT_BRIDGE_LIB NAMES crfzit_cxx_bridge PATHS ${RUST_LIB_DIR} REQUIRED)

# --- 5. 定义 crfzitd 可执行文件 ---
add_executable(crfzitd
    main.cpp
    grpc_server.cpp
    kernel_interface.cpp
    shell_executor.cpp
    # timer_manager.cpp
)

target_link_libraries(crfzitd PRIVATE
    crfzit_proto
    ${CRFZIT_LOGIC_LIB}
    ${CRFZIT_BRIDGE_LIB}
    grpc++
    protobuf::libprotobuf
    # ev
    log
    dl
    -pthread # pthreads for std::thread
)

# 头文件包含路径
set(RUST_CXX_GEN_INCLUDE_DIR ${RUST_BUILD_DIR}/cxxbridge/crfzit_core/src/lib.rs.h)
target_include_directories(crfzitd PRIVATE
    .
    # 包含 cxx 生成的头文件的上两级目录
    ${RUST_CXX_GEN_INCLUDE_DIR}/../..
    ${RUST_CXX_GEN_INCLUDE_DIR}/..
    ${PROTO_GEN_DIR}
)
add_dependencies(crfzitd crfzit_proto)