# 文件路径: /CRFzit/daemon/CMakeLists.txt (最终决战版 - 对齐验证过的版本)

cmake_minimum_required(VERSION 3.20)
project(crfzitd CXX C)

# --- 1. 全局设置 ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build all libraries statically")


# --- 2. 依赖管理 ---
include(FetchContent)
set(gRPC_INSTALL OFF CACHE BOOL "Disable gRPC install")
set(protobuf_INSTALL OFF CACHE BOOL "Disable Protobuf install")
set(ABSL_INSTALL OFF CACHE BOOL "Disable Abseil install")

FetchContent_Declare(
    gRPC
    GIT_REPOSITORY https://github.com/grpc/grpc.git
    # 【最终修正】: 使用经过大规模交叉编译验证的 v1.49.x 版本
    GIT_TAG        v1.49.2
    GIT_SUBMODULES_RECURSE TRUE
)
set(gRPC_BUILD_TESTS OFF CACHE BOOL "Disable gRPC tests")
set(gRPC_BUILD_CSHARP_EXT OFF CACHE BOOL "Disable gRPC C# extensions")
FetchContent_MakeAvailable(gRPC)


# --- 3. Rust 库集成 ---
include(ExternalProject)
set(DAEMON_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(RUST_SRC_DIR ${DAEMON_ROOT}/rust)

set(RUST_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/rust_build)
set(RUST_OUTPUT_DIR ${RUST_BUILD_DIR}/aarch64-linux-android/release)
set(RUST_LIBRARY_PATH ${RUST_OUTPUT_DIR}/libcrfzit_core_logic.a)
set(RUST_BRIDGE_LIBRARY_PATH ${RUST_OUTPUT_DIR}/libcrfzit_cxx_bridge.a)

ExternalProject_Add(
    rust_logic_lib_build
    SOURCE_DIR        ${RUST_SRC_DIR}
    BUILD_COMMAND     cd ${RUST_SRC_DIR} &&
                      ${CMAKE_COMMAND} -E env
                          CARGO_TARGET_DIR=${RUST_BUILD_DIR}
                          CARGO_TARGET_AARCH64_LINUX_ANDROID_LINKER=${CMAKE_CXX_COMPILER}
                          cargo build --release --target aarch64-linux-android
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND   ""
    BYPRODUCTS        ${RUST_LIBRARY_PATH} ${RUST_BRIDGE_LIBRARY_PATH}
)

add_library(crfzit_core_logic STATIC IMPORTED GLOBAL)
set_property(TARGET crfzit_core_logic PROPERTY IMPORTED_LOCATION ${RUST_LIBRARY_PATH})
add_dependencies(crfzit_core_logic rust_logic_lib_build)

add_library(crfzit_cxx_bridge STATIC IMPORTED GLOBAL)
set_property(TARGET crfzit_cxx_bridge PROPERTY IMPORTED_LOCATION ${RUST_BRIDGE_LIBRARY_PATH})
add_dependencies(crfzit_cxx_bridge rust_logic_lib_build)


# --- 4. Protobuf/gRPC C++ 代码生成 ---
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


# --- 5. 定义 crfzitd 可执行文件 ---
add_executable(crfzitd
    cpp/main.cpp
    cpp/grpc_server.cpp
    cpp/kernel_interface.cpp
    cpp/shell_executor.cpp
)

target_link_libraries(crfzitd PRIVATE
    crfzit_proto
    crfzit_core_logic
    crfzit_cxx_bridge
    grpc++
    protobuf::libprotobuf
    log
    dl
    -pthread
)
set(RUST_CXX_GEN_INCLUDE_DIR ${RUST_BUILD_DIR}/cxxbridge/crfzit_core/src/lib.rs.h)
target_include_directories(crfzitd PRIVATE
    cpp
    ${RUST_CXX_GEN_INCLUDE_DIR}/../..
    ${RUST_CXX_GEN_INCLUDE_DIR}/..
    ${PROTO_GEN_DIR}
)
add_dependencies(crfzitd crfzit_proto rust_logic_lib_build)