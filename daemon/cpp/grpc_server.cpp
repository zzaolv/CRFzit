// 文件路径: /CRFzit/daemon/cpp/grpc_server.cpp

#include "grpc_server.h"
#include <iostream>
#include <memory>
#include <chrono>
#include <thread> // for std::this_thread::sleep_for

// 引入 gRPC 核心头文件
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

// 引入由 protoc 生成的 gRPC 服务和消息头文件
#include "crfzit.grpc.pb.h"
// 引入由 cxx 生成的 Rust FFI 桥接头文件
#include "crfzit/daemon/rust/src/lib.rs.h"

// --- 定义 gRPC C++ 客户端使用的命名空间别名，使代码更简洁 ---
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerWriter;
using google::protobuf::Empty;

// --- 定义 protobuf 消息的命名空间别名 ---
using namespace crfzit::ipc;

// --- 定义 Rust FFI 函数的命名空间别名 ---
namespace rust_ffi = crfzit::rust_bridge;

// ===================================================================
//  服务实现 (Service Implementations)
// ===================================================================

/**
 * @brief 实现 ConfigService RPC 服务.
 * 负责处理所有与应用配置相关的请求.
 */
class ConfigServiceImpl final : public ConfigService::Service {
public:
    Status SetAppConfig(ServerContext* context, const AppConfig* request, Empty* response) override {
        std::cout << "[C++ gRPC] Received RPC: SetAppConfig for package '" << request->package_name() << "'" << std::endl;
        
        // 在未来的阶段，我们将在这里添加逻辑：
        // 1. 将 protobuf 的 AppConfig 转换为 cxx 定义的 FFI 结构体。
        // 2. 调用 rust_ffi::rust_set_app_config(...) 函数。
        // rust_set_app_config 函数将内部访问全局的 LogicController 实例来处理业务逻辑。
        
        return Status::OK;
    }

    Status GetAllConfigs(ServerContext* context, const Empty* request, ServerWriter<AppConfig>* writer) override {
        std::cout << "[C++ gRPC] Received RPC: GetAllConfigs" << std::endl;
        
        // TODO: 调用 Rust 函数获取所有配置并流式返回
        
        return Status(grpc::StatusCode::UNIMPLEMENTED, "GetAllConfigs not implemented yet");
    }
};


/**
 * @brief 实现 DashboardService RPC 服务.
 * 负责处理所有与实时仪表盘数据相关的请求.
 */
class DashboardServiceImpl final : public DashboardService::Service {
public:
    Status StreamGlobalStats(ServerContext* context, const Empty* request, ServerWriter<GlobalStats>* writer) override {
        std::cout << "[C++ gRPC] Client subscribed to StreamGlobalStats." << std::endl;
        
        // 这是一个循环，只要客户端连接着，就持续推送数据
        while (!context->IsCancelled()) {
            // TODO: 在未来阶段，这里将调用 rust_ffi::rust_get_global_stats()
            // 来获取真实的统计数据。
            
            // --- 使用假数据进行演示 ---
            GlobalStats stats;
            stats.set_total_cpu_usage_percent(15.7f);
            stats.set_total_mem_kb(12 * 1024 * 1024);
            stats.set_avail_mem_kb(4 * 1024 * 1024);
            stats.set_network_speed_down_bps(1200 * 1024); // 1.2 MB/s
            stats.set_network_speed_up_bps(512 * 1024);   // 512 KB/s
            stats.set_active_profile_name("Default Mode");

            if (!writer->Write(stats)) {
                // 如果写入失败（通常是客户端断开连接），则跳出循环
                break;
            }

            // 每秒推送一次
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::cout << "[C++ gRPC] Client unsubscribed from StreamGlobalStats." << std::endl;
        return Status::OK;
    }

    Status StreamActiveApps(ServerContext* context, const Empty* request, ServerWriter<ActiveAppsList>* writer) override {
        std::cout << "[C++ gRPC] Client subscribed to StreamActiveApps." << std::endl;
        
        // TODO: 调用 Rust 函数获取应用状态并流式返回
        
        return Status(grpc::StatusCode::UNIMPLEMENTED, "StreamActiveApps not implemented yet");
    }

    Status GetEventLogs(ServerContext* context, const Empty* request, ServerWriter<EventLog>* writer) override {
         std::cout << "[C++ gRPC] Received RPC: GetEventLogs" << std::endl;
        
        // TODO: 调用 Rust 函数获取日志并流式返回
        
        return Status(grpc::StatusCode::UNIMPLEMENTED, "GetEventLogs not implemented yet");
    }
};


/**
 * @brief 实现 SystemService RPC 服务.
 * 负责接收来自 LSPosed 模块等的系统级事件.
 */
class SystemServiceImpl final : public SystemService::Service {
public:
    Status ReportFrameworkEvent(ServerContext* context, const FrameworkEvent* request, Empty* response) override {
        std::cout << "[C++ gRPC] Received FrameworkEvent: " << request->event_type() 
                  << " for package " << request->package_name() << std::endl;
                  
        // TODO: 调用 rust_ffi::rust_handle_framework_event(...)

        return Status::OK;
    }
};


// ===================================================================
//  服务器主函数 (Server Runner)
// ===================================================================

void RunServer(const std::string& socket_path) {
    std::string server_address = "unix:" + socket_path;
    
    // 实例化我们所有的服务实现
    ConfigServiceImpl config_service;
    DashboardServiceImpl dashboard_service;
    SystemServiceImpl system_service;

    // 启用 gRPC 内置的健康检查服务，方便客户端探测服务是否存活
    grpc::EnableDefaultHealthCheckService(true);
    
    // 使用 ServerBuilder 来配置和构建服务器
    ServerBuilder builder;

    // 添加监听端口。我们使用 Unix Domain Socket，因为它在本地IPC中性能更高且更安全。
    // InsecureServerCredentials 表示我们不使用 TLS 加密，在本地 socket 通信中是安全的。
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    
    // 注册我们实现的服务
    builder.RegisterService(&config_service);
    builder.RegisterService(&dashboard_service);
    builder.RegisterService(&system_service);

    // 构建并启动 gRPC 服务器
    std::unique_ptr<Server> server(builder.BuildAndStart());
    if (server) {
        std::cout << "[C++ gRPC] Server listening on " << server_address << std::endl;
        // Wait() 是一个阻塞调用。它会使当前线程（在这里是我们的 grpc_thread）
        // 进入等待状态，直到服务器被外部命令关闭。
        server->Wait();
    } else {
        std::cerr << "[C++ gRPC] CRITICAL: Failed to start gRPC server on " << server_address << std::endl;
    }
}