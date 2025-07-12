#include "log.h"
#include "grpc_server.h"
#include "crfzit/daemon/rust/src/lib.rs.h" // cxx header
#include <unistd.h> // for getuid

int main(int argc, char** argv) {
    // 检查是否以 root 身份运行
    if (getuid() != 0) {
        LOGE("crfzitd must be run as root. Exiting.");
        return 1;
    }

    LOGI("--- CRFzit Daemon v1.0 Starting ---");

    // 1. 初始化 Rust 核心
    // 这是进入 Rust 世界的第一个调用
    crfzit::rust_bridge::rust_init_core();
    LOGI("Rust core has been initialized from C++.");

    // 2. 创建并启动 gRPC 服务器
    crfzit::GrpcServer grpc_server;
    // 我们将通过 Unix Domain Socket 进行通信，以获得更好的性能和安全性
    // Magisk 模块路径通常是 /data/adb/modules/CRFzit
    const std::string socket_path = "unix:/data/adb/modules/CRFzit/crfzitd.sock";
    
    grpc_server.Start(socket_path);

    // 服务器将在自己的线程中运行并等待，所以主线程会在这里阻塞直到服务器关闭。
    // GrpcServer::Shutdown() 将在析构函数中被调用，以实现优雅退出。

    LOGI("--- CRFzit Daemon Exiting ---");
    return 0;
}