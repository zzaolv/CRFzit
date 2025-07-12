// File: ~/projects/CRFzit/daemon/cpp/grpc_server.h

#pragma once
#include <memory>
#include <grpcpp/grpcpp.h>
#include "ipc/proto/crfzit.grpc.pb.h"

class GrpcServer final : public crfzit::ipc::SystemService::Service {
public:
    void Run();

private:
    grpc::Status Ping(grpc::ServerContext* context, const google::protobuf::Empty* request, google::protobuf::Empty* response) override;
    grpc::Status ReportFrameworkEvent(grpc::ServerContext* context, const crfzit::ipc::FrameworkEvent* request, google::protobuf.Empty* response) override;
};