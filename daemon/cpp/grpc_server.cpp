// 文件路径: /CRFzit/daemon/cpp/grpc_server.cpp

#include "grpc_server.h"
#include <iostream>
#include <memory>
#include <chrono>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

// 包含生成的头文件
#include "crfzit.grpc.pb.h"
// 包含 cxx 桥接头文件
#include "crfzit/daemon/rust/src/lib.rs.h"

// 声明在 main.cpp 中定义的全局 Rust 控制器对象
extern std::unique_ptr<crfzit::rust_bridge::LogicController> controller;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerWriter;
using google::protobuf::Empty;

using namespace crfzit::ipc;
using namespace crfzit::rust_bridge;

// --- 服务实现 ---

class ConfigServiceImpl final : public ConfigService::Service {
public:
    Status SetAppConfig(ServerContext* context, const AppConfig* request, Empty* response) override {
        if (!controller) return Status::CANCELLED;
        // TODO: 数据转换和调用 Rust
        std::cout << "[C++ gRPC] Received SetAppConfig for: " << request->package_name() << std::endl;
        return Status::OK;
    }
    // ... 其他 ConfigService 的 RPC 实现
};

class DashboardServiceImpl final : public DashboardService::Service {
public:
    Status StreamGlobalStats(ServerContext* context, const Empty* request, ServerWriter<GlobalStats>* writer) override {
        if (!controller) return Status::CANCELLED;
        // TODO: 数据转换和调用 Rust
        std::cout << "[C++ gRPC] Client subscribed to StreamGlobalStats." << std::endl;
        while (!context->IsCancelled()) {
            GlobalStats stats;
            stats.set_total_cpu_usage_percent(10.5); // 假数据
            if (!writer->Write(stats)) break;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        return Status::OK;
    }
    // ... 其他 DashboardService 的 RPC 实现
};

class SystemServiceImpl final : public SystemService::Service {
    // ... SystemService 的 RPC 实现
};


// --- 服务器主函数 ---

void RunServer(const std::string& socket_path) {
    std::string server_address = "unix:" + socket_path;
    
    ConfigServiceImpl config_service;
    DashboardServiceImpl dashboard_service;
    SystemServiceImpl system_service;

    grpc::EnableDefaultHealthCheckService(true);
    ServerBuilder builder;

    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    
    builder.RegisterService(&config_service);
    builder.RegisterService(&dashboard_service);
    builder.RegisterService(&system_service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "[C++ gRPC] Server listening on " << server_address << std::endl;

    // 这将阻塞调用线程（我们打算让 gRPC 在自己的线程中运行）
    server->Wait();
}