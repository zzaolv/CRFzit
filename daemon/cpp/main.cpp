// 文件路径: /CRFzit/daemon/cpp/main.cpp
#include <iostream>
#include <memory>
#include <thread>
#include "grpc_server.h"
#include "crfzit/daemon/rust/src/lib.rs.h" // 包含 cxx 生成的头文件

// --- 全局变量 ---
// Rust 核心逻辑控制器的唯一实例
std::unique_ptr<crfzit::rust_bridge::LogicController> controller;

// --- 常量 ---
const char* SOCKET_PATH = "/data/local/tmp/crfzitd.sock";
const char* DB_PATH = "/data/adb/modules/CRFzit/crfzit.db";

int main(int argc, char** argv) {
    std::cout << "[C++ main] Starting crfzitd daemon..." << std::endl;

    // 1. 初始化 Rust 逻辑控制器
    controller = crfzit::rust_bridge::rust_init_controller(DB_PATH);
    if (!controller) {
        std::cerr << "[C++ main] CRITICAL: Failed to initialize Rust controller. Exiting." << std::endl;
        return 1;
    }
    std::cout << "[C++ main] Rust controller initialized successfully." << std::endl;

    // 2. 在一个独立的线程中启动 gRPC 服务器
    // 这是至关重要的，因为 RunServer 是一个阻塞调用
    std::thread grpc_thread(RunServer, std::string(SOCKET_PATH));
    std::cout << "[C++ main] gRPC server thread launched." << std::endl;
    
    // 主线程可以继续做其他事，或者像这里一样，等待 gRPC 线程结束
    grpc_thread.join(); 

    std::cout << "[C++ main] Daemon shutting down." << std::endl;
    return 0;
}