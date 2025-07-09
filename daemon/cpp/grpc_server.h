// 文件路径: /CRFzit/daemon/cpp/grpc_server.h
#ifndef CRFZIT_GRPC_SERVER_H
#define CRFZIT_GRPC_SERVER_H

#include <string>

// 启动 gRPC 服务器。此函数将阻塞，直到服务器关闭。
void RunServer(const std::string& socket_path);

#endif //CRFZIT_GRPC_SERVER_H