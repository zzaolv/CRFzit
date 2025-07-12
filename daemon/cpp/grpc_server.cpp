// File: ~/projects/CRFzit/daemon/cpp/grpc_server.cpp

#include "grpc_server.hh"
#include <iostream>
#include <grpcpp/server_builder.h>
#include "daemon/rust/target/cxxbridge/crfzit_core/src/lib.rs.h" // cxx生成的头文件

// 实现logger.h中的函数
void log_info(const std::string& message) {
    std::cout << "[INFO] " << message << std::endl;
}

void GrpcServer::Run() {
    std::string server_address("unix:///data/local/tmp/crfzitd.sock");
    
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(this);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    log_info("gRPC server listening on " + server_address);

    // 调用Rust函数获取版本并打印
    auto core_version = ffi::get_core_version();
    log_info("Linked with Rust core: " + std::string(core_version.data(), core_version.length()));

    server->Wait();
}

grpc::Status GrpcServer::Ping(grpc::ServerContext* context, const google::protobuf::Empty* request, google::protobuf::Empty* response) {
    log_info("Received Ping from client.");
    return grpc::Status::OK;
}

grpc::Status GrpcServer::ReportFrameworkEvent(grpc::ServerContext* context, const crfzit::ipc::FrameworkEvent* request, google::protobuf::Empty* response) {
    log_info("Received FrameworkEvent from Observer.");
    
    // 调用Rust核心逻辑处理事件
    ffi::process_event_from_cpp(static_cast<int32_t>(request->event_type()), request->package_name().c_str());
    
    return grpc::Status::OK;
}