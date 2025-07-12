#include "grpc_server.h"
#include "log.h"
#include "crfzit/daemon/rust/src/lib.rs.h" // cxx header

namespace crfzit {

GrpcServer::~GrpcServer() {
    Shutdown();
}

void GrpcServer::Start(const std::string& address) {
    grpc::ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    
    // 注册所有服务
    builder.RegisterService(static_cast<ipc::DashboardService::Service*>(this));
    builder.RegisterService(static_cast<ipc::ConfigService::Service*>(this));
    builder.RegisterService(static_cast<ipc::SystemService::Service*>(this));

    server_ = builder.BuildAndStart();
    if (server_) {
        LOGI("gRPC server listening on %s", address.c_str());
        // 等待服务器关闭。这是一个阻塞调用。
        server_->Wait();
    } else {
        LOGE("Failed to start gRPC server on %s", address.c_str());
    }
}

void GrpcServer::Shutdown() {
    if (server_) {
        LOGI("Shutting down gRPC server...");
        server_->Shutdown();
    }
}

// --- 服务实现 ---

grpc::Status GrpcServer::StreamGlobalStats(
    grpc::ServerContext* context,
    const google::protobuf::Empty* request,
    grpc::ServerWriter<ipc::GlobalStats>* writer) {

    LOGI("Client subscribed to GlobalStats stream.");
    // TODO: 实现流式逻辑。Rust核心将通过回调推送数据。
    // 这是一个简化的示例，实际实现需要一个生产者-消费者队列。
    // Rust -> C++ 回调 -> 队列 -> writer 线程
    while (!context->IsCancelled()) {
        // 等待Rust核心的通知
        std::this_thread::sleep_for(std::chrono::seconds(1));
        // 这里只是一个演示，实际数据将从Rust回调中获取
        // rust_bridge::rust_update_dashboard_data();
    }
    return grpc::Status::OK;
}

grpc::Status GrpcServer::StreamActiveAppsState(
    grpc::ServerContext* context,
    const google::protobuf::Empty* request,
    grpc::ServerWriter<ipc::ActiveAppsStateList>* writer) {
        
    LOGI("Client subscribed to ActiveAppsState stream.");
    while (!context->IsCancelled()) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    return grpc::Status::OK;
}

grpc::Status GrpcServer::GetEventLogs(
    grpc::ServerContext* context,
    const google::protobuf::Empty* request,
    grpc::ServerWriter<ipc::EventLog>* writer) {
    LOGW("GetEventLogs is not implemented yet.");
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "Not implemented");
}

grpc::Status GrpcServer::SetAppConfig(
    grpc::ServerContext* context,
    const ipc::AppConfig* request,
    google::protobuf::Empty* response) {
    LOGW("SetAppConfig is not implemented yet.");
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "Not implemented");
}
        
grpc::Status GrpcServer::GetAllConfigs(
    grpc::ServerContext* context,
    const google::protobuf::Empty* request,
    grpc::ServerWriter<ipc::AppConfig>* writer) {
    LOGW("GetAllConfigs is not implemented yet.");
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "Not implemented");
}


grpc::Status GrpcServer::ReportFrameworkEvent(
    grpc::ServerContext* context,
    const ipc::FrameworkEvent* request,
    google::protobuf::Empty* response) {
    LOGI("Received FrameworkEvent for package: %s", request->package_name().c_str());

    // 将请求转发给 Rust 核心
    rust_bridge::rust_on_framework_event(
        request->uid(),
        request->package_name(),
        static_cast<int32_t>(request->type())
    );

    return grpc::Status::OK;
}

} // namespace crfzit

// --- C++ 函数的实现，供 Rust 调用 ---
namespace crfzit::cpp_grpc {

// 这个函数由 Rust 调用，以推送全局状态
void push_global_stats(float cpu_percent, int64_t mem_kb, int64_t net_down_bps, int64_t net_up_bps) {
    LOGI("Rust requested to push global stats. CPU: %.2f%%", cpu_percent);
    // TODO: 将此数据发送给所有订阅了 StreamGlobalStats 的客户端。
    // 这需要一个更复杂的发布/订阅系统。
}

} // namespace crfzit::cpp_grpc