// /CRFzit/daemon/cpp/main.cpp
#include <iostream>
#include <string>

// 包含由cxx生成的头文件
// 路径相对于CMake构建目录，但为了IDE友好，我们这样写
// 实际路径由CMake的include_directories决定
#include "crfzit/daemon/rust/src/lib.rs.h"

int main(int argc, char** argv) {
    // 调用Rust函数并打印结果
    rust::String rust_msg = crfzit::rust_bridge::get_initial_message();
    std::cout << "[crfzitd] C++ main received message: " << std::string(rust_msg) << std::endl;

    // TODO: 在下一阶段，我们将在这里初始化并运行gRPC服务器

    std::cout << "[crfzitd] Daemon started successfully. Exiting for now." << std::endl;

    return 0;
}