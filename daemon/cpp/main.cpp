// 文件路径: /CRFzit/daemon/cpp/main.cpp (修正版)
#include <iostream>
#include <memory>
#include <thread>
#include "grpc_server.h"
#include "crfzit/daemon/rust/src/lib.rs.h"

// --- 不再需要全局变量 ---
// std::unique_ptr<crfzit::rust_bridge::LogicController> controller;

const char* SOCKET_PATH = "/data/local/tmp/crfzitd.sock";
const char* DB_PATH = "/data/adb/modules/CRFzit/crfzit.db";

int main(int argc, char** argv) {
    std::cout << "[C++ main] Starting crfzitd daemon..." << std::endl;

    // 1. 调用 Rust 函数来初始化全局控制器
    crfzit::rust_bridge::rust_init_controller(DB_PATH);
    std::cout << "[C++ main] rust_init_controller() called." << std::endl;

    // 2. 调用 Rust 的测试函数，验证通信
    std::cout << "[C++ main] Calling rust_log_test_message()..." << std::endl;
    crfzit::rust_bridge::rust_log_test_message();

    // 3. 在独立的线程中启动 gRPC 服务器
    std::thread grpc_thread(RunServer, std::string(SOCKET_PATH));
    std::cout << "[C++ main] gRPC server thread launched." << std::endl;
    
    grpc_thread.join(); 

    std::cout << "[C++ main] Daemon shutting down." << std::endl;
    return 0;
}