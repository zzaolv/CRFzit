# CRFzitd CMake Build System
# Version: 1.0

cmake_minimum_required(VERSION 3.22)
project(crfzitd CXX C)

# --- 1. 环境与工具链设置 ---
# 使用者必须通过环境变量或CMake参数设置ANDROID_NDK_HOME
# export ANDROID_NDK_HOME=/home/zeaolv/dev/android-ndk-r28b
if(NOT DEFINED ANDROID_NDK_HOME)
    message(FATAL_ERROR "ANDROID_NDK_HOME is not set. Please set it to your Android NDK r28b+ path.")
endif()

set(CMAKE_TOOLCHAIN_FILE ${ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake)

# Android 平台配置
set(ANDROID_PLATFORM 31) # 目标平台 Android 12
set(ANDROID_ABI "arm64-v8a")
set(ANDROID_STL "c++_shared") # 关键！与gRPC动态库链接时必须使用 c++_shared

message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "NDK: ${ANDROID_NDK_HOME}")
message(STATUS "Target Platform: android-${ANDROID_PLATFORM}")
message(STATUS "Target ABI: ${ANDROID_ABI}")

# --- 2. gRPC 与 Protobuf 集成 ---
# 使用 find_package 查找在您指定位置 (~/dev/grpc) 手动编译的 gRPC
# 为了让 find_package 成功，您需要先编译并安装gRPC到某个前缀路径下
# 然后将该路径添加到 CMAKE_PREFIX_PATH
# 例如: cmake -DCMAKE_INSTALL_PREFIX=/home/zeaolv/dev/grpc_install ... && make install
# 然后在构建 crfzitd 时: cmake -DCMAKE_PREFIX_PATH=/home/zeaolv/dev/grpc_install ...
message(STATUS "Looking for gRPC...")
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Found gRPC: ${gRPC_DIR}")
message(STATUS "gRPC Libraries: ${gRPC_LIBRARIES}")
message(STATUS "gRPC++ Libraries: ${gRPC_GRPCPP_LIBRARIES}")

# 设置 Protobuf 的可执行文件路径，gRPC的find_package通常会处理好
get_target_property(PROTOC_EXECUTABLE gRPC::protoc IMPORTED_LOCATION)
message(STATUS "Found protoc: ${PROTOC_EXECUTABLE}")

# 定义 protobuf-generate-grpc-cpp 函数，用于自动生成代码
# (此函数在新版 gRPC 的 FindgRPC.cmake 中可能已提供，这里为确保兼容性而定义)
if(NOT COMMAND protobuf_generate_grpc_cpp)
    function(protobuf_generate_grpc_cpp SRCS HDRS)
        if(NOT ARGN)
            message(SEND_ERROR "Error: protobuf_generate_grpc_cpp() requires at least one proto file.")
            return()
        endif()
        set(protos ${ARGN})
        set(source_dir ${CMAKE_CURRENT_SOURCE_DIR})
        set(binary_dir ${CMAKE_CURRENT_BINARY_DIR})
        
        foreach(file ${protos})
            get_filename_component(abs_filename ${file} ABSOLUTE)
            get_filename_component(filename ${file} NAME_WE)
            
            list(APPEND ${SRCS} "${binary_dir}/${filename}.pb.cc")
            list(APPEND ${SRCS} "${binary_dir}/${filename}.grpc.pb.cc")
            list(APPEND ${HDRS} "${binary_dir}/${filename}.pb.h")
            list(APPEND ${HDRS} "${binary_dir}/${filename}.grpc.pb.h")
            
            add_custom_command(
                OUTPUT "${binary_dir}/${filename}.pb.cc" "${binary_dir}/${filename}.pb.h" "${binary_dir}/${filename}.grpc.pb.cc" "${binary_dir}/${filename}.grpc.pb.h"
                COMMAND ${PROTOC_EXECUTABLE}
                ARGS --grpc_out=${binary_dir} --cpp_out=${binary_dir} -I ${source_dir} --plugin=protoc-gen-grpc=`get_target_property(gRPC::grpc_cpp_plugin IMPORTED_LOCATION)` ${abs_filename}
                DEPENDS ${abs_filename}
                COMMENT "Generating gRPC and Protobuf code from ${file}"
                VERBATIM)
        endforeach()
        
        set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
        set(${SRCS} ${${SRCS}} PARENT_SCOPE)
        set(${HDRS} ${${HDRS}} PARENT_SCOPE)
    endfunction()
