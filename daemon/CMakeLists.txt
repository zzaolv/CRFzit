# File: ~/projects/CRFzit/daemon/CMakeLists.txt

cmake_minimum_required(VERSION 3.20)
project(crfzitd CXX C)

# --- [!] 配置项: 请根据你的环境确认 ---
# 这是你之前编译gRPC的输出目录
set(GRPC_BUILD_DIR /home/zeaolv/dev/grpc/cmake/build) 
# 这是你的NDK路径
set(ANDROID_NDK_HOME /home/zeaolv/dev/android-ndk-r28b)

# --- 设置Android交叉编译环境 ---
set(CMAKE_SYSTEM_NAME Android)
set(CMAKE_SYSTEM_VERSION 31) # Target API level
set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
set(CMAKE_ANDROID_NDK ${ANDROID_NDK_HOME})
set(CMAKE_TOOLCHAIN_FILE ${ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 1. Protobuf & gRPC 代码生成 ---
# 找到 protoc 编译器
find_program(PROTOC_COMPILER protoc HINTS ${GRPC_BUILD_DIR}/third_party/protobuf)
if(NOT PROTOC_COMPILER)
    message(FATAL_ERROR "protoc not found!")
endif()
message(STATUS "Found protoc: ${PROTOC_COMPILER}")

# 找到 gRPC C++ 插件
find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin HINTS ${GRPC_BUILD_DIR})
if(NOT GRPC_CPP_PLUGIN)
    message(FATAL_ERROR "grpc_cpp_plugin not found!")
endif()
message(STATUS "Found gRPC C++ Plugin: ${GRPC_CPP_PLUGIN}")

# 设置 .proto 文件和生成目录
set(PROTO_FILE ${CMAKE_SOURCE_DIR}/../ipc/proto/crfzit.proto)
set(PROTO_GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${PROTO_GENERATED_DIR})

# 定义代码生成命令
add_custom_command(
    OUTPUT ${PROTO_GENERATED_DIR}/crfzit.pb.cc ${PROTO_GENERATED_DIR}/crfzit.pb.h ${PROTO_GENERATED_DIR}/crfzit.grpc.pb.cc ${PROTO_GENERATED_DIR}/crfzit.grpc.pb.h
    COMMAND ${PROTOC_COMPILER}
            --proto_path=${CMAKE_SOURCE_DIR}/../ipc/proto
            --cpp_out=${PROTO_GENERATED_DIR}
            --grpc_out=${PROTO_GENERATED_DIR}
            --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
            ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
)
# 将生成的源文件添加到变量中
set(PROTO_SOURCES ${PROTO_GENERATED_DIR}/crfzit.pb.cc ${PROTO_GENERATED_DIR}/crfzit.grpc.pb.cc)

# --- 2. Rust 核心库编译 ---
# 使用 cxx-build 来处理 Rust 编译和桥接
set(CXX_BRIDGE_RUST_SOURCES ${CMAKE_SOURCE_DIR}/rust/src/lib.rs)
set(CARGO_TARGET_DIR ${CMAKE_BINARY_DIR}/cargo_target) # 避免污染源码树
set(RUST_TARGET_ARCH aarch64-linux-android)

# 自定义命令来运行 cargo build
add_custom_command(
    OUTPUT ${CARGO_TARGET_DIR}/${RUST_TARGET_ARCH}/release/libcrfzit_core.a
    COMMAND ${CMAKE_COMMAND} -E env "CARGO_TARGET_DIR=${CARGO_TARGET_DIR}" cargo build --manifest-path ${CMAKE_SOURCE_DIR}/rust/Cargo.toml --target ${RUST_TARGET_ARCH} --release
    DEPENDS ${CXX_BRIDGE_RUST_SOURCES}
    COMMENT "Building Rust core library..."
)
set(CRFZIT_CORE_LIB ${CARGO_TARGET_DIR}/${RUST_TARGET_ARCH}/release/libcrfzit_core.a)
# cxx 生成的桥接头文件路径
set(CXX_BRIDGE_HEADER_DIR ${CARGO_TARGET_DIR}/${RUST_TARGET_ARCH}/cxxbridge)


# --- 3. 定义可执行文件 crfzitd ---
add_executable(crfzitd
    cpp/main.cpp
    cpp/grpc_server.cpp
    ${PROTO_SOURCES} # 添加生成的proto源文件
)

# --- 4. 设置头文件包含路径 ---
target_include_directories(crfzitd
    PRIVATE
    ${CMAKE_SOURCE_DIR}/cpp
    ${PROTO_GENERATED_DIR}
    ${GRPC_BUILD_DIR}/include
    ${CXX_BRIDGE_HEADER_DIR} # 添加cxx桥接头文件路径
)

# --- 5. 链接所有依赖库 ---
# gRPC 和 Protobuf 的静态库列表（基于你的tree输出）
set(GRPC_LIBS
    ${GRPC_BUILD_DIR}/libgrpc++.a
    ${GRPC_BUILD_DIR}/libgrpc.a
    ${GRPC_BUILD_DIR}/libgpr.a
    ${GRPC_BUILD_DIR}/libaddress_sorting.a
    ${GRPC_BUILD_DIR}/third_party/abseil-cpp/libabsl_synchronization.a
    ${GRPC_BUILD_DIR}/third_party/abseil-cpp/libabsl_cord.a
    ${GRPC_BUILD_DIR}/third_party/abseil-cpp/libabsl_status.a
    ${GRPC_BUILD_DIR}/third_party/abseil-cpp/libabsl_statusor.a
    ${GRPC_BUILD_DIR}/third_party/abseil-cpp/libabsl_strings.a
    ${GRPC_BUILD_DIR}/third_party/abseil-cpp/libabsl_hash.a
    ${GRPC_BUILD_DIR}/third_party/abseil-cpp/libabsl_raw_hash_set.a
    ${GRPC_BUILD_DIR}/third_party/abseil-cpp/libabsl_common.a
    ${GRPC_BUILD_DIR}/third_party/abseil-cpp/libabsl_city.a
    ${GRPC_BUILD_DIR}/third_party/abseil-cpp/libabsl_time.a
    ${GRPC_BUILD_DIR}/third_party/abseil-cpp/libabsl_time_zone.a
    ${GRPC_BUILD_DIR}/third_party/protobuf/libprotobuf.a
    ${GRPC_BUILD_DIR}/third_party/zlib/libz.a
    ${GRPC_BUILD_DIR}/third_party/cares/libcares.a
    ${GRPC_BUILD_DIR}/third_party/boringssl-with-bazel/libssl.a
    ${GRPC_BUILD_DIR}/third_party/boringssl-with-bazel/libcrypto.a
)

target_link_libraries(crfzitd
    PRIVATE
    ${CRFZIT_CORE_LIB} # 链接Rust静态库
    ${GRPC_LIBS}
    dl          # 必须链接dl和pthread
    pthread
)

# 确保在链接前，代码生成和Rust编译已完成
add_dependencies(crfzitd crfzitd_proto_gen crfzitd_rust_build)