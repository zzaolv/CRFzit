#ifndef CRFZIT_GRPC_SERVER_H
#define CRFZIT_GRPC_SERVER_H

#include <memory>
#include <grpcpp/grpcpp.h>
#include "ipc/proto/crfzit.grpc.pb.h" // 包含 gRPC 生成的头文件

namespace crfzit {

class GrpcServer final : public ipc::DashboardService::Service,
                         public ipc::ConfigService::Service,
                         public ipc::SystemService::Service {
public:
    GrpcServer() = default;
    ~GrpcServer() override;

    void Start(const std::string& address);
    void Shutdown();

private:
    // --- DashboardService 实现 ---
    grpc::Status StreamGlobalStats(
        grpc::ServerContext* context,
        const google::protobuf::Empty* request,
        grpc::ServerWriter<ipc::GlobalStats>* writer) override;

    grpc::Status StreamActiveAppsState(
        grpc::ServerContext* context,
        const google::protobuf::Empty* request,
        grpc::ServerWriter<ipc::ActiveAppsStateList>* writer) override;
    
    grpc::Status GetEventLogs(
        grpc::ServerContext* context,
        const google::protobuf::Empty* request,
        grpc::ServerWriter<ipc::EventLog>* writer) override;

    // --- ConfigService 实现 ---
    grpc::Status SetAppConfig(
        grpc::ServerContext* context,
        const ipc::AppConfig* request,
        google::protobuf::Empty* response) override;
        
    grpc::Status GetAllConfigs(
        grpc::ServerContext* context,
        const google::protobuf::Empty* request,
        grpc::ServerWriter<ipc::AppConfig>* writer) override;

    // --- SystemService 实现 ---
    grpc::Status ReportFrameworkEvent(
        grpc::ServerContext* context,
        const ipc::FrameworkEvent* request,
        google::protobuf::Empty* response) override;

    std::unique_ptr<grpc::Server> server_;
};

} // namespace crfzit

#endif //CRFZIT_GRPC_SERVER_H