endif()

# 从 .proto 文件生成 C++ 源码
set(PROTO_FILES ../ipc/proto/crfzit.proto)
protobuf_generate_grpc_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

# --- 3. Rust 核心库集成 (cxx bridge) ---
message(STATUS "Configuring Rust core library build...")

# 使用 ExternalProject_Add 来调用 cargo 构建 Rust 静态库
# 这是比自定义函数更现代、更健壮的 CMake 方式
include(ExternalProject)
set(CARGO_TARGET_DIR ${CMAKE_CURRENT_BINARY_DIR}/cargo_target)
set(RUST_LIB_OUTPUT_DIR ${CARGO_TARGET_DIR}/aarch64-linux-android/release)
set(RUST_STATIC_LIB ${RUST_LIB_OUTPUT_DIR}/libcrfzit_core.a)
set(CXX_BRIDGE_DIR ${CARGO_TARGET_DIR}/cxxbridge)

ExternalProject_Add(
    rust_core_lib
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/rust
    # 不要让 ExternalProject 配置、构建或安装，我们只用它来创建目标
    CONFIGURE_COMMAND ""
    BUILD_COMMAND cargo build --release --target=aarch64-linux-android
    INSTALL_COMMAND ""
    # 确保 Cargo 的目标目录是我们控制的
    BUILD_BYPRODUCTS ${RUST_STATIC_LIB}
    LOG_BUILD 1
    LOG_OUTPUT_ON_FAILURE 1
    # 将 Cargo 目标目录传递给 cargo
    BUILD_IN_SOURCE 1 # 在源目录（rust/）中运行cargo
    COMMAND_ENV
        "CARGO_TARGET_DIR=${CARGO_TARGET_DIR}"
)

# 创建一个导入的库目标来代表 Rust 静态库
add_library(crfzit_core STATIC IMPORTED GLOBAL)
add_dependencies(crfzit_core rust_core_lib)
set_target_properties(crfzit_core PROPERTIES
    IMPORTED_LOCATION ${RUST_STATIC_LIB}
    INTERFACE_INCLUDE_DIRECTORIES ${CXX_BRIDGE_DIR}
)
message(STATUS "Rust static library target 'crfzit_core' created.")
message(STATUS "  - Library location: ${RUST_STATIC_LIB}")
message(STATUS "  - CXX bridge headers: ${CXX_BRIDGE_DIR}")

# --- 4. C++ 源代码与可执行文件定义 ---
message(STATUS "Configuring C++ executable...")

# 将生成的 protobuf 头文件和 cxx bridge 头文件目录包含进来
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CXX_BRIDGE_DIR}
)

# 定义C++源文件
set(CPP_SOURCES
    cpp/main.cpp
    cpp/grpc_server.cpp
    cpp/kernel_interface.cpp
    cpp/shell_executor.cpp
    # 添加生成的 proto 源文件
    ${PROTO_SRCS}
)

# 创建可执行文件目标
add_executable(crfzitd ${CPP_SOURCES})
target_include_directories(crfzitd PRIVATE cpp)

# --- 5. 链接所有依赖 ---
message(STATUS "Linking libraries...")
target_link_libraries(crfzitd
    PRIVATE
    # 我们的 Rust 核心库
    crfzit_core
    
    # gRPC 和它的依赖 (protobuf, ssl, zlib, etc.)
    # gRPC::grpc++ 是一个接口目标，它会处理所有 gRPC 客户端和服务器的链接
    gRPC::grpc++
    gRPC::grpc++_reflection
    
    # Android 系统库
    log      # 用于 __android_log_print
    dl       # 动态链接库
)

# 确保在链接 crfzitd 之前，Rust 库已经构建完成
add_dependencies(crfzitd crfzit_core)

# --- 6. 安装指令 (可选，但推荐) ---
# 这部分用于 'make install'，可以将最终成品安装到指定目录
install(TARGETS crfzitd
    DESTINATION bin
)

message(STATUS "Configuration of crfzitd completed.")